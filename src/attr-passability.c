/*
 * attr-passability.c
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


#include <cairo.h>

#include "attribute.h"

/* Style-Parameters: These define the visual look */
#define SIZE  0.175
#define LINEW 0.12


static struct TileAttribute tile_attribute;

static cairo_path_t *path_x;

static gint tile_clicked
( gint old_value, gdouble x, gdouble y )
{
	return !old_value;
}

static void draw_attr
( gint attr_value, cairo_t *cr, gboolean hovered,
  gdouble offset_x, gdouble offset_y )
{
	switch(attr_value)
	{
		case 0 :
			cairo_arc(cr, 0.5, 0.5, SIZE, 0, G_TAU);
			break;

		case 1 :
			cairo_append_path(cr, path_x);
			cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
	}

	tile_attr_set_color(cr, hovered, ATTR_COLOR_SEC);
	cairo_set_line_width(cr, LINEW);
	cairo_stroke_preserve(cr);
	tile_attr_set_color(cr, hovered, ATTR_COLOR_PRI);
	cairo_set_line_width(cr, LINEW/2);
	cairo_stroke(cr);
}

static void cleanup
( )
{
	cairo_path_destroy(path_x);
}

struct TileAttribute* attr_passability_create
()
{
	cairo_t *cr = cairo_dummy_create();
	cairo_move_to(cr, 0.5-SIZE, 0.5-SIZE);
	cairo_line_to(cr, 0.5+SIZE, 0.5+SIZE);
	cairo_move_to(cr, 0.5+SIZE, 0.5-SIZE);
	cairo_line_to(cr, 0.5-SIZE, 0.5+SIZE);

	path_x = cairo_copy_path(cr);
	cairo_dummy_destroy(cr);


	struct TileAttribute *attr = &tile_attribute;

	attr->name = "Passability";
	attr->default_value = 0;
	attr->icon_value = 0;
	attr->hover_precision = FALSE;
	attr->tile_clicked = &tile_clicked;
	attr->draw_attr = &draw_attr;
	attr->cleanup = NULL;

	return attr;
}

