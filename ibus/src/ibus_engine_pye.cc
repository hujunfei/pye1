/***************************************************************************
 *   Copyright (C) 2009, 2010 by Jally   *
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
#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ibus.h>
#include <dynamic_phrase.h>
#include <phrase_manager.h>
#include "engine.h"
#include "parameter.h"

/* options */
static gboolean ibus = FALSE;

static void show_version_and_quit() {
  g_print("%s - Version %s\n", g_get_application_name(), VERSION);
  exit(0);
}

static const GOptionEntry entries[] = {
  {"ibus", 'i', 0, G_OPTION_ARG_NONE, &ibus, "component is executed by ibus", NULL},
  {"version", 'v', G_OPTION_FLAG_NO_ARG, G_OPTION_ARG_CALLBACK,
      (gpointer)show_version_and_quit, "Show the application's version.", NULL},
  {NULL},
};

static void ibus_disconnected_cb(IBusBus *bus, gpointer user_data) {
  g_debug("bus disconnected");
  ibus_quit();
}

static void start_component() {
  ibus_init();

  IBusBus *bus = ibus_bus_new();
  g_signal_connect(bus, "disconnected", G_CALLBACK(ibus_disconnected_cb), NULL);

  IBusFactory *factory = ibus_factory_new(ibus_bus_get_connection(bus));
  g_object_ref_sink(factory);
  if (ibus) {
    ibus_factory_add_engine(factory, "pinyin", IBUS_TYPE_PYE_ENGINE);
    ibus_bus_request_name(bus, "org.freedesktop.IBus.Pye", 0);
  } else {
    ibus_factory_add_engine(factory, "pinyin (debug)", IBUS_TYPE_PYE_ENGINE);
    IBusComponent *component = ibus_component_new("org.freedesktop.IBus.Pye",
                                                  "Pye Component",
                                                  VERSION,
                                                  "GPL 2+",
                                                  "Jally <jallyx@163.com>",
                                                  "http://code.google.com/p/pye1/",
                                                  "",
                                                  "ibus-pye");
    IBusEngineDesc *desc = ibus_engine_desc_new("pinyin (debug)",
                                                "Pye Pinyin Engine (debug)",
                                                "pye input method (debug)",
                                                "zh_CN",
                                                "GPL 2+",
                                                "Jally <jallyx@163.com>",
                                                __PIXMAPS_PATH "/pye.png",
                                                "us");
    ibus_component_add_engine(component, desc);
    ibus_bus_register_component(bus, component);
  }

  Parameter *parameter = Parameter::getInstance();
  parameter->setConnection(ibus_bus_get_connection(bus));

  ibus_main();
}

int mkdirp(const char *pathname, mode_t mode) {
  char filename[PATH_MAX];
  realpath(pathname, filename);

  for (char *ptr = filename + 1; *ptr != '\0'; ++ptr) {
    if (*ptr == '/') {
      *ptr = '\0';
      if (access(filename, F_OK) != 0) {
        if (mkdir(filename, mode) != 0)
          return -1;
      }
      *ptr = '/';
    }
  }
  if (access(filename, F_OK) != 0) {
    if (mkdir(filename, mode) != 0)
      return -1;
  }

  return 0;
}

static void init_phrase_engine() {
  PhraseManager *phrase_manager = PhraseManager::GetInstance();
  phrase_manager->CreateSystemPhraseProxySite(__PHRASE_PATH "/config.txt");
  char path[PATH_MAX];
  snprintf(path, sizeof(path), "%s/.cache/ibus/pye", g_get_home_dir());
  mkdirp(path, 0777);
  snprintf(path, sizeof(path), "%s/.cache/ibus/pye/user.mb", g_get_home_dir());
  phrase_manager->CreateUserPhraseProxySite(path);

  DynamicPhrase *dynamic_phrase = DynamicPhrase::GetInstance();
  dynamic_phrase->CreateExpression(__PHRASE_PATH "/data.txt");
}

int main(gint argc, gchar **argv) {
  GError *error = NULL;
  GOptionContext *context = g_option_context_new("- ibus pye engine component");
  g_option_context_add_main_entries(context, entries, NULL);
  if (!g_option_context_parse(context, &argc, &argv, &error)) {
    g_print("Option parsing failed: %s\n", error->message);
    exit(0);
  }

  init_phrase_engine();
  start_component();
  return 0;
}
