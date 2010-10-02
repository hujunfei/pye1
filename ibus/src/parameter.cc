//
// C++ Implementation: parameter
//
// Description:
// 请参见头文件描述.
//
// Author: Jally <jallyx@163.com>, (C) 2009, 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "parameter.h"
#include <string.h>
#include <phrase_manager.h>
#include "pye_engine.h"

#define SECTION "engine/pye"
#define PAGESIZE "pagesize"
#define CURSOR_VISIBLE "cursor_visible"
#define PHRASE_FREQUENCY_ADJUSTABLE "phrase_frequency_adjustable"
#define ENGINE_PHRASE_SAVABLE "engine_phrase_savable"
#define MANUAL_PHRASE_SAVABLE "manual_phrase_savable"
#define MEND_PINYIN_PAIR "mend_pinyin_pair"
#define FUZZY_PINYIN_PAIR "fuzzy_pinyin_pair"
#define BACKUP_CYCLE "backup_cycle"
#define END "--end--"

/**
 * 类构造函数.
 */
Parameter::Parameter()
    : pagesize_(5), cursor_visible_(TRUE), phrase_frequency_adjustable_(TRUE),
      engine_phrase_savable_(TRUE), manual_phrase_savable_(TRUE), config_(NULL),
      timer_id_(0), mend_data_(NULL), fuzzy_data_(NULL), backup_cycle_(60) {
}

/**
 * 类析构函数.
 */
Parameter::~Parameter() {
  if (config_)
    g_object_unref(config_);
  if (timer_id_ != 0)
    g_source_remove(timer_id_);
  g_free(mend_data_);
  g_free(fuzzy_data_);
}

/**
 * 获取本类的实例对象指针.
 * @return 实例对象
 */
Parameter *Parameter::getInstance() {
  static Parameter instance;
  return &instance;
}

/**
 * 添加监听者.
 * @param engine 监听者
 */
void Parameter::addListener(PyeEngine *engine) {
  engine_list_.push_back(engine);
}

/**
 * 移除监听者.
 * @param engine 监听者
 */
void Parameter::removeListener(PyeEngine *engine) {
  engine_list_.remove(engine);
}

/**
 * 设置IBus总线连接，并更新相关配置数据.
 * @param conn 总线连接
 */
void Parameter::setConnection(IBusConnection *conn) {
  /* 新建IBus配置接口 */
  if (config_)
    g_object_unref(config_);
  config_ = ibus_config_new(conn);
  g_signal_connect_swapped(config_, "value-changed",
                           G_CALLBACK(configChanged), this);

  /* 更新配置数据 */
  updatePagesize(NULL);
  updateCursorVisible(NULL);
  updatePhraseFrequencyAdjustable(NULL);
  updateEnginePhraseSavable(NULL);
  updateManualPhraseSavable(NULL);
  updateMendPinyinPair(NULL);
  updateFuzzyPinyinPair(NULL);
  updateBackupCycle(NULL);

  /* 通知监听者 */
  notifyListener();
}

/**
 * 更新词语查询表的页面大小.
 * @param value 数值
 * @note 如果(value==NULL)则表明数值需要临时获取.
 */
void Parameter::updatePagesize(GValue *value) {
  if (value) {
    pagesize_ = g_value_get_int(value);
  } else {
    GValue local_value = {0};
    if (ibus_config_get_value(config_, SECTION, PAGESIZE, &local_value)) {
      pagesize_ = g_value_get_int(&local_value);
      g_value_unset(&local_value);
    }
  }

  if (pagesize_ < 3)
    pagesize_ = 3;
}

/**
 * 更新光标可见标记.
 * @param value 数值
 * @note 如果(value==NULL)则表明数值需要临时获取.
 */
void Parameter::updateCursorVisible(GValue *value) {
  if (value) {
    cursor_visible_ = g_value_get_boolean(value);
  } else {
    GValue local_value = {0};
    if (ibus_config_get_value(config_, SECTION, CURSOR_VISIBLE, &local_value)) {
      cursor_visible_ = g_value_get_boolean(&local_value);
      g_value_unset(&local_value);
    }
  }
}

