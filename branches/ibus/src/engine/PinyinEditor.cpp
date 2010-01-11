//
// C++ Implementation: PinyinEditor
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "PinyinEditor.h"
#include "DynamicPhrase.h"
#include "ParseString.h"

/**
 * 内置拼音修正表.
 * 这张表能够起到什么作用？ \n
 * e.g.:
 * geren-->g'er'en，我相信你一定不希望得到这个结果吧！ \n
 * 让我们调用这张表先对它做一次预处理！ \n
 * geren-->'ge'ren-->'g'e'r'en，这个结果应该可以接受了！ \n
 * \n
 * 已知bug: \n
 * zhongou-->zh'ong'ou，中欧(此为正确分解) \n
 * zhongou-->zhon'gou-->zh'g'ou，???(本程序处理结果) \n
 */
RectifyUnit PinyinEditor::irtytable[] = {
	{"ga", "\'ga", true},
	{"ge", "\'ge", true},
	{"gou", "\'gou", true},
	{"gong", "\'gong", true},
	{"gu", "\'gu", true},
	{"na", "\'na", true},
	{"ne", "\'ne", true},
	{"ni", "\'ni", true},
	{"nou", "\'nou", true},
	{"nong", "\'nong", true},
	{"nu", "\'nu", true},
	{"nv", "\'nv", true},
	{"ran", "\'ran", true},
	{"rao", "\'rao", true},
	{"re", "\'re", true},
	{"ri", "\'ri", true},
	{"rou", "\'rou", true},
	{"rong", "\'rong", true},
	{"ru", "\'ru", true},
	{"aou", "a\'ou", true},
	{"uou", "u\'ou", true},
	{"iai", "i\'ai", true},
	{"iei", "i\'ei", true},
	{"uei", "u\'ei", true},
	{"vei", "v\'ei", true},
	{NULL, NULL, true}
};

/**
 * 类构造函数.
 */
PinyinEditor::PinyinEditor(const PhraseEngine *engine):editmode(true),
 pytable(NULL), cursor(0), chidx(NULL), chlen(0), aclist(NULL),
 cclist(NULL), phregn(engine), euphrlist(NULL), timestamp(0)
{
	/* 待查询拼音表 */
	pytable = g_array_new(TRUE, FALSE, 1);
}

/**
 * 类析构函数.
 */
PinyinEditor::~PinyinEditor()
{
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
	/* 释放引擎词语索引缓冲点链表 */
	for (GSList *tlist = euphrlist; tlist; tlist = g_slist_next(tlist))
		delete (EunitPhrase *)tlist->data;
	g_slist_free(euphrlist);
}

/**
 * 拼音编辑器是否为空.
 * @return 是否为空
 */
bool PinyinEditor::IsEmpty()
{
	return (pytable->len == 0);
}

/**
 * 同步数据.
 * @return 执行状况
 */
bool PinyinEditor::SyncData()
{
	phregn->SyncEngineUnitData(&euphrlist, timestamp);
	time(&timestamp);	//更新时间戳

	return true;
}

/**
 * 设置编辑器的当前模式.
 * 主要用于避免在英文输入模式下，编辑器依然会查询词语。 \n
 * @param zh 模式;true 中文,false 英文
 * @return 执行状况
 */
bool PinyinEditor::SetEditorMode(bool zh)
{
	if (pytable->len > 0)
		return false;
	editmode = zh;
	return true;
}

/**
 * 移动当前光标点.
 * @param offset 偏移量
 * @return 执行状况
 */
bool PinyinEditor::MoveCursorPoint(int offset)
{
	int tmp;

	tmp = cursor + offset;
	if (tmp < 0)
		cursor = 0;
	else if ((guint)tmp > pytable->len)
		cursor = pytable->len;
	else
		cursor = tmp;

	return true;
}

/**
 * 插入新的拼音索引字符.
 * @param ch 字符
 * @return 执行状况
 */
bool PinyinEditor::InsertPinyinKey(char ch)
{
	/* 将字符插入待查询拼音表 */
	pytable = g_array_insert_val(pytable, cursor, ch);
	cursor++;
	/* 清空必要缓冲数据 */
	ClearEngineUnitBuffer();
	ClearPinyinEditorIdxBuffer();
	ClearPinyinEditorOldBuffer();
	ClearPinyinEditorTempBuffer();
	/* 创建汉字索引数组 */
	CreateCharsIndex();
	/* 查询词语索引 */
	InquirePhraseIndex();

	return true;
}

