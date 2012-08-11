/*
 * attribute.c
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


#include <glib.h>
#include <cairo.h>

#include "tileatteditor.h"

extern struct TileAttribute attr_passability;
extern struct TileAttribute attr_quadpassability;
extern struct TileAttribute attr_priority;
extern struct TileAttribute attr_bushflag;
extern struct TileAttribute attr_counterflag;
extern struct TileAttribute attr_terrainflag;

struct TileAttribute *attr_store[] =
{
	&attr_passability,
	&attr_quadpassability,
	&attr_priority,
	&attr_bushflag,
	&attr_counterflag,
	&attr_terrainflag
};

gint attr_store_n = G_N_ELEMENTS(attr_store);


void tile_attr_set_primary_color
( cairo_t *cr )
{
	cairo_set_source_rgb(cr, 1, 1, 1);
}

void tile_attr_set_secondary_color
( cairo_t *cr )
{
	cairo_set_source_rgb(cr, 0, 0, 0);
}

void tile_attr_set_color
( cairo_t *cr, gboolean hovered, enum AttrColor color )
{
	if (!color ^ !hovered) { tile_attr_set_secondary_color(cr); }
	else                   { tile_attr_set_primary_color(cr);   }
}

void attr_draw_empty
( cairo_t *cr, gdouble x, gdouble y, gboolean hovered )
{
	cairo_save(cr);
	cairo_arc(cr, x, y, 0.05, 0, G_TAU);
	cairo_fill_with_outline(cr, 0.03, hovered);
	cairo_restore(cr);
}

void cairo_fill_with_outline
( cairo_t *cr, gdouble outline_width, gboolean hovered )
{
	cairo_save(cr);
	tile_attr_set_color(cr, hovered, ATTR_COLOR_SEC);
	cairo_set_line_width(cr, outline_width*2);
	cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
	cairo_stroke_preserve(cr);

	tile_attr_set_color(cr, hovered, ATTR_COLOR_PRI);
	cairo_fill(cr);
	cairo_restore(cr);
}

cairo_t *cairo_dummy_create
( void )
{
	return cairo_create
		(cairo_image_surface_create(CAIRO_FORMAT_A1, 1, 1));
}

void cairo_dummy_destroy
( cairo_t *cr )
{
	cairo_surface_destroy(cairo_get_target(cr));
	cairo_destroy(cr);
}


void tile_attrs_init
( gpointer global_data )
{
	gint i;
	for (i = 0; i < attr_store_n; i++)
	{
		attr_store[i]->init();
		attr_store[i]->value_buffer = NULL;
		attr_store[i]->global_data = global_data;
	}
}

void tile_attrs_cleanup
( void )
{
	gint i;
	for (i = 0; i < attr_store_n; i++)
		{ attr_store[i]->cleanup(); }
}

