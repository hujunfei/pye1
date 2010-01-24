//
// C++ Interface: DataSettings
//
// Description:
// ibus-engine-pye的运行数据配置程序
//
// Author: Jally <jallyx@163.com>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef __SRC_DATASETTINGS_H
#define __SRC_DATASETTINGS_H

#include "engine/deplib.h"

class DataSettings
{
public:
	DataSettings();
	~DataSettings();

	void ResetDataEntry(IBusConnection *conn);
private:
	GtkWidget *CreateMainWindow();
	GtkWidget *CreateRoutine();
	GtkWidget *CreateFuzzy();
	GtkWidget *CreateRectify();
	void SetRoutineValue();
	void SetFuzzyValue();
	void SetRectifyValue();
	void ExtractRoutineValue();
	void ExtractFuzzyValue();
	void ExtractRectifyValue();
	void UpdateConfigData();

	IBusConfig *busconfig;	///< IBUS配置引擎
	GData *widset;	///< 窗体集
	GData *dtset;	///< 数据集

	static const gchar *fuzzystrv[][2];	///< 模糊拼音表
	static const gchar *rectifystrv[][2];	///< 拼音修正表
};

#endif
