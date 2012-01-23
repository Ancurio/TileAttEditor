/*
 * settings.c
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
#include <glib.h>

#include "tileatteditor.h"
#include "settings.h"

#define SETTINGS_FILE_NAME "TileAttEditor.conf"


/* private functions */
static gchar* settings_chomp_identifier
( gchar *identifier );

static gchar* settings_get_keyfile_path
( gboolean mkdir );
/* ----------------- */


#define VALUE_TO_KEY_INIT(keyfile, groupname)                  \
    GKeyFile *VALUE_TO_KEY_KEYFILE = keyfile;                  \
    const gchar *VALUE_TO_KEY_GROUPNAME = groupname;


#define VALUE_TO_KEY(val, type)                                \
{                                                              \
    gchar *chomped_value_name =                                \
        settings_chomp_identifier(g_strdup(#val));             \
    g_key_file_set_##type                                      \
        (VALUE_TO_KEY_KEYFILE,                                 \
         VALUE_TO_KEY_GROUPNAME,                               \
         chomped_value_name, val);                             \
    g_free(chomped_value_name);                                \
}


#define COLOR_TO_KEY(color)                                    \
{                                                              \
    gchar *chomped_value_name =                                \
        settings_chomp_identifier(g_strdup(#color));           \
    gdouble color_array[] =                                    \
        { color->r, color->g, color->b, color->a };            \
    g_key_file_set_double_list                                 \
        (VALUE_TO_KEY_KEYFILE,                                 \
         VALUE_TO_KEY_GROUPNAME,                               \
         chomped_value_name,                                   \
         color_array, 4);                                      \
    g_free(chomped_value_name);                                \
}


#define KEY_TO_VALUE(val, type, default_val)                   \
{                                                              \
    GError *error = NULL;                                      \
    gchar *chomped_value_name =                                \
        settings_chomp_identifier(g_strdup(#val));             \
    val = g_key_file_get_##type                                \
        (VALUE_TO_KEY_KEYFILE,                                 \
         VALUE_TO_KEY_GROUPNAME,                               \
         chomped_value_name,                                   \
         &error);                                              \
    if (error)                                                 \
    {                                                          \
        val = (default_val);                                   \
        g_error_free(error);                                   \
    }                                                          \
    g_free(chomped_value_name);                                \
}


#define KEY_TO_COLOR(color, def_r, def_g, def_b, def_a)        \
{                                                              \
    GError *error = NULL;                                      \
    gchar *chomped_value_name =                                \
        settings_chomp_identifier(g_strdup(#color));           \
    gsize array_size;                                          \
    gdouble *color_array =                                     \
        g_key_file_get_double_list                             \
            (VALUE_TO_KEY_KEYFILE,                             \
             VALUE_TO_KEY_GROUPNAME,                           \
             chomped_value_name,                               \
             &array_size,                                      \
             &error);                                          \
    if (error || array_size != 4)                              \
    {                                                          \
        color = color_new                                      \
            (def_r, def_g, def_b, def_a);                      \
        if (error) { g_error_free(error); }                    \
    }                                                          \
    else                                                       \
    {                                                          \
        color = color_new                                      \
            (color_array[0], color_array[1],                   \
             color_array[2], color_array[3]);                  \
    }                                                          \
    g_free(chomped_value_name);                                \
}


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

static gchar* settings_get_keyfile_path
( gboolean mkdir )
{
	GString *keyfile_path = g_string_new('\0');

	g_string_append(keyfile_path, getenv("HOME"));
	g_string_append(keyfile_path, "/.local/share/TileAttEditor/");
	if (mkdir)
	{
		if (!g_file_test(keyfile_path->str, G_FILE_TEST_IS_DIR))
			{ g_mkdir_with_parents(keyfile_path->str, 0x1ED); }
	}

	g_string_append(keyfile_path, SETTINGS_FILE_NAME);
	gchar *return_string = keyfile_path->str;
	g_string_free(keyfile_path, FALSE);
	return return_string;
}


void settings_write
( struct Settings *settings, struct TileAttribute **tile_attr )
{
	gchar *keyfile_filename = settings_get_keyfile_path(TRUE);
	GKeyFile *keyfile = g_key_file_new();

	VALUE_TO_KEY_INIT(keyfile, "Settings")

	VALUE_TO_KEY(settings->active_attr_id, integer);
	VALUE_TO_KEY(settings->tileset_scale_ratio, double);
	VALUE_TO_KEY(settings->attribute_alpha, double);
	VALUE_TO_KEY(settings->smooth_zoom, boolean);
	VALUE_TO_KEY(settings->workspace_flipped, boolean);

	COLOR_TO_KEY(settings->bg_color);
	COLOR_TO_KEY(settings->grid_color);

	VALUE_TO_KEY(settings->preferred_tile_width, integer);
	VALUE_TO_KEY(settings->preferred_tile_height, integer);
	VALUE_TO_KEY(settings->window_width, integer);
	VALUE_TO_KEY(settings->window_height, integer);
	VALUE_TO_KEY(settings->last_opened, string);

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

	g_free(keyfile_filename);
}

void settings_read
( struct GlobalData *global_data )
{
	gchar *keyfile_filename = settings_get_keyfile_path(FALSE);
	GKeyFile *keyfile = g_key_file_new();

	g_key_file_load_from_file
			(keyfile, keyfile_filename, G_KEY_FILE_NONE, NULL);


	struct Settings *settings =
		g_malloc( sizeof *settings );

	VALUE_TO_KEY_INIT(keyfile, "Settings")

	KEY_TO_VALUE(settings->active_attr_id, integer, 0);
	if (settings->active_attr_id > ATTRIBUTE_COUNT-1 ||
	    settings->active_attr_id < 0                   )
		{ settings->active_attr_id = 0;}

	KEY_TO_VALUE(settings->tileset_scale_ratio, double, 1.5);
	if (settings->tileset_scale_ratio < 0.1)
		{ settings->tileset_scale_ratio = 1.5; }

	KEY_TO_VALUE(settings->attribute_alpha, double, 0.8);
	if (settings->attribute_alpha < 0 ||
	    settings->attribute_alpha > 1)
		{ settings->attribute_alpha = 0.8; }

	KEY_TO_VALUE(settings->smooth_zoom, boolean, FALSE);
	KEY_TO_VALUE(settings->workspace_flipped, boolean, FALSE);

	KEY_TO_COLOR(settings->bg_color, 0.04, 0.65, 0.72, 1);
	settings->bg_color->a = 1;

	KEY_TO_COLOR(settings->grid_color, 0, 0, 0, 0.6);

	KEY_TO_VALUE(settings->preferred_tile_width, integer, 32);
	if (settings->preferred_tile_width < 1)
		{ settings->preferred_tile_width = 32; }

	KEY_TO_VALUE(settings->preferred_tile_height, integer, 32);
	if (settings->preferred_tile_height < 1)
		{ settings->preferred_tile_height = 32; }

	KEY_TO_VALUE(settings->window_width, integer, 128);
	KEY_TO_VALUE(settings->window_height, integer, 128);

	KEY_TO_VALUE(settings->last_opened, string, g_strdup(""));

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

	g_free(keyfile_filename);
}

void settings_destroy
( struct Settings *settings )
{
	color_destroy(settings->bg_color);
	color_destroy(settings->grid_color);

	if ((*settings->last_opened) != '\0')
		{ g_free(settings->last_opened); }

	g_free(settings);
}

