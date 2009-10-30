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
#include "HalfFullConverter.h"
#include "Config.h"
#include "engine/PhraseEngine.h"
extern Config config;
extern PhraseEngine phregn;

#define CMSHM_MASK \
	(IBUS_CONTROL_MASK | \
	IBUS_MOD1_MASK | \
	IBUS_SUPER_MASK | \
	IBUS_HYPER_MASK | \
	IBUS_META_MASK)
#define CMSHM_FILTER(modifiers) \
	(modifiers & (CMSHM_MASK))

/**
 * 类构造函数.
 * @param egn IBusEngine
 */
PinyinEngine::PinyinEngine(IBusEngine *egn):engine(egn), pyedit(NULL),
 lktable(NULL), props(NULL), timestamp(0), bakgap(60),
 prekey(IBUS_VoidSymbol), chmode(true), flmode(false), fpmode(true),
 squote(false), dquote(false)
{
	/* 创建拼音编辑器 */
	pyedit = new PinyinEditor(&phregn);
	/* 创建词语查询表 */
	lktable = ibus_lookup_table_new(config.GetPageSize(), 0, TRUE, FALSE);
	/* 创建属性部件表 */
	props = CreateProperty();
	/* 获取时间戳 */
	time(&timestamp);
	bakgap = config.GetBackupGap();
}

/**
 * 类析构函数.
 */
PinyinEngine::~PinyinEngine()
{
	delete pyedit;
	g_object_unref(lktable);
	g_object_unref(props);
}

/**
 * 引擎被重置.
 */
void PinyinEngine::EngineReset()
{
	ClearEngineUI();
}

/**
 * 引擎被禁止.
 */
void PinyinEngine::EngineDisable()
{
	phregn.BakUserEnginePhrase();
	time(&timestamp);
	RestoreInitState();
	ClearEngineUI();
}

/**
 * 引擎被允许.
 */
void PinyinEngine::EngineEnable()
{
}

/**
 * 获得焦点.
 */
void PinyinEngine::FocusIn()
{
	ibus_engine_register_properties(engine, props);
}

/**
 * 离开焦点.
 */
void PinyinEngine::FocusOut()
{
	phregn.BakUserEnginePhrase();
	time(&timestamp);
}

/**
 * 光标下移.
 */
void PinyinEngine::CursorDown()
{
	if (lktable->candidates->len - lktable->cursor_pos <= 1)
		AppendPageCandidate();
	ibus_lookup_table_cursor_down(lktable);
	ibus_engine_update_lookup_table_fast(engine, lktable, TRUE);
}

/**
 * 光标上移.
 */
void PinyinEngine::CursorUp()
{
	ibus_lookup_table_cursor_up(lktable);
	ibus_engine_update_lookup_table_fast(engine, lktable, TRUE);
}

/**
 * 向下翻页.
 */
void PinyinEngine::PageDown()
{
	if (lktable->candidates->len - lktable->cursor_pos <= lktable->page_size)
		AppendPageCandidate();
	ibus_lookup_table_page_down(lktable);
	ibus_engine_update_lookup_table_fast(engine, lktable, TRUE);
}

/**
 * 向上翻页.
 */
void PinyinEngine::PageUp()
{
	ibus_lookup_table_page_up(lktable);
	ibus_engine_update_lookup_table_fast(engine, lktable, TRUE);
}

/**
 * 属性单元被激活.
 * @param prop_name Unique Identity for the IBusProperty.
 * @param prop_state Key modifier flags.
 */
void PinyinEngine::PropertyActivate(const gchar *prop_name, guint prop_state)
{
	if (strcmp(prop_name, "mode.chinese") == 0)
		ToggleModeChinese();
	else if (strcmp(prop_name, "mode.full_letter") == 0)
		ToggleModeFullLetter();
	else if (strcmp(prop_name, "mode.full_punct") == 0)
		ToggleModeFullPunct ();
	else if (strcmp(prop_name, "setup"))
		ShowSetupDialog ();
}

/**
 * 候选字被点击.
 * @param index 索引值
 * @param button 鼠标按键
 * @param state Key modifier flags.
 */
void PinyinEngine::CandidateClicked(guint index, guint button, guint state)
{
	guint pages;

	pages = lktable->cursor_pos / lktable->page_size;
	SelectCandidatePhrase(pages * lktable->page_size + index);
}

