/***************************************************************************
 *   Copyright (C) 2009 by Jally   *
 *   jallyx@163.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "engine/InquireSysPhrase.h"
#include "engine/InquireUserPhrase.h"
#include "engine/ParseString.h"

int main(int argc, char *argv[])
{
	InquireSysPhrase sysphrase;
	InquireUserPhrase userphrase;
	ParseString parse;
	CharsIndex *chidx;
	int length;
	PhraseData *data;
	char buf[100];
	int8_t fuzzy[57];
	GSList *list;

#if 0
	sysphrase.CreateIndexTree("pinyin.mb");
	userphrase.CreateIndexTree("user.mb");
	memset(fuzzy, -1, 57);
	fuzzy[30] = 56, fuzzy[56] = 30;
	fuzzy[14] = 33, fuzzy[33] = 14;
	sysphrase.SetFuzzyPinyinUnits(fuzzy);

	while (gets(buf)) {
		parse.ParsePinyinString(buf, &chidx, &length);
		list = sysphrase.SearchMatchPhrase(chidx, length);
		while (list) {
			data = sysphrase.AnalysisPhraseIndex((PhraseIndex *)list->data);
			userphrase.AttachPhraseToTree(data);
			printf("%s\n", g_utf16_to_utf8(data->data, data->dtlen, NULL, NULL, NULL));
			list = g_slist_next(list);
		}
		userphrase.WritePhraseIndexTree();
	}
#else
	userphrase.CreateIndexTree("user.mb");
	memset(fuzzy, -1, 57);
	fuzzy[30] = 56, fuzzy[56] = 30;
	fuzzy[14] = 33, fuzzy[33] = 14;
	userphrase.SetFuzzyPinyinUnits(fuzzy);

	while (gets(buf)) {
		parse.ParsePinyinString(buf, &chidx, &length);
		list = userphrase.SearchMatchPhrase(chidx, length);
		while (list) {
			data = userphrase.AnalysisPhraseIndex((PhraseIndex *)list->data);
			printf("%s\n", g_utf16_to_utf8(data->data, data->dtlen, NULL, NULL, NULL));
			list = g_slist_next(list);
		}
	}
#endif
	return 0;
}

