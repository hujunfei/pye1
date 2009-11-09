//
// C++ Implementation: DynamicPhrase
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "DynamicPhrase.h"
#include "ParseString.h"

DynamicPhrase::DynamicPhrase()
{
}

DynamicPhrase::~DynamicPhrase()
{
}

/**
 * 获取动态词语数据.
 * @param string 源串
 * @retval len 词语数据链表有效长度
 * @return 词语数据链表
 */
GSList *DynamicPhrase::GetDynamicPhrase(const char *string, guint *len)
{
	*len = 0;
	if (strcmp(string, "rq") == 0)
		return GetDatePhrase(len);
	if (strcmp(string, "sj") == 0)
		return GetTimePhrase(len);
	if (strcmp(string, "xq") == 0 || strcmp(string, "lb") == 0)
		return GetWeekPhrase(len);
	return NULL;
}

/**
 * 获取日期动态词语数据.
 * @retval len 词语数据链表有效长度
 * @return 词语数据链表
 */
GSList *DynamicPhrase::GetDatePhrase(guint *len)
{
	ParseString parse;
	CharsIndex chidx[2];
	PhraseData *phrdt;
	GSList *phrlist;
	gchar *ptr;
	struct tm *tm;
	time_t tt;

	/* 预备工作 */
	chidx[0].major = parse.GetStringIndex("r");
	chidx[1].major = parse.GetStringIndex("q");
	*len = 0;
	phrlist = NULL;
	time(&tt);
	tm = localtime(&tt);

	/* 2009年11月2日 */
	phrdt = new PhraseData;
	phrdt->chidx = new CharsIndex[G_N_ELEMENTS(chidx)];
	memcpy(phrdt->chidx, chidx, sizeof(chidx));
	phrdt->chlen = G_N_ELEMENTS(chidx);
	phrdt->offset = 0;	//标记这是无效词汇
	ptr = g_strdup_printf("%d年%d月%d日", tm->tm_year + 1900,
				 tm->tm_mon + 1, tm->tm_mday);
	phrdt->data = g_utf8_to_utf16(ptr, strlen(ptr), NULL, &phrdt->dtlen, NULL);
	g_free(ptr);
	phrlist = g_slist_append(phrlist, phrdt);
	(*len)++;

	/* 2009-11-02 */
	phrdt = new PhraseData;
	phrdt->chidx = new CharsIndex[G_N_ELEMENTS(chidx)];
	memcpy(phrdt->chidx, chidx, sizeof(chidx));
	phrdt->chlen = G_N_ELEMENTS(chidx);
	phrdt->offset = 0;	//标记这是无效词汇
	ptr = g_strdup_printf("%04d-%02d-%02d", tm->tm_year + 1900,
					 tm->tm_mon + 1, tm->tm_mday);
	phrdt->data = g_utf8_to_utf16(ptr, strlen(ptr), NULL, &phrdt->dtlen, NULL);
	g_free(ptr);
	phrlist = g_slist_append(phrlist, phrdt);
	(*len)++;

	/* 2009.11.02 */
	phrdt = new PhraseData;
	phrdt->chidx = new CharsIndex[G_N_ELEMENTS(chidx)];
	memcpy(phrdt->chidx, chidx, sizeof(chidx));
	phrdt->chlen = G_N_ELEMENTS(chidx);
	phrdt->offset = 0;	//标记这是无效词汇
	ptr = g_strdup_printf("%04d.%02d.%02d", tm->tm_year + 1900,
					 tm->tm_mon + 1, tm->tm_mday);
	phrdt->data = g_utf8_to_utf16(ptr, strlen(ptr), NULL, &phrdt->dtlen, NULL);
	g_free(ptr);
	phrlist = g_slist_append(phrlist, phrdt);
	(*len)++;

	/* 11/02/2009 */
	phrdt = new PhraseData;
	phrdt->chidx = new CharsIndex[G_N_ELEMENTS(chidx)];
	memcpy(phrdt->chidx, chidx, sizeof(chidx));
	phrdt->chlen = G_N_ELEMENTS(chidx);
	phrdt->offset = 0;	//标记这是无效词汇
	ptr = g_strdup_printf("%02d/%02d/%04d", tm->tm_mon + 1, tm->tm_mday,
							 tm->tm_year + 1900);
	phrdt->data = g_utf8_to_utf16(ptr, strlen(ptr), NULL, &phrdt->dtlen, NULL);
	g_free(ptr);
	phrlist = g_slist_append(phrlist, phrdt);
	(*len)++;

	return phrlist;
}

/**
 * 获取时间动态词语数据.
 * @retval len 词语数据链表有效长度
 * @return 词语数据链表
 */
