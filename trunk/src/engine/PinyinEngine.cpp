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
		delete (PhraseIndex *)tlist->data;
	g_slist_free(phrlist);
}
/** @} */

/**
 * 类构造函数.
 */
PinyinEngine::PinyinEngine():eulist(NULL), crttable(NULL), fztable(NULL),
 pagesize(5), pytable(NULL), cursor(0), chidx(NULL), chlen(0), aclist(NULL),
 cclist(NULL), userpath(NULL), bakpath(NULL)
{
	ParseString parse;
	int8_t count, sum;

	/* 拼音矫正表 */
	crttable = g_ptr_array_new();
	/* 模糊拼音对照表 */
	sum = parse.GetPinyinUnitSum();
	fztable = (int8_t *)g_malloc(sum);
	for (count = 0; count < sum; count++)
		*(fztable + count) = -1;
	/* 待查询拼音表 */
	pytable = g_array_new(TRUE, FALSE, 1);
}

/**
 * 类析构函数.
 */
PinyinEngine::~PinyinEngine()
{
	/* 释放引擎链表 */
	for (GSList *tlist = eulist; tlist; tlist = g_slist_next(tlist))
		delete (EngineUnit *)tlist->data;
	g_slist_free(eulist);
	/* 释放拼音矫正表 */
	g_ptr_array_foreach(crttable, GFunc(g_free), NULL);
	g_ptr_array_free(crttable, TRUE);
	/* 释放模糊拼音对照表 */
	g_free(fztable);
	/* 释放待查询拼音表 */
	g_array_free(pytable, TRUE);
	/* 释放汉字索引数组 */
	delete [] chidx;
	/* 释放已接受词语链表 */
	for (GSList *tlist = aclist; tlist; tlist = g_slist_next(tlist))
		delete (PhraseData *)tlist->data;
	g_slist_free(aclist);
	/* 释放缓冲词语链表 */
	for (GSList *tlist = cclist; tlist; tlist = g_slist_next(tlist))
		delete (PhraseData *)tlist->data;
	g_slist_free(cclist);
	/* 释放码表路径 */
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
	GSList *tlist;
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
		for (tlist = eulist; tlist && ((EngineUnit *)tlist->data)->priority >
					 eu->priority; tlist = g_slist_next(tlist));
		eulist = g_slist_insert_before(eulist, tlist, eu);
	}
	g_free(dirname);
	free(lineptr);

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
}

/**
 * 增加拼音矫正对.
 * @param pinyin1 待矫正拼音串
 * @param pinyin2 矫正拼音串
 */
void PinyinEngine::AddCorrectPinyinPair(const char *pinyin1, const char *pinyin2)
{
	g_ptr_array_add(crttable, g_strdup(pinyin1));
	g_ptr_array_add(crttable, g_strdup(pinyin2));
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
	GSList *tlist;
	EngineUnit *eu;
	char *tmppath;

	/* 查找用户词语引擎 */
	tlist = eulist;
	while (tlist) {
		eu = (EngineUnit *)tlist->data;
		if (eu->type == USER_TYPE)
			break;
		tlist = g_slist_next(tlist);
	}
	if (!tlist)
		return;

	/* 写出内存数据，并更新用户词语文件(多绕圈可以避免掉电错误) */
	((InquireUserPhrase *)eu->inqphr)->WritePhraseIndexTree();
	tmppath = g_strdup_printf("%s~", userpath);
	unlink(tmppath);	//删除可能错误的文件
	pye_copy_file(tmppath, bakpath);
	rename(tmppath, userpath);
	g_free(tmppath);
}

/**
 * 移动当前光标点.
 * @param offset 偏移量
 * @return 引擎执行状况
 */
bool PinyinEngine::MoveCursorPoint(int offset)
{
	int tmp;

	tmp = cursor + offset;
	if (tmp > pytable->len)
		cursor = pytable->len;
	else if (tmp < 0)
		cursor = 0;
	else
		cursor = tmp;

	return true;
}

/**
 * 插入新的拼音索引字符.
 * @param ch 字符
 * @return 引擎执行状况
 */
bool PinyinEngine::InsertPinyinKey(const char ch)
{
	/* 将字符插入待查询拼音表 */
	pytable = g_array_insert_val(pytable, cursor, ch);
	cursor++;
	/* 清空必要缓冲数据 */
	ClearEngineUnitBuffer();
	ClearPinyinEngineOldBuffer();
	ClearPinyinEngineTempBuffer();
	/* 创建汉字索引数组 */
	CreateCharsIndex();
	/* 查询词语 */
	InquirePhraseIndex();

	return true;
}

