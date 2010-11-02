//
// C++ Implementation: parameter_settings
//
// Description:
// 请参见头文件描述.
//
// Author: Jally <jallyx@163.com>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "parameter_settings.h"
#include <string.h>

#define SECTION "engine/pye"
#define PAGESIZE "pagesize"
#define SPACE_FULLPUNCT "space_fullpunct"
#define CURSOR_VISIBLE "cursor_visible"
#define PHRASE_FREQUENCY_ADJUSTABLE "phrase_frequency_adjustable"
#define ENGINE_PHRASE_SAVABLE "engine_phrase_savable"
#define MANUAL_PHRASE_SAVABLE "manual_phrase_savable"
#define MEND_PINYIN_PAIR "mend_pinyin_pair"
#define FUZZY_PINYIN_PAIR "fuzzy_pinyin_pair"
#define BACKUP_CYCLE "backup_cycle"
#define END "--end--"

/* 模糊拼音 */
const gchar *ParameterSettings::fuzzy_strv_[][2] = {
  {"z = zh", "z|zh"},
  {"c = ch", "c|ch"},
  {"s = sh", "s|sh"},
  {"l = n", "l|n"},
  {"f = h", "f|h"},
  {"r = l", "r|l"},
  {"k = g", "k|g"},
  {"an = ang", "an|ang"},
  {"en = eng", "en|eng"},
  {"on = ong", "on|ong"},
  {"in = ing", "in|ing"},
  {"eng = ong", "eng|ong"},
  {"ian = iang", "ian|iang"},
  {"uan = uang", "uan|uang"},
  {NULL, NULL}
};
/* 拼音纠错 */
const gchar *ParameterSettings::mend_strv_[][2] = {
  {"ign ==> ing", "ign|ing"},
  {"img ==> ing", "img|ing"},
  {"uei ==> ui", "uei|ui"},
  {"uen ==> un", "uen|un"},
  {"iou ==> iu", "iou|iu"},
  {NULL, NULL}
};

/**
 * 类构造函数.
 */
ParameterSettings::ParameterSettings()
    : config_(NULL), widget_set_(NULL), data_set_(NULL) {
  g_datalist_init(&widget_set_);
  g_datalist_init(&data_set_);
}

/**
 * 类析构函数.
 */
ParameterSettings::~ParameterSettings() {
  if (config_)
    g_object_unref(config_);
  g_datalist_clear(&widget_set_);
  g_datalist_clear(&data_set_);
}

/**
 * 程序数据设置入口.
 */
void ParameterSettings::entry(IBusConnection *conn) {
  /* 创建IBUS配置引擎 */
  config_ = ibus_config_new(conn);

  /* 创建相关窗体 */
  GtkWidget *dialog = createMainWindow();
  GtkWidget *notebook = gtk_notebook_new();
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), notebook, TRUE, TRUE, 0);
  GtkWidget *label = gtk_label_new("常规选项");
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), createRoutine(), label);
  label = gtk_label_new("模糊拼音");
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), createFuzzy(), label);
  label = gtk_label_new("自动纠错");
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), createMend(), label);

  /* 为配置项设置预设数据 */
  setRoutineValue();
  setFuzzyValue();
  setMendValue();

  /* 显示并运行对话框 */
  gtk_widget_show_all(dialog);
  switch (gtk_dialog_run(GTK_DIALOG(dialog))) {
    case GTK_RESPONSE_CLOSE:
      extractRoutineValue();
      extractFuzzyValue();
      extractMendValue();
      updateConfig();
      break;
    default:
      break;
  }
  /* 销毁对话框 */
  gtk_widget_destroy(dialog);
}

/**
 * 创建程序主窗口.
 * @return 主窗口
 */
GtkWidget *ParameterSettings::createMainWindow() {
  GtkWidget *dialog = gtk_dialog_new_with_buttons(
      "Pye拼音输入法首选项", NULL,
      GTK_DIALOG_NO_SEPARATOR,
      GTK_STOCK_CLOSE, GTK_RESPONSE_CLOSE, NULL);
  gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
  gtk_container_set_border_width(GTK_CONTAINER(dialog), 5);
  gtk_widget_set_size_request(dialog, 400, 350);
  g_datalist_set_data(&widget_set_, "dialog-widget", dialog);
  return dialog;
}

/**
 * 创建常规选项页面.
 * @return 根窗体
 */
