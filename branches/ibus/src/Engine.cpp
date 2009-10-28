//
// C++ Implementation: Engine
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "Engine.h"
#include "PinyinEngine.h"

/* code of engine class of GObject */
#define IBUS_PINYIN_ENGINE(obj) \
	(G_TYPE_CHECK_INSTANCE_CAST((obj), IBUS_TYPE_PINYIN_ENGINE, IBusPinyinEngine))
#define IBUS_PINYIN_ENGINE_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_CAST((klass), IBUS_TYPE_PINYIN_ENGINE, IBusPinyinEngineClass))
#define IBUS_IS_PINYIN_ENGINE(obj) \
	(G_TYPE_CHECK_INSTANCE_TYPE((obj), IBUS_TYPE_PINYIN_ENGINE))
#define IBUS_IS_PINYIN_ENGINE_CLASS(klass) \
	(G_TYPE_CHECK_CLASS_TYPE((klass), IBUS_TYPE_PINYIN_ENGINE))
#define IBUS_PINYIN_ENGINE_GET_CLASS(obj) \
	(G_TYPE_INSTANCE_GET_CLASS((obj), IBUS_TYPE_PINYIN_ENGINE, IBusPinyinEngineClass))

typedef struct _IBusPinyinEngine IBusPinyinEngine;
typedef struct _IBusPinyinEngineClass IBusPinyinEngineClass;
struct _IBusPinyinEngine {
	IBusEngine parent;
	PinyinEngine *engine;	///< members
};
struct _IBusPinyinEngineClass {
	IBusEngineClass parent;
};

/* functions prototype */
static void ibus_pinyin_engine_class_init(IBusPinyinEngineClass *klass);
static void ibus_pinyin_engine_init(IBusPinyinEngine *pinyin);
static void ibus_pinyin_engine_destroy(IBusPinyinEngine *pinyin);

static void ibus_pinyin_engine_disable(IBusEngine *engine);
static void ibus_pinyin_engine_enable(IBusEngine *engine);
static void ibus_pinyin_engine_focus_in(IBusEngine *engine);
static void ibus_pinyin_engine_focus_out(IBusEngine *engine);
static void ibus_pinyin_engine_cursor_down(IBusEngine *engine);
static void ibus_pinyin_engine_cursor_up(IBusEngine *engine);
static void ibus_pinyin_engine_page_down(IBusEngine *engine);
static void ibus_pinyin_engine_page_up(IBusEngine *engine);
static void ibus_pinyin_engine_property_activate(IBusEngine *engine,
			 const gchar *prop_name, guint prop_state);
static void ibus_pinyin_engine_candidate_clicked(IBusEngine *engine,
			 guint index, guint button, guint state);
static gboolean ibus_pinyin_engine_process_key_event(IBusEngine *engine,
			 guint keyval, guint keycode, guint state);

static IBusEngineClass *parent_class = NULL;
GType ibus_pinyin_engine_get_type()
{
	static GType type = 0;
	static const GTypeInfo typeinfo = {
			 sizeof(IBusPinyinEngineClass),
			 (GBaseInitFunc)NULL,
			 (GBaseFinalizeFunc)NULL,
			 (GClassInitFunc)ibus_pinyin_engine_class_init,
			 NULL,
			 NULL,
			 sizeof(IBusPinyinEngine),
			 0,
			 (GInstanceInitFunc)ibus_pinyin_engine_init,
			 NULL
	};

	if (type == 0) {
		type = g_type_register_static(IBUS_TYPE_ENGINE,
					 "IBusPinyinEngine",
					 &typeinfo,
					 (GTypeFlags)0);
	}

	return type;
}

