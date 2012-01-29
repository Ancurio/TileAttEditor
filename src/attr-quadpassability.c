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


#define QDIR_DRAW(qdir, path, empty_x, empty_y)                     \
{                                                                   \
	if (attr_value & qdir)                                          \
	{                                                               \
		cairo_append_path(cr, arrow_paths[path]);                   \
		cairo_fill_with_outline                                     \
			(cr, OUTLW, hovered && (hover_dir & qdir));             \
	}                                                               \
	else                                                            \
	{                                                               \
		attr_draw_empty                                             \
			(cr, empty_x, empty_y, hovered && (hover_dir & qdir));  \
	}                                                               \
}

/* Style-Parameters: These define the visual look */
#define LINEW 0.125
#define LINEH 0.1
#define HEADW 0.07
#define HEADH 0.22
#define DISCR 0.12
#define OUTLW 0.03


static struct TileAttribute tile_attribute;

static cairo_path_t *arrow_paths[4];

/* private functions */
static enum QuadDirection get_direction
( gdouble x, gdouble y );
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

static void draw_attr
( gint attr_value, cairo_t *cr, gboolean hovered,
  gdouble offset_x, gdouble offset_y )
{
	enum QuadDirection hover_dir =
		hovered ? get_direction(offset_x, offset_y) : 0;

	QDIR_DRAW(QDIR_LEFT, 0, 0.25, 0.5);

	QDIR_DRAW(QDIR_RIGHT, 1, 0.75, 0.5);

	QDIR_DRAW(QDIR_UP, 2, 0.5, 0.25);

	QDIR_DRAW(QDIR_DOWN, 3, 0.5, 0.75);

}

static void cleanup
( )
{
	gint i;
	for (i=0;i<4;i++)
		{ cairo_path_destroy(arrow_paths[i]); }
}

struct TileAttribute* attr_quadpassability_create
()
{
	cairo_surface_t *dummy_surf =
		cairo_image_surface_create(CAIRO_FORMAT_A1, 1, 1);
	cairo_t *cr = cairo_create(dummy_surf);

	cairo_move_to(cr, 0.5-DISCR, 0.5+LINEW/2);
	cairo_rel_line_to(cr, 0, -LINEW);
	cairo_rel_line_to(cr, -LINEH, 0);
	cairo_rel_line_to(cr, 0, -HEADW);
	cairo_rel_line_to(cr, -HEADH, HEADW+LINEW/2);
	cairo_rel_line_to(cr, +HEADH, HEADW+LINEW/2);
	cairo_rel_line_to(cr, 0, -HEADW);
	cairo_rel_line_to(cr, LINEH, 0);
	cairo_close_path(cr);

	arrow_paths[0] = cairo_copy_path(cr);
	cairo_new_path(cr);

	cairo_move_to(cr, 0.5+DISCR, 0.5+LINEW/2);
	cairo_rel_line_to(cr, 0, -LINEW);
	cairo_rel_line_to(cr, +LINEH, 0);
	cairo_rel_line_to(cr, 0, -HEADW);
	cairo_rel_line_to(cr, +HEADH, HEADW+LINEW/2);
	cairo_rel_line_to(cr, -HEADH, HEADW+LINEW/2);
	cairo_rel_line_to(cr, 0, -HEADW);
	cairo_rel_line_to(cr, -LINEH, 0);
	cairo_close_path(cr);

	arrow_paths[1] = cairo_copy_path(cr);
	cairo_new_path(cr);

	cairo_move_to(cr, 0.5+LINEW/2, 0.5-DISCR);
	cairo_rel_line_to(cr, -LINEW, 0);
	cairo_rel_line_to(cr, 0, -LINEH);
	cairo_rel_line_to(cr, -HEADW, 0);
	cairo_rel_line_to(cr, HEADW+LINEW/2, -HEADH);
	cairo_rel_line_to(cr, HEADW+LINEW/2, +HEADH);
	cairo_rel_line_to(cr, -HEADW, 0);
	cairo_rel_line_to(cr, 0, +LINEH);
	cairo_close_path(cr);

	arrow_paths[2] = cairo_copy_path(cr);
	cairo_new_path(cr);

	cairo_move_to(cr, 0.5+LINEW/2, 0.5+DISCR);
	cairo_rel_line_to(cr, -LINEW, 0);
	cairo_rel_line_to(cr, 0, +LINEH);
	cairo_rel_line_to(cr, -HEADW, 0);
	cairo_rel_line_to(cr, HEADW+LINEW/2, +HEADH);
	cairo_rel_line_to(cr, HEADW+LINEW/2, -HEADH);
	cairo_rel_line_to(cr, -HEADW, 0);
	cairo_rel_line_to(cr, 0, -LINEH);
	cairo_close_path(cr);

	arrow_paths[3] = cairo_copy_path(cr);

	cairo_surface_destroy(dummy_surf);
	cairo_destroy(cr);


	struct TileAttribute *attr = &tile_attribute;

	attr->name = "QuadPassability";
	attr->default_value = 0;
	attr->icon_value = 0xF;
	attr->hover_precision = TRUE;
	attr->tile_clicked = &tile_clicked;
	attr->draw_attr = &draw_attr;
	attr->cleanup = &cleanup;

	return attr;
}

