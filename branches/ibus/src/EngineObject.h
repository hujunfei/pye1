//
// C++ Interface: EngineObject
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef __SRC_ENGINEOBJECT_H
#define __SRC_ENGINEOBJECT_H

#include "include/deplib.h"

#define IBUS_TYPE_PINYIN_ENGINE \
	ibus_pinyin_engine_get_type()

GType ibus_pinyin_engine_get_type();

#endif
