//
// C++ Implementation: Config
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "Config.h"

Config::Config()
{
}

Config::~Config()
{
}

void Config::SetConnection(IBusConnection *conn)
{
	connection = conn;
}

guint Config::GetPageSize()
{
	return 5;
}
