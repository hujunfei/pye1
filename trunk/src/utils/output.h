//
// C++ Interface: output
//
// Description:
// 消息输出封装函数
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef __SRC_UTILS_OUTPUT_H
#define __SRC_UTILS_OUTPUT_H

/* 警告信息输出 */
#ifndef WARNING
#define pwarning(format,...) /*warnx(format,##__VA_ARGS__)*/
#else
#define pwarning(format,...) warnx(format,##__VA_ARGS__)
#endif

/* 常规消息输出 */
#ifndef MESSAGE
#define pmessage(format,...) /*printf(format,##__VA_ARGS__)*/
#else
#define pmessage(format,...) printf(format,##__VA_ARGS__)
#endif

/* 程序执行踪迹输出，用于调试 */
#ifndef TRACE
#define ptrace(format,...) /*printf(format,##__VA_ARGS__)*/
#else
#define ptrace(format,...) printf(format,##__VA_ARGS__)
#endif

#endif