/**
 * 向前删除拼音索引字符.
 * @return 引擎执行状况
 */
bool PinyinEngine::DeletePinyinKey()
{
	/* 移除字符 */
	if (cursor >= pytable->len)
		return false;
	pytable = g_array_remove_index(pytable, cursor);
	/* 清空必要缓冲数据 */
	ClearEngineUnitBuffer();
	ClearPinyinEngineOldBuffer();
	ClearPinyinEngineTempBuffer();
	/* 创建汉字索引数组 */
	CreateCharsIndex();
	/* 查询词语 */
	InquirePhraseIndex();

	return true;
}

/**
 * 向后删除拼音索引字符.
 * @return 引擎执行状况
 */
bool PinyinEngine::BackspacePinyinKey()
{
	/* 移除字符 */
	if (cursor == 0 || pytable->len == 0)
		return false;
	cursor--;
	pytable = g_array_remove_index(pytable, cursor);
	/* 清空必要缓冲数据 */
	ClearEngineUnitBuffer();
	ClearPinyinEngineOldBuffer();
	ClearPinyinEngineTempBuffer();
	/* 创建汉字索引数组 */
	CreateCharsIndex();
	/* 查询词语 */
	InquirePhraseIndex();

	return true;
}

/**
 * 取消最后一个被选中的词语.
 * @return 引擎执行状况
 */
bool PinyinEngine::RevokeSelectedPhrase()
{
	GSList *tlist;

	/* 移除最后被选中的词语 */
	if (!aclist)
		return false;
	tlist = g_slist_last(aclist);
	delete (PhraseData *)tlist->data;
	aclist = g_slist_delete_link(aclist, tlist);
	/* 清空必要缓冲数据 */
	ClearEngineUnitBuffer();
	ClearPinyinEngineTempBuffer();
	/* 查询词语 */
	InquirePhraseIndex();
}

/**
 * 获取提交数据.
 * @retval text 词语数据
 * @retval len 词语数据有效长度
 * @return 引擎执行状况
 */
bool PinyinEngine::GetCommitText(gunichar2 **text, glong *len)
{
	GSList *tlist;
	PhraseData *phrdt;
	glong length;

	/* 获取数据总长度 */
	length = 0;
	tlist = aclist;
	while (tlist) {
		length += ((PhraseData *)tlist->data)->dtlen;
		tlist = g_slist_next(tlist);
	}

	/* 拷贝数据 */
	*text = (gunichar2 *)g_malloc(sizeof(gunichar2) * length);
	*len = 0;
	tlist = aclist;
	while (tlist) {
		phrdt = (PhraseData *)tlist->data;
		memcpy(*text + *len, phrdt->data, sizeof(gunichar2) * phrdt->dtlen);
		*len += phrdt->dtlen;
		tlist = g_slist_next(tlist);
	}

	return true;
}

/**
 * 获取预编辑数据.
 * @retval text 词语数据
 * @retval len 词语数据有效长度
 * @return 引擎执行状况
 */
bool PinyinEngine::GetPreeditText(gunichar2 **text, glong *len)
{
	GSList *tlist, *lastlist;
	PhraseData *phrdt;
	gunichar2 *ptr;
	glong length;

	/* 获取数据总长度 */
	length = 0;
	tlist = aclist;
	while (tlist) {
		length += ((PhraseData *)tlist->data)->dtlen;
		tlist = g_slist_next(tlist);
	}
	if (cclist) {
		lastlist = g_slist_last(cclist);
		length += ((PhraseData *)lastlist->data)->dtlen + 1;
	}

	/* 拷贝数据 */
	*text = (gunichar2 *)g_malloc(sizeof(gunichar2) * length);
	*len = 0;
	tlist = aclist;
	while (tlist) {
		phrdt = (PhraseData *)tlist->data;
		memcpy(*text + *len, phrdt->data, sizeof(gunichar2) * phrdt->dtlen);
		*len += phrdt->dtlen;
		tlist = g_slist_next(tlist);
	}
	if (cclist) {
		if (*len != 0) {
			ptr = g_utf8_to_utf16("\x20", 1, NULL, NULL, NULL);
			memcpy(*text + *len, ptr, sizeof(gunichar2));
			(*len)++;
			g_free(ptr);
		}
		phrdt = (PhraseData *)lastlist->data;
		memcpy(*text + *len, phrdt->data, sizeof(gunichar2) * phrdt->dtlen);
		*len += phrdt->dtlen;
	}

	return true;
}

