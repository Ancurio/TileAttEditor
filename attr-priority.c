
#include <gtk/gtk.h>
#include <cairo.h>

#include "attribute.h"

#define FLAG_MAX_VAL 5

#define FONT_SIZE 0.5
#define OUTL_SIZE 0.03
#define STARD     0.17


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
			cairo_set_font_size(cr, FONT_SIZE*1.5);
			cairo_text_extents(cr, "★", &ext);
			cairo_move_to
				(cr, 0.5-ext.width/2-ext.x_bearing,
					 0.5-ext.height/2-ext.y_bearing);
			tile_attr_set_color(cr, hovered, ATTR_COLOR_PRI);
			cairo_show_text(cr, "★");

			cairo_move_to
				(cr, 0.5-ext.width/2-ext.x_bearing,
					 0.5-ext.height/2-ext.y_bearing);
			cairo_text_path(cr, "★");
			tile_attr_set_color(cr, hovered, ATTR_COLOR_SEC);
			cairo_set_line_width(cr, OUTL_SIZE*1.5);
			cairo_stroke(cr);
			break;

		case 0  :
			cairo_arc(cr, 0.5, 0.5, 0.05, 0, G_TAU);
			tile_attr_set_color(cr, hovered, ATTR_COLOR_SEC);
			cairo_set_line_width(cr, 0.06);
			cairo_stroke_preserve(cr);
			tile_attr_set_color(cr, hovered, ATTR_COLOR_PRI);
			cairo_fill(cr);
			break;

		default :
			cairo_set_font_size(cr, FONT_SIZE);
			cairo_text_extents(cr, "★", &ext);
			cairo_move_to
				(cr, 0.5-STARD-ext.width/2-ext.x_bearing,
					 0.5-ext.height/2-ext.y_bearing);
			tile_attr_set_color(cr, hovered, ATTR_COLOR_PRI);
			cairo_show_text(cr, "★");

			cairo_move_to
				(cr, 0.5-STARD-ext.width/2-ext.x_bearing,
					 0.5-ext.height/2-ext.y_bearing);
			cairo_text_path(cr, "★");
			tile_attr_set_color(cr, hovered, ATTR_COLOR_SEC);
			cairo_set_line_width(cr, OUTL_SIZE);
			cairo_stroke(cr);

			g_ascii_dtostr(value_str, 4, (gdouble)attr_value);
			cairo_text_extents(cr, value_str, &ext);
			cairo_move_to
				(cr, 0.5+STARD-ext.width/2-ext.x_bearing,
					 0.5-ext.height/2-ext.y_bearing);
			tile_attr_set_color(cr, hovered, ATTR_COLOR_PRI);
			cairo_show_text(cr, value_str);

			cairo_move_to
				(cr, 0.5+STARD-ext.width/2-ext.x_bearing,
					 0.5-ext.height/2-ext.y_bearing);
			cairo_text_path(cr, value_str);
			tile_attr_set_color(cr, hovered, ATTR_COLOR_SEC);
			cairo_set_line_width(cr, OUTL_SIZE);
			cairo_stroke(cr);
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
