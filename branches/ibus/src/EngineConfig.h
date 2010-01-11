//
// C++ Interface: EngineConfig
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef __SRC_ENGINECONFIG_H
#define __SRC_ENGINECONFIG_H

#include "PinyinEngine.h"

class EngineConfig
{
private:
	EngineConfig();
	~EngineConfig();
public:
	static EngineConfig *GetInstance();

	void AddListener(PinyinEngine *pyegn);
	void RemoveListener(PinyinEngine *pyegn);
	void SetConnection(IBusConnection *conn);
	guint GetPageSize();
	uint8_t GetBitFlags();
private:
	void UpdateRectifyPinyinPair();
	void UpdateFuzzyPinyinUnit();
	void UpdateBackupGap();
	void UpdatePageSize();
	void UpdateBitFlags();

	void NotifyListener();
	void UpdatePhraseEngineRectifyPinyinPair();
	void UpdatePhraseEngineFuzzyPinyinUnit();

	IBusConfig *busconfig;	///< IBUS配置引擎
	GSList *pyegnlist;	///< 拼音引擎链表
	guint timerid;	///< 定时器ID

	gchar **rtfstrv;	///< 拼音矫正串表
	gchar **fzstrv;	///< 模糊拼音对照串表
	time_t bakgap;	///< 备份用户词语的间隔
	guint pagesize;	///< 词语查询表的页面大小
	uint8_t flags;	///< 3 光标可见,2 合成词汇,1 定义词汇,0 词频调整
private:
	static void ConfigDataChanged(EngineConfig *config, gchar *section,
						 gchar *name, GValue *value);
	static gboolean BakUserPhraseData();
private:
	friend class std::auto_ptr<EngineConfig>;
	static std::auto_ptr<EngineConfig> instance;
};

#endif
