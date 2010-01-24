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
#include "EngineConfig.h"
#include "engine/PhraseEngine.h"
#include "engine/support.h"

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
 * @param busegn IBusEngine
 * @param phregn 词语查询引擎
 */
PinyinEngine::PinyinEngine(IBusEngine *busegn): busengine(busegn),
 pyedit(NULL), lktable(NULL), props(NULL), pagesize(5), flags(0),
 prekey(IBUS_VoidSymbol), entmpmode(false), chmode(true),
 flmode(false), fpmode(true), squote(false), dquote(false)
{
	PhraseEngine *phrengine;
	EngineConfig *config;

	/* 从引擎配置类中提取必要数据 */
	config = EngineConfig::GetInstance();
	config->AddListener(this);
	pagesize = config->GetPageSize();
	flags = config->GetBitFlags();

	/* 创建拼音编辑器 */
	phrengine = PhraseEngine::GetInstance();
	pyedit = new PinyinEditor(phrengine);
	/* 创建词语查询表 */
	lktable = ibus_lookup_table_new(pagesize, 0, FLAG_ISSET(flags, 3), FALSE);
	/* 创建属性部件表 */
	props = CreateProperty();
}

/**
 * 类析构函数.
 */
PinyinEngine::~PinyinEngine()
{
	EngineConfig *config;

	config = EngineConfig::GetInstance();
	config->RemoveListener(this);

	delete pyedit;
	g_object_unref(lktable);
	g_object_unref(props);
}

/**
 * 更新引擎配置.
 */
void PinyinEngine::UpdateConfig()
{
	EngineConfig *config;

	config = EngineConfig::GetInstance();
	pagesize = config->GetPageSize();
	ibus_lookup_table_set_page_size(lktable, pagesize);
	flags = config->GetBitFlags();
	ibus_lookup_table_set_cursor_visible(lktable, FLAG_ISSET(flags, 3));
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
	ibus_engine_register_properties(busengine, props);
}

/**
 * 离开焦点.
 */
void PinyinEngine::FocusOut()
{
}

/**
 * 光标下移.
 */
void PinyinEngine::CursorDown()
{
	/* 如果光标不可见，必然无须移动光标 */
	if (!FLAG_ISSET(flags, 3))
		return;

	if (lktable->candidates->len - lktable->cursor_pos <= lktable->page_size)
		AppendPageCandidate();
	ibus_lookup_table_cursor_down(lktable);
	ibus_engine_update_lookup_table_fast(busengine, lktable, TRUE);
}

/**
 * 光标上移.
 */
void PinyinEngine::CursorUp()
{
	/* 如果光标不可见，必然无须移动光标 */
	if (!FLAG_ISSET(flags, 3))
		return;

	ibus_lookup_table_cursor_up(lktable);
	ibus_engine_update_lookup_table_fast(busengine, lktable, TRUE);
}

/**
 * 向下翻页.
 */
void PinyinEngine::PageDown()
{
	if (lktable->candidates->len - lktable->cursor_pos < (lktable->page_size << 1))
		AppendPageCandidate();
	ibus_lookup_table_page_down(lktable);
	ibus_engine_update_lookup_table_fast(busengine, lktable, TRUE);
}

/**
 * 向上翻页.
 */
