
#include <stdlib.h>
#include <limits.h>
#include <glib.h>

#include "tileatteditor.h"
#include "tileset-area.h"
#include "settings.h"
#include "file.h"


//void create_attr_buffer_dummy_function
//( struct GlobalData *global_data )
//{
	//struct TileAttribute **attributes =
		//global_data->tile_attributes;
	//gint i;
	//for (i=0;attributes[i];i++)
	//{
		//attributes[i]->value_buffer =
			//tile_attr_create_value_buffer
			//(global_data->tileset->tile_count);
	//}
//}

struct GlobalData* global_data_create
( )
{
	struct GlobalData *global_data =
		malloc( sizeof( *global_data ) );
	global_data->hovered_tile = -1;
	global_data->hover_context_id = 0;
	global_data->hover_message_id = 0;
	global_data->settings_dialog = 0;
	global_data->new_file_dialog = 0;
	global_data->open_file_path = NULL;
}

gint main
( gint argc, gchar *argv[] )
{
	struct GlobalData *global_data = global_data_create();

	global_data->tile_attributes =
		tile_attr_create(global_data);

	settings_read(global_data);

	gtk_init(&argc, &argv);

//	tileset_create_from_file(global_data, g_strdup("isometric_grass_and_water.png"), 64, 64);

//	create_attr_buffer_dummy_function(global_data);

///	tileset_area_redraw_cache(global_data);

	if (argc > 1)
	{
		gchar filepath[1024];
		realpath(argv[1], filepath);
		g_message("cwd: %s", filepath);

		struct File *file = file_open(filepath, NULL);
		if (file)
		{
			if (file_parse(global_data, file, NULL))
				{ global_data->open_file_path = g_strdup(filepath); }
		}
	}

	ui_main_window_create(global_data);

//	cairo_surface_write_to_png(global_data->tileset->cached_composition, "cached_composition.png");

	//struct Tileset *t = global_data->tileset;
	//g_message("Conclusion: %d : %d : %d : %d : %d : %d : %d : %s : %d : %d : %d",
		//t->width, t->height, t->disp_width, t->disp_height, t->tile_width, t->tile_height, t->tile_count, t->image_file, t->cairo_surface, t->cairo_scaled_surface, t->cached_composition);
	//cairo_surface_write_to_png(t->cairo_surface, "cairo_surface.png"); cairo_surface_write_to_png(t->cairo_scaled_surface, "cairo_scaled_surface.png"); cairo_surface_write_to_png(t->cached_composition, "cached_composition.png");

//	if (argc > 1)
//		{ file_open(argv[1]) }


	gtk_widget_show(global_data->main_window->window);

	gtk_main();

	settings_write
		(global_data->settings, global_data->tile_attributes);
}
