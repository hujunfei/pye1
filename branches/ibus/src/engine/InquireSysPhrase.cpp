//
// C++ Implementation: InquireSysPhrase
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "InquireSysPhrase.h"
#include "../utils/wrapper.h"
#include "../utils/output.h"

/**
 * @name 相关底层数据的构造函数&析构函数.
 * @{
 */
SysCharsLengthPoint::SysCharsLengthPoint():indexs(0), chidx(NULL),
 offset(0)
{}
SysCharsLengthPoint::~SysCharsLengthPoint()
{
	delete [] chidx;
}
SysCharsIndexPoint::SysCharsIndexPoint():indexs(0), table(NULL)
{}
SysCharsIndexPoint::~SysCharsIndexPoint()
{
	delete [] table;
}
SysRootIndexPoint::SysRootIndexPoint():indexs(0), table(NULL),
 fd(-1), offset(0)
{}
SysRootIndexPoint::~SysRootIndexPoint()
{
	delete [] table;
	close(fd);
}
/** @} */

/**
 * 类构造函数.
 */
InquireSysPhrase::InquireSysPhrase():fztable(NULL)
{
}

/**
 * 类析构函数.
 */
InquireSysPhrase::~InquireSysPhrase()
{
}

/**
 * 根据码表文件数据建立词语索引树.
 * @param mfile 码表文件
 */
void InquireSysPhrase::CreateIndexTree(const char *mfile)
{
	/* 打开码表文件 */
	if ((root.fd = open(mfile, O_RDONLY)) == -1) {
		pwarning("Open file \"%s\" failed, %s", mfile, strerror(errno));
		return;
	}

	/* 读取词语索引 */
	ReadPhraseIndexTree();
}

/**
 * 设置模糊拼音单元对照表.
 * @param fy 对照表
 */
void InquireSysPhrase::SetFuzzyPinyinUnits(const int8_t *fz)
{
	fztable = fz;
}

/**
 * 查找与汉字索引数组相匹配的词语数据的索引.
 * @param chidx 汉字索引数组
 * @param len 汉字索引数组有效长度
 * @return 词语数据索引链表
 */
GSList *InquireSysPhrase::SearchMatchPhrase(const CharsIndex *chidx, int len)
{
	GSList head = {NULL, NULL};
	GSList *list1, *list2, *tlist;
	PhraseIndex *phridx1, *phridx2;
	int balance;

	/* 对实际索引和模糊索引下的数据分别进行查询 */
	list1 = SearchIndexMatchPhrase(chidx->major, chidx, len);
	list2 = SearchIndexMatchPhrase(fztable ? *(fztable + chidx->major) : -1,
								 chidx, len);

	/* 合并数据 */
	balance = 0;
	tlist = &head;
	while (list1 || list2) {
		if (list1 && list2) {
			phridx1 = (PhraseIndex *)list1->data;
			phridx2 = (PhraseIndex *)list2->data;
			if (phridx1->chlen > phridx2->chlen
				 || phridx1->chlen == phridx2->chlen && balance <= 0) {
				tlist = g_slist_concat(tlist, list1);
				tlist = list1;
				list1 = g_slist_remove_link(list1, list1);
				balance++;
			} else {
				tlist = g_slist_concat(tlist, list2);
				tlist = list2;
				list2 = g_slist_remove_link(list2, list2);
				balance--;
			}
			if (phridx1->chlen != phridx2->chlen)
				balance = 0;
		} else if (list1 && !list2) {
			tlist = g_slist_concat(tlist, list1);
			list1 = NULL;
		} else if (!list1 && list2) {
			tlist = g_slist_concat(tlist, list2);
			list2 = NULL;
		}
	}

	return head.next;
}

/**
 * 查找与汉字索引数组最相匹配的词语数据的索引.
 * @param chidx 汉字索引数组
 * @param len 汉字索引数组有效长度
 * @return 词语数据索引
 */
PhraseIndex *InquireSysPhrase::SearchPreferPhrase(const CharsIndex *chidx, int len)
{
	PhraseIndex *phridx;

	if ( (phridx = SearchIndexPreferPhrase(chidx->major, chidx, len)))
		return phridx;
	phridx = SearchIndexPreferPhrase(fztable ? *(fztable + chidx->major) : -1,
									 chidx, len);

	return phridx;
}

/**
 * 分析词语数据索引所表达的词语数据.
 * @param phridx 词语数据索引
 * @return 词语数据
 */
PhraseData *InquireSysPhrase::AnalysisPhraseIndex(const PhraseIndex *phridx)
{
	PhraseData *phrdt;

	phrdt = new PhraseData;
	phrdt->chidx = (CharsIndex *)g_memdup(phridx->chidx, sizeof(CharsIndex) *
								 phridx->chlen);
	phrdt->chlen = phridx->chlen;
	phrdt->offset = phridx->offset;
	lseek(root.fd, phrdt->offset, SEEK_SET);
	xread(root.fd, &phrdt->offset, sizeof(phrdt->offset));
	lseek(root.fd, phrdt->offset, SEEK_SET);
	xread(root.fd, &phrdt->dtlen, sizeof(phrdt->dtlen));
	phrdt->data = (gunichar2 *)g_malloc(sizeof(gunichar2) * phrdt->dtlen);
	xread(root.fd, phrdt->data, sizeof(gunichar2) * phrdt->dtlen);
	phrdt->offset = (off_t)(-1);	//标记这是系统词语

	return phrdt;
}

/**
 * 读取码表文件的索引部分，并建立索引树.
 */