GSList *DynamicPhrase::GetTimePhrase(guint *len)
{
	ParseString parse;
	CharsIndex chidx[2];
	PhraseData *phrdt;
	GSList *phrlist;
	gchar *ptr;
	struct tm *tm;
	time_t tt;

	/* 预备工作 */
	chidx[0].major = parse.GetStringIndex("s");
	chidx[1].major = parse.GetStringIndex("j");
	*len = 0;
	phrlist = NULL;
	time(&tt);
	tm = localtime(&tt);

	/* 12时45分27秒 */
	phrdt = new PhraseData;
	phrdt->chidx = new CharsIndex[G_N_ELEMENTS(chidx)];
	memcpy(phrdt->chidx, chidx, sizeof(chidx));
	phrdt->chlen = G_N_ELEMENTS(chidx);
	phrdt->offset = 0;	//标记这是无效词汇
	ptr = g_strdup_printf("%d时%d分%d秒", tm->tm_hour, tm->tm_min, tm->tm_sec);
	phrdt->data = g_utf8_to_utf16(ptr, strlen(ptr), NULL, &phrdt->dtlen, NULL);
	g_free(ptr);
	phrlist = g_slist_append(phrlist, phrdt);
	(*len)++;

	/* 12:45:27 */
	phrdt = new PhraseData;
	phrdt->chidx = new CharsIndex[G_N_ELEMENTS(chidx)];
	memcpy(phrdt->chidx, chidx, sizeof(chidx));
	phrdt->chlen = G_N_ELEMENTS(chidx);
	phrdt->offset = 0;	//标记这是无效词汇
	ptr = g_strdup_printf("%02d:%02d:%02d", tm->tm_hour, tm->tm_min, tm->tm_sec);
	phrdt->data = g_utf8_to_utf16(ptr, strlen(ptr), NULL, &phrdt->dtlen, NULL);
	g_free(ptr);
	phrlist = g_slist_append(phrlist, phrdt);
	(*len)++;

	return phrlist;
}

/**
 * 获取星期动态词语数据.
 * @retval len 词语数据链表有效长度
 * @return 词语数据链表
 */
GSList *DynamicPhrase::GetWeekPhrase(guint *len)
{
	ParseString parse;
	CharsIndex chidx[2];
	PhraseData *phrdt;
	GSList *phrlist;
	gchar *ptr;
	const char *tstr;
	struct tm *tm;
	time_t tt;

	/* 预备工作 */
	chidx[0].major = parse.GetStringIndex("x");
	chidx[1].major = parse.GetStringIndex("q");
	*len = 0;
	phrlist = NULL;
	time(&tt);
	tm = localtime(&tt);

	/* 获取数字的汉字表示 */
	tstr = NULL;
	switch (tm->tm_wday) {
	case 0: tstr = "日"; break;
	case 1: tstr = "一"; break;
	case 2: tstr = "二"; break;
	case 3: tstr = "三"; break;
	case 4: tstr = "四"; break;
	case 5: tstr = "五"; break;
	case 6: tstr = "六"; break;
	}

	/* 星期一 */
	phrdt = new PhraseData;
	phrdt->chidx = new CharsIndex[G_N_ELEMENTS(chidx)];
	memcpy(phrdt->chidx, chidx, sizeof(chidx));
	phrdt->chlen = G_N_ELEMENTS(chidx);
	phrdt->offset = 0;	//标记这是无效词汇
	ptr = g_strdup_printf("星期%s", tstr ? tstr : "");
	phrdt->data = g_utf8_to_utf16(ptr, strlen(ptr), NULL, &phrdt->dtlen, NULL);
	g_free(ptr);
	phrlist = g_slist_append(phrlist, phrdt);
	(*len)++;

	/* 礼拜一 */
	phrdt = new PhraseData;
	phrdt->chidx = new CharsIndex[G_N_ELEMENTS(chidx)];
	memcpy(phrdt->chidx, chidx, sizeof(chidx));
	phrdt->chlen = G_N_ELEMENTS(chidx);
	phrdt->offset = 0;	//标记这是无效词汇
	ptr = g_strdup_printf("礼拜%s", tstr ? tstr : "");
	phrdt->data = g_utf8_to_utf16(ptr, strlen(ptr), NULL, &phrdt->dtlen, NULL);
	g_free(ptr);
	phrlist = g_slist_append(phrlist, phrdt);
	(*len)++;

	/* 周一 */
	phrdt = new PhraseData;
	phrdt->chidx = new CharsIndex[G_N_ELEMENTS(chidx)];
	memcpy(phrdt->chidx, chidx, sizeof(chidx));
	phrdt->chlen = G_N_ELEMENTS(chidx);
	phrdt->offset = 0;	//标记这是无效词汇
	ptr = g_strdup_printf("周%s", tstr ? tstr : "");
	phrdt->data = g_utf8_to_utf16(ptr, strlen(ptr), NULL, &phrdt->dtlen, NULL);
	g_free(ptr);
	phrlist = g_slist_append(phrlist, phrdt);
	(*len)++;

	return phrlist;
}
