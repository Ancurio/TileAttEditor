
#include <gtk/gtk.h>
#include <cairo.h>

#include "attribute.h"

#define G_TAU 2*G_PI

#define CURVD 0.11
#define AMPL  0.02
#define SPANW 0.20
#define SPANH 0.16
#define LINEW 0.14


static void set_color
( cairo_t *cr, gboolean hovered, gint color )
{
	switch (color)
	{
		case 0 :
		         if (hovered) { tile_attr_set_primary_color(cr); }
		         else         { tile_attr_set_secondary_color(cr); }
		         break;
		case 1 :
		         if (hovered) { tile_attr_set_secondary_color(cr); }
		         else         { tile_attr_set_primary_color(cr); }
	}
}

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
	cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);

	switch(attr_value)
	{
		case 0 : cairo_arc(cr, 0.5, 0.5, 0.05, 0, G_TAU);

		         set_color(cr, hovered, 0);
		         cairo_set_line_width(cr, 0.06);
		         cairo_stroke_preserve(cr);
		         set_color(cr, hovered, 1);
		         cairo_fill(cr);
		         break;

		case 1 : cairo_move_to(cr, 0.5-SPANW, 0.5-CURVD+AMPL);
		         cairo_curve_to
		             (cr, 0.5, 0.5-CURVD-SPANH,
		                  0.5, 0.5-CURVD+SPANH,
		                  0.5+SPANW, 0.5-CURVD-AMPL);
		         cairo_move_to(cr, 0.5-SPANW, 0.5+CURVD+AMPL);
		         cairo_curve_to
		             (cr, 0.5, 0.5+CURVD-SPANH,
		                  0.5, 0.5+CURVD+SPANH,
		                  0.5+SPANW, 0.5+CURVD-AMPL);

		         set_color(cr, hovered, 0);
		         cairo_set_line_width(cr, LINEW);
		         cairo_stroke_preserve(cr);
		         cairo_set_line_width(cr, LINEW/2);
		         set_color(cr, hovered, 1);
		         cairo_stroke(cr);
	}


}

struct TileAttribute* attr_bushflag_create
()
{
	struct TileAttribute *attr =
		g_malloc(sizeof(struct TileAttribute));

	attr->name = "BushFlag";
	attr->default_value = 0;
	attr->icon_value = 1;
	attr->hover_precision = FALSE;
	attr->tile_clicked = &tile_clicked;
	attr->draw_attr = &draw_attr;

	return attr;
}
