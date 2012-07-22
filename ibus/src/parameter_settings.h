//
// C++ Interface: parameter_settings
//
// Description:
// 引擎运行数据配置程序.
//
// Author: Jally <jallyx@163.com>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IBUS_PYE_PARAMETER_SETTINGS_H_
#define IBUS_PYE_PARAMETER_SETTINGS_H_

#include <gtk/gtk.h>
#include <ibus.h>

class ParameterSettings {
 public:
  ParameterSettings();
  ~ParameterSettings();

  void entry(GDBusConnection *conn);

 private:
  GtkWidget *createMainWindow();
  GtkWidget *createRoutine();
  GtkWidget *createFuzzy();
  GtkWidget *createMend();

  void setRoutineValue();
  void setFuzzyValue();
  void setMendValue();

  void extractRoutineValue();
  void extractFuzzyValue();
  void extractMendValue();

  void updateConfig();

  IBusConfig *config_;  ///< IBUS配置引擎
  GData *widget_set_;  ///< 窗体集
  GData *data_set_;  ///< 数据集

  static const gchar *fuzzy_strv_[][2];  ///< 模糊拼音表
  static const gchar *mend_strv_[][2];  ///< 拼音修正表
};

#endif  // IBUS_PYE_PARAMETER_SETTINGS_H_
