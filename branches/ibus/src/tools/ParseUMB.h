//
// C++ Interface: ParseUMB
//
// Description:
// 分析二进制的用户码表文件，并生成一份utf8编码的词语文件
// 词语文件格式: 词语 拼音 频率
// e.g.: 郁闷 yu'men 1234
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef __SRC_TOOLS_PARSEUMB_H
#define __SRC_TOOLS_PARSEUMB_H

#include "../engine/mess.h"

/**
 * 词语信息.
 */
class UserPhraseInfo {
public:
	UserPhraseInfo();
	~UserPhraseInfo();

	off_t offset;		///< 词语绝对偏移量
	uint freq;		///< 词语使用频率
};
/**
 * 按汉字索引数组长度分类的索引点.
 */
class UserCharsLengthPoint {
public:
	UserCharsLengthPoint();
	~UserCharsLengthPoint();

	guint childrens;		///< 此索引点下的总索引量
	CharsIndex *chidx;	///< 汉字索引数组索引表
	UserPhraseInfo *phrinf;	///< 词语信息数组索引表
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
};

class ParseUMB
{
public:
	ParseUMB();
	~ParseUMB();

	void CreateIndexTree(const char *sfile);
	void WriteIndexTree(const char *dfile, int length, bool reset);
private:
	void ReadPhraseIndexTree();
	void WritePhraseIndexTree(FILE *stream, int length, bool reset);
	void WritePhraseData(FILE *stream, PhraseIndex *phridx, bool reset);
	void AnalysisPhraseIndex(const PhraseIndex *phridx, PhraseData *phrdt);

	UserRootIndexPoint root;	///< 词语树的根索引点
};

#endif
