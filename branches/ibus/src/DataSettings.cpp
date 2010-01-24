//
// C++ Implementation: DataSettings
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "DataSettings.h"
#include "engine/support.h"

#define CONFIG_SECTION "engine/Pye"
#define CONFIG_NAME_RECTIFY "rectify_pinyin_pair"
#define CONFIG_NAME_FUZZY "fuzzy_pinyin_unit"
#define CONFIG_NAME_BAKGAP "backup_gap"
#define CONFIG_NAME_PAGE "page_size"
#define CONFIG_NAME_FLAGS "bit_flags"
#define CONFIG_NAME_EOF "--EOF--"

/* 模糊拼音 */
const gchar *DataSettings::fuzzystrv[][2] = {
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
const gchar *DataSettings::rectifystrv[][2] = {
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
DataSettings::DataSettings():busconfig(NULL), widset(NULL), dtset(NULL)
{
	g_datalist_init(&widset);
	g_datalist_init(&dtset);
}

/**
 * 类析构函数.
 */
DataSettings::~DataSettings()
{
	if (busconfig)
		g_object_unref(busconfig);
	g_datalist_clear(&widset);
	g_datalist_clear(&dtset);
}

/**
 * 程序数据设置入口.
 */
void DataSettings::ResetDataEntry(IBusConnection *conn)
{
	GtkWidget *dialog;
	GtkWidget *note, *label;

	/* 创建IBUS配置引擎 */
	busconfig = ibus_config_new(conn);

	/* 创建相关窗体 */
	dialog = CreateMainWindow();
	note = gtk_notebook_new();
	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), note, TRUE, TRUE, 0);
	label = gtk_label_new("常规选项");
	gtk_notebook_append_page(GTK_NOTEBOOK(note), CreateRoutine(), label);
	label = gtk_label_new("模糊拼音");
	gtk_notebook_append_page(GTK_NOTEBOOK(note), CreateFuzzy(), label);
	label = gtk_label_new("自动纠错");
	gtk_notebook_append_page(GTK_NOTEBOOK(note), CreateRectify(), label);

	/* 为配置项设置预设数据 */
	SetRoutineValue();
	SetFuzzyValue();
	SetRectifyValue();

	/* 显示并运行对话框 */
	gtk_widget_show_all(dialog);
	switch (gtk_dialog_run(GTK_DIALOG(dialog))) {
	case GTK_RESPONSE_OK:
		ExtractRoutineValue();
		ExtractFuzzyValue();
		ExtractRectifyValue();
		UpdateConfigData();
		break;
	case GTK_RESPONSE_CANCEL:
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
GtkWidget *DataSettings::CreateMainWindow()
{
	GtkWidget *dialog;

	dialog = gtk_dialog_new_with_buttons("Pye拼音输入法首选项", NULL,
				 GTK_DIALOG_NO_SEPARATOR,
				 GTK_STOCK_OK, GTK_RESPONSE_OK,
				 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
	gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
	gtk_container_set_border_width(GTK_CONTAINER(dialog), 5);
	gtk_widget_set_size_request(dialog, 400, 350);
	g_datalist_set_data(&widset, "dialog-widget", dialog);

	return dialog;
}

/**
 * 创建常规选项页面.
 * @return 根窗体
 */
GtkWidget *DataSettings::CreateRoutine()
{
	GtkWidget *box, *hbox;
	GtkWidget *widget, *label;

	box = gtk_vbox_new(FALSE, 0);
	/* 备份用户词语的时间间隔 */
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
	label = gtk_label_new("保存词语数据的周期(秒):");
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	widget = gtk_spin_button_new_with_range(30.0, 600.0, 1.0);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(widget), 0);
	gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(widget), FALSE);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, FALSE, 10);
	g_datalist_set_data(&widset, "bakgap-spinbutton-widget", widget);
	/* 查询表的页面大小 */
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
	label = gtk_label_new("候选词语的项数:");
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	widget = gtk_spin_button_new_with_range(3.0, 16.0, 1.0);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(widget), 0);
	gtk_spin_button_set_wrap(GTK_SPIN_BUTTON(widget), FALSE);
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, FALSE, 10);
	g_datalist_set_data(&widset, "pagesize-spinbutton-widget", widget);
	/* 光标是否可见 */
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
	label = gtk_label_new("标记当前的候选词语:");
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	widget = gtk_check_button_new();
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, FALSE, 10);
	g_datalist_set_data(&widset, "cursor-checkbutton-widget", widget);
	/* 合成词汇 */
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
	label = gtk_label_new("自动记忆动态词组:");
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	widget = gtk_check_button_new();
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, FALSE, 10);
	g_datalist_set_data(&widset, "compose-checkbutton-widget", widget);
	/* 自定义词汇 */
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
	label = gtk_label_new("自动记忆拼音词组:");
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	widget = gtk_check_button_new();
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, FALSE, 10);
	g_datalist_set_data(&widset, "customize-checkbutton-widget", widget);
	/* 词频调整 */
	hbox = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
	label = gtk_label_new("自动词频调整:");
	gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
	widget = gtk_check_button_new();
	gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, FALSE, 10);
	g_datalist_set_data(&widset, "frequency-checkbutton-widget", widget);

	return box;
}

