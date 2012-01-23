/*
 * main.c
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


#include <stdlib.h>
#include <limits.h>
#include <glib.h>

#include "tileatteditor.h"
#include "tileset-area.h"
#include "settings.h"
#include "file.h"


/* private functions */
static struct GlobalData* global_data_create
( );

static void global_data_destroy
( struct GlobalData *global_data );
/* ----------------- */


static struct GlobalData* global_data_create
( )
{
	struct GlobalData *global_data =
		g_malloc( sizeof( *global_data ) );
	global_data->buffer_changed = FALSE;
	global_data->hovered_tile = -1;
	global_data->statusbar_context_id = 0;
	global_data->statusbar_message_id = 0;
	global_data->reusable_surface = NULL;
	global_data->tileset = NULL;
	global_data->settings_dialog = NULL;
	global_data->new_file_dialog = NULL;
	global_data->open_file_path = NULL;

	return global_data;
}

static void global_data_destroy
( struct GlobalData *global_data )
{
	file_close(global_data);
	settings_destroy(global_data->settings);
	tile_attr_destroy(global_data->tile_attributes);
	ui_main_window_destroy(global_data->main_window);

	g_free(global_data->open_file_path);

	g_free(global_data);
}


gint main
( gint argc, gchar *argv[] )
{
	struct GlobalData *global_data = global_data_create();

	global_data->tile_attributes =
		tile_attr_create(global_data);

	settings_read(global_data);

	gtk_init(&argc, &argv);

	if (argc > 1)
	{
		gchar filepath[1024];
		realpath(argv[1], filepath);

		file_open_attempt_quiet(global_data, filepath);
	}
	else
	{
		file_open_attempt_quiet
			(global_data, global_data->settings->last_opened);
	}

	ui_main_window_create(global_data);

	//struct Tileset *t = global_data->tileset;
	//g_message("Conclusion: %d : %d : %d : %d : %d : %d : %d : %s : %d : %d : %d",
		//t->width, t->height, t->disp_width, t->disp_height, t->tile_width, t->tile_height, t->tile_count, t->image_file, t->cairo_surface, t->cairo_scaled_surface, t->cached_composition);
	//cairo_surface_write_to_png(t->cairo_surface, "cairo_surface.png"); cairo_surface_write_to_png(t->cairo_scaled_surface, "cairo_scaled_surface.png"); cairo_surface_write_to_png(t->cached_composition, "cached_composition.png");

	gtk_widget_show(global_data->main_window->window);

	gtk_main();

	if (global_data->open_file_path)
	{
		global_data->settings->last_opened =
			g_strdup(global_data->open_file_path);
	}
	else { global_data->settings->last_opened = ""; }

	settings_write
		(global_data->settings, global_data->tile_attributes);

	global_data_destroy(global_data);

	return 0;
}

