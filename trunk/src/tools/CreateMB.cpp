//
// C++ Implementation: CreateMB
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "CreateMB.h"
#include "../engine/ParseString.h"
#include "../utils/output.h"
#include "../utils/wrapper.h"

/**
 * @name 相关底层数据的构造函数&析构函数.
 * @{
 */
PhraseDatum::PhraseDatum():chidx(NULL), chlen(0),
 data(NULL), dtlen(0), freq(0)
{}
PhraseDatum::~PhraseDatum()
{
	delete [] chidx;
	g_free(data);
}
TravTreePara::TravTreePara():fd(-1)
{}
TravTreePara::~TravTreePara()
{}
/** @} */

/**
 * 类构造函数.
 */
CreateMB::CreateMB()
{
	root = g_node_new(NULL);
}

/**
 * 类析构函数.
 */
CreateMB::~CreateMB()
{
	g_node_traverse(root, G_LEVEL_ORDER, G_TRAVERSE_LEAVES, -1,
			 GNodeTraverseFunc(DeletePhraseDatum), NULL);
	g_node_destroy(root);
}

/**
 * 根据sfile源文件的数据创建词语索引树.
 * @param sfile 源文件
 */
void CreateMB::CreatePhraseIndex(const char *sfile)
{
	FILE *stream;
	PhraseDatum *phrdt;
	const char *phrase, *pinyin, *freq;
	uint phrases, converts;
	char *lineptr;
	size_t n;

	/* 打开数据源文件 */
	if (!(stream = fopen(sfile, "r")))
		errx(1, "Fopen file \"%s\" failed, %s", sfile, strerror(errno));

	/* 读取文件数据，分析，插入到索引树 */
	phrases = converts = 0;
	lineptr = NULL;
	n = 0;
	while (getline(&lineptr, &n, stream) != -1) {
		pmessage("\rReading Phrase: %d", ++phrases);
		if (!BreakPhraseString(lineptr, &phrase, &pinyin, &freq))
			continue;
		phrdt = CreatePhraseDatum(phrase, pinyin, freq);
		InsertPhraseToTree(phrdt);
		converts++;
	}
	pmessage("\n%d Phrases, %d Converted!\n", phrases, converts);
	free(lineptr);

	/* 关闭源文件 */
	fclose(stream);
}

/**
 * 写出词语的索引和数据，即创建码表文件.
 * @param tfile 目标文件
 */
void CreateMB::WritePhraseIndex(const char *tfile)
{
	TravTreePara para;
	int fd;

	/* 创建目标文件 */
	if ((fd = open(tfile, O_WRONLY | O_CREAT | O_EXCL, 00644)) == -1)
		errx(1, "Open file \"%s\" failed, %s", tfile, strerror(errno));

	/* 写出索引&数据索引&数据 */
	pmessage("Writing Phrase file ...\n");
	para.fd = fd;
	para.eleaves = 0;
	g_node_traverse(root, G_PRE_ORDER, G_TRAVERSE_ALL, -1,
		 GNodeTraverseFunc(WritePinyinMBIndex), &para);
	para.eoffset = lseek(fd, 0, SEEK_CUR) + sizeof(para.eoffset) * para.eleaves;	//计算数据部分起始偏移量
	g_node_traverse(root, G_LEVEL_ORDER, G_TRAVERSE_LEAVES, -1,
			 GNodeTraverseFunc(WritePinyinMBDtidx), &para);
	g_node_traverse(root, G_LEVEL_ORDER, G_TRAVERSE_LEAVES, -1,
				 GNodeTraverseFunc(WritePinyinMBData),
				 GINT_TO_POINTER(fd));

	/* 关闭目标文件 */
	close(fd);
}

/**
 * 分割词语串的各部分.
 * @param lineptr 源串
 * @retval phrase 词语串
 * @retval pinyin 拼音串
 * @retval freq 频率串
 * @return 串是否合法
 */
bool CreateMB::BreakPhraseString(char *lineptr, const char **phrase,
			 const char **pinyin, const char **freq)
{
	char *ptr;

	if (*(ptr = lineptr + strspn(lineptr, "\x20\t\r\n")) == '\0')
		return false;
	*phrase = ptr;

	if (*(ptr += strcspn(ptr, "\x20\t\r\n")) == '\0')
		return false;
	*ptr = '\0';
	ptr++;
	if (*(ptr += strspn(ptr, "\x20\t\r\n")) == '\0')
		return false;
	*pinyin = ptr;

	if (*(ptr += strcspn(ptr, "\x20\t\r\n")) == '\0')
		return false;
	*ptr = '\0';
	ptr++;
	if (*(ptr += strspn(ptr, "\x20\t\r\n")) == '\0')
		return false;
	*freq = ptr;

	return true;
}

/**
 * 创建词语数据资料.
 * @param phrase 词语串
 * @param pinyin 拼音串
 * @param freq 频率串
 * @return 词语数据资料
 */
PhraseDatum *CreateMB::CreatePhraseDatum(const char *phrase, const char *pinyin,
							 const char *freq)
{
	ParseString parse;
	PhraseDatum *phrdt;

	phrdt = new PhraseDatum;
	parse.ParsePinyinString(pinyin, &phrdt->chidx, &phrdt->chlen);
	phrdt->data = g_utf8_to_utf16(phrase, -1, NULL, &phrdt->dtlen, NULL);
	phrdt->freq = atoi(freq);

	return phrdt;
}