/**
 * 创建模糊拼音页面.
 * @return 根窗体
 */
GtkWidget *DataSettings::CreateFuzzy()
{
	GtkWidget *box, *hbox;
	GtkWidget *widget;
	GSList *fuzzylist;
	const gchar *(*pptr)[2];
	guint8 count;

	/* 创建根窗体 */
	box = gtk_vbox_new(FALSE, 0);
	/* 置子窗体链表为空 */
	fuzzylist = NULL;

	/* 创建所有子窗体 */
	hbox = NULL;	//避免编译警告
	count = 0;
	pptr = fuzzystrv;
	while ((*pptr)[0]) {
		/* 保证每两个选项排成一列 */
		if ((count & 0x01) == 0x00) {
			hbox = gtk_hbox_new(FALSE, 0);
			gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
		}
		/* 创建子窗体 */
		widget = gtk_check_button_new_with_label((*pptr)[0]);
		gtk_widget_set_size_request(widget, 200, -1);	//保证子窗体能够对齐
		g_object_set_data(G_OBJECT(widget), "fuzzy-value", (gpointer)(*pptr)[1]);
		/* 判断子窗体的插入位置 */
		if ((count & 0x01) == 0x00)
			gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, FALSE, 0);
		else
			gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, FALSE, 0);
		/* 将子窗体加入窗体集&链表 */
		g_datalist_set_data(&widset, (*pptr)[1], widget);
		fuzzylist = g_slist_prepend(fuzzylist, widget);
		/* 准备下一个 */
		count++;
		pptr++;
	}

	/* 将子窗体链表加入数据集 */
	g_datalist_set_data_full(&dtset, "fuzzy-list", fuzzylist,
				 GDestroyNotify(g_slist_free));

	return box;
}

/**
 * 创建拼音修正页面.
 * @return 根窗体
 */
GtkWidget *DataSettings::CreateRectify()
{
	GtkWidget *box, *hbox;
	GtkWidget *widget;
	GSList *rectifylist;
	const gchar *(*pptr)[2];
	guint8 count;

	/* 创建根窗体 */
	box = gtk_vbox_new(FALSE, 0);
	/* 置子窗体链表为空 */
	rectifylist = NULL;

	/* 创建所有子窗体 */
	hbox = NULL;	//避免编译警告
	count = 0;
	pptr = rectifystrv;
	while ((*pptr)[0]) {
		/* 保证每两个选项排成一列 */
		if ((count & 0x01) == 0x00) {
			hbox = gtk_hbox_new(FALSE, 0);
			gtk_box_pack_start(GTK_BOX(box), hbox, FALSE, FALSE, 0);
		}
		/* 创建子窗体 */
		widget = gtk_check_button_new_with_label((*pptr)[0]);
		gtk_widget_set_size_request(widget, 200, -1);	//保证子窗体能够对齐
		g_object_set_data(G_OBJECT(widget), "rectify-value",
						 (gpointer)(*pptr)[1]);
		/* 判断子窗体的插入位置 */
		if ((count & 0x01) == 0x00)
			gtk_box_pack_start(GTK_BOX(hbox), widget, FALSE, FALSE, 0);
		else
			gtk_box_pack_end(GTK_BOX(hbox), widget, FALSE, FALSE, 0);
		/* 将子窗体加入窗体集&链表 */
		g_datalist_set_data(&widset, (*pptr)[1], widget);
		rectifylist = g_slist_prepend(rectifylist, widget);
		/* 准备下一个 */
		count++;
		pptr++;
	}

	/* 将子窗体链表加入数据集 */
	g_datalist_set_data_full(&dtset, "rectify-list", rectifylist,
					 GDestroyNotify(g_slist_free));

	return box;
}

/**
 * 为常规选项预设数据.
 */