/**
 * 获取辅助数据.
 * @retval text 词语数据
 * @retval len 词语数据有效长度
 * @return 引擎执行状况
 */
bool PinyinEngine::GetAuxiliaryText(gunichar2 **text, glong *len)
{
	ParseString parse;
	GSList *tlist;
	PhraseData *phrdt;
	int offset;
	glong length;
	size_t size;
	char *pinyin;
	gunichar2 *ptr;

	/* 获取数据总长度 */
	length = 0;
	offset = 0;
	tlist = aclist;
	while (tlist) {
		phrdt = (PhraseData *)tlist->data;
		offset += phrdt->chlen;
		length += phrdt->dtlen;
		tlist = g_slist_next(tlist);
	}
	if (offset != chlen) {
		pinyin = parse.RestorePinyinString(chidx + offset, chlen - offset);
		size = strlen(pinyin);
		length += size + 1;
	}

	/* 拷贝数据 */
	*text = (gunichar2 *)g_malloc(sizeof(gunichar2) * length);
	*len = 0;
	tlist = aclist;
	while (tlist) {
		phrdt = (PhraseData *)tlist->data;
		memcpy(*text + *len, phrdt->data, sizeof(gunichar2) * phrdt->dtlen);
		*len += phrdt->dtlen;
		tlist = g_slist_next(tlist);
	}
	if (offset != chlen) {
		if (*len != 0) {
			ptr = g_utf8_to_utf16("\x20", 1, NULL, NULL, NULL);
			memcpy(*text + *len, ptr, sizeof(gunichar2));
			(*len)++;
			g_free(ptr);
		}
		ptr = g_utf8_to_utf16(pinyin, size, NULL, NULL, NULL);
		memcpy(*text + *len, ptr, sizeof(gunichar2) * size);
		*len += size;
		g_free(ptr);
		g_free(pinyin);	//幸好还没忘记
	}

	return true;
}

/**
 * 获取一个页面的词语数据.
 * @retval list 词语链表
 * @retval len 词语链表长度
 * @return 引擎执行状况
 */
bool PinyinEngine::GetPagePhrase(GSList **list, guint *len)
{
	EngineUnit *eu;
	PhraseIndex *phridx;
	PhraseData *phrdt;
	uint8_t count;

	/* 初始化参数 */
	*list = NULL;
	*len = 0;

	/* 提取一页的词语数据 */
	count = 0;
	while (count < pagesize) {
		if (!(eu = SearchPreferPhrase()))
			break;
		phridx = (PhraseIndex *)eu->phrlist->data;
		eu->phrlist = g_slist_delete_link(eu->phrlist, eu->phrlist);
		phrdt = eu->inqphr->AnalysisPhraseIndex(phridx);
		delete phridx;
		if (!IsExistCachePhrase(phrdt)) {
			*list = g_slist_append(*list, phrdt);
			cclist = g_slist_prepend(cclist, phrdt);	//减少时间开支
			(*len)++;
			count++;
		} else
			delete phrdt;
	}

	return (count != 0);
}

/**
 * 考察此词语是否已经存在词语数据缓冲区中.
 * @param phrdt 词语数据
 * @return 是否已经存在
 */
bool PinyinEngine::IsExistCachePhrase(const PhraseData *phrdt)
{
	GSList *tlist;
	PhraseData *tphrdt;

	tlist = cclist;
	while (tlist) {
		tphrdt = (PhraseData *)tlist->data;
		if (tphrdt->dtlen == phrdt->dtlen
			 && memcmp(tphrdt->data, phrdt->data,
				 sizeof(gunichar2) * tphrdt->dtlen) == 0)
			break;
		tlist = g_slist_next(tlist);
	}

	return tlist;
}

/**
 * 选定词语数据缓冲区中的词语数据.
 * @param phrdt 词语数据
 * @return 引擎执行状况
 */
bool PinyinEngine::SelectCachePhrase(const PhraseData *phrdt)
{
	GSList *tlist;

	/* 将词语数据加入已接受词语链表 */
	aclist = g_slist_append(aclist, (gpointer)phrdt);
	/* 清空缓冲数据 */
	if ( (tlist = g_slist_find(cclist, phrdt)))
		tlist->data = NULL;
	ClearEngineUnitBuffer();
	ClearPinyinEngineTempBuffer();
	/* 如果需要则继续查询词语 */
	if (!IsFinishInquirePhrase())
		InquirePhraseIndex();

	return true;
}

