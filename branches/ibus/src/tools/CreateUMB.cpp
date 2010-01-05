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
UserRootIndexPoint::UserRootIndexPoint():indexs(0), table(NULL)
{}
UserRootIndexPoint::~UserRootIndexPoint()
{
	delete [] table;
}
/** @} */

/**
 * 类构造函数.
 */
CreateUMB::CreateUMB()
{
	treeroot = g_node_new(NULL);
}

/**
 * 类析构函数.
 */
CreateUMB::~CreateUMB()
{
	g_node_traverse(treeroot, G_LEVEL_ORDER, G_TRAVERSE_LEAVES, -1,
			 GNodeTraverseFunc(DeletePhraseDatum), NULL);
	g_node_destroy(treeroot);
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
	int fd;
	off_t offset;

	/* 创建目标文件 */
	if ((fd = open(tfile, O_WRONLY | O_CREAT | O_EXCL, 00644)) == -1)
		errx(1, "Open file \"%s\" failed, %s", tfile, strerror(errno));

	/* 写出索引部分的偏移量 */
	xwrite(fd, &offset, sizeof(offset));	//仅仅占位而已
	/* 写出码表数据部分 */
	WritePinyinMBData(fd);
	/* 更新索引部分的偏移量 */
	offset = lseek(fd, 0, SEEK_CUR);
	lseek(fd, 0, SEEK_SET);
	xwrite(fd, &offset, sizeof(offset));
	lseek(fd, 0, SEEK_END);
	/* 写出码表索引部分 */
	WritePinyinMBIndex(fd);

	/* 关闭文件 */
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

	parent = SearchChildByCharsIndex(treeroot, phrdt->chidx->major);
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
 * @param fd 文件描述符
 */
void CreateUMB::WritePinyinMBData(int fd)
{
	GNode *indexn, *lengthn, *phrdtn;
	UserCharsIndexPoint *indexp;
	UserCharsLengthPoint *lengthp;
	PhraseDatum *phrdt;
	int length;
	guint number;

	/* 填充根节点数据 */
	indexn = g_node_last_child(treeroot);
	root.indexs = GPOINTER_TO_INT(indexn->data) + 1;
	root.table = new UserCharsIndexPoint[root.indexs];
	/* 构建树 */
	indexn = g_node_first_child(treeroot);
	do {
		/* 填充词语索引值节点数据 */
		indexp = (root.table + GPOINTER_TO_INT(indexn->data));
		lengthn = g_node_last_child(indexn);
		indexp->indexs = GPOINTER_TO_INT(lengthn->data);
		indexp->table = new UserCharsLengthPoint[indexp->indexs];
		/* 构建树 */
		lengthn = g_node_first_child(indexn);
		do {
			length = GPOINTER_TO_INT(lengthn->data);
			lengthp = (indexp->table + length - 1);
			lengthp->childrens = g_node_n_children(lengthn);
			lengthp->chidx = new CharsIndex[length * lengthp->childrens];
			lengthp->phrinf = new UserPhraseInfo[lengthp->childrens];
			/* 构建数组 */
			number = 0;
			phrdtn = g_node_first_child(lengthn);
			do {
				/* 填充组项 */
				phrdt = (PhraseDatum *)phrdtn->data;
				memcpy(lengthp->chidx + length * number, phrdt->chidx,
							 sizeof(CharsIndex) * length);
				(lengthp->phrinf + number)->offset = lseek(fd, 0,
									 SEEK_CUR);
				(lengthp->phrinf + number)->freq = phrdt->freq;
				/* 写出词语数据 */
				xwrite(fd, &phrdt->dtlen, sizeof(phrdt->dtlen));
				xwrite(fd, phrdt->data, sizeof(gunichar2) * phrdt->dtlen);
				/* 编号增加 */
				number++;
			} while ( (phrdtn = g_node_next_sibling(phrdtn)));
		} while ( (lengthn = g_node_next_sibling(lengthn)));
	} while ( (indexn = g_node_next_sibling(indexn)));
}

/**
 * 写出拼音码表文件的索引部分.
 * @param fd 文件描述符
 */
void CreateUMB::WritePinyinMBIndex(int fd)
{
	UserCharsIndexPoint *indexp;
	UserCharsLengthPoint *lengthp;
	int8_t index;
	int length;

	/* 写出根索引点的数据 */
	xwrite(fd, &root.indexs, sizeof(root.indexs));
	if (root.indexs == 0)
		return;
	/* 写出汉字索引值的索引数据 */
	xwrite(fd, root.table, sizeof(UserCharsIndexPoint) * root.indexs);
	index = 0;
	while (index < root.indexs) {
		indexp = root.table + index;
		if (indexp->indexs == 0) {
			index++;
			continue;
		}
		/* 写出汉字索引数组长度的索引数据 */
		xwrite(fd, indexp->table, sizeof(UserCharsLengthPoint) *
							 indexp->indexs);
		length = 1;
		while (length <= indexp->indexs) {
			lengthp = indexp->table + length - 1;
			if (lengthp->childrens == 0) {
				length++;
				continue;
			}
			/* 写出汉字索引&词语信息的数据 */
			xwrite(fd, lengthp->chidx, sizeof(CharsIndex) * length *
							 lengthp->childrens);
			xwrite(fd, lengthp->phrinf, sizeof(UserPhraseInfo) *
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
