//
// C++ Implementation: pye_engine
//
// Description:
// 请参见头文件描述.
//
// Author: Jally <jallyx@163.com>, (C) 2009, 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include "pye_engine.h"
#include <string.h>
#include "half_full_converter.h"
#include "parameter.h"

// mask for Ctrl, Alt, Super, Hyper, Meta
static const guint CMSHM_MASK = IBUS_CONTROL_MASK |
                                IBUS_MOD1_MASK |
                                IBUS_SUPER_MASK |
                                IBUS_HYPER_MASK |
                                IBUS_META_MASK;
// mask for Shift, Ctrl, Alt, Super, Hyper, Meta
static const guint SCMSHM_MASK = CMSHM_MASK |
                                 IBUS_SHIFT_MASK;

#define CMSHM_FILTER(modifiers) ((modifiers) & (CMSHM_MASK))
#define SCMSHM_FILTER(modifiers) ((modifiers) & (SCMSHM_MASK))

// duplicate binary data
#define DuplicateText(data,len) ({ \
    char *text = (char *)malloc(len + 1); \
    memcpy(text, data, len); \
    *(text + len) = '\0'; \
    text; \
  })
// reallocate binary data
#define ReallocateText(data,len) ({ \
    char *text = (char *)malloc(len + 1); \
    memcpy(text, data, len); \
    *(text + len) = '\0'; \
    free(data); \
    text; \
  })

#define MODE_CHINESE "mode.chinese"
#define MODE_FULL_LETTER "mode.full_letter"
#define MODE_FULL_PUNCT "mode.full_punct"
#define ENGINE_SETUP "engine.setup"

/**
 * 类构造函数.
 * @param engine IBusEngine
 * @param manager 词语管理者
 */
PyeEngine::PyeEngine(IBusEngine *engine, PhraseManager *manager)
    : pinyin_editor_(manager), engine_(engine), lookup_table_(NULL),
      prop_list_(NULL), pre_keyval_(IBUS_VoidSymbol), tmp_english_mode_(false),
      chinese_mode_(true), full_letter_mode_(false), full_punct_mode_(true),
      single_quote_area_(false), double_quote_area_(false) {
  /* 注册为参数类的监听者 */
  Parameter *parameter = Parameter::getInstance();
  parameter->addListener(this);
  /* 创建词语查询表 */
  lookup_table_ = ibus_lookup_table_new(parameter->pagesize_,
                                        0,
                                        parameter->cursor_visible_,
                                        FALSE);
  g_object_ref_sink(lookup_table_);
  /* 创建属性部件表 */
  prop_list_ = createPropList();
  g_object_ref_sink(prop_list_);
}

/**
 * 类析构函数.
 */
PyeEngine::~PyeEngine() {
  Parameter *parameter = Parameter::getInstance();
  parameter->removeListener(this);
  g_object_unref(lookup_table_);
  g_object_unref(prop_list_);
}

/**
 * 处理键值事件.
 * @param keyval Key symbol of a key event.
 * @param keycode Keycode of a key event.
 * @param modifiers Key modifier flags.
 * @return TRUE for successfully process the key; FALSE otherwise.
 */
gboolean PyeEngine::processKeyEvent(guint keyval, guint keycode, guint modifiers) {
  /* 对释放键(Shift)的处理 */
  if (modifiers & IBUS_RELEASE_MASK) {
    if (CMSHM_FILTER(modifiers) != 0 || pre_keyval_ != keyval)
      return FALSE;
    switch (keyval) {
      case IBUS_Shift_L:
        if (!pinyin_editor_.IsEmpty()) {
          guint pages = ibus_lookup_table_get_cursor_pos(lookup_table_) /
                        ibus_lookup_table_get_page_size(lookup_table_);
          guint index = ibus_lookup_table_get_page_size(lookup_table_) *
                        pages +
                        1;
          selectCandidatePhrase(index);
        } else {
          toggleModeChinese();
        }
        return TRUE;
      case IBUS_Shift_R:
        if (!pinyin_editor_.IsEmpty()) {
          guint pages = ibus_lookup_table_get_cursor_pos(lookup_table_) /
                        ibus_lookup_table_get_page_size(lookup_table_);
          guint index = ibus_lookup_table_get_page_size(lookup_table_) *
                        pages +
                        2;
          selectCandidatePhrase(index);
        } else {
          toggleModeChinese();
        }
        return TRUE;
      default:
        return FALSE;
    }
  }

  /* 键值处理 */
  gboolean retval = FALSE;
  switch (keyval) {
    /* letters */
    case IBUS_a ... IBUS_z:
      retval = processLetter(keyval, keycode, modifiers);
      break;
    case IBUS_A ... IBUS_Z:
      retval = processCapital(keyval, keycode, modifiers);
      break;
    /* numbers */
    case IBUS_0 ... IBUS_9:
    case IBUS_KP_0 ... IBUS_KP_9:
      retval = processNumber(keyval, keycode, modifiers);
      break;
    /* punct */
    case IBUS_exclam ... IBUS_slash:
    case IBUS_colon ... IBUS_at:
    case IBUS_bracketleft ... IBUS_quoteleft:
    case IBUS_braceleft ... IBUS_asciitilde:
      retval = processPunct(keyval, keycode, modifiers);
      break;
    /* space */
    case IBUS_space:
      retval = processSpace(keyval, keycode, modifiers);
      break;
    /* others */
    default:
      retval = processOthers(keyval, keycode, modifiers);
      break;
  }
  pre_keyval_ = retval ? IBUS_VoidSymbol : keyval;

  return retval;
}

