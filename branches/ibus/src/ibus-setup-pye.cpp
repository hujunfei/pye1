/***************************************************************************
 *   Copyright (C) 2010 by Jally   *
 *   jallyx@163.com   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "engine/deplib.h"
#include "engine/sys.h"
#include "DataSettings.h"

int main(gint argc, gchar *argv[])
{
	IBusBus *bus;
	IBusConnection *conn;
	DataSettings dtsettings;

	ibus_init();
	gtk_init(&argc, &argv);

	/* 保证当前只有一个实例在运行 */
	bus = ibus_bus_new();
	if (!ibus_bus_request_name(bus, "org.freedesktop.IBus.Pye.Settings", 0))
		exit(0);

	/* 创建数据配置对话框，并运行 */
	gtk_window_set_default_icon_from_file(__ICON_PATH "/pye.png", NULL);
	conn = ibus_bus_get_connection(bus);
	dtsettings.ResetDataEntry(conn);

	/* 释放资源 */
	ibus_bus_release_name(bus, "org.freedesktop.IBus.Pye.Settings");
	g_object_unref(bus);

	return 0;
}