/**
 * 更新调整词频标记.
 * @param value 数值
 * @note 如果(value==NULL)则表明数值需要临时获取.
 */
void Parameter::updatePhraseFrequencyAdjustable(GValue *value) {
  if (value) {
    phrase_frequency_adjustable_ = g_value_get_boolean(value);
  } else {
    GValue local_value = {0};
    if (ibus_config_get_value(config_, SECTION, PHRASE_FREQUENCY_ADJUSTABLE,
                              &local_value)) {
      phrase_frequency_adjustable_ = g_value_get_boolean(&local_value);
      g_value_unset(&local_value);
    }
  }
}

/**
 * 更新保存引擎合成词语标记.
 * @param value 数值
 * @note 如果(value==NULL)则表明数值需要临时获取.
 */
void Parameter::updateEnginePhraseSavable(GValue *value) {
  if (value) {
    engine_phrase_savable_ = g_value_get_boolean(value);
  } else {
    GValue local_value = {0};
    if (ibus_config_get_value(config_, SECTION, ENGINE_PHRASE_SAVABLE,
                              &local_value)) {
      engine_phrase_savable_ = g_value_get_boolean(&local_value);
      g_value_unset(&local_value);
    }
  }
}

/**
 * 更新保存用户合成词语标记.
 * @param value 数值
 * @note 如果(value==NULL)则表明数值需要临时获取.
 */
void Parameter::updateManualPhraseSavable(GValue *value) {
  if (value) {
    manual_phrase_savable_ = g_value_get_boolean(value);
  } else {
    GValue local_value = {0};
    if (ibus_config_get_value(config_, SECTION, MANUAL_PHRASE_SAVABLE,
                              &local_value)) {
      manual_phrase_savable_ = g_value_get_boolean(&local_value);
      g_value_unset(&local_value);
    }
  }
}

/**
 * 更新拼音矫正对.
 * @param value 数值
 * @note 如果(value==NULL)则表明数值需要临时获取.
 */
void Parameter::updateMendPinyinPair(GValue *value) {
  g_free(mend_data_);
  mend_data_ = NULL;

  if (value) {
    mend_data_ = g_value_dup_string(value);
  } else {
    GValue local_value = {0};
    if (ibus_config_get_value(config_, SECTION, MEND_PINYIN_PAIR,
                              &local_value)) {
      mend_data_ = g_value_dup_string(&local_value);
      g_value_unset(&local_value);
    }
  }

  updateMendPinyinPair();
}

/**
 * 更新模糊拼音对.
 * @param value 数值
 * @note 如果(value==NULL)则表明数值需要临时获取.
 */
void Parameter::updateFuzzyPinyinPair(GValue *value) {
  g_free(fuzzy_data_);
  fuzzy_data_ = NULL;

  if (value) {
    fuzzy_data_ = g_value_dup_string(value);
  } else {
    GValue local_value = {0};
    if (ibus_config_get_value(config_, SECTION, FUZZY_PINYIN_PAIR,
                              &local_value)) {
      fuzzy_data_ = g_value_dup_string(&local_value);
      g_value_unset(&local_value);
    }
  }

  updateFuzzyPinyinPair();
}

/**
 * 更新备份用户词语的时间周期.
 * @param value 数值
 * @note 如果(value==NULL)则表明数值需要临时获取.
 */
void Parameter::updateBackupCycle(GValue *value) {
  if (value) {
    backup_cycle_ = g_value_get_int(value);
  } else {
    GValue local_value = {0};
    if (ibus_config_get_value(config_, SECTION, BACKUP_CYCLE, &local_value)) {
      backup_cycle_ = g_value_get_int(&local_value);
      g_value_unset(&local_value);
    }
  }

  if (backup_cycle_ < 30)
    backup_cycle_ = 30;

  if (timer_id_ != 0)
    g_source_remove(timer_id_);
  timer_id_ = g_timeout_add_seconds(backup_cycle_, GSourceFunc(backupData), NULL);
}

/**
 * 通知监听者.
 */
