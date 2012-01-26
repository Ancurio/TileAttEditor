/*
 * attr-priority.c
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

#define FLAG_MAX_VAL 5

/* Style-Parameters: These define the visual look */
#define FONT_SIZE 0.5
#define STAR_SIZE 0.45
#define OUTL_SIZE 0.07
#define STARD     0.2
#define STAR_OFFY 0.02

#define STARC     "★"


static gint tile_clicked
(gint old_value, gdouble x, gdouble y)
{
	if (old_value < FLAG_MAX_VAL)
		{ return old_value+1; }
	else
		{ return 0; }
}

static void draw_attr
( gint attr_value, cairo_t *cr, gboolean hovered,
  gdouble offset_x, gdouble offset_y )
{
	gchar value_str[4];
	cairo_text_extents_t ext;
	cairo_select_font_face
		(cr, "Fantasy",
		 CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);

	switch (attr_value)
	{
		case -1 :
			cairo_set_font_size(cr, FONT_SIZE*1.2);
			cairo_text_extents(cr, STARC, &ext);

			cairo_move_to
				(cr, 0.5-ext.width/2-ext.x_bearing,
					 0.5-ext.height/2-ext.y_bearing);
			cairo_text_path(cr, STARC);
			tile_attr_set_color(cr, hovered, ATTR_COLOR_SEC);
			cairo_set_line_width(cr, OUTL_SIZE*1.2);
			cairo_stroke(cr);

			cairo_move_to
				(cr, 0.5-ext.width/2-ext.x_bearing,
					 0.5-ext.height/2-ext.y_bearing);
			tile_attr_set_color(cr, hovered, ATTR_COLOR_PRI);
//			cairo_show_text(cr, STARC);
			cairo_text_path(cr, STARC);
			cairo_fill(cr);

			break;

		case 0  :
			cairo_move_to(cr, 0.5, 0.5);
			cairo_arc(cr, 0.5, 0.5, 0.05, 0, G_TAU);
			tile_attr_set_color(cr, hovered, ATTR_COLOR_SEC);
			cairo_set_line_width(cr, 0.06);
			cairo_stroke_preserve(cr);
			tile_attr_set_color(cr, hovered, ATTR_COLOR_PRI);
			cairo_fill(cr);
			break;

		default :
			cairo_set_font_size(cr, STAR_SIZE);
			cairo_text_extents(cr, STARC, &ext);

			cairo_move_to
				(cr, 0.5-STARD-ext.width/2-ext.x_bearing,
					 0.5-STAR_OFFY-ext.height/2-ext.y_bearing);
			cairo_text_path(cr, STARC);
			tile_attr_set_color(cr, hovered, ATTR_COLOR_SEC);
			cairo_set_line_width(cr, OUTL_SIZE);
			cairo_stroke(cr);

			cairo_move_to
				(cr, 0.5-STARD-ext.width/2-ext.x_bearing,
					 0.5-STAR_OFFY-ext.height/2-ext.y_bearing);
			tile_attr_set_color(cr, hovered, ATTR_COLOR_PRI);
//			cairo_show_text(cr, STARC);
			cairo_text_path(cr, STARC);
			cairo_fill(cr);

			g_snprintf(value_str, 4, "%d", attr_value);
			cairo_set_font_size(cr, FONT_SIZE);
			cairo_text_extents(cr, value_str, &ext);

			cairo_move_to
				(cr, 0.5+STARD-ext.width/2-ext.x_bearing,
					 0.5-ext.height/2-ext.y_bearing);
			cairo_text_path(cr, value_str);
			tile_attr_set_color(cr, hovered, ATTR_COLOR_SEC);
			cairo_stroke(cr);

			cairo_move_to
				(cr, 0.5+STARD-ext.width/2-ext.x_bearing,
					 0.5-ext.height/2-ext.y_bearing);
			tile_attr_set_color(cr, hovered, ATTR_COLOR_PRI);
//			cairo_show_text(cr, value_str);
			cairo_text_path(cr, value_str);
			cairo_fill(cr);
	}
}

struct TileAttribute* attr_priority_create
()
{
	struct TileAttribute *attr =
		g_malloc(sizeof(struct TileAttribute));

	attr->name = "Priority";
	attr->default_value = 0;
	attr->icon_value = -1;
	attr->hover_precision = FALSE;
	attr->tile_clicked = &tile_clicked;
	attr->draw_attr = &draw_attr;

	return attr;
}

