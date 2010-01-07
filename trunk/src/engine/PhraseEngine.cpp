//
// C++ Implementation: PhraseEngine
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "PhraseEngine.h"
#include "ParseString.h"
#include "InquireSysPhrase.h"
#include "InquireUserPhrase.h"
#include "../utils/output.h"
#include "../utils/support.h"

/**
 * @name 相关底层数据的构造函数&析构函数.
 * @{
 */
EngineUnit::EngineUnit():inqphr(NULL), priority(0), type(SYSTEM_TYPE)
{}
EngineUnit::~EngineUnit()
{
	delete inqphr;
}
EunitPhrase::EunitPhrase():eunit(NULL), phrlist(NULL)
{}
EunitPhrase::~EunitPhrase()
{
	for (GSList *tlist = phrlist; tlist; tlist = g_slist_next(tlist))
		delete (PhraseIndex *)tlist->data;
	g_slist_free(phrlist);
}
/** @} */

/**
 * 类构造函数.
 */
PhraseEngine::PhraseEngine():eulist(NULL), rtftable(NULL), fztable(NULL),
 userpath(NULL), bakpath(NULL), timestamp(0)
{
	ParseString parse;
	int8_t count, sum;

	/* 拼音矫正表 */
	rtftable = g_array_new(FALSE, FALSE, sizeof(RectifyUnit));
	/* 模糊拼音对照表 */
	sum = parse.GetPinyinUnitSum();
	fztable = (int8_t *)g_malloc(sum);
	for (count = 0; count < sum; count++)
		*(fztable + count) = -1;
}

/**
 * 类析构函数.
 */
PhraseEngine::~PhraseEngine()
{
	guint count;
	RectifyUnit *rtfunit;

	/* 备份用户词语数据 */
	BakUserEnginePhrase();
	/* 释放引擎链表 */
	for (GSList *tlist = eulist; tlist; tlist = g_slist_next(tlist))
		delete (EngineUnit *)tlist->data;
	g_slist_free(eulist);
	/* 释放拼音矫正表 */
	count = 0;
	while (count < rtftable->len) {
		rtfunit = &g_array_index(rtftable, RectifyUnit, count);
		if (!rtfunit->isstatic) {
			g_free((gpointer)rtfunit->fstring);
			g_free((gpointer)rtfunit->tstring);
		}
		count++;
	}
	g_array_free(rtftable, TRUE);
	/* 释放模糊拼音对照表 */
	g_free(fztable);
	/* 释放码表路径 */
	g_free(userpath);
	unlink(bakpath);	//移除备份文件
	g_free(bakpath);
}

/**
 * 获取实例对象指针.
 * @return 对象指针
 */
std::auto_ptr<PhraseEngine> PhraseEngine::instance;
PhraseEngine *PhraseEngine::GetInstance()
{
	if (!instance.get())
		instance.reset(new PhraseEngine);
	return instance.get();
}

/**
 * 释放实例对象.
 */
void PhraseEngine::FreeInstance()
{
	instance.reset(NULL);
}

/**
 * 创建系统词语引擎单元.
 * @param sys 系统码表配置文件
 */
void PhraseEngine::CreateSysEngineUnits(const char *sys)
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
	g_free(dirname);
	free(lineptr);

	/* 关闭文件 */
	fclose(stream);
	/* 更新时间戳 */
	time(&timestamp);
}

/**
 * 创建用户词语引擎单元.
 * @param user 用户码表文件
 */
void PhraseEngine::CreateUserEngineUnit(const char *user)
{
	EngineUnit *eu;
	char *dirname;

	/* 如果用户词语引擎已经存在，则直接退出即可 */
	if (userpath)
		return;

	/* 创建运行环境 */
	userpath = g_strdup(user);
	dirname = g_path_get_dirname(user);
	bakpath = g_strdup_printf("%s/%s", dirname, "bak.mb");
	g_free(dirname);
	unlink(bakpath);	//删除可能错误的文件
	pye_copy_file(bakpath, userpath);

	/* 创建引擎 */
	eu = CreateEngineUnit(bakpath, G_MAXINT, USER_TYPE);
	eulist = g_slist_prepend(eulist, eu);
	/* 更新时间戳 */
	time(&timestamp);
}

/**
 * 添加拼音矫正对.
 * @param pinyin1 待矫正拼音串
 * @param pinyin2 矫正拼音串
 */
