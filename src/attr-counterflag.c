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


#include <cairo.h>

#include "attribute.h"

/* Style-Parameters: These define the visual look */
#define OUTLW 0.04


static struct TileAttribute tile_attribute;

static cairo_path_t *path;

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
			attr_draw_empty(cr, 0.5, 0.5, hovered);
			break;

		case 1 :
			cairo_append_path(cr, path);
			cairo_fill_with_outline(cr, OUTLW, hovered);

	}
}

static void cleanup
( void )
{
	cairo_path_destroy(path);
}

struct TileAttribute* attr_counterflag_create
( void )
{
	cairo_t *cr = cairo_dummy_create();
	cairo_move_to(cr, 0.5, 0.3);
	cairo_line_to(cr, 0.7, 0.5);
	cairo_line_to(cr, 0.5, 0.7);
	cairo_line_to(cr, 0.3, 0.5);
	cairo_line_to(cr, 0.5, 0.3);
	cairo_close_path(cr);

	path = cairo_copy_path(cr);
	cairo_dummy_destroy(cr);


	struct TileAttribute *attr = &tile_attribute;

	attr->name = "CounterFlag";
	attr->default_value = 0;
	attr->icon_value = 1;
	attr->hover_precision = FALSE;
	attr->tile_clicked = &tile_clicked;
	attr->draw_attr = &draw_attr;
	attr->cleanup = &cleanup;

	return attr;
}