GtkWidget *ParameterSettings::createRoutine() {
  GtkWidget *box = gtk_vbox_new(FALSE, 0);

  /* 备份用户词语的时间间隔 */
  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
  GtkWidget *label = gtk_label_new("备份用户词语数据的时间周期:");
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 10);
  GtkWidget *widget = gtk_spin_button_new_with_range(30.0, 600.0, 1.0);
  gtk_spin_button_set_digits(GTK_SPIN_BUTTON(widget), 0);
  gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(widget), FALSE);
  gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, FALSE, 10);
  g_datalist_set_data(&widget_set_, "backup-spinbutton-widget", widget);

  /* 查询表的页面大小 */
  hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
  label = gtk_label_new("候选词语的项数:");
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 10);
  widget = gtk_spin_button_new_with_range(3.0, 16.0, 1.0);
  gtk_spin_button_set_digits(GTK_SPIN_BUTTON(widget), 0);
  gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(widget), FALSE);
  gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, FALSE, 10);
  g_datalist_set_data(&widget_set_, "pagesize-spinbutton-widget", widget);

  /* 全角空格 */
  hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
  label = gtk_label_new("中文模式下输入全角空格:");
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 10);
  widget = gtk_check_button_new();
  gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, FALSE, 10);
  g_datalist_set_data(&widget_set_, "space-checkbutton-widget", widget);

  /* 光标是否可见 */
  hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
  label = gtk_label_new("高亮显示当前的候选词语:");
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 10);
  widget = gtk_check_button_new();
  gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, FALSE, 10);
  g_datalist_set_data(&widget_set_, "cursor-checkbutton-widget", widget);

  /* 词频调整 */
  hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
  label = gtk_label_new("自动调整词频:");
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 10);
  widget = gtk_check_button_new();
  gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, FALSE, 10);
  g_datalist_set_data(&widget_set_, "frequency-checkbutton-widget", widget);

  /* 引擎合成词汇 */
  hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
  label = gtk_label_new("自动记忆动态词组:");
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 10);
  widget = gtk_check_button_new();
  gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, FALSE, 10);
  g_datalist_set_data(&widget_set_, "engine-checkbutton-widget", widget);

  /* 用户合成词汇 */
  hbox = gtk_hbox_new(FALSE, 0);
  gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
  label = gtk_label_new("自动记忆拼音词组:");
  gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 10);
  widget = gtk_check_button_new();
  gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, FALSE, 10);
  g_datalist_set_data(&widget_set_, "manual-checkbutton-widget", widget);

  return box;
}

/**
 * 创建模糊拼音页面.
 * @return 根窗体
 */
GtkWidget *ParameterSettings::createFuzzy() {
  /* 创建根窗体 */
  GtkWidget *box = gtk_vbox_new(FALSE, 0);
  /* 置子窗体链表为空 */
  GSList *widget_list = NULL;

  /* 创建所有子窗体 */
  guint count = 0;
  GtkWidget *hbox = NULL;
  const gchar *(*pptr)[2] = fuzzy_strv_;
  while ((*pptr)[0]) {
    /* 保证每两个选项排成一列 */
    if (count % 2 == 0) {
      hbox = gtk_hbox_new(FALSE, 0);
      gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
    }
    /* 创建子窗体 */
    GtkWidget *widget = gtk_check_button_new_with_label((*pptr)[0]);
    gtk_widget_set_size_request(widget, 200, -1);  //保证子窗体能够对齐
    g_object_set_data(G_OBJECT(widget), "value", (gpointer)(*pptr)[1]);
    /* 插入子窗体 */
    gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, FALSE, 0);
    /* 将子窗体加入窗体集&链表 */
    g_datalist_set_data(&widget_set_, (*pptr)[1], widget);
    widget_list = g_slist_prepend(widget_list, widget);
    /* 准备下一个 */
    ++count;
    ++pptr;
  }

  /* 将子窗体链表加入数据集 */
  g_datalist_set_data_full(&data_set_, "fuzzy-widget-list", widget_list,
                           GDestroyNotify(g_slist_free));

  return box;
}

/**
 * 创建拼音修正页面.
 * @return 根窗体
 */