void DataSettings::SetRoutineValue()
{
	GValue value = {0};
	GtkWidget *widget;
	time_t bakgap;
	guint pagesize;
	guint8 flags;

	/* 设置备份用户数据的时间间隔 */
	if (ibus_config_get_value(busconfig, CONFIG_SECTION,
				 CONFIG_NAME_BAKGAP, &value)) {
		if ((bakgap = g_value_get_int(&value)) < 30)
			bakgap = 30;
		g_value_unset(&value);
	} else
		bakgap = 60;
	widget = GTK_WIDGET(g_datalist_get_data(&widset, "bakgap-spinbutton-widget"));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), bakgap);

	/* 设置词语查询表的页面大小 */
	if (ibus_config_get_value(busconfig, CONFIG_SECTION,
				 CONFIG_NAME_PAGE, &value)) {
		if ((pagesize = g_value_get_int(&value)) < 3)
			pagesize = 3;
		g_value_unset(&value);
	} else
		pagesize = 5;
	widget = GTK_WIDGET(g_datalist_get_data(&widset, "pagesize-spinbutton-widget"));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(widget), pagesize);

	/* 设置相关标记位 */
	if (ibus_config_get_value(busconfig, CONFIG_SECTION,
				 CONFIG_NAME_FLAGS, &value)) {
		flags = g_value_get_int(&value);
		g_value_unset(&value);
	} else
		flags = (~0);
	widget = GTK_WIDGET(g_datalist_get_data(&widset, "cursor-checkbutton-widget"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), FLAG_ISSET(flags, 3));
	widget = GTK_WIDGET(g_datalist_get_data(&widset, "compose-checkbutton-widget"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), FLAG_ISSET(flags, 2));
	widget = GTK_WIDGET(g_datalist_get_data(&widset, "customize-checkbutton-widget"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), FLAG_ISSET(flags, 1));
	widget = GTK_WIDGET(g_datalist_get_data(&widset, "frequency-checkbutton-widget"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget), FLAG_ISSET(flags, 0));
}

/**
 * 为模糊拼音预设数据.
 */
void DataSettings::SetFuzzyValue()
{
	GValue value = {0};
	GtkWidget *widget;
	GSList *tlist;
	gchar *fzstr, *pptr, *ptr;

	/* 获取模糊拼音串 */
	if (ibus_config_get_value(busconfig, CONFIG_SECTION,
				 CONFIG_NAME_FUZZY, &value)) {
		fzstr = g_value_dup_string(&value);
		g_value_unset(&value);
	} else
		fzstr = NULL;

	/* 清除所有模糊拼音的选中标记 */
	tlist = (GSList *)g_datalist_get_data(&dtset, "fuzzy-list");
	while (tlist) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tlist->data), FALSE);
		tlist = g_slist_next(tlist);
	}

	/* 设置模糊拼音的选中标记 */
	if (fzstr && *fzstr != '\0') {
		pptr = fzstr;
		do {
			if ( (ptr = strchr(pptr, ';')))
				*ptr = '\0';
			if ( (widget = GTK_WIDGET(g_datalist_get_data(&widset, pptr))))
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),
										 TRUE);
			pptr = ptr + 1;
		} while (ptr && *pptr != '\0');
	}
	g_free(fzstr);
}

/**
 * 为拼音修正预设数据.
 */
void DataSettings::SetRectifyValue()
{
	GValue value = {0};
	GtkWidget *widget;
	GSList *tlist;
	gchar *rtfstr, *pptr, *ptr;

	/* 获取拼音修正表 */
	if (ibus_config_get_value(busconfig, CONFIG_SECTION,
				 CONFIG_NAME_RECTIFY, &value)) {
		rtfstr = g_value_dup_string(&value);
		g_value_unset(&value);
	} else
		rtfstr = NULL;


	/* 清除所有拼音修正的选中标记 */
	tlist = (GSList *)g_datalist_get_data(&dtset, "rectify-list");
	while (tlist) {
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(tlist->data), FALSE);
		tlist = g_slist_next(tlist);
	}

	/* 设置拼音修正的选中标记 */
	if (rtfstr && *rtfstr != '\0') {
		pptr = rtfstr;
		do {
			if ( (ptr = strchr(pptr, ';')))
				*ptr = '\0';
			if ( (widget = GTK_WIDGET(g_datalist_get_data(&widset, pptr))))
				gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widget),
										 TRUE);
			pptr = ptr + 1;
		} while (ptr && *pptr != '\0');
	}
	g_free(rtfstr);
}

/**
 * 更新常规选项数据.
 */
