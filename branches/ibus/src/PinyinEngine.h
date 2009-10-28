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
#ifndef PINYINENGINE_H
#define PINYINENGINE_H

#include "engine/PinyinEditor.h"
#include "include/dep.h"

class PinyinEngine
{
public:
	PinyinEngine(IBusEngine *engine);
	~PinyinEngine();

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
	IBusEngine *busegn;	///< 引擎基类
	PinyinEditor *pyedit;	///< 拼音编辑器
	IBusLookupTable lktable;	///< 词语查询表
	IBusPropList props;	///< 属性部件表

	bool chmode;		///< 汉语模式
	bool flmode;		///< 字母全角模式
	bool fpmode;		///< 标点全角模式
	bool squote;		///< 单引号标记
	bool dquote;		///< 双引号标记
};

#endif
