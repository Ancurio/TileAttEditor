
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
		case 0 :  cairo_arc(cr, 0.5, 0.5, 0.05, 0, G_TAU);
		          cairo_set_line_width(cr, 0.06);
		          break;

		default : cairo_move_to(cr, 0.5, 0.3);
		          cairo_line_to(cr, 0.7, 0.5);
		          cairo_line_to(cr, 0.5, 0.7);
		          cairo_line_to(cr, 0.3, 0.5);
		          cairo_line_to(cr, 0.5, 0.3);
		          cairo_line_to(cr, 0.7, 0.5);
		          cairo_set_line_width(cr, 0.08);
	}

	cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
	tile_attr_set_color(cr, hovered, ATTR_COLOR_SEC);
	cairo_stroke_preserve(cr);
	tile_attr_set_color(cr, hovered, ATTR_COLOR_PRI);
	cairo_fill(cr);
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