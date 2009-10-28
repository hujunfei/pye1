//
// C++ Interface: PinyinEditor
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef __SRC_ENGINE_PINYINEDITOR_H
#define __SRC_ENGINE_PINYINEDITOR_H

#include "PhraseEngine.h"

class PinyinEditor
{
public:
	PinyinEditor(PhraseEngine *engine);
	~PinyinEditor();

	bool SyncData();
	bool MoveCursorPoint(int offset);
	bool InsertPinyinKey(char ch);
	bool DeletePinyinKey();
	bool BackspacePinyinKey();
	bool RevokeSelectedPhrase();
	bool GetCommitText(gunichar2 **text, glong *len);
	bool GetPreeditText(gunichar2 **text, glong *len);
	bool GetAuxiliaryText(gunichar2 **text, glong *len);
	bool GetPagePhrase(GSList **list, guint *len);
	bool IsExistCachePhrase(const PhraseData *phrdt);
	bool SelectCachePhrase(PhraseData *phrdt);
	bool FeedbackSelectedPhrase();
	bool IsFinishInquirePhrase();
	bool FinishInquirePhrase();
private:
	PhraseData *CreateUserPhrase();
	void CreateCharsIndex();
	void InquirePhraseIndex();
	int ComputeInquireOffset();
	char *CorrectPinyinString();
	EunitPhrase *SearchPreferEunitPhrase();
	void ClearEngineUnitBuffer();
	void ClearPinyinEditorBuffer();
	void ClearPinyinEditorOldBuffer();
	void ClearPinyinEditorTempBuffer();

	GArray *pytable;	///< 待查询拼音表
	guint cursor;		///< 光标位置
	CharsIndex *chidx;	///< 汉字索引数组
	int chlen;		///< 汉字索引数组长度
	GSList *aclist;		///< 已接受词语链表
	GSList *cclist;		///< 缓冲词语链表

	const PhraseEngine *phregn;	///< 词语引擎
	GSList *euphrlist;	///< 引擎词语索引缓冲点链表
	time_t timestamp;	///< 引擎缓冲数据的时间戳
};

#endif