/**
 * 词语查询工作已经已经完成.
 * @return 是否完成
 */
bool PinyinEngine::IsFinishInquirePhrase()
{
	return (ComputeInquireOffset() == chlen);
}

/**
 * 向用户码表引擎反馈被用户选中的词语.
 * @return 引擎执行状况
 */
bool PinyinEngine::FeedbackSelectedPhrase()
{
	GSList *tlist;
	EngineUnit *eu;
	PhraseData *phrdt;

	/* 查询用户码表引擎单元 */
	tlist = eulist;
	while (tlist) {
		eu = (EngineUnit *)tlist->data;
		if (eu->type == USER_TYPE)
			break;
		tlist = g_slist_next(tlist);
	}
	if (!tlist)
		return false;

	/* 反馈词语数据 */
	switch (g_slist_length(aclist)) {
	case 0:	//没有数据,无需处理
		break;
	case 1:	//现成的词语,直接处理即可
		phrdt = (PhraseData *)aclist->data;
		if (phrdt->offset == (off_t)(-1))
			((InquireUserPhrase *)eu->inqphr)->InsertPhraseToTree(phrdt);
		else
			((InquireUserPhrase *)eu->inqphr)->IncreasePhraseFreq(phrdt);
		break;
	default:	//需要先合并词语数据
		phrdt = CreateUserPhrase();
		((InquireUserPhrase *)eu->inqphr)->InsertPhraseToTree(phrdt);
		delete [] (CharsIndex *)phrdt->chidx;	//必须单独释放
		delete phrdt;
		break;
	}
}

/**
 * 通告拼音引擎词语查询已经完成.
 * @return 引擎执行状况
 */
bool PinyinEngine::FinishInquirePhrase()
{
	ClearEngineUnitBuffer();
	ClearPinyinEngineBuffer();

	return true;
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
	if (!isdigit(*ptr))
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
	eu->phrlist = NULL;
	eu->priority = priority;
	eu->type = type;

	return eu;
}

/**
 * 创建用户词语.
 * @return 词语数据
 */
PhraseData *PinyinEngine::CreateUserPhrase()
{
	GSList *tlist;
	PhraseData *phrdt, *tphrdt;
	glong dtlen;
	int chlen;

	/* 计算需要的内存长度 */
	chlen = 0;
	dtlen = 0;
	tlist = aclist;
	while (tlist) {
		tphrdt = (PhraseData *)tlist->data;
		chlen += tphrdt->chlen;
		dtlen += tphrdt->dtlen;
		tlist = g_slist_next(tlist);
	}

	/* 创建新的词语数据 */
	phrdt = new PhraseData;
	phrdt->chidx = new CharsIndex[chlen];
	phrdt->chlen = 0;
	phrdt->offset = (off_t)(-1);
	phrdt->data = (gunichar2 *)g_malloc(sizeof(gunichar2) * dtlen);
	phrdt->dtlen = 0;
	tlist = aclist;
	while (tlist) {
		tphrdt = (PhraseData *)tlist->data;
		memcpy((CharsIndex *)phrdt->chidx + phrdt->chlen, tphrdt->chidx,
					 sizeof(CharsIndex) * tphrdt->chlen);
		phrdt->chlen += tphrdt->chlen;
		memcpy(phrdt->data + phrdt->dtlen, tphrdt->data,
				 sizeof(gunichar2) * tphrdt->dtlen);
		phrdt->dtlen += tphrdt->dtlen;
		tlist = g_slist_next(tlist);
	}

	return phrdt;
}

/**
 * 创建汉字索引数组.
 */
void PinyinEngine::CreateCharsIndex()
{
	ParseString parse;
	char *pinyin;

	pinyin = CorrectPinyinString();
	parse.ParsePinyinString(pinyin, &chidx, &chlen);
	g_free(pinyin);	//已经没有用处了
}

/**
 * 查询词语索引.
 */
void PinyinEngine::InquirePhraseIndex()
{
	GSList *tlist;
	EngineUnit *eu;
	int offset;

	offset = ComputeInquireOffset();
	tlist = eulist;
	while (tlist) {
		eu = (EngineUnit *)tlist->data;
		eu->phrlist = eu->inqphr->SearchMatchPhrase(chidx + offset,
							 chlen - offset);
		tlist = g_slist_next(tlist);
	}
}

