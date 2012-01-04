
#include <gtk/gtk.h>
#include <cairo.h>

#include "color.h"

static gdouble check_value
( gdouble value )
{
	if (value < 0) { return 0; }
	if (value > 1) { return 1; }
	return value;
}

struct Color* color_new
( gdouble r, gdouble g, gdouble b, gdouble a )
{
	struct Color *color =
		g_malloc( sizeof( struct Color ) );
	color->r = check_value(r); color->g = check_value(g);
	color->b = check_value(b); color->a = check_value(a);

	return color;
}

void color_cairo_set_source
( cairo_t *cr, struct Color *color )
{
	cairo_set_source_rgba
		(cr, color->r, color->g, color->b, color->a);
}

void color_set_from_button
( struct Color *color, GtkWidget *button )
{
	GdkColor b_color;
	gtk_color_button_get_color
		(GTK_COLOR_BUTTON(button), &b_color);
	color->r = (gdouble)b_color.red/0x0FFFF;
	color->g = (gdouble)b_color.green/0x0FFFF;
	color->b = (gdouble)b_color.blue/0x0FFFF;
	color->a = (gdouble)gtk_color_button_get_alpha
		(GTK_COLOR_BUTTON( button )) / 0x0FFFF;

}

void color_set_button
( struct Color *color, GtkWidget *button )
{
	GdkColor b_color;
	b_color.red = (guint16)(color->r * 0x0FFFF);
	b_color.green = (guint16)(color->g * 0x0FFFF);
	b_color.blue = (guint16)(color->b * 0x0FFFF);
	gtk_color_button_set_color
		(GTK_COLOR_BUTTON(button), &b_color);
	gtk_color_button_set_alpha
		(GTK_COLOR_BUTTON( button ),
		(guint16)(color->a * 0x0FFFF));
}
