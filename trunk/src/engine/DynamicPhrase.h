//
// C++ Interface: DynamicPhrase
//
// Description:
// 生成动态词语数据.
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef __SRC_ENGINE_DYNAMICPHRASE_H
#define __SRC_ENGINE_DYNAMICPHRASE_H

#include "mess.h"

class DynamicPhrase
{
public:
	DynamicPhrase();
	~DynamicPhrase();

	GSList *GetDynamicPhrase(const char *string, guint *len);
private:
	GSList *GetDatePhrase(guint *len);
	GSList *GetTimePhrase(guint *len);
	GSList *GetWeekPhrase(guint *len);
};

#endif
