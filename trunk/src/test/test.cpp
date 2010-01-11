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
#include "../engine/PhraseEngine.h"
#include "../engine/PinyinEditor.h"

int main(int argc, char *argv[])
{
	GSList *tlist, *list;
	PhraseEngine *phregn;
	PhraseData *phrdt;
	guint length, count;
	gunichar2 *data;
	glong dtlen;
	char ch;

	phregn = PhraseEngine::GetInstance();
	phregn->CreateSysEngineUnits("mb.txt");
	phregn->CreateUserEngineUnit("user.mb");
	PinyinEditor pyedit(phregn);

	while ( (ch = getchar())) {
		length = 5;
		if (isalpha(ch)) {
			pyedit.InsertPinyinKey(ch);
			pyedit.GetPagePhrase(&list, &length);
			count = 1;
			tlist = list;
			while (tlist) {
				phrdt = (PhraseData *)tlist->data;
				printf("%u.%s ", count, g_utf16_to_utf8(phrdt->data,
						 phrdt->dtlen, NULL, NULL, NULL));
				count++;
				tlist = g_slist_next(tlist);
			}
			pyedit.GetPreeditText(&data, &dtlen);
			printf("\nPreedit: %s\n", g_utf16_to_utf8(data, dtlen, NULL, NULL, NULL));
			pyedit.GetAuxiliaryText(&data, &dtlen);
			printf("\nAuxiliary: %s\n", g_utf16_to_utf8(data, dtlen, NULL, NULL, NULL));
		} else if (isdigit(ch)) {
			if ( (tlist = g_slist_nth(list, ch - '1'))) {
				phrdt = (PhraseData *)tlist->data;
				pyedit.SelectCachePhrase(phrdt);
			}
			if (pyedit.IsFinishInquirePhrase()) {
				pyedit.GetCommitText(&data, &dtlen);
				printf("\nCommit: %s\n", g_utf16_to_utf8(data, dtlen, NULL, NULL, NULL));
				pyedit.FeedbackSelectedPhrase();
				pyedit.FinishInquirePhrase();
				phregn->BakUserEnginePhrase();
			} else {
				pyedit.GetPagePhrase(&list, &length);
				count = 1;
				tlist = list;
				while (tlist) {
					phrdt = (PhraseData *)tlist->data;
					printf("%u.%s ", count, g_utf16_to_utf8(phrdt->data,
					       phrdt->dtlen, NULL, NULL, NULL));
					count++;
					tlist = g_slist_next(tlist);
				}
				pyedit.GetPreeditText(&data, &dtlen);
				printf("\nPreedit: %s\n", g_utf16_to_utf8(data, dtlen, NULL, NULL, NULL));
				pyedit.GetAuxiliaryText(&data, &dtlen);
				printf("\nAuxiliary: %s\n", g_utf16_to_utf8(data, dtlen, NULL, NULL, NULL));
			}
		} else if (ch == '=') {
			pyedit.GetPagePhrase(&list, &length);
			count = 1;
			tlist = list;
			while (tlist) {
				phrdt = (PhraseData *)tlist->data;
				printf("%u.%s ", count, g_utf16_to_utf8(phrdt->data,
						 phrdt->dtlen, NULL, NULL, NULL));
				count++;
				tlist = g_slist_next(tlist);
			}
			pyedit.GetPreeditText(&data, &dtlen);
			printf("\nPreedit: %s\n", g_utf16_to_utf8(data, dtlen, NULL, NULL, NULL));
			pyedit.GetAuxiliaryText(&data, &dtlen);
			printf("\nAuxiliary: %s\n", g_utf16_to_utf8(data, dtlen, NULL, NULL, NULL));
		}
	}

	return 0;
}