/**
 * 向前删除拼音索引字符.
 * @return 执行状况
 */
bool PinyinEditor::DeletePinyinKey()
{
	/* 移除字符 */
	if (cursor >= pytable->len)
		return false;
	pytable = g_array_remove_index(pytable, cursor);
	/* 清空必要缓冲数据 */
	ClearEngineUnitBuffer();
	ClearPinyinEditorIdxBuffer();
	ClearPinyinEditorOldBuffer();
	ClearPinyinEditorTempBuffer();
	/* 创建汉字索引数组 */
	CreateCharsIndex();
	/* 查询词语索引 */
	InquirePhraseIndex();

	return true;
}

/**
 * 向后删除拼音索引字符.
 * @return 执行状况
 */
bool PinyinEditor::BackspacePinyinKey()
{
	/* 移除字符 */
	if (cursor == 0 || pytable->len == 0)
		return false;
	cursor--;
	pytable = g_array_remove_index(pytable, cursor);
	/* 清空必要缓冲数据 */
	ClearEngineUnitBuffer();
	ClearPinyinEditorIdxBuffer();
	ClearPinyinEditorOldBuffer();
	ClearPinyinEditorTempBuffer();
	/* 创建汉字索引数组 */
	CreateCharsIndex();
	/* 查询词语索引 */
	InquirePhraseIndex();

	return true;
}

/**
 * 取消最后一个被选中的词语.
 * @return 执行状况
 */
bool PinyinEditor::RevokeSelectedPhrase()
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
	ClearPinyinEditorTempBuffer();
	/* 查询词语索引 */
	InquirePhraseIndex();

	return true;
}

/**
 * 获取原始串.
 * @retval text 原始串
 * @retval len 原始串长度
 * @return 执行状况
 */
bool PinyinEditor::GetRawText(char **text, guint *len)
{
	if (pytable->len != 0) {
		*text = (char *)g_memdup(pytable->data, pytable->len + 1);
		*len = pytable->len;
	} else {
		*text = NULL;
		*len = 0;
	}

	return true;
}

/**
 * 获取提交数据.
 * @retval text 词语数据
 * @retval len 词语数据有效长度
 * @return 执行状况
 */
bool PinyinEditor::GetCommitText(gunichar2 **text, glong *len)
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
 * @return 执行状况
 */
