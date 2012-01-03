
#include <gtk/gtk.h>
#include <cairo.h>

#include "color.h"

struct Color* color_new
( gdouble r, gdouble g, gdouble b, gdouble a )
{
	struct Color *color =
		g_malloc( sizeof( struct Color ) );
	color->r = r; color->g = g; color->b = b; color->a = a;

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