void PhraseEngine::AddRectifyPinyinPair(const char *pinyin1, const char *pinyin2)
{
	RectifyUnit rtfunit;

	rtfunit.fstring = g_strdup(pinyin1);
	rtfunit.tstring = g_strdup(pinyin2);
	rtfunit.isstatic = false;
	g_array_append_val(rtftable, rtfunit);
}

/**
 * 添加模糊拼音单元.
 * @param unit1 拼音单元
 * @param unit2 拼音单元
 */
void PhraseEngine::AddFuzzyPinyinUnit(const char *unit1, const char *unit2)
{
	ParseString parse;
	int8_t uidx1, uidx2;

	if ((uidx1 = parse.GetStringIndex(unit1)) == -1
		 || (uidx2 = parse.GetStringIndex(unit2)) == -1)
		return;
	*(fztable + uidx1) = uidx2;
	*(fztable + uidx2) = uidx1;
}

/**
 * 清空拼音矫正对.
 */
void PhraseEngine::ClearRectifyPinyinPair()
{
	guint count;
	RectifyUnit *rtfunit;

	count = 0;
	while (count < rtftable->len) {
		rtfunit = &g_array_index(rtftable, RectifyUnit, count);
		if (!rtfunit->isstatic) {
			g_free((gpointer)rtfunit->fstring);
			g_free((gpointer)rtfunit->tstring);
		}
		count++;
	}
	rtftable = g_array_set_size(rtftable, 0);
}

/**
 * 清空模糊拼音单元.
 */
void PhraseEngine::ClearFuzzyPinyinUnit()
{
	ParseString parse;
	int8_t count, sum;

	sum = parse.GetPinyinUnitSum();
	for (count = 0; count < sum; count++)
		*(fztable + count) = -1;
}

/**
 * 备份用户词语引擎数据文件.
 */
void PhraseEngine::BakUserEnginePhrase()
{
	GSList *tlist;
	EngineUnit *eu;
	char *tmppath;

	/* 查询用户词语引擎单元 */
	tlist = eulist;
	while (tlist) {
		eu = (EngineUnit *)tlist->data;
		if (eu->type == USER_TYPE)
			break;
		tlist = g_slist_next(tlist);
	}
	if (!tlist)
		return;

	/* 写出内存数据，并更新用户词语文件(多绕圈可避免掉电错误) */
	((InquireUserPhrase *)eu->inqphr)->WritePhraseIndexTree();
	tmppath = g_strdup_printf("%s~", userpath);
	unlink(tmppath);	//删除可能错误的文件
	pye_copy_file(tmppath, bakpath);
	rename(tmppath, userpath);
	g_free(tmppath);
}

/**
 * 同步引擎单元中的词语索引缓冲数据.
 * @param euphrlist 引擎单元词语索引缓冲点链表
 */
void PhraseEngine::SyncEngineUnitData(GSList **euphrlist, time_t stamp) const
{
	GSList *tlist, *plist;
	EunitPhrase *euphr;

	/* 如果数据是最新的，则无须处理 */
	if (stamp > timestamp)
		return;

	/* 更新数据，即删除引擎已不存在的关联数据 */
	tlist = *euphrlist;
	while (tlist) {
		euphr = (EunitPhrase *)tlist->data;
		plist = tlist;
		tlist = g_slist_next(tlist);
		if (!g_slist_find(eulist, euphr->eunit)) {
			delete euphr;
			*euphrlist = g_slist_delete_link(*euphrlist, plist);
		}
	}
}

/**
 * 删除词语数据.
 * @param phrdt 词语数据
 */
void PhraseEngine::DeletePhraseData(const PhraseData *phrdt) const
{
	GSList *tlist;
	EngineUnit *eu;

	/* 如果此词汇非用户词汇则直接退出即可 */
	if (phrdt->offset <= 0)
		return;

	/* 查询用户词语引擎单元 */
	tlist = eulist;
	while (tlist) {
		eu = (EngineUnit *)tlist->data;
		if (eu->type == USER_TYPE)
			break;
		tlist = g_slist_next(tlist);
	}
	if (!tlist)
		return;

	/* 删除词语 */
	((InquireUserPhrase *)eu->inqphr)->DeletePhraseFromTree(phrdt);
}

/**
 * 反馈词语数据.
 * @param phrdt 词语数据
 */