/**
 * 纠正待查询拼音串中可能存在的错误.
 * @return 新拼音串
 */
char *PinyinEngine::CorrectPinyinString()
{
	GArray *cpytable;
	guint length, count;
	const char *ptr;
	char *pinyin;

	cpytable = g_array_sized_new(TRUE, FALSE, 1, pytable->len << 1);
	length = 0;
	while (length < pytable->len) {
		count = 0;
		while (count < crttable->len) {
			ptr = *((const char **)crttable->pdata + count);
			if (memcmp(ptr, pytable->data + length, strlen(ptr)) == 0)
				break;
			count += 2;
		}
		if (count < crttable->len) {
			length += strlen(ptr);
			ptr = *((const char **)crttable->pdata + count + 1);
			cpytable = g_array_append_vals(cpytable, ptr, strlen(ptr));
		} else {
			cpytable = g_array_append_vals(cpytable,
					 pytable->data + length, 1);
			length++;
		}
	}
	pinyin = g_array_free(cpytable, FALSE);

	return pinyin;
}

/**
 * 计算当前待查询汉字索引数组的偏移量.
 * @return 偏移量
 */
int PinyinEngine::ComputeInquireOffset()
{
	GSList *tlist;
	PhraseData *phrdt;
	int offset;

	offset = 0;
	tlist = aclist;
	while (tlist) {
		phrdt = (PhraseData *)tlist->data;
		offset += phrdt->chlen;
		tlist = g_slist_next(tlist);
	}

	return offset;
}

/**
 * 搜索各个引擎单元所提供的最佳词语.
 * @return 引擎单元
 */
EngineUnit *PinyinEngine::SearchPreferPhrase()
{
	GSList *tlist;
	EngineUnit *eu, *eunit;
	PhraseIndex *phridx;
	int chlen;

	/* 初始化数据 */
	eunit = NULL;
	chlen = 0;

	/* 查找 */
	tlist = eulist;
	while (tlist) {
		eu = (EngineUnit *)tlist->data;
		if (eu->phrlist) {
			phridx = (PhraseIndex *)eu->phrlist->data;
			if (phridx->chlen > chlen) {
				eunit = eu;
				chlen = phridx->chlen;
			}
		}
		tlist = g_slist_next(tlist);
	}

	return eunit;
}

/**
 * 清空引擎单元中的缓冲数据.
 */
void PinyinEngine::ClearEngineUnitBuffer()
{
	EngineUnit *eu;
	GSList *tlist, *phrlist;

	tlist = eulist;
	while (tlist) {
		eu = (EngineUnit *)tlist->data;
		phrlist = eu->phrlist;
		while (phrlist) {
			delete (PhraseIndex *)phrlist->data;
			phrlist = g_slist_next(phrlist);
		}
		g_slist_free(phrlist);
		eu->phrlist = NULL;
		tlist = g_slist_next(tlist);
	}
}

/**
 * 清空拼音引擎自身的缓冲数据.
 */
void PinyinEngine::ClearPinyinEngineBuffer()
{
	/* 清空待查询拼音表数据 */
	pytable = g_array_remove_range(pytable, 0, pytable->len);
	cursor = 0;
	/* 释放汉字索引数组 */
	delete [] chidx;
	chidx = NULL;
	chlen = 0;
	/* 释放已接受词语链表 */
	for (GSList *tlist = aclist; tlist; tlist = g_slist_next(tlist))
		delete (PhraseData *)tlist->data;
	g_slist_free(aclist);
	aclist = NULL;
	/* 释放缓冲词语链表 */
	for (GSList *tlist = cclist; tlist; tlist = g_slist_next(tlist))
		delete (PhraseData *)tlist->data;
	g_slist_free(cclist);
	cclist = NULL;
}

/**
 * 清空拼音引擎自身已经过时的缓冲数据.
 */
void PinyinEngine::ClearPinyinEngineOldBuffer()
{
	/* 释放已接受词语链表 */
	for (GSList *tlist = aclist; tlist; tlist = g_slist_next(tlist))
		delete (PhraseData *)tlist->data;
	g_slist_free(aclist);
	aclist = NULL;
}

/**
 * 清空拼音引擎自身的临时缓冲数据.
 */
void PinyinEngine::ClearPinyinEngineTempBuffer()
{
	/* 释放缓冲词语链表 */
	for (GSList *tlist = cclist; tlist; tlist = g_slist_next(tlist))
		delete (PhraseData *)tlist->data;
	g_slist_free(cclist);
	cclist = NULL;
}
