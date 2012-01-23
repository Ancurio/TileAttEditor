/*
 * color.c
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

#include "color.h"


/* private functions */
static gdouble check_value
( gdouble value );
/* ----------------- */


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

void color_destroy
( struct Color *color )
{
	g_free(color);
}