/**
 * 处理键值事件.
 * @param keyval Key symbol of a key event.
 * @param keycode Keycode of a key event.
 * @param state Key modifier flags.
 * @return TRUE for successfully process the key; FALSE otherwise.
 */
gboolean PinyinEngine::ProcessKeyEvent(guint keyval, guint keycode, guint state)
{
	gboolean retval;

	/* 对释放键的处理 */
	if (state & IBUS_RELEASE_MASK) {
		if (prekey != keyval)
			return FALSE;
		switch (keyval) {
		case IBUS_Shift_L:
		case IBUS_Shift_R:
			if (pyedit->IsFinishInquirePhrase())
				ToggleModeChinese();
			return TRUE;
		}
		return FALSE;
	}

	/* 键值处理 */
	retval = FALSE;
	switch (keyval) {
	/* letters */
	case IBUS_a ... IBUS_z:
		retval = ProcessPinyin(keyval, keycode, state);
		break;
	case IBUS_A ... IBUS_Z:
		retval = ProcessCapitalLetter(keyval, keycode, state);
		break;
	/* numbers */
	case IBUS_0 ... IBUS_9:
	case IBUS_KP_0 ... IBUS_KP_9:
		retval = ProcessNumber(keyval, keycode, state);
		break;
	/* punct */
	case IBUS_exclam ... IBUS_slash:
	case IBUS_colon ... IBUS_at:
	case IBUS_bracketleft ... IBUS_quoteleft:
	case IBUS_braceleft ... IBUS_asciitilde:
		retval = ProcessPunct(keyval, keycode, state);
		break;
	/* space */
	case IBUS_space:
		retval = ProcessSpace(keyval, keycode, state);
		break;
	/* others */
	default:
		retval = ProcessOthers(keyval, keycode, state);
		break;
	}
	prekey = retval ? IBUS_VoidSymbol : keyval;

	return retval;
}

/**
 * 创建属性部件表.
 * @return 部件表
 */
IBusPropList *PinyinEngine::CreateProperty()
{
	IBusProperty *property;
	IBusPropList *props;

	props = ibus_prop_list_new();

	/* 中英文 */
	property = ibus_property_new("mode.chinese",
			 PROP_TYPE_NORMAL,
			 ibus_text_new_from_static_string("CN"),
			 chmode ? PKGDATADIR "/icons/chinese.svg" :
				 PKGDATADIR "/icons/english.svg",
			 ibus_text_new_from_static_string("Chinese"),
			 TRUE,
			 TRUE,
			 PROP_STATE_UNCHECKED,
			 NULL);
	g_object_set_data(G_OBJECT(props), "mode.chinese", property);
	ibus_prop_list_append(props, property);

	/* 字母全/半角 */
	property = ibus_property_new("mode.full_letter",
			 PROP_TYPE_NORMAL,
			 ibus_text_new_from_static_string(flmode ? "Ａａ" : "Aa"),
			 flmode ? PKGDATADIR "/icons/full-letter.svg" :
				 PKGDATADIR"/icons/half-letter.svg",
			 ibus_text_new_from_static_string("Full/Half width letter"),
			 TRUE,
			 TRUE,
			 PROP_STATE_UNCHECKED,
			 NULL);
	g_object_set_data(G_OBJECT(props), "mode.full_letter", property);
	ibus_prop_list_append(props, property);

	/* 标点全/半角 */
	property = ibus_property_new("mode.full_punct",
			 PROP_TYPE_NORMAL,
			 ibus_text_new_from_static_string(fpmode ? "，。" : ",."),
			 fpmode ? PKGDATADIR "/icons/full-punct.svg" :
				 PKGDATADIR "/icons/half-punct.svg",
			 ibus_text_new_from_static_string("Full/Half width punctuation"),
			 TRUE,
			 TRUE,
			 PROP_STATE_UNCHECKED,
			 NULL);
	g_object_set_data(G_OBJECT(props), "mode.full_punct", property);
	ibus_prop_list_append(props, property);

	/* 细节设置 */
	property = ibus_property_new("setup",
			 PROP_TYPE_NORMAL,
			 ibus_text_new_from_static_string("Pinyin preferences"),
			 "gtk-preferences",
			 ibus_text_new_from_static_string("Pinyin preferences"),
			 TRUE,
			 TRUE,
			 PROP_STATE_UNCHECKED,
			 NULL);
	g_object_set_data(G_OBJECT(props), "setup", property);
	ibus_prop_list_append(props, property);

	return props;
}

