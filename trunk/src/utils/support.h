//
// C++ Interface: support
//
// Description:
// 实用函数
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef __SRC_UTILS_SUPPORT_H
#define __SRC_UTILS_SUPPORT_H

#include "../include/sys.h"

#define FLAG_ISSET(num,bit) ((num)&(1<<(bit)))
#define FLAG_SET(num,bit) ((num)|=(1<<(bit)))
#define FLAG_CLR(num,bit) ((num)&=(~(1<<(bit))))

int copy_file(const char *destfile, const char *srcfile);

#endif
