//
// C++ Implementation: wrapper
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "wrapper.h"
#include "../include/deplib.h"

/**
 * 保证new运算符申请内存一定成功.
 * @param size 需要申请的内存大小
 * @return 新内存指针
 */
void *operator new(size_t size)
{
	return g_malloc(size);
}

/**
 * 写出数据.
 * @param fd 文件描述符
 * @param buf 缓冲区
 * @param count 缓冲区有效数据长度
 * @return 成功写出的数据长度
 */
ssize_t xwrite(int fd, const void *buf, size_t count)
{
	size_t offset;
	ssize_t size;

	size = -1;
	offset = 0;
	while ((offset != count) && (size != 0)) {
		if ((size = write(fd, (char *)buf + offset, count - offset)) == -1) {
			if (errno == EINTR)
				continue;
			return -1;
		}
		offset += size;
	}

	return offset;
}

/**
 * 读取数据.
 * @param fd 文件描述符
 * @param buf 缓冲区
 * @param count 缓冲区长度
 * @return 成功读取的数据长度
 */
ssize_t xread(int fd, void *buf, size_t count)
{
	size_t offset;
	ssize_t size;

	size = -1;
	offset = 0;
	while ((offset != count) && (size != 0)) {
		if ((size = read(fd, (char *)buf + offset, count - offset)) == -1) {
			if (errno == EINTR)
				continue;
			return -1;
		}
		offset += size;
	}

	return offset;
}
