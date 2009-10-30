/***************************************************************************
 *   Copyright (C) 2009 by Jally   *
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
#include "engine/PhraseEngine.h"
#include "Config.h"
#include "Engine.h"

PhraseEngine phregn;
Config config;

/* options */
static gboolean ibus = FALSE;
static gboolean verbose = FALSE;
static const GOptionEntry entries[] = {
 {"ibus", 'i', 0, G_OPTION_ARG_NONE, &ibus, "component is executed by ibus", NULL},
 {"verbose", 'v', 0, G_OPTION_ARG_NONE, &verbose, "verbose", NULL},
 {NULL},
};

static void ibus_disconnected_cb(IBusBus *bus, gpointer user_data)
{
	g_debug("bus disconnected");
	ibus_quit();
}

static void start_component()
{
	IBusComponent *component;
	IBusEngineDesc *desc;
	IBusFactory *factory;
	IBusConnection *conn;
	IBusBus *bus;

	bus = ibus_bus_new();
	g_signal_connect(bus, "disconnected", G_CALLBACK(ibus_disconnected_cb), NULL);

	conn = ibus_bus_get_connection(bus);
	config.SetConnection(conn);
	factory = ibus_factory_new(conn);
	ibus_factory_add_engine(factory, "pinyin", IBUS_TYPE_PINYIN_ENGINE);

	if (ibus) {
		ibus_bus_request_name(bus, "org.freedesktop.IBus.Pye", 0);
	} else {
		component = ibus_component_new("org.freedesktop.IBus.Pye",
					 "Pinyin input method",
					 "0.1.0",
					 "GPL",
					 "Jally, pqiu",
					 "http://code.google.com/p/pye1/",
					 NULL,
					 NULL);
		desc = ibus_engine_desc_new("pinyin",
					 "Pinyin Engine",
					 "Pinyin input method",
					 "zh_CN",
					 "GPL",
					 "Jally, pqiu",
					 PKGDATADIR "/icons/ibus-pye.svg",
					 "us");
		ibus_component_add_engine(component, desc);
		ibus_bus_register_component(bus, component);
	}
}

static void init_phrase_engine()
{
	phregn.CreateSysEngineUnits("mb.txt");
	phregn.CreateUserEngineUnit("user.mb");
}

int main(gint argc, gchar *argv[])
{
	GError *error = NULL;
	GOptionContext *context;

	/* 参数分析 */
	context = g_option_context_new("- ibus pinyin engine component");
	g_option_context_add_main_entries(context, entries, "ibus-pinyin");
	if (!g_option_context_parse(context, &argc, &argv, &error)) {
		g_print("Option parsing failed: %s\n", error->message);
		exit(1);
	}
	g_option_context_free(context);

	/* 启动部件并运行程序 */
	ibus_init();
	start_component();
	init_phrase_engine();
	ibus_main();

	return 0;
}
