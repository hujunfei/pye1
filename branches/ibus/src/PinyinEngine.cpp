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

PinyinEngine::PinyinEngine(IBusEngine *egn):engine(egn), pyedit(NULL),
 lktable(NULL), props(NULL), prekey(IBUS_VoidSymbol), chmode(true),
 flmode(false), fpmode(true), squote(false), dquote(false)
{
	IBusProperty *property;

	/* 创建拼音编辑器 */
	pyedit = new PinyinEditor(&phregn);

	/* 创建词语查询表 */
	lktable = ibus_lookup_table_new(config.GetPageSize(), 0, TRUE, FALSE);

	/* 创建属性部件表 */
	props = ibus_prop_list_new();
	/*/* 中英文 */
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
	/*/* 字母全/半角 */
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
	/*/* 标点全/半角 */
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
	/*/* 细节设置 */
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
}

PinyinEngine::~PinyinEngine()
{
	delete pyedit;
	g_object_unref(lktable);
	g_object_unref(props);
}

void PinyinEngine::EngineDisable()
{
	pyedit->FinishInquirePhrase();
}

void PinyinEngine::EngineEnable()
{
	pyedit->FinishInquirePhrase();
}

void PinyinEngine::FocusIn()
{
	ibus_engine_register_properties(engine, props);
}

void PinyinEngine::FocusOut()
{
}

void PinyinEngine::CursorDown()
{
	ibus_lookup_table_cursor_down(lktable);
	ibus_engine_update_lookup_table_fast(engine, lktable, TRUE);
}

void PinyinEngine::CursorUp()
{
	ibus_lookup_table_cursor_up(lktable);
	ibus_engine_update_lookup_table_fast(engine, lktable, TRUE);
}

void PinyinEngine::PageDown()
{
	ibus_lookup_table_page_down(lktable);
	ibus_engine_update_lookup_table_fast(engine, lktable, TRUE);
}

void PinyinEngine::PageUp()
{
	ibus_lookup_table_page_up(lktable);
	ibus_engine_update_lookup_table_fast(engine, lktable, TRUE);
}

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

void PinyinEngine::CandidateClicked(guint index, guint button, guint state)
{
	IBusText *text;
	PhraseData *phrdt;

	text = ibus_lookup_table_get_candidate(lktable, index);
	phrdt = (PhraseData *)g_object_get_data(G_OBJECT(text), "data-pointer");
}

gboolean PinyinEngine::ProcessKeyEvent(guint keyval, guint keycode, guint state)
{
	guint prekeyval;

	/* 记录前一个键值 */
	prekeyval = prekey;
	prekey = keyval;

	/* 对释放键的处理 */
	if (state & IBUS_RELEASE_MASK) {
		if (prekeyval != keyval)
			return FALSE;
		switch (keyval) {
		case IBUS_Shift_L:
		case IBUS_Shift_R:
			if (pyedit->IsFinishInquirePhrase())
				ToggleModeChinese();
			return TRUE;
		default:
			return FALSE;
		}
	}

// 	modifiers &= (IBUS_SHIFT_MASK |
// 			IBUS_CONTROL_MASK |
// 			IBUS_MOD1_MASK |
// 			IBUS_SUPER_MASK |
// 			IBUS_HYPER_MASK |
// 			IBUS_META_MASK |
// 			IBUS_LOCK_MASK);
//
// 	switch (keyval) {
// 		/* letters */
// 		case IBUS_a ... IBUS_z:
// 			retval = processPinyin (keyval, keycode, modifiers);
// 			break;
// 		case IBUS_A ... IBUS_Z:
// 			retval = processCapitalLetter (keyval, keycode, modifiers);
// 			break;
// 			/* numbers */
// 		case IBUS_0 ... IBUS_9:
// 		case IBUS_KP_0 ... IBUS_KP_9:
// 			retval = processNumber (keyval, keycode, modifiers);
// 			break;
// 			/* punct */
// 		case IBUS_exclam ... IBUS_slash:
// 		case IBUS_colon ... IBUS_at:
// 		case IBUS_bracketleft ... IBUS_quoteleft:
// 		case IBUS_braceleft ... IBUS_asciitilde:
// 			retval = processPunct (keyval, keycode, modifiers);
// 			break;
// 			/* space */
// 		case IBUS_space:
// 			retval = processSpace (keyval, keycode, modifiers);
// 			break;
// 			/* others */
// 		default:
// 			retval = processOthers (keyval, keycode, modifiers);
// 			break;
// 	}
//
// 	m_prev_pressed_key = retval ? 0 : keyval;
//
// 	return TRUE;
}

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
	if (chmode && !fpmode || !chmode && fpmode)
		ToggleModeFullPunct();
}

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