/**
 * 插入词语数据资料到树.
 * @param phrdt 词语数据资料
 */
void CreateMB::InsertPhraseToTree(PhraseDatum *phrdt)
{
	GNode *parent, *tnode;

	parent = SearchChildByCharsIndex(root, phrdt->chidx->major);
	parent = SearchChildByCharsLength(parent, phrdt->chlen);

	tnode = g_node_first_child(parent);
	while (tnode && ((PhraseDatum *)tnode->data)->freq < phrdt->freq)
		tnode = g_node_next_sibling(tnode);
	if (!tnode)
		tnode = g_node_append_data(parent, phrdt);
	else
		tnode = g_node_insert_data_before(parent, tnode, phrdt);
}

/**
 * 按汉字索引值搜索当前节点的孩子节点，若不存在则插入新的孩子节点.
 * @param parent 当前节点指针
 * @param index 汉字索引值
 * @return 孩子节点
 */
GNode *CreateMB::SearchChildByCharsIndex(GNode *parent, int8_t index)
{
	GNode *tnode;

	tnode = g_node_first_child(parent);
	while (tnode && GPOINTER_TO_INT(tnode->data) < index)
		tnode = g_node_next_sibling(tnode);
	if (!tnode)
		tnode = g_node_append_data(parent, GINT_TO_POINTER(index));
	else if (GPOINTER_TO_INT(tnode->data) != index)
		tnode = g_node_insert_data_before(parent, tnode, GINT_TO_POINTER(index));

	return tnode;
}

/**
 * 按汉字索引数组长度搜索当前节点的孩子节点，若不存在则插入新的孩子节点.
 * @param parent 当前节点指针
 * @param len 汉字索引数组长度
 * @return 孩子节点
 */
GNode *CreateMB::SearchChildByCharsLength(GNode *parent, int len)
{
	GNode *tnode;

	tnode = g_node_first_child(parent);
	while (tnode && GPOINTER_TO_INT(tnode->data) < len)
		tnode = g_node_next_sibling(tnode);
	if (!tnode)
		tnode = g_node_append_data(parent, GINT_TO_POINTER(len));
	else if (GPOINTER_TO_INT(tnode->data) != len)
		tnode = g_node_insert_data_before(parent, tnode, GINT_TO_POINTER(len));

	return tnode;
}

/**
 * 写出拼音码表文件的索引部分.
 * @param node 树节点
 * @param para 参数
 * @return GLib库所需
 */
gboolean CreateMB::WritePinyinMBIndex(GNode *node, TravTreePara *para)
{
	int8_t index;
	int length;
	guint childrens;
	PhraseDatum *phrdt;
	GNode *tnode;

	switch (g_node_depth(node)) {
	case 1:
		/* 写出本索引树的总索引量 */
		tnode = g_node_last_child(node);
		index = tnode ? GPOINTER_TO_INT(tnode->data) + 1 : 0;
		xwrite(para->fd, &index, sizeof(index));
		break;
	case 2:
		/* 写出本节点所代表的索引值 */
		index = GPOINTER_TO_INT(node->data);
		xwrite(para->fd, &index, sizeof(index));
		/* 写出本节点下的总索引量 */
		tnode = g_node_last_child(node);
		length = tnode ? GPOINTER_TO_INT(tnode->data) : 0;
		xwrite(para->fd, &length, sizeof(length));
		break;
	case 3:
		/* 写出本节点所代表的词语长度 */
		length = GPOINTER_TO_INT(node->data);
		xwrite(para->fd, &length, sizeof(length));
		/* 写出本节点下的总词语量 */
		childrens = g_node_n_children(node);
		xwrite(para->fd, &childrens, sizeof(childrens));
		break;
	case 4:
		/* 写出本节点所包含的汉字索引数组 */
		phrdt = (PhraseDatum *)node->data;
		xwrite(para->fd, phrdt->chidx, sizeof(CharsIndex) * phrdt->chlen);
		/* 增加叶子数 */
		(para->eleaves)++;
		break;
	default:
		break;
	}

	return FALSE;
}

/**
 * 写出拼音码表文件的数据索引部分.
 * @param node 树节点
 * @param para 参数
 * @return GLib库所需
 */
gboolean CreateMB::WritePinyinMBDtidx(GNode *node, TravTreePara *para)
{
	PhraseDatum *phrdt;

	phrdt = (PhraseDatum *)node->data;
	xwrite(para->fd, &para->eoffset, sizeof(para->eoffset));
	para->eoffset += sizeof(phrdt->dtlen) + sizeof(gunichar2) * phrdt->dtlen;

	return FALSE;
}

/**
 * 写出拼音码表文件的数据部分.
 * @param node 树节点
 * @param fd file descriptor
 * @return GLib库所需
 */
gboolean CreateMB::WritePinyinMBData(GNode *node, int fd)
{
	PhraseDatum *phrdt;

	phrdt = (PhraseDatum *)node->data;
	xwrite(fd, &phrdt->dtlen, sizeof(phrdt->dtlen));
	xwrite(fd, phrdt->data, sizeof(gunichar2) * phrdt->dtlen);

	return FALSE;
}

/**
 * 删除词语数据资料.
 * @param node 节点
 * @return GLib库所需
 */
gboolean CreateMB::DeletePhraseDatum(GNode *node)
{
	delete (PhraseDatum *)node->data;
	return FALSE;
}
