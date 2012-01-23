/*
 * attr-quadpassability.c
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


#include <stdio.h>
#include <cairo.h>

#include "attribute.h"

/* Style-Parameters: These define the visual look */
#define LINEW 0.125
#define LINEH 0.1
#define HEADW 0.07
#define HEADH 0.22
#define DISCR 0.12
#define OUTLW 0.06


/* private functions */
static enum QuadDirection get_direction
( gdouble x, gdouble y );

static void draw_empty
( cairo_t *cr, gdouble x, gdouble y, gboolean hovered );

static void quaddir_draw
( cairo_t *cr, gboolean hovered );
/* ----------------- */


enum QuadDirection
{
	QDIR_LEFT  = 1 << 0,
	QDIR_RIGHT = 1 << 1,
	QDIR_UP    = 1 << 2,
	QDIR_DOWN  = 1 << 3
};

static enum QuadDirection get_direction
( gdouble x, gdouble y )
{
	gdouble angle = (0.5-y)/(x-0.5);

	return (angle*angle < 1) ?
		((x > 0.5) ? QDIR_RIGHT : QDIR_LEFT) :
		((y > 0.5) ? QDIR_DOWN  : QDIR_UP  ) ;
}


static gint tile_clicked
( gint old_value, gdouble x, gdouble y )
{
	return old_value ^ get_direction(x, y);
}

static void draw_empty
( cairo_t *cr, gdouble x, gdouble y, gboolean hovered )
{
	cairo_arc(cr, x, y, 0.05, 0, G_TAU);
	tile_attr_set_color(cr, hovered, ATTR_COLOR_SEC);
	cairo_set_line_width(cr, 0.06);
	cairo_stroke_preserve(cr);
	tile_attr_set_color(cr, hovered, ATTR_COLOR_PRI);
	cairo_fill(cr);
}

static void quaddir_draw
( cairo_t *cr, gboolean hovered )
{
	tile_attr_set_color(cr, hovered, ATTR_COLOR_SEC);
	cairo_set_line_width(cr, OUTLW);
	cairo_stroke_preserve(cr);
	tile_attr_set_color(cr, hovered, ATTR_COLOR_PRI);
	cairo_fill(cr);
}

static void draw_attr
( gint attr_value, cairo_t *cr, gboolean hovered,
  gdouble offset_x, gdouble offset_y )
{
	cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);

	enum QuadDirection hover_dir =
		hovered ? get_direction(offset_x, offset_y) : 0;
	if (attr_value & QDIR_LEFT)
		{
			cairo_move_to(cr, 0.5-DISCR, 0.5+LINEW/2);
			cairo_rel_line_to(cr, 0, -LINEW);
			cairo_rel_line_to(cr, -LINEH, 0);
			cairo_rel_line_to(cr, 0, -HEADW);
			cairo_rel_line_to(cr, -HEADH, HEADW+LINEW/2);
			cairo_rel_line_to(cr, +HEADH, HEADW+LINEW/2);
			cairo_rel_line_to(cr, 0, -HEADW);
			cairo_rel_line_to(cr, LINEH, 0);
			cairo_rel_line_to(cr, 0, -LINEW);
			quaddir_draw(cr, hovered&&(hover_dir&QDIR_LEFT));
		}
	else
		{ draw_empty(cr, 0.25, 0.5, hovered&&(hover_dir&QDIR_LEFT)); }

	if (attr_value & QDIR_RIGHT)
		{
			cairo_move_to(cr, 0.5+DISCR, 0.5+LINEW/2);
			cairo_rel_line_to(cr, 0, -LINEW);
			cairo_rel_line_to(cr, +LINEH, 0);
			cairo_rel_line_to(cr, 0, -HEADW);
			cairo_rel_line_to(cr, +HEADH, HEADW+LINEW/2);
			cairo_rel_line_to(cr, -HEADH, HEADW+LINEW/2);
			cairo_rel_line_to(cr, 0, -HEADW);
			cairo_rel_line_to(cr, -LINEH, 0);
			cairo_rel_line_to(cr, 0, -LINEW);
			quaddir_draw(cr, hovered&&(hover_dir&QDIR_RIGHT));
		}
	else
		{ draw_empty(cr, 0.75, 0.5, hovered&&(hover_dir&QDIR_RIGHT)); }

	if (attr_value & QDIR_UP)
		{
			cairo_move_to(cr, 0.5+LINEW/2, 0.5-DISCR);
			cairo_rel_line_to(cr, -LINEW, 0);
			cairo_rel_line_to(cr, 0, -LINEH);
			cairo_rel_line_to(cr, -HEADW, 0);
			cairo_rel_line_to(cr, HEADW+LINEW/2, -HEADH);
			cairo_rel_line_to(cr, HEADW+LINEW/2, +HEADH);
			cairo_rel_line_to(cr, -HEADW, 0);
			cairo_rel_line_to(cr, 0, +LINEH);
			cairo_rel_line_to(cr, -LINEW, 0);
			quaddir_draw(cr, hovered&&(hover_dir&QDIR_UP));
		}
	else
		{ draw_empty(cr, 0.5, 0.25, hovered&&(hover_dir&QDIR_UP)); }

	if (attr_value & QDIR_DOWN)
		{
			cairo_move_to(cr, 0.5+LINEW/2, 0.5+DISCR);
			cairo_rel_line_to(cr, -LINEW, 0);
			cairo_rel_line_to(cr, 0, +LINEH);
			cairo_rel_line_to(cr, -HEADW, 0);
			cairo_rel_line_to(cr, HEADW+LINEW/2, +HEADH);
			cairo_rel_line_to(cr, HEADW+LINEW/2, -HEADH);
			cairo_rel_line_to(cr, -HEADW, 0);
			cairo_rel_line_to(cr, 0, -LINEH);
			cairo_rel_line_to(cr, -LINEW, 0);
			quaddir_draw(cr, hovered&&(hover_dir&QDIR_DOWN));
		}
	else
		{ draw_empty(cr, 0.5, 0.75, hovered&&(hover_dir&QDIR_DOWN)); }

}

struct TileAttribute* attr_quadpassability_create
()
{
	struct TileAttribute *attr =
		g_malloc(sizeof(struct TileAttribute));

	attr->name = "QuadPassability";
	attr->default_value = 0;
	attr->icon_value = 0xF;
	attr->hover_precision = TRUE;
	attr->tile_clicked = &tile_clicked;
	attr->draw_attr = &draw_attr;

	return attr;
}