void PinyinEngine::ShowSetupDialog ()
{
	g_spawn_command_line_async(LIBEXECDIR "/ibus-setup-pye", NULL);
}

gboolean PinyinEngine::ProcessPinyin(guint keyval, guint keycode, guint state)
{
	IBusText *text;
	gunichar ch;

	/* 如果有修饰键则不处理 */
	if (G_UNLIKELY(CMSHM_FILTER(state) != 0))
		return FALSE;

	/* 如果不在汉语模式下 */
	if (G_UNLIKELY(!chmode)) {
		ch = flmode ? HalfFullConverter::ToFull(keyval) : keyval;
		text = ibus_text_new_from_unichar(ch);
		ibus_engine_commit_text(engine, text);
		return TRUE;
	}

	/* 让拼音编辑器搞定 */
	pyedit->InsertPinyinKey(keyval);
	//TODO

	return TRUE;
}

gboolean PinyinEngine::ProcessCapitalLetter(guint keyval, guint keycode, guint state)
{
	IBusText *text;
	gunichar ch;

	/* 如果有修饰键则不处理 */
	if (G_UNLIKELY(CMSHM_FILTER(state) != 0))
		return FALSE;

	/* 如果被(shift)键修饰 */
	if (state & IBUS_SHIFT_MASK)
		return ProcessPinyin(keyval, keycode, state);

	if (m_mode_chinese && ! isEmpty ()) {
		if (!Config::autoCommit ())
			return TRUE;
		if (m_phrase_editor.pinyinExistsAfterCursor ()) {
			selectCandidate (m_lookup_table.cursorPos ());
		}
		commit ();
	}

	ch = flmode ? HalfFullConverter::ToFull(keyval) : keyval;
	text = ibus_text_new_from_unichar(ch);
	ibus_engine_commit_text(engine, text);

	return TRUE;
}

gboolean PinyinEngine::processNumber(guint keyval, guint keycode, guint state)
{
	/* English mode */
	if (G_UNLIKELY (!m_mode_chinese)) {
		commit ((gunichar) m_mode_full ? HalfFullConverter::toFull (keyval) : keyval);
		return TRUE;
	}

	/* Chinese mode, if empty */
	if (G_UNLIKELY (isEmpty ())) {
		if (G_UNLIKELY (CMSHM_FILTER (modifiers) != 0))
			return FALSE;
		switch (keyval) {
			case IBUS_0 ... IBUS_9:
				commit ((gunichar) m_mode_full ? HalfFullConverter::toFull (keyval) : keyval);
				break;
			case IBUS_KP_0 ... IBUS_KP_9:
				commit ((gunichar) m_mode_full ? HalfFullConverter::toFull ('0' + keyval - IBUS_KP_0) : '0' + keyval - IBUS_KP_0);
				break;
		}
		return TRUE;
	}

	/* Chinese mode, if has candidates */
	guint i;
	switch (keyval) {
		case IBUS_0:
		case IBUS_KP_0:
			i = 10;
			break;
		case IBUS_1 ... IBUS_9:
			i = keyval - IBUS_1;
			break;
		case IBUS_KP_1 ... IBUS_KP_9:
			i = keyval - IBUS_KP_1;
			break;
		default:
			g_assert_not_reached ();
	}

	if (modifiers == 0)
		selectCandidateInPage (i);
	else if ((modifiers & ~ IBUS_LOCK_MASK) == IBUS_CONTROL_MASK)
		resetCandidateInPage (i);
	return TRUE;
}

