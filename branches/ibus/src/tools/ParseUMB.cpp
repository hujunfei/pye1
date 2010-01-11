//
// C++ Implementation: ParseUMB
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "ParseUMB.h"
#include "../engine/ParseString.h"
#include "../engine/wrapper.h"

/**
 * @name 相关底层数据的构造函数&析构函数.
 * @{
 */
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
 fd(-1)
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
ParseUMB::ParseUMB()
{
}

/**
 * 类析构函数.
 */
ParseUMB::~ParseUMB()
{
}

/**
 * 根据用户码表文件数据建立词语索引树.
 * @param sfile 用户码表文件
 */
void ParseUMB::CreateIndexTree(const char *sfile)
{
	/* 打开码表文件 */
	if ((root.fd = open(sfile, O_RDONLY)) == -1)
		errx(1, "Open user's code table file \"%s\" failed, %s",
						 sfile, strerror(errno));
	/* 读取词语索引 */
	ReadPhraseIndexTree();
}

/**
 * 以文本的方式写出词语索引树.
 * @param dfile 输出目标文件
 * @param length 短语有效长度
 * @param reset 是否重置短语频率
 */
void ParseUMB::WriteIndexTree(const char *dfile, int length, bool reset)
{
	FILE *stream;

	/* 打开输出文件 */
	if (!(stream = fopen(dfile, "w")))
		errx(1, "Fopen file \"%s\" failed, %s", dfile, strerror(errno));
	/* 写出词语索引树 */
	WritePhraseIndexTree(stream, length, reset);
	/* 关闭文件 */
	fclose(stream);
}

/**
 * 读取码表文件的索引部分，并建立索引树.
 */
void ParseUMB::ReadPhraseIndexTree()
{
	UserCharsIndexPoint *indexp;
	UserCharsLengthPoint *lengthp;
	off_t offset;
	int8_t index;
	int length;

	/* 读取根索引点的数据 */
	lseek(root.fd, 0, SEEK_SET);
	xread(root.fd, &offset, sizeof(offset));
	lseek(root.fd, offset, SEEK_SET);
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
			if (lengthp->childrens == 0) {
				length++;
				continue;
			}
			/* 读取汉字索引&词语信息的数据 */
			lengthp->chidx = new CharsIndex[length * lengthp->childrens];
			xread(root.fd, lengthp->chidx, sizeof(CharsIndex) * length *
								 lengthp->childrens);
			lengthp->phrinf = new UserPhraseInfo[lengthp->childrens];
			xread(root.fd, lengthp->phrinf, sizeof(UserPhraseInfo) *
							 lengthp->childrens);
			length++;
		}
		index++;
	}
}

/**
 * 写出词语索引树.
 * @param stream 输出流
 * @param length 短语有效长度
 * @param reset 是否重置短语频率
 */
void ParseUMB::WritePhraseIndexTree(FILE *stream, int length, bool reset)
{
	PhraseIndex phridx;
	UserCharsIndexPoint *indexp;
	UserCharsLengthPoint *lengthp;
	int8_t idxcnt;
	int lencnt;
	guint numcnt;

	idxcnt = 0;
	while (idxcnt < root.indexs) {
		if (!(indexp = root.table + idxcnt)) {
			idxcnt++;
			continue;
		}
		lencnt = length > 0 ? length : 1;	//小于此长度的词语将被忽略
		while (lencnt <= indexp->indexs) {
			if (!(lengthp = indexp->table + lencnt - 1)) {
				lencnt++;
				continue;
			}
			numcnt = lengthp->childrens;
			while (numcnt > 0) {
				numcnt--;
				phridx.chidx = lengthp->chidx + lencnt * numcnt;
				phridx.chlen = lencnt;
				phridx.offset = (lengthp->phrinf + numcnt)->offset;
				phridx.freq = (lengthp->phrinf + numcnt)->freq;
				WritePhraseData(stream, &phridx, reset);
			}
			lencnt++;
		}
		idxcnt++;
	}
}

/**
 * 写出词语数据.
 * @param stream 输出流
 * @param phridx 词语索引信息
 * @param reset 是否重置短语频率
 */
void ParseUMB::WritePhraseData(FILE *stream, PhraseIndex *phridx, bool reset)
{
	ParseString parse;
	PhraseData phrdt;
	gchar *phrase, *pinyin;
	uint freq;

	AnalysisPhraseIndex(phridx, &phrdt);
	phrase = g_utf16_to_utf8(phrdt.data, phrdt.dtlen, NULL, NULL, NULL);
	pinyin = parse.RestorePinyinString(phrdt.chidx, phrdt.chlen);
	freq = !reset ? phridx->freq : 0;
	fprintf(stream, "%s\t%s\t%u\n", phrase, pinyin, freq);
	g_free(pinyin);
	g_free(phrase);
}

/**
 * 分析词语数据索引所表达的词语数据.
 * @param phridx 词语数据索引
 * @retval phrdt 词语数据
 */
void ParseUMB::AnalysisPhraseIndex(const PhraseIndex *phridx, PhraseData *phrdt)
{
	phrdt->chidx = new CharsIndex[phridx->chlen];
	memcpy(phrdt->chidx, phridx->chidx, sizeof(CharsIndex) * phridx->chlen);
	phrdt->chlen = phridx->chlen;
	phrdt->offset = phridx->offset;
	lseek(root.fd, phrdt->offset, SEEK_SET);
	xread(root.fd, &phrdt->dtlen, sizeof(phrdt->dtlen));
	phrdt->data = (gunichar2 *)g_malloc(sizeof(gunichar2) * phrdt->dtlen);
	xread(root.fd, phrdt->data, sizeof(gunichar2) * phrdt->dtlen);
}
