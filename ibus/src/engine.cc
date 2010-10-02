//
// C++ Implementation: engine
//
// Description:
// 请参见头文件描述.
//
// Author: Jally <jallyx@163.com>, (C) 2009, 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "engine.h"
#include "pye_engine.h"

/* code of engine class of GObject */
#define IBUS_PYE_ENGINE(obj) \
    (G_TYPE_CHECK_INSTANCE_CAST((obj), IBUS_TYPE_PYE_ENGINE, IBusPyeEngine))
#define IBUS_PYE_ENGINE_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_CAST((klass), IBUS_TYPE_PYE_ENGINE, IBusPyeEngineClass))
#define IBUS_IS_PYE_ENGINE(obj) \
    (G_TYPE_CHECK_INSTANCE_TYPE((obj), IBUS_TYPE_PYE_ENGINE))
#define IBUS_IS_PYE_ENGINE_CLASS(klass) \
    (G_TYPE_CHECK_CLASS_TYPE((klass), IBUS_TYPE_PYE_ENGINE))
#define IBUS_PYE_ENGINE_GET_CLASS(obj) \
    (G_TYPE_INSTANCE_GET_CLASS((obj), IBUS_TYPE_PYE_ENGINE, IBusPyeEngineClass))

typedef struct _IBusPyeEngine IBusPyeEngine;
typedef struct _IBusPyeEngineClass IBusPyeEngineClass;
struct _IBusPyeEngine {
  IBusEngine parent;
  PyeEngine *engine;
};
struct _IBusPyeEngineClass {
  IBusEngineClass parent;
};

/* functions prototype */
static void ibus_pye_engine_class_init(IBusPyeEngineClass *klass);
static void ibus_pye_engine_init(IBusPyeEngine *pye);
static void ibus_pye_engine_destroy(IBusPyeEngine *pye);

static gboolean ibus_pye_engine_process_key_event(IBusEngine *engine,
                                                  guint keyval,
                                                  guint keycode,
                                                  guint modifiers);
static void ibus_pye_engine_reset(IBusEngine *engine);
static void ibus_pye_engine_enable(IBusEngine *engine);
static void ibus_pye_engine_disable(IBusEngine *engine);
static void ibus_pye_engine_focus_in(IBusEngine *engine);
static void ibus_pye_engine_focus_out(IBusEngine *engine);
static void ibus_pye_engine_page_up(IBusEngine *engine);
static void ibus_pye_engine_page_down(IBusEngine *engine);
static void ibus_pye_engine_cursor_up(IBusEngine *engine);
static void ibus_pye_engine_cursor_down(IBusEngine *engine);
static void ibus_pye_engine_candidate_clicked(IBusEngine *engine,
                                              guint index,
                                              guint button,
                                              guint state);
static void ibus_pye_engine_property_activate(IBusEngine *engine,
                                              const gchar *prop_name,
                                              guint prop_state);

GType ibus_pye_engine_get_type() {
  static GType type = 0;
  static const GTypeInfo typeinfo = {
    sizeof(IBusPyeEngineClass),
    (GBaseInitFunc)NULL,
    (GBaseFinalizeFunc)NULL,
    (GClassInitFunc)ibus_pye_engine_class_init,
    (GClassFinalizeFunc)NULL,
    NULL,
    sizeof(IBusPyeEngine),
    0,
    (GInstanceInitFunc)ibus_pye_engine_init,
    NULL
  };

  if (type == 0) {
    type = g_type_register_static(IBUS_TYPE_ENGINE,
                                  "IBusPyeEngine",
                                  &typeinfo,
                                  (GTypeFlags)0);
  }

  return type;
}

static IBusEngineClass *parent_class = NULL;
static void ibus_pye_engine_class_init(IBusPyeEngineClass *klass) {
  IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS(klass);
  IBusEngineClass *ibus_engine_class = IBUS_ENGINE_CLASS(klass);
  parent_class = (IBusEngineClass *)g_type_class_peek_parent(klass);

  ibus_object_class->destroy = (IBusObjectDestroyFunc)ibus_pye_engine_destroy;

  ibus_engine_class->process_key_event = ibus_pye_engine_process_key_event;
  ibus_engine_class->reset = ibus_pye_engine_reset;
  ibus_engine_class->enable = ibus_pye_engine_enable;
  ibus_engine_class->disable = ibus_pye_engine_disable;
  ibus_engine_class->focus_in = ibus_pye_engine_focus_in;
  ibus_engine_class->focus_out = ibus_pye_engine_focus_out;
  ibus_engine_class->page_up = ibus_pye_engine_page_up;
  ibus_engine_class->page_down = ibus_pye_engine_page_down;
  ibus_engine_class->cursor_up = ibus_pye_engine_cursor_up;
  ibus_engine_class->cursor_down = ibus_pye_engine_cursor_down;
  ibus_engine_class->candidate_clicked = ibus_pye_engine_candidate_clicked;
  ibus_engine_class->property_activate = ibus_pye_engine_property_activate;
}

static void ibus_pye_engine_init(IBusPyeEngine *engine) {
  if (g_object_is_floating(engine))
    g_object_ref_sink(engine);
  PhraseManager *manager = PhraseManager::GetInstance();
  engine->engine = new PyeEngine(IBUS_ENGINE(engine), manager);
}

static void ibus_pye_engine_destroy(IBusPyeEngine *engine) {
  delete engine->engine;
  IBUS_OBJECT_CLASS(parent_class)->destroy((IBusObject *)engine);
}

static gboolean ibus_pye_engine_process_key_event(IBusEngine *engine,
                                                  guint keyval,
                                                  guint keycode,
                                                  guint modifiers) {
  IBusPyeEngine *pye = (IBusPyeEngine *)engine;
  return pye->engine->processKeyEvent(keyval, keycode, modifiers);
}

static void ibus_pye_engine_candidate_clicked(IBusEngine *engine,
                                              guint index,
                                              guint button,
                                              guint state) {
  IBusPyeEngine *pye = (IBusPyeEngine *)engine;
  pye->engine->candidateClicked(index, button, state);
}

static void ibus_pye_engine_property_activate(IBusEngine *engine,
                                              const gchar *prop_name,
                                              guint prop_state) {
  IBusPyeEngine *pye = (IBusPyeEngine *)engine;
  pye->engine->propertyActivate(prop_name, prop_state);
}

#define FUNCTION(name, Name) \
static void ibus_pye_engine_##name(IBusEngine *engine) { \
  IBusPyeEngine *pye = (IBusPyeEngine *)engine; \
  pye->engine->Name(); \
}
FUNCTION(reset, reset)
FUNCTION(enable, enable)
FUNCTION(disable, disable)
FUNCTION(focus_in, focusIn)
FUNCTION(focus_out, focusOut)
FUNCTION(page_up, pageUp)
FUNCTION(page_down, pageDown)
FUNCTION(cursor_up, cursorUp)
FUNCTION(cursor_down, cursorDown)
#undef FUNCTION