/**
 * 恢复初始化状态.
 */
void PinyinEngine::RestoreInitState()
{
	pyedit->FinishInquirePhrase();
	ibus_lookup_table_clear(lktable);
	prekey = IBUS_VoidSymbol;
	if (!chmode)
		ToggleModeChinese();
	if (flmode)
		ToggleModeFullLetter();
	if (!fpmode)
		ToggleModeFullPunct();
	squote = false;
	dquote = false;
}

/**
 * 切换中/英文模式.
 */
void PinyinEngine::ToggleModeChinese()
{
	IBusProperty *property;
	IBusText *text;

	chmode = !chmode;
	property = IBUS_PROPERTY(g_object_get_data(G_OBJECT(props), "mode.chinese"));
	text = ibus_text_new_from_static_string(chmode ? "CN" : "EN");
	ibus_property_set_label(property, text);
	ibus_property_set_icon(property, chmode ? PKGDATADIR "/icons/chinese.svg" :
						 PKGDATADIR "/icons/english.svg");
	ibus_engine_update_property(engine, property);

	/* 字母无条件回到半角模式 */
	if (flmode)
		ToggleModeFullLetter();
	/* 根据情况改变标点的模式 */
	if ((chmode && !fpmode) || (!chmode && fpmode))
		ToggleModeFullPunct();
}

/**
 * 切换字母全/半角模式.
 */
void PinyinEngine::ToggleModeFullLetter()
{
	IBusProperty *property;
	IBusText *text;

	flmode = !flmode;
	property = IBUS_PROPERTY(g_object_get_data(G_OBJECT(props), "mode.full_letter"));
	text = ibus_text_new_from_static_string(flmode ? "Ａａ" : "Aa");
	ibus_property_set_label(property, text);
	ibus_property_set_icon(property, flmode ? PKGDATADIR "/icons/full-letter.svg" :
						 PKGDATADIR "/icons/half-letter.svg");
	ibus_engine_update_property(engine, property);
}

/**
 * 切换标点全/半角模式.
 */
void PinyinEngine::ToggleModeFullPunct ()
{
	IBusProperty *property;
	IBusText *text;

	fpmode = !fpmode;
	property = IBUS_PROPERTY(g_object_get_data(G_OBJECT(props), "mode.full_punct"));
	text = ibus_text_new_from_static_string(fpmode ? "，。" : ",.");
	ibus_property_set_label(property, text);
	ibus_property_set_icon(property, fpmode ? PKGDATADIR "/icons/full-punct.svg" :
						 PKGDATADIR "/icons/half-punct.svg");
	ibus_engine_update_property(engine, property);
}

/**
 * 显示设置对话框.
 */
void PinyinEngine::ShowSetupDialog ()
{
	g_spawn_command_line_async(LIBEXECDIR "/ibus-setup-pye", NULL);
}

/**
 * 处理拼音.
 * @param keyval Key symbol of a key event.
 * @param keycode Keycode of a key event.
 * @param state Key modifier flags.
 * @return TRUE for successfully process the key; FALSE otherwise.
 */
gboolean PinyinEngine::ProcessPinyin(guint keyval, guint keycode, guint state)
{
	/* 如果有修饰键则不处理 */
	if (CMSHM_FILTER(state) != 0)
		return FALSE;

	/* 英文模式 */
	if (!chmode) {
		CommitLetter(keyval);
		return TRUE;
	}

	/* 剩下的由拼音编辑器搞定 */
	pyedit->InsertPinyinKey(keyval);
	UpdateEngineUI();

	return TRUE;
}

/**
 * 处理大写字母.
 * @param keyval Key symbol of a key event.
 * @param keycode Keycode of a key event.
 * @param state Key modifier flags.
 * @return TRUE for successfully process the key; FALSE otherwise.
 */
