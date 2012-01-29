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
#define ATTRIBUTE_COUNT 6


struct TileAttribute
{
	/* defined by attribute constructor */
	const gchar *name;
	gint default_value;
	gint icon_value;
	gboolean hover_precision;

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

};

enum AttrColor
{
	ATTR_COLOR_PRI,
	ATTR_COLOR_SEC,
};

struct TileAttribute* tile_attr_create
( );
void tile_attr_set_primary_color
( cairo_t *cr );
void tile_attr_set_secondary_color
( cairo_t *cr );
void tile_attr_set_color
( cairo_t *cr, gboolean hovered, enum AttrColor color );
void attr_draw_empty
( cairo_t *cr, gdouble x, gdouble y, gboolean hovered );


struct TileAttribute** tile_attrs_create
( gpointer global_data );

void tile_attrs_destroy
( struct TileAttribute **tile_attr );

/* Individual attribute constructors */
struct TileAttribute* attr_passability_create();
struct TileAttribute* attr_quadpassability_create();
struct TileAttribute* attr_priority_create();
struct TileAttribute* attr_bushflag_create();
struct TileAttribute* attr_counterflag_create();
struct TileAttribute* attr_terrainflag_create();