gboolean PinyinEngine::ProcessSpace(guint keyval, guint keycode, guint state)
{
	if (CMSHM_FILTER (modifiers) != 0)
		return FALSE;

	if (G_UNLIKELY (modifiers & IBUS_SHIFT_MASK)) {
		toggleModeFull ();
		return TRUE;
	}

	/* Chinese mode */
	if (G_UNLIKELY (m_mode_chinese && !isEmpty ())) {
		if (m_phrase_editor.pinyinExistsAfterCursor ()) {
			selectCandidate (m_lookup_table.cursorPos ());
		}
		else {
			commit ();
		}
	}
	else {
		commit (m_mode_full ? "　" : " ");
	}
	return TRUE;
}

gboolean PinyinEngine::ProcessPunct(guint keyval, guint keycode, guint state)
{
	guint cmshm_modifiers = CMSHM_FILTER (modifiers);

	if (G_UNLIKELY (keyval == IBUS_period && cmshm_modifiers == IBUS_CONTROL_MASK)) {
		toggleModeFullPunct ();
		return TRUE;
	}

	/* check ctrl, alt, hyper, supper masks */
	if (cmshm_modifiers != 0)
		return FALSE;

	/* English mode */
	if (G_UNLIKELY (!m_mode_chinese)) {
		if (G_UNLIKELY (m_mode_full))
			commit (HalfFullConverter::toFull (keyval));
		else
			commit (keyval);
		return TRUE;
	}

	/* Chinese mode */
	if (G_UNLIKELY (!isEmpty ())) {
		switch (keyval) {
			case IBUS_apostrophe:
				return processPinyin (keyval, keycode, modifiers);
			case IBUS_comma:
				if (Config::commaPeriodPage ()) {
					pageUp ();
					return TRUE;
				}
				break;
			case IBUS_minus:
				if (Config::minusEqualPage ()) {
					pageUp ();
					return TRUE;
				}
				break;
			case IBUS_period:
				if (Config::commaPeriodPage ()) {
					pageDown ();
					return TRUE;
				}
				break;
			case IBUS_equal:
				if (Config::minusEqualPage ()) {
					pageDown ();
					return TRUE;
				}
				break;
			case IBUS_semicolon:
				if (G_UNLIKELY (Config::doublePinyin ())) {
					/* double pinyin need process ';' */
					if (processPinyin (keyval, keycode, modifiers))
						return TRUE;
				}
				break;
		}

		if (G_LIKELY (!Config::autoCommit ()))
			return TRUE;

		if (m_phrase_editor.pinyinExistsAfterCursor ()) {
			selectCandidate (m_lookup_table.cursorPos ());
		}
		commit ();
	}

	g_assert (isEmpty ());

	if (m_mode_full_punct) {
		switch (keyval) {
			case '`':
				commit ("·"); return TRUE;
			case '~':
				commit ("～"); return TRUE;
			case '!':
				commit ("！"); return TRUE;
        // case '@':
        // case '#':
			case '$':
				commit ("￥"); return TRUE;
        // case '%':
			case '^':
				commit ("……"); return TRUE;
        // case '&':
        // case '*':
			case '(':
				commit ("（"); return TRUE;
			case ')':
				commit ("）"); return TRUE;
        // case '-':
			case '_':
				commit ("——"); return TRUE;
        // case '=':
        // case '+':
			case '[':
				commit ("【"); return TRUE;
			case ']':
				commit ("】"); return TRUE;
			case '{':
				commit ("『"); return TRUE;
			case '}':
				commit ("』"); return TRUE;
			case '\\':
				commit ("、"); return TRUE;
        // case '|':
			case ';':
				commit ("；"); return TRUE;
			case ':':
				commit ("："); return TRUE;
			case '\'':
				commit (m_quote ? "‘" : "’");
				m_quote = !m_quote;
				return TRUE;
			case '"':
				commit (m_double_quote ? "“" : "”");
				m_double_quote = !m_double_quote;
				return TRUE;
			case ',':
				commit ("，"); return TRUE;
			case '.':
				if (m_prev_commited_char >= '0' && m_prev_commited_char <= '9')
					commit (keyval);
				else
					commit ("。");
				return TRUE;
			case '<':
				commit ("《"); return TRUE;
			case '>':
				commit ("》"); return TRUE;
        // case '/':
			case '?':
				commit ("？"); return TRUE;
		}
	}

	commit (m_mode_full ? HalfFullConverter::toFull (keyval) : keyval);
	return TRUE;
}