void Parameter::notifyListener() {
  for (std::list<PyeEngine *>::iterator iterator = engine_list_.begin();
       iterator != engine_list_.end();
       ++iterator) {
    PyeEngine *pye_engine = *iterator;
    pye_engine->updateConfig();
  }
}

/**
 * 更新词语引擎的拼音矫正对.
 * 拼音对之间以'|'作为分割，e.g."ign|ing". \n
 * 每两组拼音对之间以';'作为分割，e.g."ign|ing;img|ing". \n
 */
void Parameter::updateMendPinyinPair() {
  if (!mend_data_ || *mend_data_ == '\0')
    return;

  PhraseManager *phrase_manager = PhraseManager::GetInstance();
  phrase_manager->ClearMendPinyinPair();

  char *pptr = mend_data_;
  char *ptr = NULL;
  do {
    ptr = strchr(pptr, ';');
    char *data = strndup(pptr, ptr ? ptr - pptr : strlen(pptr));
    char *tptr = NULL;
    if (isalpha(*data) && (tptr = strchr(data, '|')) && isalpha(*(tptr + 1))) {
      *tptr = '\0';
      phrase_manager->AppendMendPinyinPair(data, tptr + 1);
    }
    free(data);
    pptr = ptr + 1;
  } while (ptr && *pptr != '\0');
}

/**
 * 更新词语引擎的模糊拼音对.
 * 拼音对之间以'|'作为分割，e.g."zh|z". \n
 * 每两组拼音对之间以';'作为分割，e.g."zh|z;ch|c". \n
 */
void Parameter::updateFuzzyPinyinPair() {
  if (!fuzzy_data_ || *fuzzy_data_ == '\0')
    return;

  PhraseManager *phrase_manager = PhraseManager::GetInstance();
  phrase_manager->ClearFuzzyPinyinPair();

  char *pptr = fuzzy_data_;
  char *ptr = NULL;
  do {
    ptr = strchr(pptr, ';');
    char *data = strndup(pptr, ptr ? ptr - pptr : strlen(pptr));
    char *tptr = NULL;
    if (isalpha(*data) && (tptr = strchr(data, '|')) && isalpha(*(tptr + 1))) {
      *tptr = '\0';
      phrase_manager->AppendFuzzyPinyinPair(data, tptr + 1);
    }
    free(data);
    pptr = ptr + 1;
  } while (ptr && *pptr != '\0');
}

/**
 * 配置数据改变.
 * @param object An IBusConfig
 * @param section Section name of the configuration option.
 * @param name Name of the configure option its self.
 * @param value GValue that holds the value.
 */
void Parameter::configChanged(Parameter *object, gchar *section,
                              gchar *name, GValue *value) {
  if (strcmp(section, SECTION) == 0) {
    if (strcmp(name, PAGESIZE) == 0)
      object->updatePagesize(value);
    else if (strcmp(name, CURSOR_VISIBLE) == 0)
      object->updateCursorVisible(value);
    else if (strcmp(name, PHRASE_FREQUENCY_ADJUSTABLE) == 0)
      object->updatePhraseFrequencyAdjustable(value);
    else if (strcmp(name, ENGINE_PHRASE_SAVABLE) == 0)
      object->updateEnginePhraseSavable(value);
    else if (strcmp(name, MANUAL_PHRASE_SAVABLE) == 0)
      object->updateManualPhraseSavable(value);
    else if (strcmp(name, MEND_PINYIN_PAIR) == 0)
      object->updateMendPinyinPair(value);
    else if (strcmp(name, FUZZY_PINYIN_PAIR) == 0)
      object->updateFuzzyPinyinPair(value);
    else if (strcmp(name, BACKUP_CYCLE) == 0)
      object->updateBackupCycle(value);
    else if (strcmp(name, END) == 0)
      object->notifyListener();
  }
}

/**
 * 备份用户词语数据.
 * @return GBOOLEAN
 */
gboolean Parameter::backupData() {
  PhraseManager *phrase_manager = PhraseManager::GetInstance();
  phrase_manager->BackupUserPhrase();
  return TRUE;
}
