//
// C++ Interface: CreateUMB
//
// Description:
// 分析utf8编码的词语文件，并生成一份二进制的用户码表文件
// 词语文件格式: 词语 拼音 频率
// e.g.: 郁闷 yu'men 1234
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef __SRC_TOOLS_CREATEUMB_H
#define __SRC_TOOLS_CREATEUMB_H

#include "../engine/mess.h"

/**
 * 词语数据资料.
 */
class PhraseDatum {
public:
	PhraseDatum();
	~PhraseDatum();

	CharsIndex *chidx;	///< 词语的汉字索引数组
	int chlen;		///< 词语的汉字索引数组的长度
	gunichar2 *data;	///< 词语的utf16编码数据
	glong dtlen;		///< 词语的utf16编码数据的长度
	uint freq;		///< 词语的使用频率
};
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
};

class CreateUMB
{
public:
	CreateUMB();
	~CreateUMB();

	void CreateIndexTree(const char *sfile);
	void WriteIndexTree(const char *tfile);
private:
	bool BreakPhraseString(char *lineptr, const char **phrase,
			 const char **pinyin, const char **freq);
	PhraseDatum *CreatePhraseDatum(const char *phrase, const char *pinyin,
							 const char *freq);
	void InsertPhraseToTree(PhraseDatum *phrdt);
	GNode *SearchChildByCharsIndex(GNode *parent, int8_t index);
	GNode *SearchChildByCharsLength(GNode *parent, int len);

	void WritePinyinMBData(int fd);
	void WritePinyinMBIndex(int fd);

	GNode *treeroot;	///< 词语索引树的根节点
	UserRootIndexPoint root;	///< 词语树的根索引点
private:
	static gboolean DeletePhraseDatum(GNode *node);
};

#endif
