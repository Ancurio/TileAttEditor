/*
 * settings.h
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


struct Settings
{
	gint active_attr_id;
	gdouble tileset_scale_ratio;
	gdouble attribute_alpha;
	gboolean smooth_zoom;
	gboolean show_button_labels;
	gboolean workspace_flipped;

	struct Color *bg_color;
	struct Color *grid_color;

	gint preferred_tile_width;
	gint preferred_tile_height;
	gint window_width;
	gint window_height;
	gchar *last_opened;
};


struct Settings* settings_read
( struct TileAttribute **tile_attr, gboolean *dirty_flag );

void settings_write
( struct Settings *settings, struct TileAttribute **tile_attr );

void settings_destroy
( struct Settings *settings );
