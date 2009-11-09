//
// C++ Interface: mess
//
// Description:
// 很杂乱的一些数据结构
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef __SRC_ENGINE_MESS_H
#define __SRC_ENGINE_MESS_H

#include "../include/sys.h"
#include "../include/dep.h"

/**
 * 汉字索引结构.
 * 将汉字拼音分解为两个串，并将串转换为其在拼音单元数组中的索引值 \n
 * (-1)代表本部分没有索引值，也即不存在，e.g.<er[xx,-1]> \n
 */
class CharsIndex {
public:
	CharsIndex() {major = minor = -1;}
	~CharsIndex() {}

	int8_t major;	///< 第一部分的索引值
	int8_t minor;	///< 第二部分的索引值
};

/**
 * 词语数据索引结构.
 */
class PhraseIndex {
public:
	PhraseIndex():chidx(NULL), chlen(0), offset(0), freq(0) {}
	~PhraseIndex() {}

	const CharsIndex *chidx;///< 汉字索引数组 *
	int chlen;		///< 汉字索引数组有效长度
	off_t offset;		///< 词语索引偏移量
	uint freq;		///< 词语使用频率
};

/**
 * 词语数据结构.
 */
class PhraseData {
public:
	PhraseData():chidx(NULL), chlen(0), offset(0), data(NULL), dtlen(0) {}
	~PhraseData() {delete [] chidx; g_free(data);}

	CharsIndex *chidx;	///< 汉字索引数组 *
	int chlen;		///< 汉字索引数组有效长度
	off_t offset;		///< 词语索引偏移量,系统词汇(-1),无效词汇(0),用户词汇(>=1)
	gunichar2 *data;	///< 词语数据 *
	glong dtlen;		///< 词语数据有效长度
};

/**
 * 词语数据查询虚基类.
 */
class InquirePhrase {
public:
	InquirePhrase() {}
	virtual ~InquirePhrase() {}

	virtual void CreateIndexTree(const char *mfile) = 0;
	virtual void SetFuzzyPinyinUnits(const int8_t *fy) = 0;
	virtual GSList *SearchMatchPhrase(const CharsIndex *chidx, int len) = 0;
	virtual PhraseIndex *SearchPreferPhrase(const CharsIndex *chidx, int len) = 0;
	virtual PhraseData *AnalysisPhraseIndex(const PhraseIndex *phridx) = 0;
};

#endif