GtkWidget *ParameterSettings::createMend() {
  /* 创建根窗体 */
  GtkWidget *box = gtk_vbox_new(FALSE, 0);
  /* 置子窗体链表为空 */
  GSList *widget_list = NULL;

  /* 创建所有子窗体 */
  guint count = 0;
  GtkWidget *hbox = NULL;
  const gchar *(*pptr)[2] = mend_strv_;
  while ((*pptr)[0]) {
    /* 保证每两个选项排成一列 */
    if (count % 2 == 0) {
      hbox = gtk_hbox_new(FALSE, 0);
      gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
    }
    /* 创建子窗体 */
    GtkWidget *widget = gtk_check_button_new_with_label((*pptr)[0]);
    gtk_widget_set_size_request(widget, 200, -1);  //保证子窗体能够对齐
    g_object_set_data(G_OBJECT(widget), "value", (gpointer)(*pptr)[1]);
    /* 插入子窗体 */
    gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, FALSE, 0);
    /* 将子窗体加入窗体集&链表 */
    g_datalist_set_data(&widget_set_, (*pptr)[1], widget);
    widget_list = g_slist_prepend(widget_list, widget);
    /* 准备下一个 */
    ++count;
    ++pptr;
  }

  /* 将子窗体链表加入数据集 */
  g_datalist_set_data_full(&data_set_, "mend-widget-list", widget_list,
                           GDestroyNotify(g_slist_free));

  return box;
}

/**
 * 为常规选项预设数据.
 */
void ParameterSettings::setRoutineValue() {
  /* 设置备份用户数据的时间间隔 */
  GValue value = {0};
  guint backup_cycle = 60;
  if (ibus_config_get_value(config_, SECTION, BACKUP_CYCLE, &value)) {
    if ((backup_cycle = g_value_get_int(&value)) < 30)
      backup_cycle = 30;
    g_value_unset(&value);
  }
  GtkWidget *widget =
      GTK_WIDGET(g_datalist_get_data(&widget_set_, "backup-spinbutton-widget"));
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), backup_cycle);

  /* 设置词语查询表的页面大小 */
  guint pagesize = 5;
  if (ibus_config_get_value(config_, SECTION, PAGESIZE, &value)) {
    if ((pagesize = g_value_get_int(&value)) < 3)
      pagesize = 3;
    g_value_unset(&value);
  }
  widget = GTK_WIDGET(g_datalist_get_data(&widget_set_, "pagesize-spinbutton-widget"));
  gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), pagesize);

  /* 设置空格全角 */
  gboolean space_fullpunct = TRUE;
  if (ibus_config_get_value(config_, SECTION, SPACE_FULLPUNCT, &value)) {
    space_fullpunct = g_value_get_boolean(&value);
    g_value_unset(&value);
  }
  widget = GTK_WIDGET(g_datalist_get_data(&widget_set_, "space-checkbutton-widget"));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), space_fullpunct);

  /* 设置光标是否可见 */
  gboolean cursor_visible = TRUE;
  if (ibus_config_get_value(config_, SECTION, CURSOR_VISIBLE, &value)) {
    cursor_visible = g_value_get_boolean(&value);
    g_value_unset(&value);
  }
  widget = GTK_WIDGET(g_datalist_get_data(&widget_set_, "cursor-checkbutton-widget"));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), cursor_visible);

  /* 设置词频是否调整 */
  gboolean phrase_frequency_adjustable = TRUE;
  if (ibus_config_get_value(config_, SECTION, PHRASE_FREQUENCY_ADJUSTABLE, &value)) {
    phrase_frequency_adjustable = g_value_get_boolean(&value);
    g_value_unset(&value);
  }
  widget = GTK_WIDGET(g_datalist_get_data(&widget_set_, "frequency-checkbutton-widget"));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), phrase_frequency_adjustable);

  /* 是否保存引擎合成词语 */
  gboolean engine_phrase_savable = TRUE;
  if (ibus_config_get_value(config_, SECTION, ENGINE_PHRASE_SAVABLE, &value)) {
    engine_phrase_savable = g_value_get_boolean(&value);
    g_value_unset(&value);
  }
  widget = GTK_WIDGET(g_datalist_get_data(&widget_set_, "engine-checkbutton-widget"));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), engine_phrase_savable);

  /* 是否保存用户合成词语 */
  gboolean manual_phrase_savable = TRUE;
  if (ibus_config_get_value(config_, SECTION, MANUAL_PHRASE_SAVABLE, &value)) {
    manual_phrase_savable = g_value_get_boolean(&value);
    g_value_unset(&value);
  }
  widget = GTK_WIDGET(g_datalist_get_data(&widget_set_, "manual-checkbutton-widget"));
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), manual_phrase_savable);
}

/**
 * 为模糊拼音预设数据.
 */