/**
 * 重置引擎.
 */
void PyeEngine::reset() {
  hideUI();
  resetEngine();
}

/**
 * 启用引擎.
 */
void PyeEngine::enable() {
}

/**
 * 禁用引擎.
 */
void PyeEngine::disable() {
  hideUI();
  resetStatus();
}

/**
 * 获得焦点.
 */
void PyeEngine::focusIn() {
  ibus_engine_register_properties(engine_, prop_list_);
}

/**
 * 失去焦点.
 */
void PyeEngine::focusOut() {
}

/**
 * 向上翻页.
 */
void PyeEngine::pageUp() {
  ibus_lookup_table_page_up(lookup_table_);
  ibus_engine_update_lookup_table_fast(engine_, lookup_table_, TRUE);
}

/**
 * 向下翻页.
 */
void PyeEngine::pageDown() {
  if (ibus_lookup_table_get_number_of_candidates(lookup_table_) -
      ibus_lookup_table_get_cursor_pos(lookup_table_) <
      ibus_lookup_table_get_page_size(lookup_table_) << 1)
    appendPageCandidate();

  ibus_lookup_table_page_down(lookup_table_);
  ibus_engine_update_lookup_table_fast(engine_, lookup_table_, TRUE);
}

/**
 * 上移光标.
 */
void PyeEngine::cursorUp() {
  /* 如果光标不可见，必然无须移动光标 */
  Parameter *parameter = Parameter::getInstance();
  if (!parameter->cursor_visible_)
    return;

  ibus_lookup_table_cursor_up(lookup_table_);
  ibus_engine_update_lookup_table_fast(engine_, lookup_table_, TRUE);
}

/**
 * 下移光标.
 */
void PyeEngine::cursorDown() {
  /* 如果光标不可见，必然无须移动光标 */
  Parameter *parameter = Parameter::getInstance();
  if (!parameter->cursor_visible_)
    return;

  if (ibus_lookup_table_get_number_of_candidates(lookup_table_) -
      ibus_lookup_table_get_cursor_pos(lookup_table_) <
      ibus_lookup_table_get_page_size(lookup_table_) + 1)
    appendPageCandidate();

  ibus_lookup_table_cursor_down(lookup_table_);
  ibus_engine_update_lookup_table_fast(engine_, lookup_table_, TRUE);
}

/**
* 候选字被点击.
* @param index 索引值
* @param button 鼠标按键
* @param state Key modifier flags.
*/
void PyeEngine::candidateClicked(guint index, guint button, guint state) {
  guint pages = ibus_lookup_table_get_cursor_pos(lookup_table_) /
                ibus_lookup_table_get_page_size(lookup_table_);
  guint number = ibus_lookup_table_get_page_size(lookup_table_) *
                 pages +
                 index;
  selectCandidatePhrase(number);
}

/**
 * 属性单元被激活.
 * @param prop_name Unique Identity for the IBusProperty.
 * @param prop_state Key modifier flags.
 */
void PyeEngine::propertyActivate(const gchar *prop_name, guint prop_state) {
  if (strcmp(prop_name, MODE_CHINESE) == 0)
    toggleModeChinese();
  else if (strcmp(prop_name, MODE_FULL_LETTER) == 0)
    toggleModeFullLetter();
  else if (strcmp(prop_name, MODE_FULL_PUNCT) == 0)
    toggleModeFullPunct();
  else if (strcmp(prop_name, ENGINE_SETUP) == 0)
    showSetupDialog();
}

/**
* 更新引擎配置.
*/
void PyeEngine::updateConfig() {
  Parameter *parameter = Parameter::getInstance();
  ibus_lookup_table_set_page_size(lookup_table_, parameter->pagesize_);
  ibus_lookup_table_set_cursor_visible(lookup_table_, parameter->cursor_visible_);
  gboolean visible = !pinyin_editor_.IsEmpty() || tmp_english_mode_;
  ibus_engine_update_lookup_table_fast(engine_, lookup_table_, visible);
}

/**
 * 更新UI数据.
 */
