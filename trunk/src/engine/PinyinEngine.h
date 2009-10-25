//
// C++ Interface: PinyinEngine
//
// Description:
// 拼音引擎接口，此类管理着多个拼音查询类，并借助它们完成引擎的具体工作
// 系统码表配置文件格式: 文件名 优先级
// e.g.:	pinyin1.mb 18
//		pinyin2.mb 12
//		pinyin3.mb 50
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef __SRC_ENGINE_PINYINENGINE_H
#define __SRC_ENGINE_PINYINENGINE_H

#include "mess.h"

/**
 * 引擎单元的类型.
 */
typedef enum {
	SYSTEM_TYPE,	///< 系统词语引擎
	USER_TYPE	///< 用户词语引擎
}ENGINE_TYPE;
/**
 * 引擎单元.
 */
class EngineUnit {
public:
	EngineUnit();
	~EngineUnit();

	InquirePhrase *inqphr;	///< 短语查询类
	GSList *phrlist;		///< 短语缓存链表
	int priority;		///< 引擎单元优先级
	ENGINE_TYPE type;	///< 引擎单元类型
};

/**
 * 拼音引擎.
 */
class PinyinEngine
{
public:
	PinyinEngine();
	~PinyinEngine();

	void InitSysEngineUnits(const char *sys);
	void InitUserEngineUnit(const char *user);
	void AddCorrectPinyinPair(const char *pinyin1, const char *pinyin2);
	void AddFuzzyPinyinUnit(const char *unit1, const char *unit2);
	void BakUserPhrase();

	bool MoveCursorPoint(int offset);
	bool InsertPinyinKey(const char ch);
	bool DeletePinyinKey();
	bool BackspacePinyinKey();
	bool RevokeSelectedPhrase();
	bool GetCommitText(gunichar2 **text, glong *len);
	bool GetPreeditText(gunichar2 **text, glong *len);
	bool GetAuxiliaryText(gunichar2 **text, glong *len);
	bool GetPagePhrase(GSList **list, guint *len);
	bool IsExistCachePhrase(const PhraseData *phrdt);
	bool SelectCachePhrase(const PhraseData *phrdt);
	bool FeedbackSelectedPhrase();
	bool IsFinishInquirePhrase();
	bool FinishInquirePhrase();
private:
	bool BreakMbfileString(char *lineptr, const char **mfile, const char **priority);
	EngineUnit *CreateEngineUnit(const char *mfile, int priority, ENGINE_TYPE type);
	PhraseData *CreateUserPhrase();
	void CreateCharsIndex();
	void InquirePhraseIndex();
	char *CorrectPinyinString();
	int ComputeInquireOffset();
	EngineUnit *SearchPreferPhrase();
	void ClearEngineUnitBuffer();
	void ClearPinyinEngineBuffer();
	void ClearPinyinEngineOldBuffer();
	void ClearPinyinEngineTempBuffer();

	GSList *eulist;		///< 引擎链表
	GPtrArray *crttable;	///< 拼音矫正表
	int8_t *fztable;		///< 模糊拼音对照表
	uint8_t pagesize;	///< 页面大小

	GArray *pytable;	///< 待查询拼音表
	guint cursor;		///< 光标位置
	CharsIndex *chidx;	///< 汉字索引数组
	int chlen;		///< 汉字索引数组长度
	GSList *aclist;		///< 已接受词语链表
	GSList *cclist;		///< 缓冲词语链表

	char *userpath;	///< 用户码表路径
	char *bakpath;		///< 备份码表路径
};

#endif
