//
// C++ Implementation: EngineConfig
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "EngineConfig.h"

#define CONFIG_SECTION "engine/Pye"
#define CONFIG_NAME_RECTIFY "rectify_pinyin_pair"
#define CONFIG_NAME_FUZZY "fuzzy_pinyin_unit"
#define CONFIG_NAME_BAKGAP "backup_gap"
#define CONFIG_NAME_PAGE "page_size"
#define CONFIG_NAME_FLAGS "bit_flags"
#define CONFIG_NAME_EOF "--EOF--"

/**
 * 类构造函数.
 */
EngineConfig::EngineConfig(): busconfig(NULL), pyegnlist(NULL),
 timerid(0), rtfstr(NULL), fzstr(NULL), bakgap(60), pagesize(5),
 flags(~0)
{
}

/**
 * 类析构函数.
 */
EngineConfig::~EngineConfig()
{
	if (busconfig)
		g_object_unref(busconfig);
	g_slist_free(pyegnlist);
	if (timerid != 0)
		g_source_remove(timerid);
	g_free(rtfstr);
	g_free(fzstr);
}

/**
 * 获取本类的实例对象指针.
 * @return 实例对象
 */
std::auto_ptr<EngineConfig> EngineConfig::instance;
EngineConfig *EngineConfig::GetInstance()
{
	if (!instance.get())
		instance.reset(new EngineConfig);
	return instance.get();
}

/**
 * 添加数据监听者.
 * @param pyegn 数据监听者
 */
void EngineConfig::AddListener(PinyinEngine *pyegn)
{
	pyegnlist = g_slist_prepend(pyegnlist, pyegn);
}

/**
 * 移除数据监听者.
 * @param pyegn 数据监听者
 */
void EngineConfig::RemoveListener(PinyinEngine *pyegn)
{
	pyegnlist = g_slist_remove(pyegnlist, pyegn);
}

/**
 * 设置IBUS总线连接，并更新相关配置数据.
 * @param conn 总线连接
 */
void EngineConfig::SetConnection(IBusConnection *conn)
{
	/* 新建IBUS配置引擎 */
	if (busconfig)
		g_object_unref(busconfig);
	busconfig = ibus_config_new(conn);
	g_signal_connect_swapped(busconfig, "value-changed",
			 G_CALLBACK(ConfigDataChanged), this);

	/* 更新配置数据 */
	UpdateRectifyPinyinPair(NULL);
	UpdateFuzzyPinyinUnit(NULL);
	UpdateBackupGap(NULL);
	UpdatePageSize(NULL);
	UpdateBitFlags(NULL);

	/* 通知数据监听者 */
	NotifyListener();
}

/**
 * 获取词语查询表的页面大小
 * @return 页面大小
 */
guint EngineConfig::GetPageSize()
{
	return pagesize;
}

/**
 * 获取相关位标记.
 * @return 位标记
 */
guint8 EngineConfig::GetBitFlags()
{
	return flags;
}

/**
 * 更新拼音修正对.
 * @param value 最新拼音修正对数据
 * @note 如果(value==NULL)则表明数据需要临时获取
 */
void EngineConfig::UpdateRectifyPinyinPair(GValue *value)
{
	GValue localvalue = {0};
	GValue *pvalue;

	if (value || ibus_config_get_value(busconfig, CONFIG_SECTION,
				 CONFIG_NAME_RECTIFY, &localvalue)) {
		pvalue = value ? value : &localvalue;
		g_free(rtfstr);
		rtfstr = g_value_dup_string(pvalue);
		UpdatePhraseEngineRectifyPinyinPair();
		if (!value)
			g_value_unset(&localvalue);
	}

}

/**
 * 更新模糊拼音单元.
 * @param value 最新模糊拼音单元数据
 * @note 如果(value==NULL)则表明数据需要临时获取
 */
void EngineConfig::UpdateFuzzyPinyinUnit(GValue *value)
{
	GValue localvalue = {0};
	GValue *pvalue;

	if (value || ibus_config_get_value(busconfig, CONFIG_SECTION,
				 CONFIG_NAME_FUZZY, &localvalue)) {
		pvalue = value ? value : &localvalue;
		g_free(fzstr);
		fzstr = g_value_dup_string(pvalue);
		UpdatePhraseEngineFuzzyPinyinUnit();
		if (!value)
			g_value_unset(&localvalue);
	}
}

/**
 * 更新备份用户词语的时间间隔.
 * @param value 最新备份时间间隔数据
 * @note 如果(value==NULL)则表明数据需要临时获取
 */
void EngineConfig::UpdateBackupGap(GValue *value)
{
	GValue localvalue = {0};
	GValue *pvalue;

	if (value || ibus_config_get_value(busconfig, CONFIG_SECTION,
				 CONFIG_NAME_BAKGAP, &localvalue)) {
		pvalue = value ? value : &localvalue;
		if ((bakgap = g_value_get_int(pvalue)) < 30)
			bakgap = 30;
		if (timerid != 0)
			g_source_remove(timerid);
		timerid = g_timeout_add_seconds(bakgap,
				 GSourceFunc(BakUserPhraseData), NULL);
		if (!value)
			g_value_unset(&localvalue);
	}
}

