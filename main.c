
#include <stdlib.h>
#include <limits.h>
#include <glib.h>

#include "tileatteditor.h"
#include "tileset-area.h"
#include "settings.h"
#include "file.h"


struct GlobalData* global_data_create
( )
{
	struct GlobalData *global_data =
		g_malloc( sizeof( *global_data ) );
	global_data->buffer_changed = FALSE;
	global_data->hovered_tile = -1;
	global_data->hover_context_id = 0;
	global_data->hover_message_id = 0;
	global_data->tileset = NULL;
	global_data->settings_dialog = NULL;
	global_data->new_file_dialog = NULL;
	global_data->open_file_path = NULL;

	return global_data;
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

		file_open_attempt_noerror(global_data, filepath);
	}
	else
	{
		file_open_attempt_noerror
			(global_data, global_data->settings->last_opened);
	}

	ui_main_window_create(global_data);

	//struct Tileset *t = global_data->tileset;
	//g_message("Conclusion: %d : %d : %d : %d : %d : %d : %d : %s : %d : %d : %d",
		//t->width, t->height, t->disp_width, t->disp_height, t->tile_width, t->tile_height, t->tile_count, t->image_file, t->cairo_surface, t->cairo_scaled_surface, t->cached_composition);
	//cairo_surface_write_to_png(t->cairo_surface, "cairo_surface.png"); cairo_surface_write_to_png(t->cairo_scaled_surface, "cairo_scaled_surface.png"); cairo_surface_write_to_png(t->cached_composition, "cached_composition.png");

//	if (argc > 1)
//		{ file_open(argv[1]) }


	gtk_widget_show(global_data->main_window->window);

	gtk_main();

	if (global_data->open_file_path)
	{
		global_data->settings->last_opened =
			global_data->open_file_path;
	}
	else { global_data->settings->last_opened = ""; }

	settings_write
		(global_data->settings, global_data->tile_attributes);
	return 0;
}
