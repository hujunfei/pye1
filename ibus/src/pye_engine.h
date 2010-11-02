//
// C++ Interface: pye_engine
//
// Description:
// Pye引擎的IBus封装.
//
// Author: Jally <jallyx@163.com>, (C) 2009, 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IBUS_PYE_PYE_ENGINE_H_
#define IBUS_PYE_PYE_ENGINE_H_

#include <ibus.h>
#include <phrase_manager.h>
#include <pinyin_editor.h>

class PyeEngine {
 public:
  PyeEngine(IBusEngine *engine, PhraseManager *manager);
  virtual ~PyeEngine();

  virtual gboolean processKeyEvent(guint keyval, guint keycode, guint modifiers);
  virtual void reset();
  virtual void enable();
  virtual void disable();
  virtual void focusIn();
  virtual void focusOut();
  virtual void pageUp();
  virtual void pageDown();
  virtual void cursorUp();
  virtual void cursorDown();
  virtual void candidateClicked(guint index, guint button, guint state);
  virtual void propertyActivate(const gchar *prop_name, guint prop_state);

  virtual void updateConfig();

private:
  void updateUIData();
  void resetUIData();
  void hideUI();
  void resetEngine();

  void appendEnginePhrase();
  void appendDynamicPhrase();
  void appendPageCandidate();

  void selectCandidatePhrase(guint index);
  void deleteCandidatePhrase(guint index);

  void commitPhrase();
  void commitRawPhrase();
  void commitLetter(gunichar ch);
  void commitPunct(gunichar ch);
  void commitFinalChars(gunichar ch);
  void commitString(const gchar *str);
  void commitStaticString(const gchar *str);

  IBusPropList *createPropList();
  void resetStatus();

  void toggleModeChinese();
  void toggleModeFullLetter();
  void toggleModeFullPunct();
  void showSetupDialog();

  gboolean processLetter(guint keyval, guint keycode, guint state);
  gboolean processCapital(guint keyval, guint keycode, guint state);
  gboolean processNumber(guint keyval, guint keycode, guint state);
  gboolean processPunct(guint keyval, guint keycode, guint state);
  gboolean processSpace(guint keyval, guint keycode, guint state);
  gboolean processOthers(guint keyval, guint keycode, guint state);

  PinyinEditor pinyin_editor_;  ///< 拼音编辑器
  IBusEngine *engine_;  ///< IBus引擎接口

  IBusLookupTable *lookup_table_;  ///< 词语查询表
  IBusPropList *prop_list_;  ///< 属性部件表

  guint pre_keyval_;  ///< 前一个键值
  bool tmp_english_mode_;  ///< 临时英文模式
  bool chinese_mode_;  ///< 中文输入模式
  bool full_letter_mode_;  ///< 字母全角模式
  bool full_punct_mode_;  ///< 标点全角模式
  bool single_quote_area_;  ///< 单引号标记
  bool double_quote_area_;  ///< 双引号标记
};

#endif  // IBUS_PYE_PYE_ENGINE_H_
