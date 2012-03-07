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


#include <glib.h>

#include "tileatteditor.h"
#include "tileset-area.h"
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
		g_malloc0( sizeof( *global_data ) );

	global_data->hovered_tile = -1;

	return global_data;
}

static void global_data_destroy
( struct GlobalData *global_data )
{
	file_close(global_data);
	settings_destroy(global_data->settings);
	tile_attrs_destroy(global_data->tile_attributes);
	ui_main_window_destroy(global_data->main_window);

	g_free(global_data->open_file_path);

	g_free(global_data);
}


gint main
( gint argc, gchar *argv[] )
{
	struct GlobalData *global_data = global_data_create();

	global_data->tile_attributes =
		tile_attrs_create(global_data);

	global_data->settings =
		settings_read
			(global_data->tile_attributes,
			 &global_data->settings_dirty);

	gtk_init(&argc, &argv);

	if (argc > 1)
	{
		gchar filepath[1024];
		g_realpath(argv[1], filepath);

		file_open_attempt_quiet(global_data, filepath);
	}
	else
	{
		file_open_attempt_quiet
			(global_data, global_data->settings->last_opened);
	}

	ui_main_window_create(global_data);

	gtk_widget_show(global_data->main_window->window);

	gtk_main();

	if (global_data->open_file_path)
	{
		global_data->settings->last_opened =
			g_strdup(global_data->open_file_path);
	}
	else { global_data->settings->last_opened = ""; }

	if (global_data->settings_dirty)
	{
		settings_write
			(global_data->settings, global_data->tile_attributes);
	}

	global_data_destroy(global_data);

	return 0;
}

