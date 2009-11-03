//
// C++ Implementation: InquireUserPhrase
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "InquireUserPhrase.h"
#include "../utils/wrapper.h"

/**
 * @name 相关底层数据的构造函数&析构函数.
 * @{
 */
UserPhraseInfo::UserPhraseInfo():offset(0), freq(0)
{}
UserPhraseInfo::~UserPhraseInfo()
{}
UserCharsLengthPoint::UserCharsLengthPoint():indexs(0), chidx(NULL),
 table(NULL)
{}
 UserCharsLengthPoint::~UserCharsLengthPoint()
{
	delete [] chidx;
	delete [] table;
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

/**
 * 类构造函数.
 */
InquireUserPhrase::InquireUserPhrase():fztable(NULL)
{
}

/**
 * 类析构函数.
 */
InquireUserPhrase::~InquireUserPhrase()
{
}

/**
 * 根据用户码表文件数据建立词语索引树.
 * @param mfile 用户码表文件
 */
void InquireUserPhrase::CreateIndexTree(const char *mfile)
{
	/* 打开码表文件 */
	if (access(mfile, F_OK) == -1) {
		if ((root.fd = open(mfile, O_RDWR | O_CREAT, 00644)) == -1)
			errx(1, "Open user's code table file \"%s\" failed, %s",
							 mfile, strerror(errno));
		WriteEmptyIndexTree();
	} else if ((root.fd = open(mfile, O_RDWR)) == -1)
		errx(1, "Open user's code table file \"%s\" failed, %s",
						 mfile, strerror(errno));

	/* 读取词语索引 */
	ReadPhraseIndexTree();
}

/**
 * 设置模糊拼音单元对照表.
 * @param fy 对照表
 */
void InquireUserPhrase::SetFuzzyPinyinUnits(const int8_t *fz)
{
	fztable = fz;
}

/**
 * 查找与汉字索引数组相匹配的词语数据的索引.
 * @param chidx 汉字索引数组
 * @param len 汉字索引数组有效长度
 * @return 词语数据索引链表
 */
GSList *InquireUserPhrase::SearchMatchPhrase(const CharsIndex *chidx, int len)
{
	GSList head = {NULL, NULL};
	GSList *list1, *list2, *tlist;
	PhraseIndex *phridx1, *phridx2;
	int8_t index;

	/* 对实际索引和模糊索引下的数据分别进行查询 */
	list1 = SearchIndexMatchPhrase(chidx->major, chidx, len);
	index = fztable ? *(fztable + chidx->major) : -1;
	list2 = SearchIndexMatchPhrase(index, chidx, len);

	/* 合并数据 */
	tlist = &head;
	while (list1 || list2) {
		if (list1 && list2) {
			phridx1 = (PhraseIndex *)list1->data;
			phridx2 = (PhraseIndex *)list2->data;
			if (phridx1->chlen > phridx2->chlen
				 || (phridx1->chlen == phridx2->chlen
					  && phridx1->freq >= phridx2->freq)) {
				tlist = g_slist_concat(tlist, list1);
				tlist = list1;
				list1 = g_slist_remove_link(list1, list1);
			} else {
				tlist = g_slist_concat(tlist, list2);
				tlist = list2;
				list2 = g_slist_remove_link(list2, list2);
			}
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
PhraseIndex *InquireUserPhrase::SearchPreferPhrase(const CharsIndex *chidx, int len)
{
	PhraseIndex *phridx1, *phridx2;
	int8_t index;

	phridx1 = SearchIndexPreferPhrase(chidx->major, chidx, len);
	index = fztable ? *(fztable + chidx->major) : -1;
	phridx2 = SearchIndexPreferPhrase(index, chidx, len);
	if (phridx1 && phridx2) {
		if (phridx1->chlen > phridx2->chlen
			 || (phridx1->chlen == phridx2->chlen
				 && phridx1->freq >= phridx2->freq)) {
			delete phridx2;
			phridx2 = phridx1;
		} else
			delete phridx1;
	} else if (phridx1)
		phridx2 = phridx1;

	return phridx2;
}

/**
 * 分析词语数据索引所表达的词语数据.
 * @param phridx 词语数据索引
 * @return 词语数据
 */
PhraseData *InquireUserPhrase::AnalysisPhraseIndex(const PhraseIndex *phridx)
{
	PhraseData *phrdt;

	phrdt = new PhraseData;
	phrdt->chidx = (CharsIndex *)g_memdup(phridx->chidx, sizeof(CharsIndex) *
								 phridx->chlen);
	phrdt->chlen = phridx->chlen;
	phrdt->offset = phridx->offset;
	lseek(root.fd, phrdt->offset, SEEK_SET);
	xread(root.fd, &phrdt->dtlen, sizeof(phrdt->dtlen));
	phrdt->data = (gunichar2 *)g_malloc(sizeof(gunichar2) * phrdt->dtlen);
	xread(root.fd, phrdt->data, sizeof(gunichar2) * phrdt->dtlen);

	return phrdt;
}

/**
 * 插入一个新的词语到词语索引树.
 * @param phrdt 词语数据
 */
void InquireUserPhrase::InsertPhraseToTree(const PhraseData *phrdt)
{
	UserCharsIndexPoint *indexp;
	UserCharsLengthPoint *lengthp;
	CharsIndex *chidx;
	UserPhraseInfo *phrinf;
	guint number;

	/* 获取词语将要插入的按汉字索引值分类的索引点. */
	if (root.indexs <= phrdt->chidx->major) {
		indexp = root.table;
		root.table = new UserCharsIndexPoint[phrdt->chidx->major + 1];
		if (root.indexs != 0) {
			memcpy(root.table, indexp, sizeof(UserCharsIndexPoint) *
								 root.indexs);
			/* 如果不对内存置0，将导致额外的数据被析构释放掉 */
			bzero(indexp, sizeof(UserCharsIndexPoint) * root.indexs);
			delete [] indexp;
		}
		root.indexs = phrdt->chidx->major + 1;
	}
	indexp = root.table + phrdt->chidx->major;

	/* 获取词语将要插入的按汉字索引数组长度分类的索引点 */
	if (indexp->indexs < phrdt->chlen) {
		lengthp = indexp->table;
		indexp->table = new UserCharsLengthPoint[phrdt->chlen];
		if (indexp->indexs != 0) {
			memcpy(indexp->table, lengthp, sizeof(UserCharsLengthPoint) *
								 indexp->indexs);
			/* 如果不对内存置0，将导致额外的数据被析构掉 */
			bzero(lengthp, sizeof(UserCharsLengthPoint) * indexp->indexs);
			delete [] lengthp;
		}
		indexp->indexs = phrdt->chlen;
	}
	lengthp = indexp->table + phrdt->chlen - 1;

	/* 定位词语位置 */
	number = 0;
	while (number < lengthp->indexs) {
		/* 新插入的词语拥有更高优先级 */
		if ((lengthp->table + number)->freq > 1)
			break;
		number++;
	}
	/* 将词语的汉字索引数组添加到索引点下 */
	chidx = lengthp->chidx;
	lengthp->chidx = new CharsIndex[phrdt->chlen * (lengthp->indexs + 1)];
	if (number != 0)
		memcpy(lengthp->chidx, chidx, sizeof(CharsIndex) * phrdt->chlen * number);
	memcpy(lengthp->chidx + phrdt->chlen * number, phrdt->chidx,
				 sizeof(CharsIndex) * phrdt->chlen);
	if (number != lengthp->indexs)
		memcpy(lengthp->chidx + phrdt->chlen * (number + 1),
				 chidx + phrdt->chlen * number,
				 sizeof(CharsIndex) * phrdt->chlen *
					 (lengthp->indexs - number));
	if (chidx)
		delete [] chidx;
	/* 将词语的相关信息添加到索引点下 */
	phrinf = lengthp->table;
	lengthp->table = new UserPhraseInfo[lengthp->indexs + 1];
	if (number != 0)
		memcpy(lengthp->table, phrinf, sizeof(UserPhraseInfo) * number);
	(lengthp->table + number)->offset = root.offset;
	(lengthp->table + number)->freq = 1;	//预设新词语使用频率为1
	if (number != lengthp->indexs)
		memcpy(lengthp->table + number + 1, phrinf + number,
			 sizeof(UserPhraseInfo) * (lengthp->indexs - number));
	if (phrinf)
		delete [] phrinf;
	/* 更新数据 */
	(lengthp->indexs)++;
	lseek(root.fd, root.offset, SEEK_SET);
	xwrite(root.fd, &phrdt->dtlen, sizeof(phrdt->dtlen));
	xwrite(root.fd, phrdt->data, sizeof(gunichar2) * phrdt->dtlen);
	root.offset += sizeof(phrdt->dtlen) + sizeof(gunichar2) * phrdt->dtlen;
}

/**
 * 从词语索引树中删除一个词语.
 * @param phrdt 词语数据
 */
void InquireUserPhrase::DeletePhraseFromTree(const PhraseData *phrdt)
{
	UserCharsIndexPoint *indexp;
	UserCharsLengthPoint *lengthp;
	guint number;

	/* 检查是否有满足条件的索引点 */
	if (root.indexs <= phrdt->chidx->major)
		return;
	indexp = root.table + phrdt->chidx->major;
	if (indexp->indexs < phrdt->chlen)
		return;
	lengthp = indexp->table + phrdt->chlen - 1;

	/* 定位词语所在位置 */
	number = 0;
	while (number < lengthp->indexs) {
		if ((lengthp->table + number)->offset == phrdt->offset)
			break;
		number++;
	}
	if (number == lengthp->indexs)
		return;

	/* 移动数据 */
	if (number + 1 != lengthp->indexs) {
		memmove(lengthp->chidx + phrdt->chlen * number,
			 lengthp->chidx + phrdt->chlen * (number + 1),
			 sizeof(CharsIndex) * phrdt->chlen *
				 (lengthp->indexs - number - 1));
		memmove(lengthp->table + number, lengthp->table + number + 1,
			 sizeof(UserPhraseInfo) * (lengthp->indexs - number - 1));
	}

	/* 更新数据 */
	(lengthp->indexs)--;
}

/**
 * 增加指定词语的使用频率.
 * @param phrdt 词语数据
 */
void InquireUserPhrase::IncreasePhraseFreq(const PhraseData *phrdt)
{
	UserCharsIndexPoint *indexp;
	UserCharsLengthPoint *lengthp;
	UserPhraseInfo phrinf;
	guint number, position;

	/* 检查是否有满足条件的索引点 */
	if (root.indexs <= phrdt->chidx->major)
		return;
	indexp = root.table + phrdt->chidx->major;
	if (indexp->indexs < phrdt->chlen)
		return;
	lengthp = indexp->table + phrdt->chlen - 1;

	/* 定位词语此刻所在位置 */
	number = 0;
	while (number < lengthp->indexs) {
		if ((lengthp->table + number)->offset == phrdt->offset) {
			((lengthp->table + number)->freq)++;
			break;
		}
		number++;
	}
	if (number == lengthp->indexs)
		return;

	/* 查询新位置 */
	position = number + 1;
	while (position < lengthp->indexs) {
		/* 最近选中的词语拥有更高优先级 */
		if ((lengthp->table + position)->freq > (lengthp->table + number)->freq)
			break;
		position++;
	}
	if (number + 1 == position)
		return;

	/* 移动数据 */
	memmove(lengthp->chidx + phrdt->chlen * number,
		 lengthp->chidx + phrdt->chlen * (number + 1),
		 sizeof(CharsIndex) * phrdt->chlen * (position - number - 1));
	memcpy(lengthp->chidx + phrdt->chlen * (position - 1), phrdt->chidx,
					 sizeof(CharsIndex) * phrdt->chlen);
	phrinf = *(lengthp->table + number);
	memmove(lengthp->table + number, lengthp->table + number + 1,
		 sizeof(UserPhraseInfo) * (position - number - 1));
	*(lengthp->table + position - 1) = phrinf;
}

/**
 * 写出词语索引树，即码表文件的索引部分.
 */
void InquireUserPhrase::WritePhraseIndexTree()
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
			if (lengthp->indexs == 0) {
				length++;
				continue;
			}
			/* 写出汉字索引&词语信息的数据 */
			xwrite(root.fd, lengthp->chidx, sizeof(CharsIndex) * length *
								 lengthp->indexs);
			xwrite(root.fd, lengthp->table, sizeof(UserPhraseInfo) *
								 lengthp->indexs);
			length++;
		}
		index++;
	}
}

/**
 * 写出空索引树的数据.
 */
void InquireUserPhrase::WriteEmptyIndexTree()
{
	off_t offset = sizeof(offset);
	int8_t indexs = 0;

	lseek(root.fd, 0, SEEK_SET);
	xwrite(root.fd, &offset, sizeof(offset));
	xwrite(root.fd, &indexs, sizeof(indexs));
}

/**
 * 读取码表文件的索引部分，并建立索引树.
 */
void InquireUserPhrase::ReadPhraseIndexTree()
{
	UserCharsIndexPoint *indexp;
	UserCharsLengthPoint *lengthp;
	int8_t index;
	int length;

	/* 读取根索引点的数据 */
	lseek(root.fd, 0, SEEK_SET);
	xread(root.fd, &root.offset, sizeof(root.offset));
	lseek(root.fd, root.offset, SEEK_SET);
	xread(root.fd, &root.indexs, sizeof(root.indexs));
	if (root.indexs == 0)
		return;
	/* 读取汉字索引值的索引数据 */
	root.table = new UserCharsIndexPoint[root.indexs];
	xread(root.fd, root.table, sizeof(UserCharsIndexPoint) * root.indexs);
	index = 0;
	while (index < root.indexs) {
		indexp = root.table + index;
		if (indexp->indexs == 0) {
			index++;
			continue;
		}
		/* 读取汉字索引数组长度的索引数据 */
		indexp->table = new UserCharsLengthPoint[indexp->indexs];
		xread(root.fd, indexp->table, sizeof(UserCharsLengthPoint) *
							 indexp->indexs);
		length = 1;
		while (length <= indexp->indexs) {
			lengthp = indexp->table + length - 1;
			if (lengthp->indexs == 0) {
				length++;
				continue;
			}
			/* 读取汉字索引&词语信息的数据 */
			lengthp->chidx = new CharsIndex[length * lengthp->indexs];
			xread(root.fd, lengthp->chidx, sizeof(CharsIndex) * length *
								 lengthp->indexs);
			lengthp->table = new UserPhraseInfo[lengthp->indexs];
			xread(root.fd, lengthp->table, sizeof(UserPhraseInfo) *
								 lengthp->indexs);
			length++;
		}
		index++;
	}
}

/**
 * 查找位于本索引值下与汉字索引数组相匹配的词语数据的索引.
 * @param index 索引值
 * @param chidx 汉字索引数组
 * @param len 汉字索引数组有效长度
 * @return 词语数据索引链表
 */
GSList *InquireUserPhrase::SearchIndexMatchPhrase(int8_t index,
				 const CharsIndex *chidx, int len)
{
	UserCharsLengthPoint *lengthp;
	UserCharsIndexPoint *indexp;
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
				phridx->offset = (lengthp->table + number)->offset;
				phridx->freq = (lengthp->table + number)->freq;
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
PhraseIndex *InquireUserPhrase::SearchIndexPreferPhrase(int8_t index,
					 const CharsIndex *chidx, int len)
{
	UserCharsLengthPoint *lengthp;
	UserCharsIndexPoint *indexp;
	PhraseIndex *phridx;
	int length;
	guint number;

	/* 检查条件是否满足 */
	if (index == -1 || index >= root.indexs)
		return NULL;
	indexp = root.table + index;
	if (indexp->indexs == 0)
		return NULL;
	phridx = NULL;	//预设数据为空

	/* 查询数据 */
	length = len < indexp->indexs ? len : indexp->indexs;
	while (length >= 1) {
		lengthp = indexp->table + length - 1;
		number = lengthp->indexs;
		while (number >= 1) {
			number--;
			if (MatchCharsIndex(lengthp->chidx + length * number,
							 chidx, length)) {
				phridx = new PhraseIndex;
				phridx->chidx = lengthp->chidx + length * number;
				phridx->chlen = length;
				phridx->offset = (lengthp->table + number)->offset;
				phridx->freq = (lengthp->table + number)->freq;
				break;
			}
		}
		if (phridx)
			break;
		length--;
	}

	return phridx;
}

/**
 * 检查两个汉字索引数组是否匹配.
 * @param sidx 源汉字索引数组
 * @param didx 目标汉字索引数组
 * @param len 需要检查的长度
 * @return 是否匹配
 */
bool InquireUserPhrase::MatchCharsIndex(const CharsIndex *sidx, const CharsIndex *didx,
										 int len)
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
