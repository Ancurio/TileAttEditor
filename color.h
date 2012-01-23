/*
 * color.h
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




/* Colors are bit-depth independent RGBA values in range [0;1]
 * This is basically a re-implementation of GdkRGBA available in GDK3
 */

struct Color
{
	gdouble r;
	gdouble g;
	gdouble b;
	gdouble a;
};

struct Color* color_new
( gdouble r, gdouble g, gdouble b, gdouble a );

void color_cairo_set_source
( cairo_t *cr, struct Color *color );

void color_set_from_button
( struct Color *color, GtkWidget *button );

void color_set_button
( struct Color *color, GtkWidget *button );

void color_destroy
( struct Color *color );
