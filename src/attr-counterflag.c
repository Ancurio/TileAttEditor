/*
 * attr-counterflag.c
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
#include <cairo.h>

#include "attribute.h"


static gint tile_clicked
(gint old_value, gdouble x, gdouble y)
{
	if (old_value)
		{ return 0; }
	else
		{ return 1; }
}

static void draw_attr
( gint attr_value, cairo_t *cr, gboolean hovered,
  gdouble offset_x, gdouble offset_y )
{
	gint odd = attr_value % 2;
	switch(odd)
	{
		case 0 :  attr_draw_empty(cr, 0.5, 0.5, hovered);
		          break;

		default : cairo_move_to(cr, 0.5, 0.3);
		          cairo_line_to(cr, 0.7, 0.5);
		          cairo_line_to(cr, 0.5, 0.7);
		          cairo_line_to(cr, 0.3, 0.5);
		          cairo_line_to(cr, 0.5, 0.3);
		          cairo_close_path(cr);
		          cairo_set_line_width(cr, 0.08);
		          cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
		          tile_attr_set_color(cr, hovered, ATTR_COLOR_SEC);
		          cairo_stroke_preserve(cr);
		          tile_attr_set_color(cr, hovered, ATTR_COLOR_PRI);
		          cairo_fill(cr);
	}
}

struct TileAttribute* attr_counterflag_create
()
{
	struct TileAttribute *attr =
		g_malloc(sizeof(struct TileAttribute));

	attr->name = "CounterFlag";
	attr->default_value = 0;
	attr->icon_value = 1;
	attr->hover_precision = FALSE;
	attr->tile_clicked = &tile_clicked;
	attr->draw_attr = &draw_attr;

	return attr;
}

