//
// C++ Interface: InquireUserPhrase
//
// Description:
// 根据用户码表文件数据建立词语索引树，并接受以汉字索引数组为参数的查询方式
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef __SRC_ENGINE_INQUIREUSERPHRASE_H
#define __SRC_ENGINE_INQUIREUSERPHRASE_H

#include "mess.h"

/**
 * 词语信息.
 */
class UserPhraseInfo {
public:
	UserPhraseInfo();
	~UserPhraseInfo();

	off_t offset;		///< 词语索引偏移量
	uint freq;		///< 词语使用频率
};
/**
 * 按汉字索引数组长度分类的索引点.
 */
class UserCharsLengthPoint {
public:
	UserCharsLengthPoint();
	~UserCharsLengthPoint();

	guint indexs;		///< 此索引点下的总索引量
	CharsIndex *chidx;	///< 汉字索引数组索引表
	UserPhraseInfo *table;	///< 词语信息表
};
/**
 * 按汉字索引值分类的索引点.
 */
class UserCharsIndexPoint {
public:
	UserCharsIndexPoint();
	~UserCharsIndexPoint();

	int indexs;		///< 此索引点下的总索引量
	UserCharsLengthPoint *table;	///< 按汉字索引数组长度分类的索引点数组索引表
};
/**
 * 词语树的根索引点.
 */
class UserRootIndexPoint {
public:
	UserRootIndexPoint();
	~UserRootIndexPoint();

	int8_t indexs;		///< 此索引点下的总索引量
	UserCharsIndexPoint *table;	///< 按汉字索引值分类的索引点数组索引表
	int fd;			///< 词语数据文件描述符
	off_t offset;		///< 索引部分的绝对偏移量
};

class InquireUserPhrase: public InquirePhrase
{
public:
	InquireUserPhrase();
	virtual ~InquireUserPhrase();

	virtual void CreateIndexTree(const char *mfile);
	virtual void SetFuzzyPinyinUnits(const int8_t *fz);
	virtual GSList *SearchMatchPhrase(const CharsIndex *chidx, int len);
	virtual PhraseIndex *SearchPreferPhrase(const CharsIndex *chidx, int len);
	virtual PhraseData *AnalysisPhraseIndex(const PhraseIndex *phridx);
	void InsertPhraseToTree(const PhraseData *phrdt);
	void DeletePhraseFromTree(const PhraseData *phrdt);
	void IncreasePhraseFreq(const PhraseData *phrdt);
	void WritePhraseIndexTree();
private:
	void WriteEmptyIndexTree();
	void ReadPhraseIndexTree();
	GSList *SearchIndexMatchPhrase(int8_t index, const CharsIndex *chidx, int len);
	PhraseIndex *SearchIndexPreferPhrase(int8_t index, const CharsIndex *chidx,
									 int len);
	bool MatchCharsIndex(const CharsIndex *sidx, const CharsIndex *didx, int len);

	const int8_t *fztable;		///< 模模糊拼音单元对照表
	UserRootIndexPoint root;	///< 词语树的根索引点
};

#endif