static void ibus_pinyin_engine_class_init(IBusPinyinEngineClass *klass)
{
	IBusObjectClass *ibus_object_class = IBUS_OBJECT_CLASS(klass);
	IBusEngineClass *engine_class = IBUS_ENGINE_CLASS(klass);

	parent_class = (IBusEngineClass *)g_type_class_peek_parent(klass);

	ibus_object_class->destroy = (IBusObjectDestroyFunc)ibus_pinyin_engine_destroy;

	engine_class->disable = ibus_pinyin_engine_disable;
	engine_class->enable = ibus_pinyin_engine_enable;
	engine_class->focus_in = ibus_pinyin_engine_focus_in;
	engine_class->focus_out = ibus_pinyin_engine_focus_out;
	engine_class->cursor_down = ibus_pinyin_engine_cursor_down;
	engine_class->cursor_up = ibus_pinyin_engine_cursor_up;
	engine_class->page_down = ibus_pinyin_engine_page_down;
	engine_class->page_up = ibus_pinyin_engine_page_up;
	engine_class->property_activate = ibus_pinyin_engine_property_activate;
	engine_class->candidate_clicked = ibus_pinyin_engine_candidate_clicked;
	engine_class->process_key_event = ibus_pinyin_engine_process_key_event;
}

static void ibus_pinyin_engine_init(IBusPinyinEngine *pinyin)
{
	if (g_object_is_floating(pinyin))
		g_object_ref_sink(pinyin);
	pinyin->engine = new PinyinEngine(IBUS_ENGINE(pinyin));
}

static void ibus_pinyin_engine_destroy(IBusPinyinEngine *pinyin)
{
	if (pinyin->engine) {
		delete pinyin->engine;
		pinyin->engine = NULL;
	}
	IBUS_OBJECT_CLASS(parent_class)->destroy((IBusObject *)pinyin);
}

static void ibus_pinyin_engine_disable(IBusEngine *engine)
{
	IBusPinyinEngine *pinyin = (IBusPinyinEngine *)engine;
	pinyin->engine->EngineDisable();
}

static void ibus_pinyin_engine_enable(IBusEngine *engine)
{
	IBusPinyinEngine *pinyin = (IBusPinyinEngine *)engine;
	pinyin->engine->EngineEnable();
}

static void ibus_pinyin_engine_focus_in(IBusEngine *engine)
{
	IBusPinyinEngine *pinyin = (IBusPinyinEngine *)engine;
	pinyin->engine->FocusIn();
}

static void ibus_pinyin_engine_focus_out(IBusEngine *engine)
{
	IBusPinyinEngine *pinyin = (IBusPinyinEngine *)engine;
	pinyin->engine->FocusOut();
}

static void ibus_pinyin_engine_cursor_down(IBusEngine *engine)
{
	IBusPinyinEngine *pinyin = (IBusPinyinEngine *)engine;
	pinyin->engine->CursorDown();
}

static void ibus_pinyin_engine_cursor_up(IBusEngine *engine)
{
	IBusPinyinEngine *pinyin = (IBusPinyinEngine *)engine;
	pinyin->engine->CursorUp();
}

static void ibus_pinyin_engine_page_down(IBusEngine *engine)
{
	IBusPinyinEngine *pinyin = (IBusPinyinEngine *)engine;
	pinyin->engine->PageDown();
}

static void ibus_pinyin_engine_page_up(IBusEngine *engine)
{
	IBusPinyinEngine *pinyin = (IBusPinyinEngine *)engine;
	pinyin->engine->PageUp();
}

static void ibus_pinyin_engine_property_activate(IBusEngine *engine,
			 const gchar *prop_name, guint prop_state)
{
	IBusPinyinEngine *pinyin = (IBusPinyinEngine *)engine;
	pinyin->engine->PropertyActivate(prop_name, prop_state);
}

static void ibus_pinyin_engine_candidate_clicked(IBusEngine *engine,
			 guint index, guint button, guint state)
{
	IBusPinyinEngine *pinyin = (IBusPinyinEngine *)engine;
	pinyin->engine->CandidateClicked(index, button, state);
}

static gboolean ibus_pinyin_engine_process_key_event(IBusEngine *engine,
			 guint keyval, guint keycode, guint state)
{
	IBusPinyinEngine *pinyin = (IBusPinyinEngine *)engine;
	return pinyin->engine->ProcessKeyEvent(keyval, keycode, state);
}