void PyeEngine::updateUIData() {
  /* 如果在临时英文模式下 */
  if (tmp_english_mode_) {
    ibus_lookup_table_clear(lookup_table_);
    ibus_engine_update_lookup_table(engine_, lookup_table_, TRUE);
    char *data = NULL;
    int len = 0;
    pinyin_editor_.GetRawText(&data, &len);
    IBusText *text = NULL;
    glong cursor_pos = 0;
    if (data) {
      char *text_data = ReallocateText(data, len);
      text = ibus_text_new_from_static_string(text_data);
      g_object_set_data_full(G_OBJECT(text), "text", text_data,
                             GDestroyNotify(free));
      cursor_pos = g_utf8_strlen(text_data, -1);
    } else {
      text = ibus_text_new_from_static_string("");
    }
    g_object_ref_sink(text);
    ibus_engine_update_auxiliary_text(engine_, text, TRUE);
    ibus_engine_update_preedit_text(engine_, text, cursor_pos, TRUE);
    g_object_unref(text);
    return;
  }

  /* 更新候选字 */
  ibus_lookup_table_clear(lookup_table_);
  appendDynamicPhrase();
  appendComposePhrase();
  appendPageCandidate();
  ibus_engine_update_lookup_table(engine_, lookup_table_, TRUE);

  /* 更新辅助文本 */
  char *data = NULL;
  int len = 0;
  pinyin_editor_.GetAuxiliaryText(&data, &len);
  if (data) {
    char *text_data = ReallocateText(data, len);
    IBusText *text = ibus_text_new_from_static_string(text_data);
    g_object_set_data_full(G_OBJECT(text), "text", text_data,
                           GDestroyNotify(free));
    ibus_engine_update_auxiliary_text(engine_, text, TRUE);
  } else {
    ibus_engine_hide_auxiliary_text(engine_);
  }

  /* 更新预编辑文本 */
  pinyin_editor_.GetPreeditText(&data, &len);
  if (data) {
    char *text_data = ReallocateText(data, len);
    IBusText *text = ibus_text_new_from_static_string(text_data);
    g_object_set_data_full(G_OBJECT(text), "text", text_data,
                           GDestroyNotify(free));
    glong cursor_pos = g_utf8_strlen(text_data, -1);
    ibus_engine_update_preedit_text(engine_, text, cursor_pos, TRUE);
  } else {
    ibus_engine_hide_preedit_text(engine_);
  }
}

/**
 * 重置UI数据.
 */
void PyeEngine::resetUIData() {
  ibus_lookup_table_clear(lookup_table_);
  ibus_engine_update_lookup_table_fast(engine_, lookup_table_, TRUE);
  IBusText *text = ibus_text_new_from_static_string("");
  g_object_ref_sink(text);
  ibus_engine_update_auxiliary_text(engine_, text, TRUE);
  ibus_engine_update_preedit_text(engine_, text, 0, TRUE);
  g_object_unref(text);
}

/**
 * 隐藏UI.
 */
void PyeEngine::hideUI() {
  ibus_engine_hide_lookup_table(engine_);
  ibus_engine_hide_auxiliary_text(engine_);
  ibus_engine_hide_preedit_text(engine_);
}

/**
 * 重置引擎.
 */
void PyeEngine::resetEngine() {
  pinyin_editor_.StopTask();
  if (chinese_mode_)
    pinyin_editor_.SetEditorMode(true);
  ibus_lookup_table_clear(lookup_table_);
  pre_keyval_ = IBUS_VoidSymbol;
  tmp_english_mode_ = false;
}

/**
 * 添加合成词语.
 */
void PyeEngine::appendComposePhrase() {
  const PhraseDatum *phrase_datum = pinyin_editor_.GetComposePhrase();
  if (!phrase_datum)
    return;

  char *text_data = DuplicateText(phrase_datum->raw_data_,
                                  phrase_datum->raw_data_length_);
  IBusText *text = ibus_text_new_from_static_string(text_data);
  ibus_text_append_attribute(text, IBUS_ATTR_TYPE_FOREGROUND, 0xff0000,
                             0, phrase_datum->raw_data_length_);
  g_object_set_data_full(G_OBJECT(text), "text", text_data,
                         GDestroyNotify(free));
  g_object_set_data(G_OBJECT(text), "data", (gpointer)phrase_datum);
  ibus_lookup_table_append_candidate(lookup_table_, text);
}

/**
 * 添加动态词语.
 */