gboolean PinyinEngine::ProcessCapitalLetter(guint keyval, guint keycode, guint state)
{
	/* 如果有修饰键则不处理 */
	if (CMSHM_FILTER(state) != 0)
		return FALSE;

	/* 提交字符 */
	if (!chmode || (chmode && pyedit->IsFinishInquirePhrase()))
		CommitLetter(keyval);

	return TRUE;
}

/**
 * 处理数字.
 * @param keyval Key symbol of a key event.
 * @param keycode Keycode of a key event.
 * @param state Key modifier flags.
 * @return TRUE for successfully process the key; FALSE otherwise.
 */
gboolean PinyinEngine::ProcessNumber(guint keyval, guint keycode, guint state)
{
	guint pages, index;

	/* 英语模式 */
	if (!chmode) {
		CommitLetter(keyval);
		return TRUE;
	}

	/* 汉语模式，编辑器为空 */
	if (pyedit->IsFinishInquirePhrase()) {
		if (CMSHM_FILTER(state) != 0)
			return FALSE;
		switch (keyval) {
		case IBUS_0 ... IBUS_9:
			CommitLetter(keyval);
			break;
		case IBUS_KP_0 ... IBUS_KP_9:
			CommitLetter('0' + keyval - IBUS_KP_0);
			break;
		}
		return TRUE;
	}

	/* 汉语模式，编辑器不为空 */
	switch (keyval) {
	case IBUS_0:
	case IBUS_KP_0:
		index = 9;
		break;
	case IBUS_1 ... IBUS_9:
		index = keyval - IBUS_1;
		break;
	case IBUS_KP_1 ... IBUS_KP_9:
		index = keyval - IBUS_KP_1;
		break;
	default:
		g_assert_not_reached();
	}

	/* 选择词语 */
	pages = lktable->cursor_pos / lktable->page_size;
	SelectCandidatePhrase(pages * lktable->page_size + index);

	return TRUE;
}

/**
 * 处理空格.
 * @param keyval Key symbol of a key event.
 * @param keycode Keycode of a key event.
 * @param state Key modifier flags.
 * @return TRUE for successfully process the key; FALSE otherwise.
 */
gboolean PinyinEngine::ProcessSpace(guint keyval, guint keycode, guint state)
{
	/* 如果有修饰键则不处理 */
	if (CMSHM_FILTER(state) != 0)
		return FALSE;

	/* 中文模式，编辑器不为空 */
	if (chmode && !pyedit->IsFinishInquirePhrase())
		SelectCandidatePhrase(lktable->cursor_pos);
	else
		CommitPunct(keyval);

	return TRUE;
}

gboolean PinyinEngine::ProcessPunct(guint keyval, guint keycode, guint state)
{
	/* 英文模式 */
	if  (!chmode) {
		CommitPunct(keyval);
		return TRUE;
	}

	/* 中文模式，编辑器不为空 */
	if (!pyedit->IsFinishInquirePhrase()) {
		switch (keyval) {
		case IBUS_minus:
		case IBUS_comma:
			PageUp();
			return TRUE;
		case IBUS_equal:
		case IBUS_period:
			PageDown();
			return TRUE;
		case IBUS_apostrophe:
			ProcessPinyin(keyval, keycode, state);
			return TRUE;
		}
		return FALSE;
	}

	/* 中文模式，编辑器为空 */
	if (fpmode) {
		switch (keyval) {
		case '$':
			CommitStaticString("￥");
			break;
		case '^':
			CommitStaticString("……");
			break;
		case '_':
			CommitStaticString("——");
			break;
		case '[':
			CommitStaticString("【");
			break;
		case ']':
			CommitStaticString("】");
			break;
		case '{':
			CommitStaticString("『");
			break;
		case '}':
			CommitStaticString("』");
			break;
		case '\\':
			CommitStaticString("、");
			break;
		case '\'':
			CommitStaticString(squote ? "‘" : "’");
			squote = !squote;
			break;
		case '"':
			CommitStaticString(dquote ? "“" : "”");
			dquote = !dquote;
			break;
		case '.':
			CommitStaticString("。");
			break;
		case '<':
			CommitStaticString("《");
			break;
		case '>':
			CommitStaticString("》");
			break;
		default:
			CommitPunct(keyval);
		}
	} else
		CommitFinalChars(keyval);

	return TRUE;
}