gboolean PinyinEngine::processOthers (guint keyval, guint keycode, guint modifiers)
{
	if (G_UNLIKELY (isEmpty ()))
		return FALSE;

	/* ignore numlock */
	modifiers &= ~IBUS_MOD2_MASK;

	/* process some cursor control keys */
	gboolean _update = FALSE;
	switch (keyval) {
		case IBUS_Shift_L:
			if (Config::shiftSelectCandidate () &&
						 m_mode_chinese) {
				selectCandidateInPage (1);
						 }
						 break;

		case IBUS_Shift_R:
			if (Config::shiftSelectCandidate () &&
						 m_mode_chinese) {
				selectCandidateInPage (2);
						 }
						 break;

		case IBUS_Return:
		case IBUS_KP_Enter:
			commit ();
			break;

		case IBUS_BackSpace:
			if (G_LIKELY (modifiers == 0))
				_update = m_pinyin_editor->removeCharBefore ();
			else if (G_LIKELY (modifiers == IBUS_CONTROL_MASK))
				_update = m_pinyin_editor->removeWordBefore ();
			break;

		case IBUS_Delete:
		case IBUS_KP_Delete:
			if (G_LIKELY (modifiers == 0))
				_update = m_pinyin_editor->removeCharAfter ();
			else if (G_LIKELY (modifiers == IBUS_CONTROL_MASK))
				_update = m_pinyin_editor->removeWordAfter ();
			break;

		case IBUS_Left:
		case IBUS_KP_Left:
			if (G_LIKELY (modifiers == 0)) {
            // move left single char
				_update = m_pinyin_editor->moveCursorLeft ();
			}
			else if (G_LIKELY (modifiers == IBUS_CONTROL_MASK)) {
            // move left one pinyin
				_update = m_pinyin_editor->moveCursorLeftByWord ();
			}
			break;

		case IBUS_Right:
		case IBUS_KP_Right:
			if (G_LIKELY (modifiers == 0)) {
            // move right single char
				_update = m_pinyin_editor->moveCursorRight ();
			}
			else if (G_LIKELY (modifiers == IBUS_CONTROL_MASK)) {
            // move right to end
				_update = m_pinyin_editor->moveCursorToEnd ();
			}
			break;

		case IBUS_Home:
		case IBUS_KP_Home:
			if (G_LIKELY (modifiers == 0)) {
            // move to begin
				_update = m_pinyin_editor->moveCursorToBegin ();
			}
			break;

		case IBUS_End:
		case IBUS_KP_End:
			if (G_LIKELY (modifiers == 0)) {
            // move to end
				_update = m_pinyin_editor->moveCursorToEnd ();
			}
			break;

		case IBUS_Up:
		case IBUS_KP_Up:
			cursorUp (); break;
		case IBUS_Down:
		case IBUS_KP_Down:
			cursorDown (); break;
		case IBUS_Page_Up:
		case IBUS_KP_Page_Up:
			pageUp (); break;
		case IBUS_Page_Down:
		case IBUS_KP_Page_Down:
			pageDown (); break;
		case IBUS_Escape:
			reset (); break;
	}
	if (G_LIKELY (_update)) {
		updatePhraseEditor ();
		updateUI (FALSE);
	}
	return TRUE;
}

void PinyinEngine::UpdateEngineUI()
{
	IBusText *text;
	gunichar2 *data;
	glong length;


}

void CommitPhrase()
{
}

void CommitWord(gunchar ch)
{
}