/**
 * 更新查询表页面大小.
 * @param value 最新页面大小数据
 * @note 如果(value==NULL)则表明数据需要临时获取
 */
void EngineConfig::UpdatePageSize(GValue *value)
{
	GValue localvalue = {0};
	GValue *pvalue;

	if (value || ibus_config_get_value(busconfig, CONFIG_SECTION,
				 CONFIG_NAME_PAGE, &localvalue)) {
		pvalue = value ? value : &localvalue;
		if ((pagesize = g_value_get_int(pvalue)) < 3)
			pagesize = 3;
		if (!value)
			g_value_unset(&localvalue);
	}
}

/**
 * 更新相关标记位.
 * @param value 最新标记位数据
 * @note 如果(value==NULL)则表明数据需要临时获取
 */
void EngineConfig::UpdateBitFlags(GValue *value)
{
	GValue localvalue = {0};
	GValue *pvalue;

	if (value || ibus_config_get_value(busconfig, CONFIG_SECTION,
				 CONFIG_NAME_FLAGS, &localvalue)) {
		pvalue = value ? value : &localvalue;
		flags = g_value_get_int(pvalue);
		if (!value)
			g_value_unset(&localvalue);
	}
}

/**
 * 通知监听者.
 */
void EngineConfig::NotifyListener()
{
	GSList *tlist;

	tlist = pyegnlist;
	while (tlist) {
		((PinyinEngine *)tlist->data)->UpdateConfig();
		tlist = g_slist_next(tlist);
	}
}

/**
 * 更新词语查询引擎的拼音修正对.
 * 拼音修正对之间以'|'作为分割，e.g."ign|ing" \n
 *
 */
void EngineConfig::UpdatePhraseEngineRectifyPinyinPair()
{
	PhraseEngine *phrengine;
	gchar *tptr, *pptr, *ptr;

	phrengine = PhraseEngine::GetInstance();
	phrengine->ClearRectifyPinyinPair();
	if (!rtfstr || *rtfstr == '\0')
		return;
	pptr = rtfstr;
	do {
		ptr = strchr(pptr, ';');
		pptr = g_strndup(pptr, ptr ? ptr - pptr : strlen(pptr));
		if ((tptr = strchr(pptr, '|')) && tptr != pptr && *(tptr + 1) != '\0') {
			*tptr = '\0';
			phrengine->AddRectifyPinyinPair(pptr, tptr + 1);
		}
		g_free(pptr);
		pptr = ptr + 1;
	} while (ptr && *pptr != '\0');
}

/**
 * 更新词语查询引擎的模糊拼音单元.
 * 模糊拼音单元之间以'|'作为分割，e.g."zh|z" \n
 *
 */
void EngineConfig::UpdatePhraseEngineFuzzyPinyinUnit()
{
	PhraseEngine *phrengine;
	gchar *tptr, *pptr, *ptr;

	phrengine = PhraseEngine::GetInstance();
	phrengine->ClearFuzzyPinyinUnit();
	if (!fzstr || *fzstr == '\0')
		return;
	pptr = fzstr;
	do {
		ptr = strchr(pptr, ';');
		pptr = g_strndup(pptr, ptr ? ptr - pptr : strlen(pptr));
		if ((tptr = strchr(pptr, '|')) && tptr != pptr && *(tptr + 1) != '\0') {
			*tptr = '\0';
			phrengine->AddFuzzyPinyinUnit(pptr, tptr + 1);
		}
		g_free(pptr);
		pptr = ptr + 1;
	} while (ptr && *pptr != '\0');
}

/**
 * 配置数据改变.
 * @param ibusconfig An IBusConfig
 * @param section Section name of the configuration option.
 * @param name Name of the configure option its self.
 * @param value GValue that holds the value.
 */
void EngineConfig::ConfigDataChanged(EngineConfig *config, gchar *section,
						 gchar *name, GValue *value)
{
	if (strcmp(section, CONFIG_SECTION) == 0) {
		if (strcmp(name, CONFIG_NAME_RECTIFY) == 0)
			config->UpdateRectifyPinyinPair(value);
		else if (strcmp(name, CONFIG_NAME_FUZZY) == 0)
			config->UpdateFuzzyPinyinUnit(value);
		else if (strcmp(name, CONFIG_NAME_BAKGAP) == 0)
			config->UpdateBackupGap(value);
		else if (strcmp(name, CONFIG_NAME_PAGE) == 0)
			config->UpdatePageSize(value);
		else if (strcmp(name, CONFIG_NAME_FLAGS) == 0)
			config->UpdateBitFlags(value);
		else if (strcmp(name, CONFIG_NAME_EOF) == 0)
			config->NotifyListener();
	}
}

/**
 * 备份用户词语数据.
 * @return GLib库所需
 */
gboolean EngineConfig::BakUserPhraseData()
{
	PhraseEngine *phrengine;

	phrengine = PhraseEngine::GetInstance();
	phrengine->BakUserEnginePhrase();
	return TRUE;
}
