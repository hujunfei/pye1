//
// C++ Implementation: PinyinEngine
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "PinyinEngine.h"

/**
 * @name 相关底层数据的构造函数&析构函数.
 * @{
 */
EngineUnit::EngineUnit():inqphr(NULL), phrlist(NULL), priority(0)
{}
EngineUnit::~EngineUnit()
{
	delete inqphr;
	g_slist_foreach(phrlist, GFunc(g_free), NULL);
	g_slist_free(phrlist);
}
/** @} */

PinyinEngine::PinyinEngine()
{
}

PinyinEngine::~PinyinEngine()
{
}

void PinyinEngine::SetOutputCoding()
{
}

void PinyinEngine::InitSysEngineUnits(const char *sys)
{
}

void PinyinEngine::InitUserEngineUnit(const char *user)
{
}
