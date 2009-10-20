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
 * 引擎单元.
 */
class EngineUnit {
public:
	EngineUnit();
	~EngineUnit();

	InquirePhrase *inqphr;	///< 短语查询类
	GSList *phrlist;		///< 短语缓存链表
	int priority;		///< 引擎单元优先级
};

/**
 * 拼音引擎.
 */
class PinyinEngine
{
public:
	PinyinEngine();
	~PinyinEngine();

	void SetOutputCoding();
	void InitSysEngineUnits(const char *sys);
	void InitUserEngineUnit(const char *user);
private:
};

#endif
