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
#include <math.h>

#include "attribute.h"

#define FLAG_MAX_VAL 5

/* Style-Parameters: These define the visual look */
#define FONT_SIZE 0.5
#define OUTL_SIZE 0.035
#define STARD     0.2
#define STAR_OFFY 0.02
#define STAR_INR  0.08
#define STAR_OUTR 0.18
#define STAR_ARMS 5
#define STAR_BSZ  1.4


static struct TileAttribute tile_attribute;

static cairo_path_t *star_attr;
static cairo_path_t *star_button;

static cairo_path_t* create_star
( gdouble x, gdouble y, gint arms, gdouble r_inner, gdouble r_outer )
{
	cairo_t *cr = cairo_dummy_create();
	gdouble delta = G_TAU/arms+G_PI_2;
	gint i;
	for (i=0; i<arms*2; i++)
	{
		gdouble px, py;
		gdouble r = (i % 2 == 0) ? r_outer : r_inner;
		px = r*cos(i*(G_PI/arms)-delta)+x;
		py = r*sin(i*(G_PI/arms)-delta)+y;
		if (i == 0) { cairo_move_to(cr, px, py); }
		else        { cairo_line_to(cr, px, py); }
	}
	cairo_close_path(cr);
	cairo_path_t *path = cairo_copy_path(cr);
	cairo_dummy_destroy(cr);
	return path;
}


static gint tile_clicked
(gint old_value, gdouble x, gdouble y)
{
	return (old_value < FLAG_MAX_VAL) ? old_value+1 : 0;
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
		case -1 :  /* This routine is solely for the button icon */
			cairo_append_path(cr, star_button);
			cairo_fill_with_outline(cr, OUTL_SIZE*1.2, FALSE);
			break;

		case 0  :
			attr_draw_empty(cr, 0.5, 0.5, hovered);
			break;

		default :
			cairo_append_path(cr, star_attr);
			cairo_fill_with_outline(cr, OUTL_SIZE, hovered);

			g_snprintf(value_str, 4, "%d", attr_value);
			cairo_set_font_size(cr, FONT_SIZE);
			cairo_text_extents(cr, value_str, &ext);

			cairo_move_to
				(cr, 0.5+STARD-ext.width/2-ext.x_bearing,
					 0.5-ext.height/2-ext.y_bearing);
			cairo_text_path(cr, value_str);
			cairo_fill_with_outline(cr, OUTL_SIZE, hovered);
	}
}

static void cleanup
( )
{
	cairo_path_destroy(star_attr);
	cairo_path_destroy(star_button);
}

struct TileAttribute* attr_priority_create
()
{
	star_attr = create_star
		(0.5-STARD, 0.5, STAR_ARMS, STAR_INR, STAR_OUTR);

	star_button = create_star
		(0.5, 0.5, STAR_ARMS, STAR_INR*STAR_BSZ, STAR_OUTR*STAR_BSZ);


	struct TileAttribute *attr = &tile_attribute;

	attr->name = "Priority";
	attr->default_value = 0;
	attr->icon_value = -1;
	attr->hover_precision = FALSE;
	attr->tile_clicked = &tile_clicked;
	attr->draw_attr = &draw_attr;
	attr->cleanup = &cleanup;

	return attr;
}