void ParameterSettings::setFuzzyValue() {
  /* 获取模糊拼音串 */
  GValue value = {0};
  gchar *fuzzy_pair = NULL;
  if (ibus_config_get_value(config_, SECTION, FUZZY_PINYIN_PAIR, &value)) {
    fuzzy_pair = g_value_dup_string(&value);
    g_value_unset(&value);
  }

  /* 清除所有模糊拼音的选中标记 */
  GSList *widget_list = (GSList *)g_datalist_get_data(&data_set_, "fuzzy-widget-list");
  for (; widget_list; widget_list = g_slist_next(widget_list))
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget_list->data), FALSE);

  /* 设置模糊拼音的选中标记 */
  if (fuzzy_pair && *fuzzy_pair != '\0') {
    gchar *pptr = fuzzy_pair;
    gchar *ptr = NULL;
    do {
      if ( (ptr = strchr(pptr, ';')))
        *ptr = '\0';
      GtkWidget *widget = GTK_WIDGET(g_datalist_get_data(&widget_set_, pptr));
      if (widget)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), TRUE);
      pptr = ptr + 1;
    } while (ptr && *pptr != '\0');
  }

  /* 释放资源 */
  g_free(fuzzy_pair);
}

/**
 * 为拼音修正预设数据.
 */
void ParameterSettings::setMendValue() {
  /* 获取修正拼音串 */
  GValue value = {0};
  gchar *mend_pair = NULL;
  if (ibus_config_get_value(config_, SECTION, MEND_PINYIN_PAIR, &value)) {
    mend_pair = g_value_dup_string(&value);
    g_value_unset(&value);
  }

  /* 清除所有修正拼音的选中标记 */
  GSList *widget_list = (GSList *)g_datalist_get_data(&data_set_, "mend-widget-list");
  for (; widget_list; widget_list = g_slist_next(widget_list))
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget_list->data), FALSE);

  /* 设置修正拼音的选中标记 */
  if (mend_pair && *mend_pair != '\0') {
    gchar *pptr = mend_pair;
    gchar *ptr = NULL;
    do {
      if ( (ptr = strchr(pptr, ';')))
        *ptr = '\0';
      GtkWidget *widget = GTK_WIDGET(g_datalist_get_data(&widget_set_, pptr));
      if (widget)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), TRUE);
      pptr = ptr + 1;
    } while (ptr && *pptr != '\0');
  }

  /* 释放资源 */
  g_free(mend_pair);
}

/**
 * 更新常规选项数据.
 */
void ParameterSettings::extractRoutineValue() {
  /* 更新备份用户词语的时间间隔 */
  GValue value = {0};
  GtkWidget *widget = GTK_WIDGET(g_datalist_get_data(&widget_set_, "backup-spinbutton-widget"));
  guint backup_cycle = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
  g_value_init(&value, G_TYPE_INT);
  g_value_set_int(&value, backup_cycle);
  ibus_config_set_value(config_, SECTION, BACKUP_CYCLE, &value);
  g_value_unset(&value);

  /* 更新词语查询表的页面大小 */
  widget = GTK_WIDGET(g_datalist_get_data(&widget_set_, "pagesize-spinbutton-widget"));
  guint pagesize = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
  g_value_init(&value, G_TYPE_INT);
  g_value_set_int(&value, pagesize);
  ibus_config_set_value(config_, SECTION, PAGESIZE, &value);
  g_value_unset(&value);

  /* 更新空格全角标记 */
  widget = GTK_WIDGET(g_datalist_get_data(&widget_set_, "space-checkbutton-widget"));
  gboolean space_fullpunct = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
  g_value_init(&value, G_TYPE_BOOLEAN);
  g_value_set_boolean(&value, space_fullpunct);
  ibus_config_set_value(config_, SECTION, SPACE_FULLPUNCT, &value);
  g_value_unset(&value);

  /* 更新光标可见标记 */
  widget = GTK_WIDGET(g_datalist_get_data(&widget_set_, "cursor-checkbutton-widget"));
  gboolean cursor_visible = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
  g_value_init(&value, G_TYPE_BOOLEAN);
  g_value_set_boolean(&value, cursor_visible);
  ibus_config_set_value(config_, SECTION, CURSOR_VISIBLE, &value);
  g_value_unset(&value);

  /* 更新词频调整标记 */
  widget = GTK_WIDGET(g_datalist_get_data(&widget_set_, "frequency-checkbutton-widget"));
  gboolean phrase_frequency_adjustable = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
  g_value_init(&value, G_TYPE_BOOLEAN);
  g_value_set_boolean(&value, phrase_frequency_adjustable);
  ibus_config_set_value(config_, SECTION, PHRASE_FREQUENCY_ADJUSTABLE, &value);
  g_value_unset(&value);

  /* 更新保存引擎合成词语标记 */
  widget = GTK_WIDGET(g_datalist_get_data(&widget_set_, "engine-checkbutton-widget"));
  gboolean engine_phrase_savable = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
  g_value_init(&value, G_TYPE_BOOLEAN);
  g_value_set_boolean(&value, engine_phrase_savable);
  ibus_config_set_value(config_, SECTION, ENGINE_PHRASE_SAVABLE, &value);
  g_value_unset(&value);

  /* 更新保存用户合成词语标记 */
  widget = GTK_WIDGET(g_datalist_get_data(&widget_set_, "manual-checkbutton-widget"));
  gboolean manual_phrase_savable = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
  g_value_init(&value, G_TYPE_BOOLEAN);
  g_value_set_boolean(&value, manual_phrase_savable);
  ibus_config_set_value(config_, SECTION, MANUAL_PHRASE_SAVABLE, &value);
  g_value_unset(&value);
}

