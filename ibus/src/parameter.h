//
// C++ Interface: parameter
//
// Description:
// 程序参数类.
//
// Author: Jally <jallyx@163.com>, (C) 2009, 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IBUS_PYE_PARAMETER_H_
#define IBUS_PYE_PARAMETER_H_

#include <list>
#include <ibus.h>

class PyeEngine;

/**
 * 程序参数类.
 */
class Parameter {
 public:
  static Parameter *getInstance();

  void addListener(PyeEngine *engine);
  void removeListener(PyeEngine *engine);

  void setConnection(IBusConnection *conn);

  guint pagesize_;  ///< 词语查询表的页面大小
  gboolean cursor_visible_;  ///< 光标可见
  gboolean phrase_frequency_adjustable_;  ///< 调整词频
  gboolean engine_phrase_savable_;  ///< 保存引擎合成词语
  gboolean manual_phrase_savable_;  ///< 保存用户合成词语

 private:
  Parameter();
  ~Parameter();

  void updatePagesize(GValue *value);
  void updateCursorVisible(GValue *value);
  void updatePhraseFrequencyAdjustable(GValue *value);
  void updateEnginePhraseSavable(GValue *value);
  void updateManualPhraseSavable(GValue *value);

  void updateMendPinyinPair(GValue *value);
  void updateFuzzyPinyinPair(GValue *value);
  void updateBackupCycle(GValue *value);

  void notifyListener();
  void updateMendPinyinPair();
  void updateFuzzyPinyinPair();

  static void configChanged(Parameter *object, gchar *section,
                            gchar *name, GValue *value);
  static gboolean backupData();

  IBusConfig *config_;  ///< IBUS配置接口
  std::list<PyeEngine *> engine_list_;  ///< 引擎链表
  guint timer_id_;  ///< 定时器ID

  gchar *mend_data_;  ///< 拼音矫正串
  gchar *fuzzy_data_;  ///< 模糊拼音串
  guint backup_cycle_;  ///< 备份用户词语
};

#endif  // IBUS_PYE_PARAMETER_H_
