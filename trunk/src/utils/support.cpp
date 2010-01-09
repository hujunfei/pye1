//
// C++ Implementation: support
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "support.h"
#include "wrapper.h"

/**
 * 拷贝文件.
 * @param destfile 目标文件
 * @param srcfile 数据源文件
 * @return 结果值，-1 失败;0 成功
 */
int copy_file(const char *destfile, const char *srcfile)
{
	char buf[4096];
	ssize_t size;
	int sfd, dfd;

	if ((sfd = open(srcfile, O_RDONLY)) == -1)
		return -1;
	if ((dfd = open(destfile, O_WRONLY | O_CREAT | O_TRUNC, 00644)) == -1) {
		close(sfd);
		return -1;
	}

	while (1) {
		if ((size = xread(sfd, buf, 4096)) <= 0)
			break;
		if (xwrite(dfd, buf, size) <= 0)
			break;
	}

	close(dfd);
	close(sfd);

	return size;
}