/**
 * 处理其他字符.
 * @param keyval Key symbol of a key event.
 * @param keycode Keycode of a key event.
 * @param state Key modifier flags.
 * @return TRUE for successfully process the key; FALSE otherwise.
 */
gboolean PinyinEngine::ProcessOthers(guint keyval, guint keycode, guint state)
{
	guint pages;

	/* 编辑器为空 */
	if (pyedit->IsFinishInquirePhrase())
		return FALSE;

	/* ignore numlock */
	state &= ~IBUS_MOD2_MASK;

	/* process some cursor control keys */
	switch (keyval) {
	case IBUS_Shift_L:
		pages = lktable->cursor_pos / lktable->page_size;
		SelectCandidatePhrase(pages * lktable->page_size + 1);
		break;
	case IBUS_Shift_R:
		pages = lktable->cursor_pos / lktable->page_size;
		SelectCandidatePhrase(pages * lktable->page_size + 2);
		break;
	case IBUS_Return:
	case IBUS_KP_Enter:
		CommitRawPhrase();
		ClearEngineUI();
		break;
	case IBUS_BackSpace:
		if (!pyedit->RevokeSelectedPhrase())
			pyedit->BackspacePinyinKey();
		if (!pyedit->IsFinishInquirePhrase())
			UpdateEngineUI();
		else
			ClearEngineUI();
		break;
	case IBUS_Up:
	case IBUS_KP_Up:
		CursorUp();
		break;
	case IBUS_Down:
	case IBUS_KP_Down:
		CursorDown();
		break;
	case IBUS_Page_Up:
	case IBUS_KP_Page_Up:
		PageUp();
		break;
	case IBUS_Page_Down:
	case IBUS_KP_Page_Down:
		PageDown();
		break;
	case IBUS_Escape:
		EngineReset();
		break;
	}

	return TRUE;
}

/**
 * 更新引擎UI.
 */
void PinyinEngine::UpdateEngineUI()
{
	IBusText *text;
	char *textdt;
	gunichar2 *data;
	glong length;

	/* 更新候选字 */
	ibus_lookup_table_clear(lktable);
	AppendPageCandidate();

	/* 更新辅助文本 */
	pyedit->GetAuxiliaryText(&data, &length);
	if (length != 0) {
		textdt = g_utf16_to_utf8(data, length, NULL, NULL, NULL);
		g_free(data);
		text = ibus_text_new_from_static_string(textdt);
		g_object_set_data_full(G_OBJECT(text), "text", textdt,
						 GDestroyNotify(g_free));
		ibus_engine_update_auxiliary_text(engine, text, TRUE);
	} else
		ibus_engine_hide_auxiliary_text(engine);

	/* 更新预编辑文本 */
	pyedit->GetPreeditText(&data, &length);
	if (length != 0) {
		textdt = g_utf16_to_utf8(data, length, NULL, NULL, NULL);
		g_free(data);
		text = ibus_text_new_from_static_string(textdt);
		g_object_set_data_full(G_OBJECT(text), "text", textdt,
						 GDestroyNotify(g_free));
		ibus_engine_update_preedit_text(engine, text, length, TRUE);
	} else
		ibus_engine_hide_preedit_text(engine);
}

/**
 * 显示引擎的UI.
 */
void PinyinEngine::ShowEngineUI()
{
	ibus_engine_show_lookup_table(engine);
	ibus_engine_show_auxiliary_text(engine);
	ibus_engine_show_preedit_text(engine);
}

/**
 * 隐藏引擎的UI.
 */
void PinyinEngine::HideEngineUI()
{
	ibus_engine_hide_lookup_table(engine);
	ibus_engine_hide_auxiliary_text(engine);
	ibus_engine_hide_preedit_text(engine);
}

/**
 * 清空引擎UI关联数据.
 */
void PinyinEngine::ClearEngineUI()
{
	pyedit->FinishInquirePhrase();
	ibus_lookup_table_clear(lktable);
	HideEngineUI();
}

/**
 * 添加一个页面的候选词语.
 */