/**
 * 更新模糊拼音数据.
 */
void ParameterSettings::extractFuzzyValue() {
  /* 获取所有模糊拼音 */
  GSList *pair_list = NULL;
  guint len = 0;
  GSList *widget_list = (GSList *)g_datalist_get_data(&data_set_, "fuzzy-widget-list");
  for (; widget_list; widget_list = g_slist_next(widget_list)) {
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget_list->data))) {
      const gchar *pair = (const gchar *)g_object_get_data(G_OBJECT(widget_list->data), "value");
      pair_list = g_slist_prepend(pair_list, (gpointer)pair);
      len += strlen(pair) + 1;
    }
  }

  /* 将模糊拼音放入串 */
  gchar *fuzzy_pair = (gchar *)g_malloc(len + 1);
  gchar *ptr = fuzzy_pair;
  for (GSList *tlist = pair_list; tlist; tlist = g_slist_next(tlist)) {
    sprintf(ptr, "%s;", (const gchar *)tlist->data);
    ptr += strlen(ptr);
  }
  *ptr = '\0';

  /* 更新数据 */
  GValue value = {0};
  g_value_init(&value, G_TYPE_STRING);
  g_value_set_string(&value, fuzzy_pair);
  ibus_config_set_value(config_, SECTION, FUZZY_PINYIN_PAIR, &value);
  g_value_unset(&value);

  /* 释放资源 */
  g_slist_free(pair_list);
  g_free(fuzzy_pair);
}

/**
 * 更新拼音修正数据.
 */
void ParameterSettings::extractMendValue() {
  /* 获取所有修正拼音 */
  GSList *pair_list = NULL;
  guint len = 0;
  GSList *widget_list = (GSList *)g_datalist_get_data(&data_set_, "mend-widget-list");
  for (; widget_list; widget_list = g_slist_next(widget_list)) {
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget_list->data))) {
      const gchar *pair = (const gchar *)g_object_get_data(G_OBJECT(widget_list->data), "value");
      pair_list = g_slist_prepend(pair_list, (gpointer)pair);
      len += strlen(pair) + 1;
    }
  }

  /* 将修正拼音放入串 */
  gchar *mend_pair = (gchar *)g_malloc(len + 1);
  gchar *ptr = mend_pair;
  for (GSList *tlist = pair_list; tlist; tlist = g_slist_next(tlist)) {
    sprintf(ptr, "%s;", (const gchar *)tlist->data);
    ptr += strlen(ptr);
  }
  *ptr = '\0';

  /* 更新数据 */
  GValue value = {0};
  g_value_init(&value, G_TYPE_STRING);
  g_value_set_string(&value, mend_pair);
  ibus_config_set_value(config_, SECTION, MEND_PINYIN_PAIR, &value);
  g_value_unset(&value);

  /* 释放资源 */
  g_slist_free(pair_list);
  g_free(mend_pair);
}

/**
 * 更新配置数据.
 */
void ParameterSettings::updateConfig() {
  GValue value = {0};
  g_value_init(&value, G_TYPE_INT);
  g_value_set_int(&value, time(NULL));
  ibus_config_set_value(config_, SECTION, END, &value);
  g_value_unset(&value);
}
