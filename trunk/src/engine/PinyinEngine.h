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
	void AddFuzzyPinyinUnit(const char *unit1, const char *unit2);
	void BakUserPhrase();
private:
	bool BreakMbfileString(char *lineptr, const char **mfile, const char **priority);
	EngineUnit *CreateEngineUnit(const char *mfile, int priority, ENGINE_TYPE type);

	GSList *eulist;		///< 引擎链表
	int8_t *fztable;		///< 模糊拼音对照表
	uint8_t pagesize;	///< 页面大小

	char *userpath;	///< 用户码表路径
	char *bakpath;		///< 备份码表路径
};

#endif