void PyeEngine::appendDynamicPhrase() {
  std::list<const PhraseDatum *> phrase_list;
  pinyin_editor_.GetDynamicPhrase(&phrase_list);

  for (std::list<const PhraseDatum *>::iterator iterator = phrase_list.begin();
       iterator != phrase_list.end();
       ++iterator) {
    const PhraseDatum *phrase_datum = *iterator;
    char *text_data = DuplicateText(phrase_datum->raw_data_,
                                    phrase_datum->raw_data_length_);
    IBusText *text = ibus_text_new_from_static_string(text_data);
    ibus_text_append_attribute(text, IBUS_ATTR_TYPE_FOREGROUND, 0x00ff00,
                               0, phrase_datum->raw_data_length_);
    g_object_set_data_full(G_OBJECT(text), "text", text_data,
                           GDestroyNotify(free));
    g_object_set_data(G_OBJECT(text), "data", (gpointer)phrase_datum);
    ibus_lookup_table_append_candidate(lookup_table_, text);
  }
}

/**
 * 添加一个页面的候选词语.
 */
void PyeEngine::appendPageCandidate() {
  std::list<const PhraseDatum *> phrase_list;
  guint pagesize = ibus_lookup_table_get_page_size(lookup_table_);
  pinyin_editor_.GetPagePhrase(pagesize, &phrase_list);

  for (std::list<const PhraseDatum *>::iterator iterator = phrase_list.begin();
       iterator != phrase_list.end();
       ++iterator) {
    const PhraseDatum *phrase_datum = *iterator;
    char *text_data = DuplicateText(phrase_datum->raw_data_,
                                    phrase_datum->raw_data_length_);
    IBusText *text = ibus_text_new_from_static_string(text_data);
    if (phrase_datum->phrase_data_offset_ >= UserPhrasePoint) {
      ibus_text_append_attribute(text, IBUS_ATTR_TYPE_FOREGROUND, 0x0000ff,
                                 0, phrase_datum->raw_data_length_);
    }
    g_object_set_data_full(G_OBJECT(text), "text", text_data,
                           GDestroyNotify(free));
    g_object_set_data(G_OBJECT(text), "data", (gpointer)phrase_datum);
    ibus_lookup_table_append_candidate(lookup_table_, text);
  }
}

/**
 * 选择候选词语.
 * @param index 索引值
 */
void PyeEngine::selectCandidatePhrase(guint index) {
  IBusText *text= ibus_lookup_table_get_candidate(lookup_table_, index);
  if (!text)
    return;

  const PhraseDatum *phrase_datum =
      (const PhraseDatum *)g_object_get_data(G_OBJECT(text), "data");
  pinyin_editor_.SelectCachePhrase(phrase_datum);
  if (pinyin_editor_.IsFinishTask()) {
    commitPhrase();
    hideUI();
    resetEngine();
  } else {
    updateUIData();
  }
}

/**
 * 删除候选词语.
 * @param index 索引值
 */
void PyeEngine::deleteCandidatePhrase(guint index) {
  IBusText *text = ibus_lookup_table_get_candidate(lookup_table_, index);
  if (!text)
    return;

  /* 删除词语并更新词语查询表 */
  const PhraseDatum *phrase_datum =
      (const PhraseDatum *)g_object_get_data(G_OBJECT(text), "data");
  if (phrase_datum->phrase_data_offset_ < UserPhrasePoint)
    return;
  g_array_remove_index(lookup_table_->candidates, index);
  g_object_unref(text);
  pinyin_editor_.DeletePhraseData(phrase_datum);
  appendPageCandidate();
  ibus_engine_update_lookup_table_fast(engine_, lookup_table_, TRUE);
  if (index != 0)
    return;

  /* 更新辅助文本 */
  char *data = NULL;
  int len = 0;
  pinyin_editor_.GetAuxiliaryText(&data, &len);
  if (data) {
    char *text_data = ReallocateText(data, len);
    text = ibus_text_new_from_static_string(text_data);
    g_object_set_data_full(G_OBJECT(text), "text", text_data,
                           GDestroyNotify(free));
    ibus_engine_update_auxiliary_text(engine_, text, TRUE);
  } else {
    ibus_engine_hide_auxiliary_text(engine_);
  }

  /* 更新预编辑文本 */
  pinyin_editor_.GetPreeditText(&data, &len);
  if (data) {
    char *text_data = ReallocateText(data, len);
    text = ibus_text_new_from_static_string(text_data);
    g_object_set_data_full(G_OBJECT(text), "text", text_data,
                           GDestroyNotify(free));
    glong cursor_pos = g_utf8_strlen(text_data, -1);
    ibus_engine_update_preedit_text(engine_, text, cursor_pos, TRUE);
  } else {
    ibus_engine_hide_preedit_text(engine_);
  }
}

/**
 * 提交词语.
 */
