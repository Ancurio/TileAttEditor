
#include <glib.h>

#include "tileatteditor.h"
#include "settings.h"

#define SETTINGS_FILE_NAME "TileAttEditor.conf"

#define VALUE_TO_KEY_INIT(keyfile, groupname)                  \
	GKeyFile *VALUE_TO_KEY_KEYFILE = keyfile;	               \
	const gchar *VALUE_TO_KEY_GROUPNAME = groupname;	       \
	gchar *VALUE_TO_KEY_TEMP_HEAP_STRING;	                   \
	gdouble VALUE_TO_KEY_TEMP_ARRAY[4];	                       \
	gdouble *VALUE_TO_KEY_TEMP_ARRAY_PTR;	                   \
	gsize VALUE_TO_KEY_TEMP_ARRAY_LEN;	                       \
	GError *VALUE_TO_KEY_ERROR = NULL;	                       \


#define VALUE_TO_KEY(val, type)                                \
	VALUE_TO_KEY_TEMP_HEAP_STRING =	                           \
		settings_chomp_identifier(g_strdup(#val));		       \
	g_key_file_set_##type	                                   \
		(VALUE_TO_KEY_KEYFILE,		                           \
		 VALUE_TO_KEY_GROUPNAME,		                       \
		 VALUE_TO_KEY_TEMP_HEAP_STRING, val);		           \
	g_free(VALUE_TO_KEY_TEMP_HEAP_STRING);


#define COLOR_TO_KEY(color)                                    \
	VALUE_TO_KEY_TEMP_HEAP_STRING =	                           \
		settings_chomp_identifier(g_strdup(#color));		   \
	VALUE_TO_KEY_TEMP_ARRAY[0] = color->r;	                   \
	VALUE_TO_KEY_TEMP_ARRAY[1] = color->g;	                   \
	VALUE_TO_KEY_TEMP_ARRAY[2] = color->b;	                   \
	VALUE_TO_KEY_TEMP_ARRAY[3] = color->a;	                   \
	g_key_file_set_double_list	                               \
		(VALUE_TO_KEY_KEYFILE,		                           \
		 VALUE_TO_KEY_GROUPNAME,		                       \
		 VALUE_TO_KEY_TEMP_HEAP_STRING,		                   \
		 VALUE_TO_KEY_TEMP_ARRAY, 4);		                   \
	g_free(VALUE_TO_KEY_TEMP_HEAP_STRING);


#define KEY_TO_VALUE(val, type, default_val)                   \
	VALUE_TO_KEY_TEMP_HEAP_STRING =	                           \
		settings_chomp_identifier(g_strdup(#val));		       \
	val = g_key_file_get_##type	                               \
		(VALUE_TO_KEY_KEYFILE,		                           \
		 VALUE_TO_KEY_GROUPNAME,		                       \
		 VALUE_TO_KEY_TEMP_HEAP_STRING,		                   \
		 &VALUE_TO_KEY_ERROR);		                           \
	if (VALUE_TO_KEY_ERROR)	                                   \
	{	                                                       \
		val = (default_val);		                           \
		g_error_free(VALUE_TO_KEY_ERROR);		               \
		VALUE_TO_KEY_ERROR = NULL;		                       \
	}	                                                       \
	g_free(VALUE_TO_KEY_TEMP_HEAP_STRING);


#define KEY_TO_COLOR(color, def_r, def_g, def_b, def_a)        \
	VALUE_TO_KEY_TEMP_HEAP_STRING =	                           \
		settings_chomp_identifier(g_strdup(#color));		   \
	VALUE_TO_KEY_TEMP_ARRAY_PTR =	                           \
		g_key_file_get_double_list		                       \
			(VALUE_TO_KEY_KEYFILE,			                   \
			 VALUE_TO_KEY_GROUPNAME,			               \
			 VALUE_TO_KEY_TEMP_HEAP_STRING,			           \
			 &VALUE_TO_KEY_TEMP_ARRAY_LEN,			           \
			 &VALUE_TO_KEY_ERROR);			                   \
	if (VALUE_TO_KEY_ERROR ||	                               \
	    VALUE_TO_KEY_TEMP_ARRAY_LEN != 4)	                   \
	{	                                                       \
		color = color_new		                               \
			(def_r, def_g, def_b, def_a);			           \
		if (VALUE_TO_KEY_ERROR)		                           \
			{ g_error_free(VALUE_TO_KEY_ERROR); }			   \
		VALUE_TO_KEY_ERROR = NULL;		                       \
	}	                                                       \
	else	                                                   \
	{	                                                       \
		color = color_new		                               \
			(VALUE_TO_KEY_TEMP_ARRAY_PTR[0],			       \
			 VALUE_TO_KEY_TEMP_ARRAY_PTR[1],			       \
			 VALUE_TO_KEY_TEMP_ARRAY_PTR[2],			       \
			 VALUE_TO_KEY_TEMP_ARRAY_PTR[3]);			       \
	}	                                                       \
	g_free(VALUE_TO_KEY_TEMP_HEAP_STRING);



//static void settings_set_default_values
//( struct GlobalData *global_data )
//{
	//struct Settings *settings =
		//g_malloc( sizeof( struct Settings ) );

	//settings->active_attr_id = 0;
	//settings->tileset_scale_ratio = 1.9;
	//settings->attribute_alpha = 0.9;
	//settings->smooth_zoom = FALSE;
	//settings->bg_color = color_new(0.04, 0.65, 0.72, 1);
	//settings->grid_color = color_new(0, 0, 0, 0.6);
	//settings->preferred_tile_width = 32;
	//settings->preferred_tile_height = 32;

	//global_data->settings = settings;

//}

static gchar* settings_chomp_identifier
( gchar *identifier )
{
	guint read_head, write_head;

	for (read_head = 0;
	     identifier[read_head] != '>' &&
	     identifier[read_head] != '\0';
	     read_head++) {}

	for (write_head = 0;
	     identifier[read_head+1] != 0;
	     write_head++)
	{
		read_head++;
		identifier[write_head] = identifier[read_head];
	}

	identifier[write_head] = '\0';
	return identifier;
}


void settings_write
( struct Settings *settings, struct TileAttribute **tile_attr )
{
	const char *keyfile_filename = SETTINGS_FILE_NAME;
	GKeyFile *keyfile = g_key_file_new();

	VALUE_TO_KEY_INIT(keyfile, "Settings")

	VALUE_TO_KEY(settings->active_attr_id, integer)
	VALUE_TO_KEY(settings->tileset_scale_ratio, double)
	VALUE_TO_KEY(settings->attribute_alpha, double)
	VALUE_TO_KEY(settings->smooth_zoom, boolean)
	VALUE_TO_KEY(settings->workspace_flipped, boolean)

	COLOR_TO_KEY(settings->bg_color)
	COLOR_TO_KEY(settings->grid_color)

	VALUE_TO_KEY(settings->preferred_tile_width, integer)
	VALUE_TO_KEY(settings->preferred_tile_height, integer)

	struct TileAttribute **_tile_attr;
	for (_tile_attr = tile_attr; *_tile_attr; _tile_attr++)
	{
		gchar *key =
			g_strconcat((*_tile_attr)->name, "_enabled", NULL);
		g_key_file_set_boolean
			(keyfile, "TileAttributes", key, (*_tile_attr)->enabled);
		g_free(key);
	}

	gsize data_length;
	char *keyfile_data =
		g_key_file_to_data(keyfile, &data_length, NULL);
	g_file_set_contents
		(keyfile_filename, keyfile_data, (gssize)data_length, NULL);
}

void settings_read
( struct GlobalData *global_data )
{
	const char *keyfile_filename = SETTINGS_FILE_NAME;
	GKeyFile *keyfile = g_key_file_new();

	g_key_file_load_from_file
			(keyfile, keyfile_filename, G_KEY_FILE_NONE, NULL);


	struct Settings *settings =
		g_malloc( sizeof *settings );

	VALUE_TO_KEY_INIT(keyfile, "Settings")

	KEY_TO_VALUE(settings->active_attr_id, integer, 0)
	if (settings->active_attr_id > ATTRIBUTE_COUNT-1)
		{ settings->active_attr_id = 0;}

	KEY_TO_VALUE(settings->tileset_scale_ratio, double, 1.5)
	if (settings->tileset_scale_ratio < 0.1)
		{ settings->tileset_scale_ratio = 1.5; }

	KEY_TO_VALUE(settings->attribute_alpha, double, 0.8)
	if (settings->attribute_alpha < 0 ||
	    settings->attribute_alpha > 1)
		{ settings->attribute_alpha = 0.8; }

	KEY_TO_VALUE(settings->smooth_zoom, boolean, FALSE)
	KEY_TO_VALUE(settings->workspace_flipped, boolean, FALSE)

	KEY_TO_COLOR(settings->bg_color, 0.04, 0.65, 0.72, 1)
	settings->bg_color->a = 1;

	KEY_TO_COLOR(settings->grid_color, 0, 0, 0, 0.6)

	KEY_TO_VALUE(settings->preferred_tile_width, integer, 32)
	if (settings->preferred_tile_width < 1)
		{ settings->preferred_tile_width = 32; }

	KEY_TO_VALUE(settings->preferred_tile_height, integer, 32)
	if (settings->preferred_tile_height < 1)
		{ settings->preferred_tile_height = 32; }

	global_data->settings = settings;

	struct TileAttribute **_tile_attr;
	for (_tile_attr = global_data->tile_attributes;
	     *_tile_attr; _tile_attr++)
	{
		GError *error = NULL;
		gchar *key =
			g_strconcat((*_tile_attr)->name, "_enabled", NULL);
		(*_tile_attr)->enabled = g_key_file_get_boolean
			(keyfile, "TileAttributes", key, &error);
		if (error)
		{
			(*_tile_attr)->enabled = TRUE;
			g_error_free(error);
		}
		g_free(key);
	}
}
