//
// C++ Interface: half_full_converter
//
// Description:
// 全/半角转换.
//
// Author: Jally <jallyx@163.com>, (C) 2009, 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IBUS_PYE_HALF_FULL_CONVERTER_H_
#define IBUS_PYE_HALF_FULL_CONVERTER_H_

#include <ibus.h>

class HalfFullConverter {
 public:
  static gunichar toFull(gunichar ch);
  static gunichar toHalf(gunichar ch);

 private:
  static const guint table_[][3];
};

#endif  // IBUS_PYE_HALF_FULL_CONVERTER_H_