void PyeEngine::commitPhrase() {
  /* 获取提交词语数据 */
  char *data = NULL;
  int len = 0;
  pinyin_editor_.GetCommitText(&data, &len);
  if (!data)
    return;

  /* 向UI提交词语 */
  char *text_data = ReallocateText(data, len);
  IBusText *text = ibus_text_new_from_static_string(text_data);
  g_object_set_data_full(G_OBJECT(text), "text", text_data,
                         GDestroyNotify(free));
  ibus_engine_commit_text(engine_, text);

  /* 反馈词语 */
  Parameter *parameter = Parameter::getInstance();
  int offset = pinyin_editor_.GetPhraseOffset();
  if ((offset == EnginePhraseType && parameter->engine_phrase_savable_) ||
      (offset == ManualPhraseType && parameter->manual_phrase_savable_) ||
      ((offset == SystemPhraseType || offset >= UserPhrasePoint) &&
       parameter->phrase_frequency_adjustable_))
    pinyin_editor_.FeedbackSelectedPhrase();
}

/**
 * 提交原始串.
 */
void PyeEngine::commitRawPhrase() {
  /* 获取原始串 */
  char *data = NULL;
  int len = 0;
  pinyin_editor_.GetRawText(&data, &len);
  if (!data)
    return;

  /* 向UI提交词语 */
  char *text_data = ReallocateText(data, len);
  IBusText *text = ibus_text_new_from_static_string(text_data);
  g_object_set_data_full(G_OBJECT(text), "text", text_data,
                         GDestroyNotify(free));
  ibus_engine_commit_text(engine_, text);
}

/**
 * 提交字母.
 * @param ch 字母
 */
void PyeEngine::commitLetter(gunichar ch) {
  if (full_letter_mode_)
    ch = HalfFullConverter::toFull(ch);
  IBusText *text = ibus_text_new_from_unichar(ch);
  ibus_engine_commit_text(engine_, text);
}

/**
 * 提交标点.
 * @param ch 标点
 */
void PyeEngine::commitPunct(gunichar ch) {
  if (full_punct_mode_)
    ch = HalfFullConverter::toFull(ch);
  IBusText *text = ibus_text_new_from_unichar(ch);
  ibus_engine_commit_text(engine_, text);
}

/**
 * 提交最终字符.
 * @param ch 字符
 */
void PyeEngine::commitFinalChars(gunichar ch) {
  IBusText *text = ibus_text_new_from_unichar(ch);
  ibus_engine_commit_text(engine_, text);
}

/**
 * 提交串.
 * @param str 串
 */
void PyeEngine::commitString(const gchar *str) {
  IBusText *text = ibus_text_new_from_string(str);
  ibus_engine_commit_text(engine_, text);
}

/**
 * 提交静态串.
 * @param str 串
 */
void PyeEngine::commitStaticString(const gchar *str) {
  IBusText *text = ibus_text_new_from_static_string(str);
  ibus_engine_commit_text(engine_, text);
}

/**
 * 创建属性部件表.
 * @return 部件表
 */
IBusPropList *PyeEngine::createPropList() {
  IBusPropList *prop_list = ibus_prop_list_new();

  /* 中英文 */
  IBusProperty *property = ibus_property_new(
      MODE_CHINESE,
      PROP_TYPE_NORMAL,
      ibus_text_new_from_static_string(chinese_mode_ ? "CN" : "EN"),
      chinese_mode_ ? __PIXMAPS_PATH "/chinese.png" :
                      __PIXMAPS_PATH "/english.png",
      ibus_text_new_from_static_string("Chinese/English"),
      TRUE,
      TRUE,
      PROP_STATE_UNCHECKED,
      NULL);
  g_object_set_data(G_OBJECT(prop_list), MODE_CHINESE, property);
  ibus_prop_list_append(prop_list, property);

  /* 字母全/半角 */
  property = ibus_property_new(
      MODE_FULL_LETTER,
      PROP_TYPE_NORMAL,
      ibus_text_new_from_static_string(full_letter_mode_ ? "Ａａ" : "Aa"),
      full_letter_mode_ ? __PIXMAPS_PATH "/full-letter.png" :
                          __PIXMAPS_PATH "/half-letter.png",
      ibus_text_new_from_static_string("Full/Half width letter"),
      TRUE,
      TRUE,
      PROP_STATE_UNCHECKED,
      NULL);
  g_object_set_data(G_OBJECT(prop_list), MODE_FULL_LETTER, property);
  ibus_prop_list_append(prop_list, property);

  /* 标点全/半角 */
  property = ibus_property_new(
      MODE_FULL_PUNCT,
      PROP_TYPE_NORMAL,
      ibus_text_new_from_static_string(full_punct_mode_ ? "，。" : ",."),
      full_punct_mode_ ? __PIXMAPS_PATH "/full-punct.png" :
                         __PIXMAPS_PATH "/half-punct.png",
      ibus_text_new_from_static_string("Full/Half width punctuation"),
      TRUE,
      TRUE,
      PROP_STATE_UNCHECKED,
      NULL);
  g_object_set_data(G_OBJECT(prop_list), MODE_FULL_PUNCT, property);
  ibus_prop_list_append(prop_list, property);

  /* 细节设置 */
  property = ibus_property_new(
      ENGINE_SETUP,
      PROP_TYPE_NORMAL,
      ibus_text_new_from_static_string("Preferences"),
      "gtk-preferences",
      ibus_text_new_from_static_string("Pinyin preferences"),
      TRUE,
      TRUE,
      PROP_STATE_UNCHECKED,
      NULL);
  g_object_set_data(G_OBJECT(prop_list), ENGINE_SETUP, property);
  ibus_prop_list_append(prop_list, property);

  return prop_list;
}