bool PinyinEditor::GetPreeditText(gunichar2 **text, glong *len)
{
	GSList *tlist, *lastlist;
	PhraseData *phrdt;
	gunichar2 *ptr;
	glong length;

	/* 获取数据总长度 */
	length = 0;
	lastlist = NULL;
	tlist = aclist;
	while (tlist) {
		length += ((PhraseData *)tlist->data)->dtlen;
		tlist = g_slist_next(tlist);
	}
	if (cclist) {
		lastlist = g_slist_last(cclist);	//请注意(cclist)数据格式
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
 * @return 执行状况
 */
bool PinyinEditor::GetAuxiliaryText(gunichar2 **text, glong *len)
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
	size = 0;
	pinyin = NULL;
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
 * @retval len 页面大小/词语链表长度
 * @return 执行状况
 */
bool PinyinEditor::GetPagePhrase(GSList **list, guint *len)
{
	EunitPhrase *euphr;
	PhraseIndex *phridx;
	PhraseData *phrdt;
	guint pagesize;

	/* 初始化参数 */
	pagesize = *len;
	*list = NULL;
	*len = 0;

	/* 提取一页的词语数据 */
	while (*len < pagesize) {
		if (!(euphr = SearchPreferEunitPhrase()))
			break;
		phridx = (PhraseIndex *)euphr->phrlist->data;
		euphr->phrlist = g_slist_delete_link(euphr->phrlist, euphr->phrlist);
		phrdt = euphr->eunit->inqphr->AnalysisPhraseIndex(phridx);
		delete phridx;
		if (!IsExistCachePhrase(phrdt)) {
			*list = g_slist_append(*list, phrdt);
			cclist = g_slist_prepend(cclist, phrdt);
			(*len)++;
		} else
			delete phrdt;
	}

	return (*list);
}

/**
 * 获取动态词语.
 * @retval list 词语链表
 * @retval len 词语链表长度
 * @return 执行状况
 */
bool PinyinEditor::GetDynamicPhrase(GSList **list, guint *len)
{
	DynamicPhrase dyphr;
	GSList *tlist;

	/* 初始化参数 */
	*list = NULL;
	*len = 0;

	/* 提取动态词语并放入缓冲链表 */
	*list = dyphr.GetDynamicPhrase(pytable->data, len);
	tlist = *list;
	while (tlist) {
		cclist = g_slist_prepend(cclist, tlist->data);
		tlist = g_slist_next(tlist);
	}

	return (*list);
}

/**
 * 获取引擎合成词语.
 * @retval phrdt 词语数据
 * @return 执行状况
 */
bool PinyinEditor::GetComposePhrase(PhraseData **phrdt)
{
	GSList *tlist, *phrdtlist;
	EunitPhrase *euphr;
	PhraseIndex *phridx;
	PhraseData *tphrdt;
	int offset;
	glong length;

	/* 初始化 */
	*phrdt = NULL;

	/* 获取合成词语的各部分词语数据 */
	phrdtlist = NULL;
	/*/* 第一个词语数据 */
	tphrdt = NULL;
	if ( (tlist = g_slist_last(cclist))) {
		tphrdt = (PhraseData *)tlist->data;
	} else if ( (euphr = SearchPreferEunitPhrase())) {
		phridx = (PhraseIndex *)euphr->phrlist->data;
		tphrdt = euphr->eunit->inqphr->AnalysisPhraseIndex(phridx);
	}
	if (!tphrdt)
		return false;
	phrdtlist = g_slist_append(phrdtlist, tphrdt);
	offset = ComputeInquireOffset() + tphrdt->chlen;
	/*/* 其余词语数据 */
	while (offset != chlen) {
		if (!(euphr = phregn->InquirePerferPhraseIndex(chidx + offset,
							 chlen - offset)))
			break;
		phridx = (PhraseIndex *)euphr->phrlist->data;
		tphrdt = euphr->eunit->inqphr->AnalysisPhraseIndex(phridx);
		phrdtlist = g_slist_append(phrdtlist, tphrdt);
		offset += tphrdt->chlen;
		delete euphr;
	}
	if (g_slist_length(phrdtlist) == 1) {
		if (!cclist)
			delete (PhraseData *)phrdtlist->data;
		g_slist_free(phrdtlist);
		return false;
	}

	/* 计算需要的空间 */
	offset = 0;
	length = 0;
	tlist = phrdtlist;
	while (tlist) {
		tphrdt = (PhraseData *)tlist->data;
		offset += tphrdt->chlen;
		length += tphrdt->dtlen;
		tlist = g_slist_next(tlist);
	}

	/* 组装词语 */
	*phrdt = new PhraseData;
	(*phrdt)->chidx = new CharsIndex[offset];
	(*phrdt)->chlen = 0;
	(*phrdt)->offset = (off_t)(-3);	//标记这是合成词汇
	(*phrdt)->data = (gunichar2 *)g_malloc(sizeof(gunichar2) * length);
	(*phrdt)->dtlen = 0;
	tlist = phrdtlist;
	while (tlist) {
		tphrdt = (PhraseData *)tlist->data;
		memcpy((*phrdt)->chidx + (*phrdt)->chlen, tphrdt->chidx,
				 sizeof(CharsIndex) * tphrdt->chlen);
		(*phrdt)->chlen += tphrdt->chlen;
		memcpy((*phrdt)->data + (*phrdt)->dtlen, tphrdt->data,
				 sizeof(gunichar2) * tphrdt->dtlen);
		(*phrdt)->dtlen += tphrdt->dtlen;
		tlist = g_slist_next(tlist);
	}

	/* 善后工作 */
	tlist = cclist ? phrdtlist->next : phrdtlist;
	while (tlist) {
		delete (PhraseData *)tlist->data;
		tlist = g_slist_next(tlist);
	}
	g_slist_free(phrdtlist);
	cclist = g_slist_prepend(cclist, *phrdt);

	return true;
}

/**
 * 选定词语数据缓冲区中的词语数据.
 * @param phrdt 词语数据
 * @return 执行状况
 */
bool PinyinEditor::SelectCachePhrase(PhraseData *phrdt)
{
	/* 将词语数据加入已接受词语链表 */
	aclist = g_slist_append(aclist, phrdt);
	/* 清空缓冲数据 */
	cclist = g_slist_remove(cclist, phrdt);	//避免二次释放
	ClearEngineUnitBuffer();
	ClearPinyinEditorTempBuffer();
	/* 如果需要则继续查询词语 */
	if (!IsFinishInquirePhrase())
		InquirePhraseIndex();

	return true;
}

/**
 * 删除词语数据.
 * @param phrdt 词语数据
 * @return 执行状况
 */
bool PinyinEditor::DeletePhraseData(PhraseData *phrdt)
{
	phregn->DeletePhraseData(phrdt);
	cclist = g_slist_remove(cclist, phrdt);
	delete phrdt;

	return true;
}

/**
 * 向用户词语引擎反馈被用户选中的词语.
 * @return 执行状况
 */
bool PinyinEditor::FeedbackSelectedPhrase()
{
	PhraseData *phrdt;
	guint length;

	/* 若没有数据则无需处理 */
	if ((length = g_slist_length(aclist)) == 0)
		return false;
	/* 获取需要被反馈的词语数据 */
	if (length > 1)
		phrdt = CreateUserPhrase();
	else
		phrdt = (PhraseData *)aclist->data;
	/* 反馈词语数据 */
	phregn->FeedbackPhraseData(phrdt);
	/* 如果词语是临时合成，则需要手工释放 */
	if (length > 1)
		delete phrdt;

	return true;
}

/**
 * 词语查询工作是否已经完成.
 * @return 是否完成
 */
bool PinyinEditor::IsFinishInquirePhrase()
{
	return (ComputeInquireOffset() == chlen);
}

/**
 * 通告此编辑器词语查询已经完成.
 * @return 执行状况
 */
bool PinyinEditor::FinishInquirePhrase()
{
	ClearEngineUnitBuffer();
	ClearPinyinEditorBuffer();

	return true;
}

/**
 * 获取当前光标的偏移量.
 * @return 当前光标偏移量
 */
guint PinyinEditor::GetCursorPoint()
{
	return cursor;
}

/**
 * 获取即将被用来反馈的词汇的偏移量.
 * @return 词汇偏移量
 * @note 请不要将此偏移量用作其他用途，它只应该被用来判断词汇所属的类型
 */
off_t PinyinEditor::GetPhraseOffset()
{
	switch (g_slist_length(aclist)) {
	case 0:
		return 0;
	case 1:
		return ((PhraseData *)aclist->data)->offset;
	default:
		return (off_t)(-2);
	}
}

/**
 * 创建用户词语.
 * @return 词语数据
 */
PhraseData *PinyinEditor::CreateUserPhrase()
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
	phrdt->offset = (off_t)(-2);	//标记这是自定义词汇
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
void PinyinEditor::CreateCharsIndex()
{
	ParseString parse;
	GArray *rtftable;
	char *string, *pinyin;

	/* 如果处于英文模式，则直接退出 */
	if (!editmode)
		return;

	/* 创建汉字索引 */
	rtftable = phregn->GetRectifyTable();
	string = RectifyPinyinString(pytable->data, rtftable);
	pinyin = RectifyPinyinString(string, irtytable);
	g_free(string);
	parse.ParsePinyinString(pinyin, &chidx, &chlen);
	g_free(pinyin);
}

/**
 * 查询词语索引.
 */
void PinyinEditor::InquirePhraseIndex()
{
	int offset;

	/* 如果处于英文模式，则直接退出 */
	if (!editmode)
		return;

	/* 查询词语索引 */
	offset = ComputeInquireOffset();
	euphrlist = phregn->InquireMatchPhraseIndex(chidx + offset, chlen - offset);
	time(&timestamp);	//更新时间戳
}

/**
 * 计算当前待查询汉字索引数组的偏移量.
 * @return 偏移量
 */
int PinyinEditor::ComputeInquireOffset()
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
 * @return 引擎词语索引缓冲点
 */
EunitPhrase *PinyinEditor::SearchPreferEunitPhrase()
{
	GSList *tlist;
	EunitPhrase *euphr, *teuphr;
	PhraseIndex *phridx;
	int chlen, priority;

	/* 初始化数据 */
	euphr = NULL;
	chlen = 0;
	priority = 0;

	/* 查找 */
	tlist = euphrlist;
	while (tlist) {
		teuphr = (EunitPhrase *)tlist->data;
		if (teuphr->phrlist) {
			phridx = (PhraseIndex *)teuphr->phrlist->data;
			if (phridx->chlen > chlen
				 || (phridx->chlen == chlen
					 && teuphr->eunit->priority > priority)) {
				euphr = teuphr;
				chlen = phridx->chlen;
				priority = teuphr->eunit->priority;
			}
		}
		tlist = g_slist_next(tlist);
	}

	return euphr;
}

/**
 * 考察此词语是否已经存在词语数据缓冲区中.
 * @param phrdt 词语数据
 * @return 是否已经存在
 */
bool PinyinEditor::IsExistCachePhrase(const PhraseData *phrdt)
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
 * 纠正拼音串中可能存在的错误.
 * 使用(PhraseEngine)类提供的拼音修正表. \n
 * @param string 原拼音串
 * @param rtftable 拼音修正参考表
 * @return 新拼音串
 */
char *PinyinEditor::RectifyPinyinString(const char *string, const GArray *rtftable)
{
	GArray *rpytable;
	size_t count, length;
	guint index;
	const RectifyUnit *rtfunit;
	char *pinyin;

	/* 创建足够的缓冲区 */
	length = strlen(string);
	rpytable = g_array_sized_new(TRUE, FALSE, 1, length << 1);

	/* 纠正拼音串 */
	count = 0;
	while (count < length) {
		index = 0;
		while (index < rtftable->len) {
			rtfunit = &g_array_index(rtftable, RectifyUnit, index);
			if (memcmp(rtfunit->fstring, string + count,
					 strlen(rtfunit->fstring)) == 0)
				break;
			index++;
		}
		if (index < rtftable->len) {
			rpytable = g_array_append_vals(rpytable, rtfunit->tstring,
							 strlen(rtfunit->tstring));
			count += strlen(rtfunit->fstring);
		} else {
			rpytable = g_array_append_vals(rpytable, string + count, 1);
			count++;
		}
	}
	pinyin = g_array_free(rpytable, FALSE);

	return pinyin;
}

/**
 * 纠正拼音串中可能存在的错误.
 * 使用本类内置的拼音修正表. \n
 * @param string 原拼音串
 * @param rtftable 拼音修正参考表
 * @return 新拼音串
 */
char *PinyinEditor::RectifyPinyinString(const char *string, const RectifyUnit *rtftable)
{
	GArray *rpytable;
	size_t count, length;
	const RectifyUnit *rtfunit;
	char *pinyin;

	/* 创建足够的缓冲区 */
	length = strlen(string);
	rpytable = g_array_sized_new(TRUE, FALSE, 1, length << 1);

	/* 纠正拼音串 */
	count = 0;
	while (count < length) {
		rtfunit = rtftable;
		while (rtfunit->fstring) {
			if (memcmp(rtfunit->fstring, string + count,
					 strlen(rtfunit->fstring)) == 0)
				break;
			rtfunit++;
		}
		if (rtfunit->fstring) {
			rpytable = g_array_append_vals(rpytable, rtfunit->tstring,
							 strlen(rtfunit->tstring));
			count += strlen(rtfunit->fstring);
		} else {
			rpytable = g_array_append_vals(rpytable, string + count, 1);
			count++;
		}
	}
	pinyin = g_array_free(rpytable, FALSE);

	return pinyin;
}

/**
 * 清空为引擎单元缓冲的数据.
 */
void PinyinEditor::ClearEngineUnitBuffer()
{
	GSList *tlist;

	tlist = euphrlist;
	while (tlist) {
		delete (EunitPhrase *)tlist->data;
		tlist = g_slist_next(tlist);
	}
	g_slist_free(euphrlist);
	euphrlist = NULL;
}

/**
 * 清空拼音引擎自身的缓冲数据.
 */
void PinyinEditor::ClearPinyinEditorBuffer()
{
	/* 清空待查询拼音表数据 */
	if (pytable->len != 0)
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
 * 清空拼音引擎自身的待查询索引缓冲数据.
 */
void PinyinEditor::ClearPinyinEditorIdxBuffer()
{
	delete [] chidx;
	chidx = NULL;
	chlen = 0;
}

/**
 * 清空拼音引擎自身已经过时的缓冲数据.
 */
void PinyinEditor::ClearPinyinEditorOldBuffer()
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
void PinyinEditor::ClearPinyinEditorTempBuffer()
{
	/* 释放缓冲词语链表 */
	for (GSList *tlist = cclist; tlist; tlist = g_slist_next(tlist))
		delete (PhraseData *)tlist->data;
	g_slist_free(cclist);
	cclist = NULL;
}
