//
// C++ Interface: Config
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef __SRC_CONFIG_H
#define __SRC_CONFIG_H

#include "include/dep.h"
#include "include/sys.h"

class Config
{
public:
	Config();
	~Config();

	void SetConnection(IBusConnection *conn);
	guint GetPageSize();
	time_t GetBackupGap();
private:
	IBusConnection *connection;
};

#endif