/**
 * 恢复初始化状态.
 */
void PyeEngine::resetStatus() {
  pinyin_editor_.StopTask();
  pinyin_editor_.SetEditorMode(true);
  ibus_lookup_table_clear(lookup_table_);
  pre_keyval_ = IBUS_VoidSymbol;
  tmp_english_mode_ = false;
   if (!chinese_mode_)
    toggleModeChinese();
  if (full_letter_mode_)
    toggleModeFullLetter();
  if (!full_punct_mode_)
    toggleModeFullPunct();
  single_quote_area_ = false;
  double_quote_area_ = false;
}

/**
 * 切换中/英文模式.
 */
void PyeEngine::toggleModeChinese() {
  if (!pinyin_editor_.SetEditorMode(!chinese_mode_))
    return;

  chinese_mode_ = !chinese_mode_;
  IBusProperty *property =
      IBUS_PROPERTY(g_object_get_data(G_OBJECT(prop_list_), MODE_CHINESE));
  IBusText *text = ibus_text_new_from_static_string(chinese_mode_ ? "CN" : "EN");
  ibus_property_set_label(property, text);
  ibus_property_set_icon(property, chinese_mode_ ? __PIXMAPS_PATH "/chinese.png" :
                                                   __PIXMAPS_PATH "/english.png");
  ibus_engine_update_property(engine_, property);

  /* 字母无条件回到半角模式 */
  if (full_letter_mode_)
    toggleModeFullLetter();
  /* 根据情况改变标点的模式 */
  if ((chinese_mode_ && !full_punct_mode_) ||
      (!chinese_mode_ && full_punct_mode_))
    toggleModeFullPunct();
}

/**
 * 切换字母全/半角模式.
 */
void PyeEngine::toggleModeFullLetter() {
  full_letter_mode_ = !full_letter_mode_;
  IBusProperty *property =
      IBUS_PROPERTY(g_object_get_data(G_OBJECT(prop_list_), MODE_FULL_LETTER));
  IBusText *text = ibus_text_new_from_static_string(full_letter_mode_ ? "Ａａ" : "Aa");
  ibus_property_set_label(property, text);
  ibus_property_set_icon(property, full_letter_mode_ ? __PIXMAPS_PATH "/full-letter.png" :
                                                       __PIXMAPS_PATH "/half-letter.png");
  ibus_engine_update_property(engine_, property);
}

/**
 * 切换标点全/半角模式.
 */
void PyeEngine::toggleModeFullPunct() {
  full_punct_mode_ = !full_punct_mode_;
  IBusProperty *property =
      IBUS_PROPERTY(g_object_get_data(G_OBJECT(prop_list_), MODE_FULL_PUNCT));
  IBusText *text = ibus_text_new_from_static_string(full_punct_mode_ ? "，。" : ",.");
  ibus_property_set_label(property, text);
  ibus_property_set_icon(property, full_punct_mode_ ? __PIXMAPS_PATH "/full-punct.png" :
                                                      __PIXMAPS_PATH "/half-punct.png");
  ibus_engine_update_property(engine_, property);
}

/**
 * 显示设置对话框.
 */
void PyeEngine::showSetupDialog() {
  g_spawn_command_line_async(__LIBEXEC_PATH "/ibus-setup-pye", NULL);
}

/**
 * 处理字母.
 * @param keyval Key symbol of a key event.
 * @param keycode Keycode of a key event.
 * @param state Key modifier flags.
 * @return TRUE for successfully process the key; FALSE otherwise.
 */
gboolean PyeEngine::processLetter(guint keyval, guint keycode, guint state) {
  /* 如果有修饰键则不处理 */
  if (CMSHM_FILTER(state) != 0)
    return FALSE;

  /* 英文模式 */
  if (!chinese_mode_) {
    commitLetter(keyval);
    return TRUE;
  }

  /* 是否需要进入临时英文模式 */
  if (pinyin_editor_.IsEmpty() &&
      !tmp_english_mode_ &&
      (keyval == 'u' || keyval == 'v')) {
    pinyin_editor_.SetEditorMode(false);
    tmp_english_mode_ = true;
    resetUIData();
    return TRUE;
  }

  /* 剩下的由拼音编辑器搞定 */
  pinyin_editor_.InsertPinyinKey(keyval);
  updateUIData();

  return TRUE;
}