void PhraseEngine::FeedbackPhraseData(const PhraseData *phrdt) const
{
	GSList *tlist;
	EngineUnit *eu;

	/* 如果此词汇无效则直接退出即可 */
	if (phrdt->offset == 0)
		return;

	/* 查询用户词语引擎单元 */
	tlist = eulist;
	while (tlist) {
		eu = (EngineUnit *)tlist->data;
		if (eu->type == USER_TYPE)
			break;
		tlist = g_slist_next(tlist);
	}
	if (!tlist)
		return;

	/* 将词语数据反馈到用户词语引擎中 */
	if (phrdt->offset < 0)
		((InquireUserPhrase *)eu->inqphr)->InsertPhraseToTree(phrdt);
	else
		((InquireUserPhrase *)eu->inqphr)->IncreasePhraseFreq(phrdt);
}

/**
 * 查询匹配的词语索引.
 * @param chidx 汉字索引数组
 * @param chlen 汉字索引数组有效长度
 * @return 引擎单元词语索引缓冲点链表
 */
GSList *PhraseEngine::InquireMatchPhraseIndex(const CharsIndex *chidx, int chlen) const
{
	GSList *tlist, *euphrlist;
	EunitPhrase *euphr;
	EngineUnit *eu;

	euphrlist = NULL;
	tlist = eulist;
	while (tlist) {
		eu = (EngineUnit *)tlist->data;
		euphr = new EunitPhrase;
		euphr->eunit = eu;
		euphr->phrlist = eu->inqphr->SearchMatchPhrase(chidx, chlen);
		euphrlist = g_slist_prepend(euphrlist, euphr);
		tlist = g_slist_next(tlist);
	}

	return euphrlist;
}

/**
 * 查询首选的词语索引.
 * @param chidx 汉字索引数组
 * @param chlen 汉字索引数组有效长度
 * @return 词语索引
 */
EunitPhrase *PhraseEngine::InquirePerferPhraseIndex(const CharsIndex *chidx,
							 int chlen) const
{
	GSList *tlist;
	EunitPhrase *euphr;
	EngineUnit *eu, *teu;
	PhraseIndex *phridx, *tphridx;

	/* 初始化 */
	euphr = NULL;
	eu = NULL;
	phridx = NULL;

	/* 查询最佳的词语索引 */
	tlist = eulist;
	while (tlist) {
		teu = (EngineUnit *)tlist->data;
		tphridx = teu->inqphr->SearchPreferPhrase(chidx, chlen);
		if (phridx && tphridx) {
			if (phridx->chlen < tphridx->chlen
				 || (phridx->chlen == tphridx->chlen
					 && eu->priority < teu->priority)) {
				delete phridx;
				phridx = tphridx;
				eu = teu;
			}
		} else if (!phridx && tphridx) {
			phridx = tphridx;
			eu = teu;
		}
		tlist = g_slist_next(tlist);
	}

	/* 准备返回数据 */
	if (eu && phridx) {
		euphr = new EunitPhrase;
		euphr->eunit = eu;
		euphr->phrlist = g_slist_prepend(euphr->phrlist, phridx);
	}

	return euphr;
}

/**
 * 获取拼音矫正表.
 * @return 拼音矫正表
 */
GArray *PhraseEngine::GetRectifyTable() const
{
	return rtftable;
}

/**
 * 分割码表文件信息串的各部分.
 * @param lineptr 源串
 * @retval file 码表文件
 * @retval priority 优先级
 * @return 串是否合法
 */
bool PhraseEngine::BreakMbfileString(char *lineptr, const char **file,
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
	if (*(ptr += strspn(ptr, "\x20\t\r\n")) == '\0' || !isdigit(*ptr))
		return false;
	*priority = ptr;

	return true;
}

/**
 * 创建引擎单元.
 * @param mfile 码表文件
 * @param priority 优先级
 * @param type 引擎单元类型
 * @return 引擎单元
 */
EngineUnit *PhraseEngine::CreateEngineUnit(const char *mfile, int priority,
							 EUNIT_TYPE type)
{
	EngineUnit *eu;

	eu = new EngineUnit;
	switch (type) {
	case SYSTEM_TYPE:
		eu->inqphr = new InquireSysPhrase;
		break;
	case USER_TYPE:
		eu->inqphr = new InquireUserPhrase;
		break;
	default:
		pwarning("Fatal Error!\nEngine types can not be identified!\n");
		eu->inqphr = NULL;
		break;
	}
	eu->inqphr->CreateIndexTree(mfile);
	eu->inqphr->SetFuzzyPinyinUnits(fztable);
	eu->priority = priority;
	eu->type = type;

	return eu;
}
