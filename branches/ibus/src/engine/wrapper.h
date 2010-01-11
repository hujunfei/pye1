//
// C++ Interface: wrapper
//
// Description:
// 打包函数，使某些函数更加好用
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef __SRC_ENGINE_WRAPPER_H
#define __SRC_ENGINE_WRAPPER_H

#include "sys.h"

void *operator new(size_t size);

ssize_t xwrite(int fd, const void *buf, size_t count);
ssize_t xread(int fd, void *buf, size_t count);

#endif
