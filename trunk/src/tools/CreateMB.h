//
// C++ Interface: CreateMB
//
// Description:
// 分析utf8编码的词语文件，并生成一份二进制的系统码表文件
// 词语文件格式: 词语 拼音 频率
// e.g.: 郁闷 yu'men 1234
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef __SRC_TOOLS_CREATEMB_H
#define __SRC_TOOLS_CREATEMB_H

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
 * 遍历树的参数结构.
 */
class TravTreePara {
public:
	TravTreePara();
	~TravTreePara();

	int fd;		///< 文件描述符
	union {
		uint leaves;	///< 叶子数
		off_t offset;	///< 偏移量
	}extra;
};
#define eleaves extra.leaves
#define eoffset extra.offset

/**
 * 创建码表.
 */
class CreateMB
{
public:
	CreateMB();
	~CreateMB();

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

	GNode *root;	///< 词语索引树的根节点
private:
	static gboolean WritePinyinMBIndex(GNode *node, TravTreePara *para);
	static gboolean WritePinyinMBDtidx(GNode *node, TravTreePara *para);
	static gboolean WritePinyinMBData(GNode *node, int fd);
	static gboolean DeletePhraseDatum(GNode *node);
};

#endif