/**
 * 处理大写字母.
 * @param keyval Key symbol of a key event.
 * @param keycode Keycode of a key event.
 * @param state Key modifier flags.
 * @return TRUE for successfully process the key; FALSE otherwise.
 */
gboolean PyeEngine::processCapital(guint keyval, guint keycode, guint state) {
  /* 如果有修饰键则不处理 */
  if (CMSHM_FILTER(state) != 0)
    return FALSE;

  /* 英文模式 */
  if (!chinese_mode_) {
    commitLetter(keyval);
    return TRUE;
  }

  /* 中文模式，编辑器为空 */
  if (pinyin_editor_.IsEmpty()) {
    pinyin_editor_.SetEditorMode(false);
    tmp_english_mode_ = true;
  }

  /* 英文编辑模式 */
  /* @note 如果不是英文编辑模式则忽略 */
  if (tmp_english_mode_) {
    pinyin_editor_.InsertPinyinKey(keyval);
    updateUIData();
  }

  return TRUE;
}

/**
 * 处理数字.
 * @param keyval Key symbol of a key event.
 * @param keycode Keycode of a key event.
 * @param state Key modifier flags.
 * @return TRUE for successfully process the key; FALSE otherwise.
 */
gboolean PyeEngine::processNumber(guint keyval, guint keycode, guint state) {
  /* 考察是否为(Ctrl+数字)模式 */
  if ((state & IBUS_CONTROL_MASK) != 0 &&
      (state & ~IBUS_CONTROL_MASK) == 0 &&
      !pinyin_editor_.IsEmpty()) {
    guint index = 0;
    switch (keyval) {
      case IBUS_0:
      case IBUS_KP_0:
        index = 9;
        break;
      case IBUS_1 ... IBUS_9:
        index = keyval - IBUS_1;
        break;
      case IBUS_KP_1 ... IBUS_KP_9:
        index = keyval - IBUS_KP_1;
        break;
      default:
        g_assert_not_reached();
    }
    if (index >= ibus_lookup_table_get_page_size(lookup_table_))
      return TRUE;
    guint pages = ibus_lookup_table_get_cursor_pos(lookup_table_) /
                  ibus_lookup_table_get_page_size(lookup_table_);
    guint number = ibus_lookup_table_get_page_size(lookup_table_) *
                   pages +
                   index;
    deleteCandidatePhrase(number);
    return TRUE;
  }
  /* 如果有修饰键则不处理 */
  if (CMSHM_FILTER(state) != 0)
    return FALSE;

  /* 英语模式 */
  if (!chinese_mode_) {
    commitLetter(keyval);
    return TRUE;
  }

  /* 英文编辑模式 */
  if (tmp_english_mode_) {
    pinyin_editor_.InsertPinyinKey(keyval);
    updateUIData();
    return TRUE;
  }

  /* 汉语模式，编辑器为空 */
  if (pinyin_editor_.IsEmpty()) {
    switch (keyval) {
      case IBUS_0 ... IBUS_9:
        commitLetter(keyval);
        break;
      case IBUS_KP_0 ... IBUS_KP_9:
        commitLetter('0' + keyval - IBUS_KP_0);
        break;
      default:
        g_assert_not_reached();
    }
    return TRUE;
  }

  /* 汉语模式，编辑器不为空 */
  guint index = 0;
  switch (keyval) {
    case IBUS_0:
    case IBUS_KP_0:
      index = 9;
      break;
    case IBUS_1 ... IBUS_9:
      index = keyval - IBUS_1;
      break;
    case IBUS_KP_1 ... IBUS_KP_9:
      index = keyval - IBUS_KP_1;
      break;
    default:
      g_assert_not_reached();
  }

  /* 选择词语 */
  if (index >= ibus_lookup_table_get_page_size(lookup_table_))
    return TRUE;
  guint pages = ibus_lookup_table_get_cursor_pos(lookup_table_) /
                ibus_lookup_table_get_page_size(lookup_table_);
  guint number = ibus_lookup_table_get_page_size(lookup_table_) *
                 pages +
                 index;
  selectCandidatePhrase(number);

  return TRUE;
}

/**
 * 处理空格.
 * @param keyval Key symbol of a key event.
 * @param keycode Keycode of a key event.
 * @param state Key modifier flags.
 * @return TRUE for successfully process the key; FALSE otherwise.
 */
