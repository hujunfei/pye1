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
extern PhraseEngine phregn;

PinyinEngine::PinyinEngine(IBusEngine *engine):busegn(engine), pyedit(NULL),
 chmode(true), flmode(false), fpmode(true), squote(false), dquote(false)
{
	IBusProperty *property;

	/* 创建拼音编辑器 */
	pyedit = new PinyinEditor(&phregn);

	/* 创建属性部件表 */
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
	ibus_prop_list_append(&props, property);

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
	ibus_prop_list_append(&props, property);

	property = ibus_property_new("mode.full_punct",
			 PROP_TYPE_NORMAL,
			 ibus_text_new_from_static_string(fpmode ? "，。" : ",."),
			 fpmode ? PKGDATADIR"/icons/full-punct.svg" :
				 PKGDATADIR"/icons/half-punct.svg",
			 ibus_text_new_from_static_string("Full/Half width punctuation"),
			 TRUE,
			 TRUE,
			 PROP_STATE_UNCHECKED,
			 NULL);
	ibus_prop_list_append(&props, property);

	property = ibus_property_new("setup",
			 PROP_TYPE_NORMAL,
			 ibus_text_new_from_static_string("Pinyin preferences"),
			 "gtk-preferences",
			 ibus_text_new_from_static_string("Pinyin preferences"),
			 TRUE,
			 TRUE,
			 PROP_STATE_UNCHECKED,
			 NULL);
	ibus_prop_list_append(&props, property);
}

PinyinEngine::~PinyinEngine()
{
	delete pyedit;
}

void PinyinEngine::EngineDisable()
{
}

void PinyinEngine::EngineEnable()
{
}

void PinyinEngine::FocusIn()
{
}

void PinyinEngine::FocusOut()
{
}

void PinyinEngine::CursorDown()
{
}

void PinyinEngine::CursorUp()
{
}

void PinyinEngine::PageDown()
{
}

void PinyinEngine::PageUp()
{
}

void PinyinEngine::PropertyActivate(const gchar *prop_name, guint prop_state)
{
}

void PinyinEngine::CandidateClicked(guint index, guint button, guint state)
{
}

gboolean PinyinEngine::ProcessKeyEvent(guint keyval, guint keycode, guint state)
{
	return TRUE;
}
