//
// C++ Interface: PinyinEngine
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef __SRC_PINYINENGINE_H
#define __SRC_PINYINENGINE_H

#include "engine/PinyinEditor.h"

class PinyinEngine
{
public:
	PinyinEngine(IBusEngine *busegn);
	~PinyinEngine();

	void UpdateConfig();
	void EngineReset();
	void EngineDisable();
	void EngineEnable();
	void FocusIn();
	void FocusOut();
	void CursorDown();
	void CursorUp();
	void PageDown();
	void PageUp();
	void PropertyActivate(const gchar *prop_name, guint prop_state);
	void CandidateClicked(guint index, guint button, guint state);
	gboolean ProcessKeyEvent(guint keyval, guint keycode, guint state);
private:
	IBusPropList *CreateProperty();
	void RestoreInitState();

	void ToggleModeChinese();
	void ToggleModeFullLetter();
	void ToggleModeFullPunct();
	void ShowSetupDialog();

	gboolean ProcessPinyin(guint keyval, guint keycode, guint state);
	gboolean ProcessCapitalLetter(guint keyval, guint keycode, guint state);
	gboolean ProcessNumber(guint keyval, guint keycode, guint state);
	gboolean ProcessSpace(guint keyval, guint keycode, guint state);
	gboolean ProcessPunct(guint keyval, guint keycode, guint state);
	gboolean ProcessOthers(guint keyval, guint keycode, guint state);

	void UpdateEngineUI();
	void ShowEngineUI();
	void HideEngineUI();
	void ClearEngineUI();
	void AppendComposePhrase();
	void AppendDynamicPhrase();
	void AppendPageCandidate();
	void SelectCandidatePhrase(guint index);
	void DeleteCandidatePhrase(guint index);
	void CommitPhrase();
	void CommitRawPhrase();
	void CommitLetter(gunichar ch);
	void CommitPunct(gunichar ch);
	void CommitFinalChars(gunichar ch);
	void CommitString(const gchar *str);
	void CommitStaticString(const gchar *str);

	IBusEngine *busengine;	///< IBUS输入法引擎基类
	PinyinEditor *pyedit;	///< 拼音编辑器
	IBusLookupTable *lktable;	///< 词语查询表
	IBusPropList *props;	///< 属性部件表

	guint pagesize;		///< 词语查询表的页面大小
	uint8_t flags;		///< @see EngineConfig::flags

	guint prekey;		///< 前一个键值
	bool entmpmode;		///< 临时英文编辑模式
	bool chmode;		///< 汉语模式
	bool flmode;		///< 字母全角模式
	bool fpmode;		///< 标点全角模式
	bool squote;		///< 单引号标记
	bool dquote;		///< 双引号标记
};

#endif