void InquireSysPhrase::ReadPhraseIndexTree()
{
	SysCharsIndexPoint *indexp;
	SysCharsLengthPoint *lengthp;
	off_t offset;
	int8_t index;
	int length;

	/* 预设相对偏移量为0 */
	offset = 0;

	/* 读取根索引点的数据 */
	xread(root.fd, &root.indexs, sizeof(root.indexs));
	root.table = new SysCharsIndexPoint[root.indexs];
	/* root.offset = 0;//稍候获取 */
	/* 读取汉字索引值的索引数据 */
	while (1) {
		xread(root.fd, &index, sizeof(index));
		if (index == -1)
			break;
		indexp = root.table + index;
		xread(root.fd, &indexp->indexs, sizeof(indexp->indexs));
		indexp->table = new SysCharsLengthPoint[indexp->indexs];
		/* 读取汉字索引数组长度的索引数据 */
		while (1) {
			xread(root.fd, &length, sizeof(length));
			if (length == 0)
				break;
			lengthp = indexp->table + length - 1;
			xread(root.fd, &lengthp->indexs, sizeof(lengthp->indexs));
			lengthp->chidx = new CharsIndex[length * lengthp->indexs];
			lengthp->offset = offset;
			/* 读取汉字索引的数据 */
			xread(root.fd, lengthp->chidx, sizeof(CharsIndex) * length *
								 lengthp->indexs);
			/* 修正相对偏移量 */
			offset += sizeof(off_t) * lengthp->indexs;
		}
	}

	/* 获取数据段的起始位置 */
	root.offset = lseek(root.fd, 0, SEEK_CUR);
}

/**
 * 查找位于本索引值下与汉字索引数组相匹配的词语数据的索引.
 * @param index 索引值
 * @param chidx 汉字索引数组
 * @param len 汉字索引数组有效长度
 * @return 词语数据索引链表
 */
GSList *InquireSysPhrase::SearchIndexMatchPhrase(int8_t index,
				 const CharsIndex *chidx, int len)
{
	SysCharsLengthPoint *lengthp;
	SysCharsIndexPoint *indexp;
	PhraseIndex *phridx;
	GSList *list;
	int length;
	guint number;

	/* 检查条件是否满足 */
	if (index == -1 || index >= root.indexs)
		return NULL;
	indexp = root.table + index;
	if (indexp->indexs == 0)
		return NULL;
	list = NULL;	//预设链表为空

	/* 查询数据 */
	length = 1;	//因为不可能存在长度为0的词语，所以长度预设为1
	while (length <= indexp->indexs && length <= len) {
		lengthp = indexp->table + length - 1;
		number = 0;
		while (number < lengthp->indexs) {
			if (MatchCharsIndex(lengthp->chidx + length * number,
							 chidx, length)) {
				phridx = new PhraseIndex;
				phridx->chidx = lengthp->chidx + length * number;
				phridx->chlen = length;
				phridx->offset = root.offset + lengthp->offset +
							 sizeof(off_t) * number;
				/* phridx->freq = 0;//无须设置 */
				list = g_slist_prepend(list, phridx);
			}
			number++;
		}
		length++;
	}

	return list;
}

/**
 * 查找位于本索引值下与汉字索引数组最相匹配的词语数据的索引.
 * @param index 索引值
 * @param chidx 汉字索引数组
 * @param len 汉字索引数组有效长度
 * @return 词语数据索引
 */
PhraseIndex *InquireSysPhrase::SearchIndexPreferPhrase(int8_t index,
					 const CharsIndex *chidx, int len)
{
	SysCharsLengthPoint *lengthp, *preflenp;
	SysCharsIndexPoint *indexp;
	PhraseIndex *phridx;
	int length, preflen;
	guint number, prefnum;

	/* 检查条件是否满足 */
	if (index == -1 || index >= root.indexs)
		return NULL;
	indexp = root.table + index;
	if (indexp->indexs == 0)
		return NULL;
	preflenp = NULL;	// 预设已选中数值为空

	/* 查询数据 */
	length = 1;	//因为不可能存在长度为0的词语，所以长度预设为1
	while (length <= indexp->indexs && length <= len) {
		lengthp = indexp->table + length - 1;
		number = 0;
		while (number < lengthp->indexs) {
			if (MatchCharsIndex(lengthp->chidx + length * number,
							 chidx, length)) {
				preflenp = lengthp;
				preflen = length;
				prefnum = number;
			}
			number++;
		}
		length++;
	}

	/* 检查处理结果 */
	if (!preflenp)
		return NULL;
	phridx = new PhraseIndex;
	phridx->chidx = preflenp->chidx + preflen * prefnum;
	phridx->chlen = preflen;
	phridx->offset = root.offset + preflenp->offset +
				 sizeof(off_t) * prefnum;
	/* phridx->freq = 0;//无须设置 */

	return phridx;
}

/**
 * 检查两个汉字索引数组是否匹配.
 * @param sidx 源汉字索引数组
 * @param didx 目标汉字索引数组
 * @param len 需要检查的长度
 * @return 是否匹配
 */
bool InquireSysPhrase::MatchCharsIndex(const CharsIndex *sidx,
				 const CharsIndex *didx, int len)
{
	int8_t index1, index2;
	int count;

	count = 0;
	while (count < len) {
		index1 = (sidx + count)->major;
		index2 = (didx + count)->major;
		if (index1 != index2 && (!fztable || *(fztable + index1) != index2))
			break;
		index1 = (sidx + count)->minor;
		index2 = (didx + count)->minor;
		if (index2 != -1 && index1 != index2
			 && (!fztable || *(fztable + index1) != index2))
			break;
		count++;
	}

	return (count == len);
}
