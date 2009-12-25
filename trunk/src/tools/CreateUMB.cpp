//
// C++ Implementation: CreateUMB
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "CreateUMB.h"
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
UserPhraseInfo::UserPhraseInfo():offset(0), freq(0)
{}
UserPhraseInfo::~UserPhraseInfo()
{}
UserCharsLengthPoint::UserCharsLengthPoint():childrens(0), chidx(NULL),
 phrinf(NULL)
{}
UserCharsLengthPoint::~UserCharsLengthPoint()
{
	delete [] chidx;
	delete [] phrinf;
}
UserCharsIndexPoint::UserCharsIndexPoint():indexs(0), table(NULL)
{}
UserCharsIndexPoint::~UserCharsIndexPoint()
{
	delete [] table;
}
UserRootIndexPoint::UserRootIndexPoint():indexs(0), table(NULL),
 fd(-1), offset(0)
{}
UserRootIndexPoint::~UserRootIndexPoint()
{
	delete [] table;
	close(fd);
}
/** @} */

CreateUMB::CreateUMB()
{
	rnode = g_node_new(NULL);
}

CreateUMB::~CreateUMB()
{
	g_node_traverse(rnode, G_LEVEL_ORDER, G_TRAVERSE_LEAVES, -1,
			 GNodeTraverseFunc(DeletePhraseDatum), NULL);
	g_node_destroy(rnode);
}

/**
 * 根据sfile源文件的数据创建词语索引树.
 * @param sfile 源文件
 */
void CreateUMB::CreateIndexTree(const char *sfile)
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
void CreateUMB::WriteIndexTree(const char *tfile)
{
	/* 创建目标文件 */
	if ((root.fd = open(tfile, O_WRONLY | O_CREAT | O_EXCL, 00644)) == -1)
		errx(1, "Open file \"%s\" failed, %s", tfile, strerror(errno));

	/* 写出索引部分偏移量 */
	xwrite(root.fd, &root.offset, sizeof(root.offset));
	/* 写出码表数据部分 */
	WritePinyinMBData();
	/* 更新索引部分偏移量 */
	root.offset = lseek(root.fd, 0, SEEK_CUR);
	lseek(root.fd, 0, SEEK_SET);
	xwrite(root.fd, &root.offset, sizeof(root.offset));
	lseek(root.fd, 0, SEEK_END);
	/* 写出码表索引部分 */
	WritePinyinMBIndex();

	/* 关闭目标文件 */
	close(root.fd);
}

/**
 * 分割词语串的各部分.
 * @param lineptr 源串
 * @retval phrase 词语串
 * @retval pinyin 拼音串
 * @retval freq 频率串
 * @return 串是否合法
 */
bool CreateUMB::BreakPhraseString(char *lineptr, const char **phrase,
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
PhraseDatum *CreateUMB::CreatePhraseDatum(const char *phrase, const char *pinyin,
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
void CreateUMB::InsertPhraseToTree(PhraseDatum *phrdt)
{
	GNode *parent, *tnode;

	parent = SearchChildByCharsIndex(rnode, phrdt->chidx->major);
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
GNode *CreateUMB::SearchChildByCharsIndex(GNode *parent, int8_t index)
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
GNode *CreateUMB::SearchChildByCharsLength(GNode *parent, int len)
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
 * 写出拼音码表文件的数据部分.
 */
void CreateUMB::WritePinyinMBData()
{

}

/**
 * 写出拼音码表文件的索引部分.
 */
void CreateUMB::WritePinyinMBIndex()
{
	UserCharsIndexPoint *indexp;
	UserCharsLengthPoint *lengthp;
	int8_t index;
	int length;

	/* 写出根索引点的数据 */
	lseek(root.fd, 0, SEEK_SET);
	xwrite(root.fd, &root.offset, sizeof(root.offset));
	lseek(root.fd, root.offset, SEEK_SET);
	xwrite(root.fd, &root.indexs, sizeof(root.indexs));
	if (root.indexs == 0)
		return;
	/* 写出汉字索引值的索引数据 */
	xwrite(root.fd, root.table, sizeof(UserCharsIndexPoint) * root.indexs);
	index = 0;
	while (index < root.indexs) {
		indexp = root.table + index;
		if (indexp->indexs == 0) {
			index++;
			continue;
		}
		/* 写出汉字索引数组长度的索引数据 */
		xwrite(root.fd, indexp->table, sizeof(UserCharsLengthPoint) *
							 indexp->indexs);
		length = 1;
		while (length <= indexp->indexs) {
			lengthp = indexp->table + length - 1;
			if (lengthp->childrens == 0) {
				length++;
				continue;
			}
			/* 写出汉字索引&词语信息的数据 */
			xwrite(root.fd, lengthp->chidx, sizeof(CharsIndex) * length *
								 lengthp->childrens);
			xwrite(root.fd, lengthp->phrinf, sizeof(UserPhraseInfo) *
							 lengthp->childrens);
			length++;
		}
		index++;
	}
}

/**
 * 删除词语数据资料.
 * @param node 节点
 * @return GLib库所需
 */
gboolean CreateUMB::DeletePhraseDatum(GNode *node)
{
	delete (PhraseDatum *)node->data;
	return FALSE;
}
