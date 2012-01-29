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


struct TileAttribute* tile_attr_create
( const gchar *name, gint default_value,
  gint icon_value, gboolean hover_precision,
  gint (*tile_clicked)(gint, gdouble, gdouble),
  void (*draw_attr)(gint, cairo_t*, gboolean, gdouble, gdouble),
  void (*cleanup) )
{
	struct TileAttribute *attr =
		g_malloc(sizeof(struct TileAttribute));

	attr->name = name;
	attr->default_value = default_value;
	attr->icon_value = icon_value;
	attr->hover_precision = hover_precision;
	attr->tile_clicked = tile_clicked;
	attr->draw_attr = draw_attr;
	attr->cleanup = cleanup;

	return attr;
}

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
	switch (color)
	{
		case ATTR_COLOR_PRI :
			if (hovered) { tile_attr_set_secondary_color(cr); }
			else         { tile_attr_set_primary_color(cr); }
			break;
		case ATTR_COLOR_SEC :
			if (hovered) { tile_attr_set_primary_color(cr); }
			else         { tile_attr_set_secondary_color(cr); }
	}
}

void attr_draw_empty
( cairo_t *cr, gdouble x, gdouble y, gboolean hovered )
{
	cairo_arc(cr, x, y, 0.05, 0, G_TAU);
	tile_attr_set_color(cr, hovered, ATTR_COLOR_SEC);
	cairo_set_line_width(cr, 0.06);
	cairo_stroke_preserve(cr);
	tile_attr_set_color(cr, hovered, ATTR_COLOR_PRI);
	cairo_fill(cr);
}


struct TileAttribute** tile_attrs_create
( gpointer global_data )
{
	struct TileAttribute **tile_attr;
	tile_attr =
		g_malloc(sizeof(struct TileAttribute*) * ATTRIBUTE_COUNT+1);

	tile_attr[0] = attr_passability_create();
	tile_attr[1] = attr_quadpassability_create();
	tile_attr[2] = attr_priority_create();
	tile_attr[3] = attr_bushflag_create();
	tile_attr[4] = attr_counterflag_create();
	tile_attr[5] = attr_terrainflag_create();
	tile_attr[ATTRIBUTE_COUNT] = NULL;

	gint i; for (i=0;tile_attr[i];i++)
	{
		tile_attr[i]->value_buffer = NULL;
		tile_attr[i]->global_data = global_data;
	}

	return tile_attr;
}

void tile_attrs_destroy
( struct TileAttribute **tile_attr )
{
	struct TileAttribute **_tile_attr;
	for (_tile_attr = tile_attr; *_tile_attr; _tile_attr++)
	{
		if ((*_tile_attr)->cleanup)
			{ (*(*_tile_attr)->cleanup)(); }
//		g_free(*_tile_attr);
	}

	g_free(tile_attr);
}