void PinyinEngine::PageUp()
{
	ibus_lookup_table_page_up(lktable);
	ibus_engine_update_lookup_table_fast(busengine, lktable, TRUE);
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
		ToggleModeFullPunct();
	else if (strcmp(prop_name, "engine.setup") == 0)
		ShowSetupDialog();
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
	guint pages;
	gboolean retval;

	/* 对释放键(Shift)的处理 */
	if (state & IBUS_RELEASE_MASK) {
		if (CMSHM_FILTER(state) != 0 || prekey != keyval)
			return FALSE;
		switch (keyval) {
		case IBUS_Shift_L:
			if (!pyedit->IsEmpty()) {
				pages = lktable->cursor_pos / lktable->page_size;
				SelectCandidatePhrase(pages * lktable->page_size + 1);
			} else
				ToggleModeChinese();
			return TRUE;
		case IBUS_Shift_R:
			if (!pyedit->IsEmpty()) {
				pages = lktable->cursor_pos / lktable->page_size;
				SelectCandidatePhrase(pages * lktable->page_size + 2);
			} else
				ToggleModeChinese();
			return TRUE;
		default:
			return FALSE;
		}
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
			 chmode ? __ICON_PATH "/chinese.png" : __ICON_PATH "/english.png",
			 ibus_text_new_from_static_string("Chinese"),
			 TRUE,
			 TRUE,
			 PROP_STATE_UNCHECKED,
			 NULL);
	g_object_set_data(G_OBJECT(props), "mode.chinese", property);
	ibus_prop_list_append(props, property);
	g_object_unref(property);

	/* 字母全/半角 */
	property = ibus_property_new("mode.full_letter",
			 PROP_TYPE_NORMAL,
			 ibus_text_new_from_static_string(flmode ? "Ａａ" : "Aa"),
			 flmode ? __ICON_PATH "/full-letter.png" :
				 __ICON_PATH "/half-letter.png",
			 ibus_text_new_from_static_string("Full/Half width letter"),
			 TRUE,
			 TRUE,
			 PROP_STATE_UNCHECKED,
			 NULL);
	g_object_set_data(G_OBJECT(props), "mode.full_letter", property);
	ibus_prop_list_append(props, property);
	g_object_unref(property);

	/* 标点全/半角 */
	property = ibus_property_new("mode.full_punct",
			 PROP_TYPE_NORMAL,
			 ibus_text_new_from_static_string(fpmode ? "，。" : ",."),
			 fpmode ? __ICON_PATH "/full-punct.png" :
				 __ICON_PATH "/half-punct.png",
			 ibus_text_new_from_static_string("Full/Half width punctuation"),
			 TRUE,
			 TRUE,
			 PROP_STATE_UNCHECKED,
			 NULL);
	g_object_set_data(G_OBJECT(props), "mode.full_punct", property);
	ibus_prop_list_append(props, property);
	g_object_unref(property);

	/* 细节设置 */
	property = ibus_property_new("engine.setup",
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
	g_object_unref(property);

	return props;
}

/**
 * 恢复初始化状态.
 */
void PinyinEngine::RestoreInitState()
{
	pyedit->FinishInquirePhrase();
	pyedit->SetEditorMode(true);
	ibus_lookup_table_clear(lktable);
	prekey = IBUS_VoidSymbol;
	entmpmode = false;
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
	ibus_property_set_icon(property, chmode ? __ICON_PATH "/chinese.png" :
						 __ICON_PATH "/english.png");
	ibus_engine_update_property(busengine, property);

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
	ibus_property_set_icon(property, flmode ? __ICON_PATH "/full-letter.png" :
						 __ICON_PATH "/half-letter.png");
	ibus_engine_update_property(busengine, property);
}

/**
 * 切换标点全/半角模式.
 */
void PinyinEngine::ToggleModeFullPunct()
{
	IBusProperty *property;
	IBusText *text;

	fpmode = !fpmode;
	property = IBUS_PROPERTY(g_object_get_data(G_OBJECT(props), "mode.full_punct"));
	text = ibus_text_new_from_static_string(fpmode ? "，。" : ",.");
	ibus_property_set_label(property, text);
	ibus_property_set_icon(property, fpmode ? __ICON_PATH "/full-punct.png" :
						 __ICON_PATH "/half-punct.png");
	ibus_engine_update_property(busengine, property);
}

/**
 * 显示设置对话框.
 */
void PinyinEngine::ShowSetupDialog()
{
	g_spawn_command_line_async(__LIBEXEC_PATH "/ibus-setup-pye", NULL);
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
		CommitFinalChars(keyval);
		return TRUE;
	}

	/* 是否需要进入临时英文模式 */
	if (pyedit->IsEmpty() && !entmpmode && (keyval == 'u' || keyval == 'v')) {
		pyedit->SetEditorMode(false);	//强制编辑器进入英文模式
		entmpmode = true;
		ShowEngineUI();
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

	/* 英文模式 */
	if (!chmode) {
		CommitFinalChars(keyval);
		return TRUE;
	}

	/* 中文模式，编辑器为空 */
	if (pyedit->IsEmpty()) {
		pyedit->SetEditorMode(false);	//强制编辑器进入英文模式
		entmpmode = true;
	}

	/* 英文编辑模式 */
	if (entmpmode) {
		pyedit->InsertPinyinKey(keyval);
		UpdateEngineUI();
	}

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

	/* 考察是否为(Ctrl + 数字)模式 */
	if ((state & IBUS_CONTROL_MASK) != 0 && (state & ~IBUS_CONTROL_MASK) == 0
							 && !pyedit->IsEmpty()) {
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
		if (index >= lktable->page_size)
			return TRUE;
		pages = lktable->cursor_pos / lktable->page_size;
		index += pages * lktable->page_size;
		if (index >= lktable->candidates->len)
			return TRUE;
		DeleteCandidatePhrase(index);
		return TRUE;
	}
	/* 如果有修饰键则不处理 */
	if (CMSHM_FILTER(state) != 0)
		return FALSE;

	/* 英语模式 */
	if (!chmode) {
		CommitFinalChars(keyval);
		return TRUE;
	}

	/* 英文编辑模式 */
	if (entmpmode) {
		pyedit->InsertPinyinKey(keyval);
		UpdateEngineUI();
		return TRUE;
	}

	/* 汉语模式，编辑器为空 */
	if (pyedit->IsEmpty()) {
		switch (keyval) {
		case IBUS_0 ... IBUS_9:
			CommitLetter(keyval);
			break;
		case IBUS_KP_0 ... IBUS_KP_9:
			CommitLetter('0' + keyval - IBUS_KP_0);
			break;
		default:
			g_assert_not_reached();
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
	if (index >= lktable->page_size)
		return TRUE;
	pages = lktable->cursor_pos / lktable->page_size;
	index += pages * lktable->page_size;
	if (index >= lktable->candidates->len)
		return TRUE;
	SelectCandidatePhrase(index);

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
	/* 考察是否为(Shift + Space)模式 */
	if ((state & IBUS_SHIFT_MASK) != 0 && (state & ~IBUS_SHIFT_MASK) == 0) {
		ToggleModeFullLetter();
		return TRUE;
	}
	/* 如果有修饰键则不处理 */
	if (CMSHM_FILTER(state) != 0)
		return FALSE;

	/* 英语模式 */
	if (!chmode) {
		CommitFinalChars(keyval);
		return TRUE;
	}

	/* 英文编辑模式 */
	if (entmpmode) {
		pyedit->InsertPinyinKey(keyval);
		UpdateEngineUI();
		return TRUE;
	}

	/* 中文模式，编辑器不为空 */
	if (!pyedit->IsEmpty()) {
		SelectCandidatePhrase(lktable->cursor_pos);
		return TRUE;
	}

	/* 中文模式，编辑器为空 */
	CommitPunct(keyval);

	return TRUE;
}

gboolean PinyinEngine::ProcessPunct(guint keyval, guint keycode, guint state)
{
	/* 考察是否为(Ctrl + .)模式 */
	if ((state & IBUS_CONTROL_MASK) != 0 && (state & ~IBUS_CONTROL_MASK) == 0) {
		ToggleModeFullPunct();
		return TRUE;
	}
	/* 如果有修饰键则不处理 */
	if (CMSHM_FILTER(state) != 0)
		return FALSE;

	/* 英语模式 */
	if (!chmode) {
		CommitFinalChars(keyval);
		return TRUE;
	}

	/* 英文编辑模式 */
	if (entmpmode) {
		pyedit->InsertPinyinKey(keyval);
		UpdateEngineUI();
		return TRUE;
	}

	/* 中文模式，编辑器不为空 */
	if (!pyedit->IsEmpty()) {
		switch (keyval) {
		case IBUS_minus:
		case IBUS_comma:
			PageUp();
			break;
		case IBUS_equal:
		case IBUS_period:
			PageDown();
			break;
		case IBUS_apostrophe:
			ProcessPinyin(keyval, keycode, state);
			break;
		}
		return TRUE;
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
			CommitStaticString(squote ? "’" : "‘");
			squote = !squote;
			break;
		case '"':
			CommitStaticString(dquote ? "”" : "“");
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
	/* 如果有修饰键则不处理 */
	if (CMSHM_FILTER(state) != 0)
		return FALSE;

	/* 编辑器为空 */
	if (pyedit->IsEmpty())
		return FALSE;

	/* process some cursor control keys */
	switch (keyval) {
	case IBUS_Return:
	case IBUS_KP_Enter:
		CommitRawPhrase();
		ClearEngineUI();
		return TRUE;
	case IBUS_BackSpace:
		if (!pyedit->RevokeSelectedPhrase())
			pyedit->BackspacePinyinKey();
		if (!pyedit->IsEmpty())
			UpdateEngineUI();
		else
			ClearEngineUI();
		return TRUE;
	case IBUS_Up:
	case IBUS_KP_Up:
		CursorUp();
		return TRUE;
	case IBUS_Down:
	case IBUS_KP_Down:
		CursorDown();
		return TRUE;
	case IBUS_Page_Up:
	case IBUS_KP_Page_Up:
		PageUp();
		return TRUE;
	case IBUS_Page_Down:
	case IBUS_KP_Page_Down:
		PageDown();
		return TRUE;
	case IBUS_Escape:
		EngineReset();
		return TRUE;
	}

	return FALSE;
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
	guint size;

	/* 如果在临时英文模式下 */
	if (entmpmode) {
		ibus_lookup_table_clear(lktable);
		ibus_engine_update_lookup_table(busengine, lktable, TRUE);
		pyedit->GetRawText(&textdt, &size);
		if (size != 0) {
			text = ibus_text_new_from_static_string(textdt);
			g_object_set_data_full(G_OBJECT(text), "text", textdt,
						 GDestroyNotify(g_free));
		} else
			text = ibus_text_new_from_static_string("");
		ibus_engine_update_auxiliary_text(busengine, text, TRUE);
		ibus_engine_update_preedit_text(busengine, text, size, TRUE);
		g_object_unref(text);
		return;
	}

	/* 更新候选字 */
	ibus_lookup_table_clear(lktable);
	AppendDynamicPhrase();
	AppendComposePhrase();
	AppendPageCandidate();
	ibus_engine_update_lookup_table(busengine, lktable, TRUE);

	/* 更新辅助文本 */
	pyedit->GetAuxiliaryText(&data, &length);
	if (length != 0) {
		textdt = g_utf16_to_utf8(data, length, NULL, NULL, NULL);
		g_free(data);
		text = ibus_text_new_from_static_string(textdt);
		g_object_set_data_full(G_OBJECT(text), "text", textdt,
						 GDestroyNotify(g_free));
		ibus_engine_update_auxiliary_text(busengine, text, TRUE);
		g_object_unref(text);
	} else
		ibus_engine_hide_auxiliary_text(busengine);

	/* 更新预编辑文本 */
	pyedit->GetPreeditText(&data, &length);
	if (length != 0) {
		textdt = g_utf16_to_utf8(data, length, NULL, NULL, NULL);
		g_free(data);
		text = ibus_text_new_from_static_string(textdt);
		g_object_set_data_full(G_OBJECT(text), "text", textdt,
						 GDestroyNotify(g_free));
		ibus_engine_update_preedit_text(busengine, text, length, TRUE);
		g_object_unref(text);
	} else
		ibus_engine_hide_preedit_text(busengine);
}

/**
 * 重置并显示引擎的UI.
 */
void PinyinEngine::ShowEngineUI()
{
	IBusText *text;

	ibus_lookup_table_clear(lktable);
	ibus_engine_update_lookup_table_fast(busengine, lktable, TRUE);
	text = ibus_text_new_from_static_string("");
	ibus_engine_update_auxiliary_text(busengine, text, TRUE);
	ibus_engine_update_preedit_text(busengine, text, 0, TRUE);
	g_object_unref(text);
}

/**
 * 隐藏引擎的UI.
 */
void PinyinEngine::HideEngineUI()
{
	ibus_engine_hide_lookup_table(busengine);
	ibus_engine_hide_auxiliary_text(busengine);
	ibus_engine_hide_preedit_text(busengine);
}

/**
 * 清空引擎UI关联数据.
 */
void PinyinEngine::ClearEngineUI()
{
	pyedit->FinishInquirePhrase();
	pyedit->SetEditorMode(true);
	ibus_lookup_table_clear(lktable);
	HideEngineUI();
	entmpmode = false;
}

/**
 * 添加合成词语.
 */
void PinyinEngine::AppendComposePhrase()
{
	PhraseData *phrdt;
	IBusText *text;
	char *textdt;

	phrdt = NULL;
	pyedit->GetComposePhrase(&phrdt);
	if (!phrdt)
		return;
	textdt = g_utf16_to_utf8(phrdt->data, phrdt->dtlen, NULL, NULL, NULL);
	text = ibus_text_new_from_static_string(textdt);
	ibus_text_append_attribute(text, IBUS_ATTR_TYPE_FOREGROUND, 0xff0000,
							 0, phrdt->dtlen);
	g_object_set_data_full(G_OBJECT(text), "text", textdt,
					 GDestroyNotify(g_free));
	g_object_set_data(G_OBJECT(text), "data", phrdt);
	ibus_lookup_table_append_candidate(lktable, text);
	g_object_unref(text);
}

/**
 * 添加动态词语.
 */
void PinyinEngine::AppendDynamicPhrase()
{
	PhraseData *phrdt;
	IBusText *text;
	char *textdt;
	GSList *phrlist, *tlist;
	guint length;

	pyedit->GetDynamicPhrase(&phrlist, &length);
	tlist = phrlist;
	while (tlist) {
		phrdt = (PhraseData *)tlist->data;
		textdt = g_utf16_to_utf8(phrdt->data, phrdt->dtlen, NULL, NULL, NULL);
		text = ibus_text_new_from_static_string(textdt);
		g_object_set_data_full(G_OBJECT(text), "text", textdt,
						 GDestroyNotify(g_free));
		g_object_set_data(G_OBJECT(text), "data", phrdt);
		ibus_lookup_table_append_candidate(lktable, text);
		g_object_unref(text);
		tlist = g_slist_next(tlist);
	}
	g_slist_free(phrlist);
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
		if (phrdt->offset >= 1)
			ibus_text_append_attribute(text, IBUS_ATTR_TYPE_FOREGROUND,
							 0x0000ff, 0, phrdt->dtlen);
		g_object_set_data_full(G_OBJECT(text), "text", textdt,
						 GDestroyNotify(g_free));
		g_object_set_data(G_OBJECT(text), "data", phrdt);
		ibus_lookup_table_append_candidate(lktable, text);
		g_object_unref(text);
		tlist = g_slist_next(tlist);
	}
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

	if (!(text = ibus_lookup_table_get_candidate(lktable, index)))
		return;
	phrdt = (PhraseData *)g_object_get_data(G_OBJECT(text), "data");
	pyedit->SelectCachePhrase(phrdt);
	if (pyedit->IsFinishInquirePhrase()) {
		CommitPhrase();
		ClearEngineUI();
	} else
		UpdateEngineUI();
}

/**
 * 删除候选词语.
 * @param index 索引值
 */
void PinyinEngine::DeleteCandidatePhrase(guint index)
{
	IBusText *text;
	PhraseData *phrdt;
	char *textdt;
	gunichar2 *data;
	glong length;

	if (!(text = ibus_lookup_table_get_candidate(lktable, index)))
		return;

	/* 删除词语并更新词语查询表 */
	phrdt = (PhraseData *)g_object_get_data(G_OBJECT(text), "data");
	if (phrdt->offset <= 0)
		return;
	g_array_remove_index(lktable->candidates, index);
	g_object_unref(text);
	pyedit->DeletePhraseData(phrdt);
	AppendPageCandidate();
	ibus_engine_update_lookup_table_fast(busengine, lktable, TRUE);
	if (index != 0)
		return;

	/* 更新辅助文本 */
	pyedit->GetAuxiliaryText(&data, &length);
	if (length != 0) {
		textdt = g_utf16_to_utf8(data, length, NULL, NULL, NULL);
		g_free(data);
		text = ibus_text_new_from_static_string(textdt);
		g_object_set_data_full(G_OBJECT(text), "text", textdt,
				       GDestroyNotify(g_free));
		ibus_engine_update_auxiliary_text(busengine, text, TRUE);
		g_object_unref(text);
	} else
		ibus_engine_hide_auxiliary_text(busengine);

	/* 更新预编辑文本 */
	pyedit->GetPreeditText(&data, &length);
	if (length != 0) {
		textdt = g_utf16_to_utf8(data, length, NULL, NULL, NULL);
		g_free(data);
		text = ibus_text_new_from_static_string(textdt);
		g_object_set_data_full(G_OBJECT(text), "text", textdt,
				       GDestroyNotify(g_free));
		ibus_engine_update_preedit_text(busengine, text, length, TRUE);
		g_object_unref(text);
	} else
		ibus_engine_hide_preedit_text(busengine);
}

/**
 * 提交词语.
 */
void PinyinEngine::CommitPhrase()
{
	IBusText *text;
	char *textdt;
	gunichar2 *data;
	glong length;
	off_t offset;

	/* 提交词语数据 */
	pyedit->GetCommitText(&data, &length);
	if (length != 0) {
		/* 向UI提交词语 */
		textdt = g_utf16_to_utf8(data, length, NULL, NULL, NULL);
		g_free(data);
		text = ibus_text_new_from_static_string(textdt);
		g_object_set_data_full(G_OBJECT(text), "text", textdt,
						 GDestroyNotify(g_free));
		ibus_engine_commit_text(busengine, text);
		g_object_unref(text);
		/* 反馈词语 */
		offset = pyedit->GetPhraseOffset();
		if ((offset == -3 && FLAG_ISSET(flags, 2))
			 || (offset == -2 && FLAG_ISSET(flags, 1))
			 || ((offset == -1 || offset > 0) && FLAG_ISSET(flags, 0)))
			pyedit->FeedbackSelectedPhrase();
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
		ibus_engine_commit_text(busengine, text);
		g_object_unref(text);
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
	ibus_engine_commit_text(busengine, text);
	g_object_unref(text);
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
	ibus_engine_commit_text(busengine, text);
	g_object_unref(text);
}

/**
 * 提交最终字符.
 * @param ch 字符
 */
void PinyinEngine::CommitFinalChars(gunichar ch)
{
	IBusText *text;

	text = ibus_text_new_from_unichar(ch);
	ibus_engine_commit_text(busengine, text);
	g_object_unref(text);
}

/**
 * 提交串.
 * @param str 串
 */
void PinyinEngine::CommitString(const gchar *str)
{
	IBusText *text;

	text = ibus_text_new_from_string(str);
	ibus_engine_commit_text(busengine, text);
	g_object_unref(text);
}

/**
 * 提交静态串.
 * @param str 串
 */
void PinyinEngine::CommitStaticString(const gchar *str)
{
	IBusText *text;

	text = ibus_text_new_from_static_string(str);
	ibus_engine_commit_text(busengine, text);
	g_object_unref(text);
}
