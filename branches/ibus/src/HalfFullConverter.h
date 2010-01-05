//
// C++ Interface: HalfFullConverter
//
// Description:
// 全/半角转换
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef HALFFULLCONVERTER_H
#define HALFFULLCONVERTER_H

#include "include/deplib.h"

class HalfFullConverter {
public:
	static gunichar ToFull(gunichar ch);
	static gunichar ToHalf(gunichar ch);
private:
	static const guint table[][3];
};

#endif
