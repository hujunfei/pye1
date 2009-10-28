//
// C++ Interface: InquireSysPhrase
//
// Description:
// 根据码表文件数据建立词语索引树，并接受以汉字索引数组为参数的查询方式
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef __SRC_ENGINE_INQUIRESYSPHRASE_H
#define __SRC_ENGINE_INQUIRESYSPHRASE_H

#include "mess.h"

/**
 * 按汉字索引数组长度分类的索引点.
 */
class SysCharsLengthPoint {
public:
	SysCharsLengthPoint();
	~SysCharsLengthPoint();

	guint indexs;		///< 此索引点下的总索引量
	CharsIndex *chidx;	///< 汉字索引数组索引表
	off_t offset;		///< 本部分数据索引的相对偏移量
};
/**
 * 按汉字索引值分类的索引点.
 */
class SysCharsIndexPoint {
public:
	SysCharsIndexPoint();
	~SysCharsIndexPoint();

	int indexs;		///< 此索引点下的总索引量
	SysCharsLengthPoint *table;	///< 按汉字索引数组长度分类的索引点数组索引表
};
/**
 * 词语树的根索引点.
 */
class SysRootIndexPoint {
public:
	SysRootIndexPoint();
	~SysRootIndexPoint();

	int8_t indexs;		///< 此索引点下的总索引量
	SysCharsIndexPoint *table;	///< 按汉字索引值分类的索引点数组索引表
	int fd;			///< 词语数据文件描述符
	off_t offset;		///< 数据部分的绝对偏移量
};

/**
 * 查询词语.
 */
class InquireSysPhrase: public InquirePhrase
{
public:
	InquireSysPhrase();
	virtual ~InquireSysPhrase();

	virtual void CreateIndexTree(const char *mfile);
	virtual void SetFuzzyPinyinUnits(const int8_t *fz);
	virtual GSList *SearchMatchPhrase(const CharsIndex *chidx, int len);
	virtual PhraseIndex *SearchPreferPhrase(const CharsIndex *chidx, int len);
	virtual PhraseData *AnalysisPhraseIndex(const PhraseIndex *phridx);
private:
	void ReadPhraseIndexTree();
	GSList *SearchIndexMatchPhrase(int8_t index, const CharsIndex *chidx, int len);
	PhraseIndex *SearchIndexPreferPhrase(int8_t index, const CharsIndex *chidx,
									 int len);
	bool MatchCharsIndex(const CharsIndex *sidx, const CharsIndex *didx, int len);

	const int8_t *fztable;		///< 模模糊拼音单元对照表
	SysRootIndexPoint root;	///< 词语树的根索引点
};

#endif