void DataSettings::ExtractRoutineValue()
{
	GValue value = {0};
	GtkWidget *widget;
	time_t bakgap;
	guint pagesize;
	guint8 flags;

	/* 更新备份用户词语的时间间隔 */
	widget = GTK_WIDGET(g_datalist_get_data(&widset, "bakgap-spinbutton-widget"));
	bakgap = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
	g_value_init(&value, G_TYPE_INT);
	g_value_set_int(&value, bakgap);
	ibus_config_set_value(busconfig, CONFIG_SECTION, CONFIG_NAME_BAKGAP, &value);
	g_value_unset(&value);

	/* 更新词语查询表的页面大小 */
	widget = GTK_WIDGET(g_datalist_get_data(&widset, "pagesize-spinbutton-widget"));
	pagesize = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(widget));
	g_value_init(&value, G_TYPE_INT);
	g_value_set_int(&value, pagesize);
	ibus_config_set_value(busconfig, CONFIG_SECTION, CONFIG_NAME_PAGE, &value);
	g_value_unset(&value);

	/* 更新相关位标记 */
	flags = (~0);
	widget = GTK_WIDGET(g_datalist_get_data(&widset, "cursor-checkbutton-widget"));
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
		FLAG_CLR(flags, 3);
	widget = GTK_WIDGET(g_datalist_get_data(&widset, "compose-checkbutton-widget"));
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
		FLAG_CLR(flags, 2);
	widget = GTK_WIDGET(g_datalist_get_data(&widset, "customize-checkbutton-widget"));
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
		FLAG_CLR(flags, 1);
	widget = GTK_WIDGET(g_datalist_get_data(&widset, "frequency-checkbutton-widget"));
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
		FLAG_CLR(flags, 0);
	g_value_init(&value, G_TYPE_INT);
	g_value_set_int(&value, flags);
	ibus_config_set_value(busconfig, CONFIG_SECTION, CONFIG_NAME_FLAGS, &value);
	g_value_unset(&value);
}

/**
 * 更新模糊拼音数据.
 */
void DataSettings::ExtractFuzzyValue()
{
	GValue value = {0};
	GSList *tlist, *strlist;
	const gchar *string;
	gchar *fzstr;
	gsize len, pos;

	/* 获取所有模糊拼音 */
	strlist = NULL;
	len = 0;
	tlist = (GSList *)g_datalist_get_data(&dtset, "fuzzy-list");
	while (tlist) {
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tlist->data))) {
			string = (const gchar *)g_object_get_data(G_OBJECT(tlist->data),
									 "fuzzy-value");
			strlist = g_slist_prepend(strlist, (gpointer)string);
			len += strlen(string) + 1;
		}
		tlist = g_slist_next(tlist);
	}

	/* 将模糊拼音放入串 */
	fzstr = (gchar *)g_malloc(len + 1);
	pos = 0;
	tlist = strlist;
	while (tlist) {
		snprintf(fzstr + pos, len - pos, "%s;", (const gchar *)tlist->data);
		pos += strlen((const gchar *)tlist->data) + 1;
		tlist = g_slist_next(tlist);
	}
	*(fzstr + len) = '\0';

	/* 更新数据 */
	g_value_init(&value, G_TYPE_STRING);
	g_value_set_string(&value, fzstr);
	ibus_config_set_value(busconfig, CONFIG_SECTION, CONFIG_NAME_FUZZY, &value);
	g_value_unset(&value);

	/* 释放资源 */
	g_slist_free(strlist);
	g_free(fzstr);
}

/**
 * 更新拼音修正数据.
 */
void DataSettings::ExtractRectifyValue()
{
	GValue value = {0};
	GSList *tlist, *strlist;
	const gchar *string;
	gchar *rtfstr;
	gsize len, pos;

	/* 获取所有拼音修正串 */
	strlist = NULL;
	len = 0;
	tlist = (GSList *)g_datalist_get_data(&dtset, "rectify-list");
	while (tlist) {
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(tlist->data))) {
			string = (const gchar *)g_object_get_data(G_OBJECT(tlist->data),
									 "rectify-value");
			strlist = g_slist_prepend(strlist, (gpointer)string);
			len += strlen(string) + 1;
		}
		tlist = g_slist_next(tlist);
	}

	/* 将拼音修正串放入串 */
	rtfstr = (gchar *)g_malloc(len + 1);
	pos = 0;
	tlist = strlist;
	while (tlist) {
		snprintf(rtfstr + pos, len - pos, "%s;", (const gchar *)tlist->data);
		pos += strlen((const gchar *)tlist->data) + 1;
		tlist = g_slist_next(tlist);
	}
	*(rtfstr + len) = '\0';

	/* 更新数据 */
	g_value_init(&value, G_TYPE_STRING);
	g_value_set_string(&value, rtfstr);
	ibus_config_set_value(busconfig, CONFIG_SECTION, CONFIG_NAME_RECTIFY, &value);
	g_value_unset(&value);

	/* 释放资源 */
	g_slist_free(strlist);
	g_free(rtfstr);
}

/**
 * 更新配置数据.
 */
void DataSettings::UpdateConfigData()
{
	GValue value = {0};

	g_value_init(&value, G_TYPE_BOOLEAN);
	g_value_set_boolean(&value, TRUE);
	ibus_config_set_value(busconfig, CONFIG_SECTION, CONFIG_NAME_EOF, &value);
	g_value_unset(&value);
}