void PinyinEngine::AppendPageCandidate()
{
	PhraseData *phrdt;
	IBusText *text;
	char *textdt;
	GSList *phrlist, *tlist;
	guint pagesize;

	pagesize = ibus_lookup_table_get_page_size(lktable);
	pyedit->GetPagePhrase(&phrlist, &pagesize);
	tlist = phrlist;
	while (tlist) {
		phrdt = (PhraseData *)tlist->data;
		textdt = g_utf16_to_utf8(phrdt->data, phrdt->dtlen, NULL, NULL, NULL);
		text = ibus_text_new_from_static_string(textdt);
		g_object_set_data_full(G_OBJECT(text), "text", textdt,
						 GDestroyNotify(g_free));
		g_object_set_data(G_OBJECT(text), "data", phrdt);
		ibus_lookup_table_append_candidate(lktable, text);
		tlist = g_slist_next(tlist);
	}
	ibus_engine_update_lookup_table(engine, lktable, TRUE);
	g_slist_free(phrlist);
}

/**
 * 选择候选词语.
 * @param index 索引值
 */
void PinyinEngine::SelectCandidatePhrase(guint index)
{
	IBusText *text;
	PhraseData *phrdt;

	if ( (text = ibus_lookup_table_get_candidate(lktable, index))) {
		phrdt = (PhraseData *)g_object_get_data(G_OBJECT(text), "data");
		pyedit->SelectCachePhrase(phrdt);
		if (pyedit->IsFinishInquirePhrase()) {
			CommitPhrase();
			ClearEngineUI();
		} else
			UpdateEngineUI();
	}
}

/**
 * 提交词语.
 */
void PinyinEngine::CommitPhrase()
{
	time_t stamp;
	IBusText *text;
	char *textdt;
	gunichar2 *data;
	glong length;

	/* 提交词语数据 */
	pyedit->GetCommitText(&data, &length);
	if (length != 0) {
		/* 向UI提交词语 */
		textdt = g_utf16_to_utf8(data, length, NULL, NULL, NULL);
		g_free(data);
		text = ibus_text_new_from_static_string(textdt);
		g_object_set_data_full(G_OBJECT(text), "text", textdt,
						 GDestroyNotify(g_free));
		ibus_engine_commit_text(engine, text);
		/* 反馈词语 */
		pyedit->FeedbackSelectedPhrase();
	}

	/* 检查是否需要备份数据 */
	time(&stamp);
	if (stamp - timestamp > bakgap) {
		phregn.BakUserEnginePhrase();
		timestamp = stamp;
	}
}

/**
 * 提交原始串.
 */
void PinyinEngine::CommitRawPhrase()
{
	IBusText *text;
	char *textdt;
	guint length;

	/* 提交原始串 */
	pyedit->GetRawText(&textdt, &length);
	if (length != 0) {
		text = ibus_text_new_from_static_string(textdt);
		g_object_set_data_full(G_OBJECT(text), "text", textdt,
						 GDestroyNotify(g_free));
		ibus_engine_commit_text(engine, text);
	}
}

/**
 * 提交字母.
 * @param ch 字母
 */
void PinyinEngine::CommitLetter(gunichar ch)
{
	IBusText *text;

	ch = flmode ? HalfFullConverter::ToFull(ch) : ch;
	text = ibus_text_new_from_unichar(ch);
	ibus_engine_commit_text(engine, text);
}

/**
 * 提交标点.
 * @param ch 标点
 */
void PinyinEngine::CommitPunct(gunichar ch)
{
	IBusText *text;

	ch = fpmode ? HalfFullConverter::ToFull(ch) : ch;
	text = ibus_text_new_from_unichar(ch);
	ibus_engine_commit_text(engine, text);
}

/**
 * 提交最终字符.
 * @param ch 字符
 */
void PinyinEngine::CommitFinalChars(gunichar ch)
{
	IBusText *text;

	text = ibus_text_new_from_unichar(ch);
	ibus_engine_commit_text(engine, text);
}

/**
 * 提交串.
 * @param str 串
 */
void PinyinEngine::CommitString(const gchar *str)
{
	IBusText *text;

	text = ibus_text_new_from_string(str);
	ibus_engine_commit_text(engine, text);
}

/**
 * 提交静态串.
 * @param str 串
 */
void PinyinEngine::CommitStaticString(const gchar *str)
{
	IBusText *text;

	text = ibus_text_new_from_static_string(str);
	ibus_engine_commit_text(engine, text);
}
