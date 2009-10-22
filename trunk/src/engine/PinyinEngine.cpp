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
#include "InquireSysPhrase.h"
#include "InquireUserPhrase.h"
#include "ParseString.h"
#include "../utils/output.h"
#include "../utils/support.h"

/**
 * @name 相关底层数据的构造函数&析构函数.
 * @{
 */
EngineUnit::EngineUnit():inqphr(NULL), phrlist(NULL), priority(0),
 type(SYSTEM_TYPE)
{}
EngineUnit::~EngineUnit()
{
	delete inqphr;
	for (GSList *tlist = phrlist; tlist; tlist = g_slist_next(tlist))
		g_free(tlist->data);
	g_slist_free(phrlist);
}
/** @} */

/**
 * 类构造函数.
 */
PinyinEngine::PinyinEngine():eulist(NULL), fztable(NULL), pagesize(5),
 userpath(NULL), bakpath(NULL)
{
	ParseString parse;
	int8_t count, sum;

	sum = parse.GetPinyinUnitSum();
	fztable = (int8_t *)g_malloc(sum);
	for (count = 0; count < sum; count++)
		*(fztable + count) = -1;
}

/**
 * 类析构函数.
 */
PinyinEngine::~PinyinEngine()
{
	for (GSList *tlist = eulist; tlist; tlist = g_slist_next(tlist))
		delete (EngineUnit *)tlist->data;
	g_slist_free(eulist);
	g_free(fztable);
	g_free(userpath);
	g_free(bakpath);
}

/**
 * 初始化系统引擎单元.
 * @param sys 系统码表配置文件
 */
void PinyinEngine::InitSysEngineUnits(const char *sys)
{
	FILE *stream;
	EngineUnit *eu;
	const char *file, *priority;
	char *lineptr, *dirname, *path;
	size_t n;

	/* 打开文件 */
	if (!(stream = fopen(sys, "r"))) {
		pwarning("Fopen file \"%s\" failed, %s", sys, strerror(errno));
		return;
	}

	/* 读取文件数据，分析，并创建新引擎添加到链表 */
	lineptr = NULL;
	n = 0;
	dirname = g_path_get_dirname(sys);
	while (getline(&lineptr, &n, stream) != -1) {
		if (!BreakMbfileString(lineptr, &file, &priority))
			continue;
		path = g_strdup_printf("%s/%s", dirname, file);
		eu = CreateEngineUnit(path, atoi(priority), SYSTEM_TYPE);
		g_free(path);
		eulist = g_slist_prepend(eulist, eu);
	}
	free(lineptr);
	g_free(dirname);

	/* 关闭文件 */
	fclose(stream);
}

/**
 * 初始化用户引擎单元.
 * @param user 用户码表文件
 */
void PinyinEngine::InitUserEngineUnit(const char *user)
{
	EngineUnit *eu;
	char *dirname;

	userpath = g_strdup(user);
	dirname = g_path_get_dirname(user);
	bakpath = g_strdup_printf("%s/%s", dirname, "bak.mb");
	g_free(dirname);
	pye_copy_file(bakpath, userpath);

	eu = CreateEngineUnit(bakpath, G_MAXINT, USER_TYPE);
	eulist = g_slist_prepend(eulist, eu);
}

/**
 * 添加模糊拼音单元.
 * @param unit1 拼音单元
 * @param unit2 拼音单元
 */
void PinyinEngine::AddFuzzyPinyinUnit(const char *unit1, const char *unit2)
{
	ParseString parse;
	int8_t uidx1, uidx2;

	uidx1 = parse.GetStringIndex(unit1);
	uidx2 = parse.GetStringIndex(unit2);
	*(fztable + uidx1) = uidx2;
	*(fztable + uidx2) = uidx1;
}

/**
 * 备份用户词语文件.
 */
void PinyinEngine::BakUserPhrase()
{
	EngineUnit *eu;
	GSList *tlist;

	/* 查找用户词语引擎 */
	tlist = eulist;
	while (tlist) {
		eu = (EngineUnit *)tlist->data;
		if (eu->type == USER_TYPE)
			break;
		tlist = g_slist_next(tlist);
	}

	/* 写出内存数据，并更新用户词语文件 */
	if (tlist) {
		((InquireUserPhrase *)eu->inqphr)->WritePhraseIndexTree();
		pye_copy_file(userpath, bakpath);
	}
}

/**
 * 分割码表文件信息串的各部分.
 * @param lineptr 源串
 * @retval file 码表文件
 * @retval priority 优先级
 * @return 串是否合法
 */
bool PinyinEngine::BreakMbfileString(char *lineptr, const char **file,
						 const char **priority)
{
	char *ptr;

	if (*(ptr = lineptr + strspn(lineptr, "\x20\t\r\n")) == '\0' || *ptr == '#')
		return false;
	*file = ptr;

	if (*(ptr += strcspn(ptr, "\x20\t\r\n")) == '\0')
		return false;
	*ptr = '\0';
	ptr++;
	if (*(ptr += strspn(ptr, "\x20\t\r\n")) == '\0' || *ptr == '#')
		return false;
	*priority = ptr;

	return true;
}

/**
 * 创建引擎单元.
 * @param mfile 码表文件
 * @param priority 优先级
 * @return 引擎单元
 */
EngineUnit *PinyinEngine::CreateEngineUnit(const char *mfile, int priority,
							 ENGINE_TYPE type)
{
	EngineUnit *eu;

	eu = new EngineUnit;
	eu->inqphr = new InquireSysPhrase;
	eu->inqphr->CreateIndexTree(mfile);
	eu->phrlist = NULL;
	eu->priority = priority;
	eu->type = type;

	return eu;
}
