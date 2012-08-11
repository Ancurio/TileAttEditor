/*
 * attribute.h
 *
 * Copyright (c) 2011-2012 Jonas Kulla <Nyocurio@googlemail.com>
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 *  1. The origin of this software must not be misrepresented; you must not
 *  claim that you wrote the original software. If you use this software
 *  in a product, an acknowledgment in the product documentation would be
 *  appreciated but is not required.
 *
 *  2. Altered source versions must be plainly marked as such, and must not be
 *  misrepresented as being the original software.
 *
 *  3. This notice may not be removed or altered from any source
 *  distribution.
 *
 *
 * WARNING: This is BAD code. Please do not try to learn from this.
 *          You are, however, free to learn how NOT to write good software.
 *          Have a nice day.
 */


#include <gtk/gtk.h>

#define G_TAU 2*G_PI

#define ATTR_STATIC_FUNCS                                 \
    static void init                                      \
        ( void );                                         \
    static gint tile_clicked                              \
        ( gint old_value, gdouble x, gdouble y );         \
    static void draw_attr                                 \
        ( gint attr_value, cairo_t *cr, gboolean hovered, \
          gdouble offset_x, gdouble offset_y );           \
    static void cleanup                                   \
        ( void );

#define ATTR_DEFINE(name, name_str, def_val, icon_val, hover_prec) \
    struct TileAttribute attr_##name =                             \
        {                                                          \
            name_str, def_val, icon_val, hover_prec,               \
            init, tile_clicked, draw_attr, cleanup,                \
        };


struct TileAttribute
{
	/* defined by attribute constructor */
	const gchar *name;
	gint default_value;
	gint icon_value;
	gboolean hover_precision;

	void (*init)
	( );

	gint (*tile_clicked)
	( gint old_value, gdouble x, gdouble y );

	void (*draw_attr)
	( gint attr_value, cairo_t *cr, gboolean hovered,
	  gdouble offset_x, gdouble offset_y );

	void (*cleanup)
	( );

	/* defined by program */
	gboolean enabled;
	gint *value_buffer;
	guint buffer_size;
	gulong signal_handler_id;
	gpointer global_data;
	GtkWidget *button;
	GtkWidget *label;

};

extern struct TileAttribute *attr_store[];
extern gint attr_store_n;

enum AttrColor
{
	ATTR_COLOR_PRI = 0,
	ATTR_COLOR_SEC = 1,
};

void tile_attr_set_primary_color
( cairo_t *cr );
void tile_attr_set_secondary_color
( cairo_t *cr );
void tile_attr_set_color
( cairo_t *cr, gboolean hovered, enum AttrColor color );
void attr_draw_empty
( cairo_t *cr, gdouble x, gdouble y, gboolean hovered );
cairo_t *cairo_dummy_create
( void );
void cairo_dummy_destroy
( cairo_t *cr );
void cairo_fill_with_outline
( cairo_t *cr, gdouble outline_width, gboolean hovered );


void tile_attrs_init
( gpointer global_data );

void tile_attrs_cleanup
( void );