gboolean PyeEngine::processSpace(guint keyval, guint keycode, guint state) {
  /* 考察是否为(Shift+Space)模式 */
  if ((state & IBUS_SHIFT_MASK) != 0 && (state & ~IBUS_SHIFT_MASK) == 0) {
    toggleModeFullLetter();
    return TRUE;
  }
  /* 如果有修饰键则不处理 */
  if (CMSHM_FILTER(state) != 0)
    return FALSE;

  /* 英语模式 */
  if (!chinese_mode_) {
    commitPunct(keyval);
    return TRUE;
  }

  /* 英文编辑模式 */
  if (tmp_english_mode_) {
    pinyin_editor_.InsertPinyinKey(keyval);
    updateUIData();
    return TRUE;
  }

  /* 中文模式，编辑器不为空 */
  if (!pinyin_editor_.IsEmpty()) {
    guint index = ibus_lookup_table_get_cursor_pos(lookup_table_);
    selectCandidatePhrase(index);
    return TRUE;
  }

  /* 中文模式，编辑器为空 */
  commitPunct(keyval);

  return TRUE;
}

/**
 * 处理标点符号.
 * @param keyval Key symbol of a key event.
 * @param keycode Keycode of a key event.
 * @param state Key modifier flags.
 * @return TRUE for successfully process the key; FALSE otherwise.
 */
gboolean PyeEngine::processPunct(guint keyval, guint keycode, guint state) {
  /* 考察是否为(Ctrl+.)模式 */
  if ((state & IBUS_CONTROL_MASK) != 0 &&
      (state & ~IBUS_CONTROL_MASK) == 0 &&
      keyval == '.') {
    toggleModeFullPunct();
    return TRUE;
  }
  /* 如果有修饰键则不处理 */
  if (CMSHM_FILTER(state) != 0)
    return FALSE;

  /* 英语模式 */
  if (!chinese_mode_) {
    commitPunct(keyval);
    return TRUE;
  }

  /* 英文编辑模式 */
  if (tmp_english_mode_) {
    pinyin_editor_.InsertPinyinKey(keyval);
    updateUIData();
    return TRUE;
  }

  /* 中文模式，编辑器不为空 */
  if (!pinyin_editor_.IsEmpty()) {
    switch (keyval) {
      case IBUS_minus:
      case IBUS_comma:
        pageUp();
        break;
      case IBUS_equal:
      case IBUS_period:
        pageDown();
        break;
      case IBUS_apostrophe:
        processLetter(keyval, keycode, state);
        break;
    }
    return TRUE;
  }

  /* 中文模式，编辑器为空 */
  if (full_punct_mode_) {
    switch (keyval) {
      case '$':
        commitStaticString("￥");
        break;
      case '^':
        commitStaticString("……");
        break;
      case '_':
        commitStaticString("——");
        break;
      case '[':
        commitStaticString("【");
        break;
      case ']':
        commitStaticString("】");
        break;
      case '{':
        commitStaticString("『");
        break;
      case '}':
        commitStaticString("』");
        break;
      case '\\':
        commitStaticString("、");
        break;
      case '\'':
        commitStaticString(single_quote_area_ ? "’" : "‘");
        single_quote_area_ = !single_quote_area_;
        break;
      case '\"':
        commitStaticString(double_quote_area_ ? "”" : "“");
        double_quote_area_ = !double_quote_area_;
        break;
      case '.':
        commitStaticString("。");
        break;
      case '<':
        commitStaticString("《");
        break;
      case '>':
        commitStaticString("》");
        break;
      default:
        commitPunct(keyval);
    }
  } else {
    commitFinalChars(keyval);
  }

  return TRUE;
}

/**
 * 处理其他字符.
 * @param keyval Key symbol of a key event.
 * @param keycode Keycode of a key event.
 * @param state Key modifier flags.
 * @return TRUE for successfully process the key; FALSE otherwise.
 */
gboolean PyeEngine::processOthers(guint keyval, guint keycode, guint state) {
  /* 如果有修饰键则不处理 */
  if (CMSHM_FILTER(state) != 0)
    return FALSE;

  /* 编辑器为空 */
  if (pinyin_editor_.IsEmpty())
    return FALSE;

  /* process some cursor control keys */
  switch (keyval) {
    case IBUS_Return:
    case IBUS_KP_Enter:
      commitRawPhrase();
      hideUI();
      resetEngine();
      return TRUE;
    case IBUS_BackSpace:
      if (!pinyin_editor_.RevokeSelectedPhrase())
        pinyin_editor_.BackspacePinyinKey();
      if (!pinyin_editor_.IsEmpty()) {
        updateUIData();
      } else {
        hideUI();
        resetEngine();
      }
      return TRUE;
    case IBUS_Up:
    case IBUS_KP_Up:
      cursorUp();
      return TRUE;
    case IBUS_Down:
    case IBUS_KP_Down:
      cursorDown();
      return TRUE;
    case IBUS_Page_Up:
    case IBUS_KP_Page_Up:
      pageUp();
      return TRUE;
    case IBUS_Page_Down:
    case IBUS_KP_Page_Down:
      pageDown();
      return TRUE;
    case IBUS_Escape:
      reset();
      return TRUE;
  }

  return FALSE;
}
