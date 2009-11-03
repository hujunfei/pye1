//
// C++ Interface: PhraseEngine
//
// Description:
// 词语引擎接口，此类管理着多个词语查询类，并借助它们完成引擎的具体工作
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
#ifndef __SRC_ENGINE_PHRASEENGINE_H
#define __SRC_ENGINE_PHRASEENGINE_H

#include "mess.h"

/**
 * 拼音校正单元.
 * 为什么不声明成类？考虑一下(PinyinEditor)的实现，你就应该会明白了！ \n
 */
typedef struct {
	const char *fstring;	///< 错误串
	const char *tstring;	///< 正确串
	bool isstatic;	///< 是否为静态串
}RectifyUnit;
/**
 * 引擎单元的类型.
 */
typedef enum {
	SYSTEM_TYPE,	///< 系统词语引擎
	USER_TYPE	///< 用户词语引擎
}EUNIT_TYPE;
/**
 * 引擎单元.
 */
class EngineUnit {
public:
	EngineUnit();
	~EngineUnit();

	InquirePhrase *inqphr;	///< 词语查询类
	int priority;		///< 引擎单元优先级
	EUNIT_TYPE type;	///< 引擎单元类型
};
/**
 * 引擎单元的词语索引缓冲点.
 */
class EunitPhrase {
public:
	EunitPhrase();
	~EunitPhrase();

	const EngineUnit *eunit;	///< 引擎单元
	GSList *phrlist;		///< 词语索引缓冲链表
};

class PhraseEngine
{
public:
	PhraseEngine();
	~PhraseEngine();

	void CreateSysEngineUnits(const char *sys);
	void CreateUserEngineUnit(const char *user);
	void AddRectifyPinyinPair(const char *pinyin1, const char *pinyin2);
	void AddFuzzyPinyinUnit(const char *unit1, const char *unit2);
	void BakUserEnginePhrase();

	void SyncEngineUnitData(GSList **euphrlist, time_t stamp) const;
	void DeletePhraseData(const PhraseData *phrdt) const;
	void FeedbackPhraseData(const PhraseData *phrdt) const;
	GSList *InquirePhraseIndex(const CharsIndex *chidx, int chlen) const;
	GArray *GetRectifyTable() const;
private:
	bool BreakMbfileString(char *lineptr, const char **mfile, const char **priority);
	EngineUnit *CreateEngineUnit(const char *mfile, int priority, EUNIT_TYPE type);

	GSList *eulist;		///< 引擎链表
	GArray *rtftable;	///< 拼音矫正表
	int8_t *fztable;		///< 模糊拼音对照表

	char *userpath;	///< 用户码表路径
	char *bakpath;		///< 备份码表路径
	time_t timestamp;	///< 引擎最后一次变动的时间戳
};

#endif
