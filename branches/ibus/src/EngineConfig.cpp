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
 timerid(0), rtfstrv(NULL), fzstrv(NULL), bakgap(60), pagesize(5),
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
	g_strfreev(rtfstrv);
	g_strfreev(fzstrv);
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
	UpdateRectifyPinyinPair();
	UpdateFuzzyPinyinUnit();
	UpdateBackupGap();
	UpdatePageSize();
	UpdateBitFlags();

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
uint8_t EngineConfig::GetBitFlags()
{
	return flags;
}

/**
 * 更新拼音修正对.
 */
void EngineConfig::UpdateRectifyPinyinPair()
{
	GValue value = {0};

	g_value_init(&value, G_TYPE_STRV);
	if (ibus_config_get_value(busconfig, CONFIG_SECTION,
				 CONFIG_NAME_RECTIFY, &value)) {
		g_strfreev(rtfstrv);
		if ( (rtfstrv = (gchar **)g_value_get_boxed(&value)))
			UpdatePhraseEngineRectifyPinyinPair();
	}
}

/**
 * 更新模糊拼音单元.
 */
void EngineConfig::UpdateFuzzyPinyinUnit()
{
	GValue value = {0};

	g_value_init(&value, G_TYPE_STRV);
	if (ibus_config_get_value(busconfig, CONFIG_SECTION, CONFIG_NAME_FUZZY, &value)) {
		g_strfreev(fzstrv);
		if ( (fzstrv = (gchar **)g_value_get_boxed(&value)))
			UpdatePhraseEngineFuzzyPinyinUnit();
	}
}

/**
 * 更新备份用户词语的时间间隔.
 */
void EngineConfig::UpdateBackupGap()
{
	GValue value = {0};

	g_value_init(&value, G_TYPE_INT);
	if (ibus_config_get_value(busconfig, CONFIG_SECTION,
				 CONFIG_NAME_BAKGAP, &value)) {
		if ((bakgap = g_value_get_int(&value)) < 30)
			bakgap = 30;
		if (timerid != 0)
			g_source_remove(timerid);
		timerid = g_timeout_add_seconds(bakgap,
				 GSourceFunc(BakUserPhraseData), NULL);
	}
}

/**
 * 更新查询表页面大小.
 */
void EngineConfig::UpdatePageSize()
{
	GValue value = {0};

	g_value_init(&value, G_TYPE_UINT);
	if (ibus_config_get_value(busconfig, CONFIG_SECTION, CONFIG_NAME_PAGE, &value)) {
		if ((pagesize = g_value_get_uint(&value)) < 5)
			pagesize = 5;
	}
}

/**
 * 更新相关标记位.
 */
void EngineConfig::UpdateBitFlags()
{
	GValue value = {0};

	g_value_init(&value, G_TYPE_UCHAR);
	if (ibus_config_get_value(busconfig, CONFIG_SECTION, CONFIG_NAME_FLAGS, &value))
		flags = g_value_get_uchar(&value);
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
 */
void EngineConfig::UpdatePhraseEngineRectifyPinyinPair()
{
	PhraseEngine *phrengine;
	gchar **pstr, *pinyin1, *pinyin2;
	const gchar *ptr;

	phrengine = PhraseEngine::GetInstance();
	phrengine->ClearRectifyPinyinPair();
	pstr = rtfstrv;
	while (*pstr) {
		if ((ptr = strchr(*pstr, '|')) && (*pstr != ptr) && *(ptr + 1)) {
			pinyin1 = g_strndup(*pstr, ptr - *pstr);
			pinyin2 = g_strndup(ptr + 1, strlen(ptr + 1));
			phrengine->AddRectifyPinyinPair(pinyin1, pinyin2);
			g_free(pinyin1);
			g_free(pinyin2);
		}
		pstr++;
	}
}

/**
 * 更新词语查询引擎的模糊拼音单元.
 * 模糊拼音单元之间以'|'作为分割，e.g."zh|z" \n
 */
void EngineConfig::UpdatePhraseEngineFuzzyPinyinUnit()
{
	PhraseEngine *phrengine;
	gchar **pstr, *unit1, *unit2;
	const gchar *ptr;

	phrengine = PhraseEngine::GetInstance();
	phrengine->ClearFuzzyPinyinUnit();
	pstr = fzstrv;
	while (*pstr) {
		if ((ptr = strchr(*pstr, '|')) && (*pstr != ptr) && *(ptr + 1)) {
			unit1 = g_strndup(*pstr, ptr - *pstr);
			unit2 = g_strndup(ptr + 1, strlen(ptr + 1));
			phrengine->AddFuzzyPinyinUnit(unit1, unit2);
			g_free(unit1);
			g_free(unit2);
		}
		pstr++;
	}
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
			config->UpdateRectifyPinyinPair();
		else if (strcmp(name, CONFIG_NAME_FUZZY) == 0)
			config->UpdateFuzzyPinyinUnit();
		else if (strcmp(name, CONFIG_NAME_BAKGAP) == 0)
			config->UpdateBackupGap();
		else if (strcmp(name, CONFIG_NAME_PAGE) == 0)
			config->UpdatePageSize();
		else if (strcmp(name, CONFIG_NAME_FLAGS) == 0)
			config->UpdateBitFlags();
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
