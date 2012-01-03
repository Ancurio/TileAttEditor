
#include <stdlib.h>
#include <stdio.h>
#include <cairo.h>

#include "attribute.h"

#define G_TAU 2*G_PI

static gint tile_clicked
( gint old_value, gdouble x, gdouble y )
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
		case 0 : cairo_arc(cr, 0.5, 0.5, 0.2, 0, G_TAU);
		         break;

		case 1 : cairo_move_to(cr, 0.3, 0.3);
		         cairo_line_to(cr, 0.7, 0.7);
		         cairo_move_to(cr, 0.7, 0.3);
		         cairo_line_to(cr, 0.3, 0.7);
	}

	if (hovered) { tile_attr_set_primary_color(cr); }
	else         { tile_attr_set_secondary_color(cr); }
	cairo_set_line_width(cr, 0.1);
	cairo_stroke_preserve(cr);
	if (hovered) { tile_attr_set_secondary_color(cr); }
	else         { tile_attr_set_primary_color(cr); }
	cairo_set_line_width(cr, 0.05);
	cairo_stroke(cr);
}

struct TileAttribute* attr_passability_create
()
{
	struct TileAttribute *attr =
		malloc(sizeof(struct TileAttribute));

	attr->name = "Passability";
	attr->default_value = 0;
	attr->hover_precision = FALSE;
//	attr->value_buffer = malloc(sizeof(gint)*buffer_size);
	attr->tile_clicked = &tile_clicked;
	attr->draw_attr = &draw_attr;

	return attr;
}
