/*
 * callback.c
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
#include <gdk/gdkkeysyms.h>

#include "tileatteditor.h"
#include "tileset-area.h"
#include "callback.h"
#include "dialog.h"
#include "file.h"
#include "util.h"

#define CAST_BUTTON_EVENT GdkEventButton *b_event = (GdkEventButton*)


/* private functions */
static void statusbar_update_message
( struct GlobalData *global_data, const gchar *message );

static void show_error_message
( GtkWidget *parent, const gchar *title, const gchar *message );

static void show_file_open_error_message
( GtkWidget *parent, enum ErrorFileOpen error );

static void show_file_parse_error_message
( GtkWidget *parent, enum ErrorFileParse error );

static void file_open_attempt
( struct GlobalData *global_data, const gchar *filename );

static gboolean file_save_attempt
( struct GlobalData *global_data );

static gboolean save_changes
( struct GlobalData *global_data );

static gint tileset_area_determine_tile_id
( struct Tileset *tileset, gdouble pointer_x, gdouble pointer_y,
  gdouble *tile_offset_x, gdouble *tile_offset_y                 );

static void tileset_area_update_statusbar_hover
( struct GlobalData *global_data );

static gboolean tileset_are_update_pointer_status
( struct GlobalData *global_data,
  gdouble pointer_x, gdouble pointer_y );

static void tile_attr_toggle_buffer_value
( struct GlobalData *global_data, gint *copy_value );

static void tile_attr_set_buffer_value
( struct GlobalData *global_data, gint value );
/* ----------------- */


static void statusbar_update_message
( struct GlobalData *global_data, const gchar *message )
{
	if (global_data->statusbar_message_id)
	{
		gtk_statusbar_remove
			(GTK_STATUSBAR( global_data->main_window->statusbar ),
			 global_data->statusbar_context_id,
			 global_data->statusbar_message_id);
	}

	global_data->statusbar_message_id =
		gtk_statusbar_push
			(GTK_STATUSBAR(global_data->main_window->statusbar),
			 global_data->statusbar_context_id, message);
}

static void show_error_message
( GtkWidget *parent, const gchar *title, const gchar *message )
{
	GtkWidget *error_dialog =
		gtk_message_dialog_new_with_markup
			(GTK_WINDOW(parent),
			 0, GTK_MESSAGE_ERROR,
			 GTK_BUTTONS_CLOSE,
			 "<b>%s</b>", title);
	gtk_message_dialog_format_secondary_text
		(GTK_MESSAGE_DIALOG(error_dialog), message);
	gtk_dialog_run(GTK_DIALOG(error_dialog));
	gtk_widget_destroy(error_dialog);
}

static void show_file_open_error_message
( GtkWidget *parent, enum ErrorFileOpen error )
{
	static gchar *msg;

	switch (error)
	{
		case NONEXISTANT_FILE:
			msg = "The selected file does not exist.";
			break;
		case DOCUMENT_MALFORMED:
			msg = "The selected file is malformed.";
			break;
		case DOCUMENT_EMPTY:
			msg = "The selected file is empty.";
			break;
		case NOT_TILESET_FILE:
			msg = "The selected file is not a tileset xml file.";
			break;
		case NO_TILE_SIZE_PROPS:
			msg = "The selected file does not contain tile size data.";
			break;
		case NO_IMAGE_NODE:
		case NO_IMAGE_SOURCE:
			msg = "The tileset image file path could not\n"
			      "be found inside selected file.";
			break;

		default:
			msg = "The selected file could not be loaded.";
	}

	show_error_message(parent, "File Loading Error", msg);
}

static void show_file_parse_error_message
( GtkWidget *parent, enum ErrorFileParse error )
{
	const gchar *msg;

	switch (error)
	{
		case BAD_TILE_SIZES:
			msg = "The containing tile size data is invalid.";
			break;
		case BAD_IMAGE_FILE:
			msg = "The tileset image file could not be loaded.";
			break;

		default:
			msg = "The selected file could not be loaded.";
	}

	show_error_message(parent, "File Parsing Error", msg);
}

/* should always be used after 'save_changes' */
static void file_open_attempt
( struct GlobalData *global_data, const gchar *filename )
{
	gchar *_filename = g_strdup(filename);
	enum ErrorFileOpen error_open;
	enum ErrorFileParse error_parse;

	struct File *file = file_open(_filename, &error_open);
	if (!file)
	{
		show_file_open_error_message
			(global_data->main_window->window, error_open);
		g_free(_filename);
		return;
	}
	else
	{
		if (!file_check(global_data, file, &error_parse))
		{
			file_destroy(file);
			show_file_parse_error_message
				(global_data->main_window->window, error_parse);
			g_free(_filename);
			return;
		}
	}

	file_close(global_data);
	if (global_data->open_file_path)
	{
		g_free(global_data->open_file_path);
	}
	file_parse(global_data, file);
	global_data->open_file_path = _filename;
	ui_filemenu_set_action_sensitivity
		(global_data->main_window, TRUE);
	ui_update_tileset_frame(global_data);
	ui_set_buffer_changed(global_data, file->image_path_found);
	tileset_area_update_viewport(global_data);
	tileset_area_redraw_cache(global_data);
	gtk_widget_show(global_data->main_window->tileset_area);
	gtk_widget_queue_draw
		(global_data->main_window->tileset_area);
	statusbar_update_message(global_data, "File opened.");
}

/* returns FALSE if unsaved data remains */
static gboolean file_save_attempt
( struct GlobalData *global_data )
{
//	if (!global_data->open_file) { return TRUE; }

	if (!global_data->buffer_changed)
	{
		statusbar_update_message(global_data, "No changes to save.");
		return TRUE;
	}

	if (!global_data->open_file_path)
	{
		GtkWidget *file_chooser =
			gtk_file_chooser_dialog_new
				("Specify File Name",
				 GTK_WINDOW(global_data->main_window->window),
				 GTK_FILE_CHOOSER_ACTION_SAVE,
				 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				 GTK_STOCK_SAVE,   GTK_RESPONSE_ACCEPT,
				 NULL);
		gtk_file_chooser_set_do_overwrite_confirmation
			(GTK_FILE_CHOOSER(file_chooser), TRUE);
		gtk_file_chooser_set_create_folders
			(GTK_FILE_CHOOSER(file_chooser), TRUE);
		gtk_file_chooser_set_current_name
			(GTK_FILE_CHOOSER(file_chooser), "untitled.tsx");

		if (gtk_dialog_run(GTK_DIALOG(file_chooser))
		    == GTK_RESPONSE_ACCEPT)
		{
			ui_set_open_file_path
				(global_data,
				 gtk_file_chooser_get_filename
					(GTK_FILE_CHOOSER(file_chooser)));
			gtk_widget_destroy(file_chooser);
		}
		else
		{
			gtk_widget_destroy(file_chooser);
			return FALSE;
		}
	}

	file_save(global_data, global_data->open_file_path);
	ui_set_buffer_changed(global_data, FALSE);
	statusbar_update_message(global_data, "File saved.");
	return TRUE;
}

/* returns FALSE if current data shall not be lost */
static gboolean save_changes
( struct GlobalData *global_data )
{
	if (!global_data->buffer_changed) { return TRUE; }

	GtkWidget *dialog =
		save_changes_dialog_new(global_data->main_window->window);
	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	gtk_widget_destroy(dialog);

	switch (response)
	{
		case GTK_RESPONSE_ACCEPT : return file_save_attempt(global_data);

		case GTK_RESPONSE_REJECT : return TRUE;

		case GTK_RESPONSE_CANCEL       :
		case GTK_RESPONSE_DELETE_EVENT : return FALSE;
	}

	return FALSE;
}


void cb_filemenu_new
( GtkAction *action, gpointer data )
{
	CAST_GLOBAL_DATA

	if (!save_changes(global_data)) { return; }
	new_file_dialog_run(global_data, NULL);
}

void cb_filemenu_open
( GtkAction *action, gpointer data )
{
	CAST_GLOBAL_DATA

	if (!save_changes(global_data)) { return; }

	GtkWidget *file_chooser =
		gtk_file_chooser_dialog_new
			("Open File",
			 GTK_WINDOW(global_data->main_window->window),
			 GTK_FILE_CHOOSER_ACTION_OPEN,
			 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			 GTK_STOCK_OPEN,   GTK_RESPONSE_ACCEPT,
			 NULL);
	GtkFileFilter *filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "*.tsx files");
	gtk_file_filter_add_pattern(filter, "*.tsx");
	gtk_file_chooser_add_filter
		(GTK_FILE_CHOOSER(file_chooser), filter);
	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "All");
	gtk_file_filter_add_pattern(filter, "*");
	gtk_file_chooser_add_filter
		(GTK_FILE_CHOOSER(file_chooser), filter);


	if (gtk_dialog_run(GTK_DIALOG(file_chooser))
	     == GTK_RESPONSE_ACCEPT)
	{
		gchar *filename =
			gtk_file_chooser_get_filename
				(GTK_FILE_CHOOSER(file_chooser));
		gtk_widget_destroy(file_chooser);
		file_open_attempt(global_data, filename);
	}
	else
	{
		gtk_widget_destroy(file_chooser);
	}
}

void cb_filemenu_save
( GtkAction *action, gpointer data )
{
	CAST_GLOBAL_DATA

	if (!global_data->open_file)
	{
		statusbar_update_message(global_data, "No file to save.");
		return;
	}

	file_save_attempt(global_data);
}

void cb_filemenu_close
( GtkAction *action, gpointer data )
{
	CAST_GLOBAL_DATA
	if (!global_data->open_file) { return; }

	if (!save_changes(global_data)) { return; }

	file_close(global_data);
	if (global_data->open_file_path)
	{
		g_free(global_data->open_file_path);
		global_data->open_file_path = NULL;
	}

	ui_filemenu_set_action_sensitivity
		(global_data->main_window, FALSE);
	ui_update_tileset_frame(global_data);
	ui_set_buffer_changed(global_data, FALSE);
	tileset_area_update_viewport(global_data);
	gtk_widget_hide(global_data->main_window->tileset_area);
	statusbar_update_message(global_data, "File closed.");
}

void cb_filemenu_quit
( GtkAction *action, gpointer data )
{
	CAST_GLOBAL_DATA
	cb_window_delete(NULL, NULL, global_data);
}

void cb_editmenu_preferences
( GtkAction *action, gpointer data )
{
	CAST_GLOBAL_DATA
	if (!global_data->settings_dialog)
		{ settings_dialog_run(global_data); }
}

void cb_editmenu_flip
( GtkAction *action, gpointer data )
{
	CAST_GLOBAL_DATA

	GtkOrientation workspace_orient;
	GtkOrientation children_orent;
	gboolean state;

	if (gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(action)))
	{
		workspace_orient = GTK_ORIENTATION_VERTICAL;
		children_orent = GTK_ORIENTATION_HORIZONTAL;
		state = TRUE;
	}
	else
	{
		workspace_orient = GTK_ORIENTATION_HORIZONTAL;
		children_orent = GTK_ORIENTATION_VERTICAL;
		state = FALSE;
	}

	ui_workspace_box_flip_packing
		(global_data->main_window->workspace_box);
//	ui_attr_button_set_show_label(global_data, !state);

	gtk_orientable_set_orientation
		(GTK_ORIENTABLE(global_data->main_window->workspace_box),
		 workspace_orient);
	gtk_orientable_set_orientation
		(GTK_ORIENTABLE(global_data->main_window->workspace_separator),
		 children_orent);
	gtk_orientable_set_orientation
		(GTK_ORIENTABLE(global_data->main_window->attr_button_box),
		 children_orent);
	ui_attr_button_box_set_expand(global_data, state);
	global_data->settings->workspace_flipped = state;


	tileset_area_update_viewport(global_data);

}


gboolean cb_window_delete
( GtkWidget *widget, GdkEvent *event, gpointer data )
{
	CAST_GLOBAL_DATA
	if (!save_changes(global_data)) { return TRUE; }
	gtk_main_quit();
	return FALSE;
}

gboolean cb_window_configure
( GtkWidget *widget, GdkEventConfigure *event, gpointer data )
{
	CAST_GLOBAL_DATA

	gtk_window_get_size
		(GTK_WINDOW(widget),
		 &global_data->settings->window_width,
		 &global_data->settings->window_height);
	return FALSE;
}

gboolean cb_window_key_press
( GtkWidget *widget, GdkEventKey *kevent, gpointer data )
{
	CAST_GLOBAL_DATA

	guint keyval = kevent->keyval - GDK_KEY_0;
	if (keyval < 0 || keyval > 9 || keyval > ATTRIBUTE_COUNT)
		{ return FALSE; }

	if (keyval == 0)
		{ activate_zero_attribute(global_data); return FALSE; }

	/* collect all enabled attributes and map to keypress */
	struct TileAttribute *enabled_attr[ATTRIBUTE_COUNT];
	gint i, j = 0;
	for (i=0; i<ATTRIBUTE_COUNT; i++)
	{
		if (global_data->tile_attributes[i]->enabled)
		{
			enabled_attr[j] = global_data->tile_attributes[i];
			j++;
		}
	}

	if (keyval > j) { return FALSE; }

	struct TileAttribute *selected_attr = enabled_attr[keyval-1];

	if (selected_attr->enabled)
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON(selected_attr->button), TRUE);
	}

	return TRUE;
}


void cb_attr_icon_expose
( GtkWidget *icon, GdkEventExpose *event, gpointer data )
{
	struct TileAttribute *tile_attr =
		(struct TileAttribute*)data;

	cairo_t *cr = gdk_cairo_create(icon->window);

	cairo_save(cr);
	cairo_rectangle(cr, 0.5, 0.5, 31, 31);
	cairo_set_line_width(cr, 2);
	cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
	cairo_set_source_rgb(cr, 0, 0, 0);
	cairo_stroke(cr);
	cairo_restore(cr);

	cairo_scale(cr, 32, 32);
	(*tile_attr->draw_attr)(tile_attr->icon_value, cr, FALSE, 0, 0);
	cairo_destroy(cr);
}



void cb_attr_button_toggled
( GtkWidget *button, gpointer data )
{
	struct TileAttribute *tile_attr =
		(struct TileAttribute*)data;
	CAST_GLOBAL_DATA_PTR(tile_attr->global_data)

	if (tile_attr != global_data->active_attribute)
	{
		/* make sure "active_attribute" isn't acutally 0 */
		if (global_data->active_attribute)
		{
			toggle_button_disable_signal(global_data->active_attribute);
			gtk_toggle_button_set_active
				(GTK_TOGGLE_BUTTON(global_data->active_attribute->button),
				 FALSE);
			toggle_button_enable_signal(global_data->active_attribute);
			global_data->settings->active_attr_id =
				tile_attr_find_id(tile_attr);
		}
		global_data->active_attribute = tile_attr;
	}
	else
	{
		/* already active button was toggled, so it has to be toggled back in */
		toggle_button_disable_signal(tile_attr);
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
		toggle_button_enable_signal(tile_attr);
	}

	tileset_area_redraw_cache(global_data);
	gtk_widget_queue_draw(global_data->main_window->tileset_area);
}


gboolean cb_tileset_area_expose
( GtkWidget *widget, GdkEventExpose *event, gpointer data )
{
	CAST_GLOBAL_DATA

	struct Tileset *tileset = global_data->tileset;

	if (tileset)
	{
		if (widget->allocation.width != tileset->disp_width ||
			widget->allocation.height != tileset->disp_height)
		{
			gtk_widget_set_size_request(widget,
				tileset->disp_width, tileset->disp_height);
		}
	}
	else
	{
		if (widget->allocation.width != 128 ||
			widget->allocation.height != 256)
		{
			gtk_widget_set_size_request(widget, 128, 256);
		}
		return FALSE;
	}

	cairo_t *cr = gdk_cairo_create(widget->window);
	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	gdk_cairo_region(cr, event->region);
	cairo_clip(cr);
	cairo_set_source_surface
		(cr, tileset->cached_composition, 0, 0);
	cairo_paint(cr);
	cairo_destroy(cr);

	return FALSE;
}

static gint tileset_area_determine_tile_id
( struct Tileset *tileset, gdouble pointer_x, gdouble pointer_y,
  gdouble *tile_offset_x, gdouble *tile_offset_y                 )
{
	if (pointer_x < 0 || pointer_y < 0     ||
	    pointer_x > tileset->disp_width -1 ||
	    pointer_y > tileset->disp_height-1    ) { return -1; }

	gint tile_id,
	     tile_count_x = tileset->width/tileset->tile_width,
	     tile_x = (gint)(pointer_x / tileset->tile_disp_width),
	     tile_y = (gint)(pointer_y / tileset->tile_disp_height);

	if (!(tile_offset_x && tile_offset_y)) { goto skipped_offsets; }
	*tile_offset_x =
		(pointer_x / tileset->tile_disp_width) - tile_x;
	*tile_offset_y =
		(pointer_y / tileset->tile_disp_height) - tile_y;

skipped_offsets:
	 tile_id = tile_y * tile_count_x + tile_x;

	return (tile_id < 0 || tile_id > tileset->tile_count - 1)
	? -1 : tile_id;
}

static void tileset_area_update_statusbar_hover
( struct GlobalData *global_data )
{
	if (global_data->statusbar_message_id)
	{
		gtk_statusbar_remove
			(GTK_STATUSBAR( global_data->main_window->statusbar ),
			 global_data->statusbar_context_id,
			 global_data->statusbar_message_id);
	}

	GString *buffer = g_string_new('\0');
	if (global_data->active_attribute)
	{
		g_string_printf(buffer,
			"Tile: %d, Value: %d", global_data->hovered_tile+1,
			global_data->active_attribute->value_buffer
				[global_data->hovered_tile]);
	}
	else
	{
		g_string_printf(buffer,
			"Tile: %d", global_data->hovered_tile+1);
	}

	global_data->statusbar_message_id =
		gtk_statusbar_push
			(GTK_STATUSBAR(global_data->main_window->statusbar),
			 global_data->statusbar_context_id,
			 buffer->str);

	g_string_free(buffer, TRUE);
}

static gboolean tileset_are_update_pointer_status
( struct GlobalData *global_data,
  gdouble pointer_x, gdouble pointer_y )
{
	gint hovered_tile =
		tileset_area_determine_tile_id
			(global_data->tileset, pointer_x, pointer_y,
			 &global_data->hovered_offset_x,
			 &global_data->hovered_offset_y);

	if (hovered_tile == -1)
		{ return FALSE; }

	global_data->hovered_tile = hovered_tile;
	return TRUE;
}

static void tile_attr_toggle_buffer_value
( struct GlobalData *global_data, gint *copy_value )
{
	gint tile_id = global_data->hovered_tile;
	struct TileAttribute *active_attr =
		global_data->active_attribute;

	gint new_value =
		(*active_attr->tile_clicked)
			(active_attr->value_buffer[tile_id],
			 global_data->hovered_offset_x,
			 global_data->hovered_offset_y);

	active_attr->value_buffer[tile_id] = new_value;

	if (copy_value) { *copy_value = new_value; }
}

static void tile_attr_set_buffer_value
( struct GlobalData *global_data, gint value )
{
	gint tile_id = global_data->hovered_tile;
	struct TileAttribute *active_attr =
		global_data->active_attribute;

	active_attr->value_buffer[tile_id] = value;
}


gboolean cb_tileset_area_button_press
( GtkWidget *widget, GdkEventButton *b_event, gpointer data )
{
	CAST_GLOBAL_DATA

	if (b_event->type == GDK_2BUTTON_PRESS ||
	    b_event->type == GDK_3BUTTON_PRESS ||
	    !global_data->active_attribute)          { return FALSE; }

	struct Tileset *tileset = global_data->tileset;

	if (!tileset) { return FALSE; }

	gboolean pointer_over_tileset =
		tileset_are_update_pointer_status
			(global_data, b_event->x, b_event->y);

	if (!pointer_over_tileset) { return FALSE; }

	tile_attr_toggle_buffer_value
		(global_data, &global_data->swipe_value);

	tileset_area_update_statusbar_hover(global_data);
	ui_set_buffer_changed(global_data, TRUE);
	tileset_area_redraw_cache_tile
		(global_data, global_data->hovered_tile);
	tileset_area_queue_tile_redraw
		(global_data, 1, global_data->hovered_tile);

	return FALSE;
}

gboolean cb_tileset_area_motion_notify
( GtkWidget *widget, GdkEventMotion *m_event, gpointer data )
{
	CAST_GLOBAL_DATA

	struct Tileset *tileset = global_data->tileset;

	if (!tileset) { return FALSE; }

	if (m_event->type == GDK_SCROLL)
	{
		gdk_window_get_pointer
			(gtk_widget_get_window
				(global_data->main_window->tileset_area),
			 (gint*)&m_event->x, (gint*)&m_event->y, NULL);
	}

	gint hovered_tile_old = global_data->hovered_tile;

	if (!tileset_are_update_pointer_status
		     (global_data, m_event->x, m_event->y)) { return FALSE; }

	tileset_area_update_statusbar_hover(global_data);

	struct TileAttribute *active_attr =
				global_data->active_attribute;

	if (!active_attr) { return FALSE; }

	gboolean hover_precision = active_attr->hover_precision;

	if (hovered_tile_old != global_data->hovered_tile)
	{
		// Mouse pointer entered new tile
		// If mouse button is pressed, execute swipe

		// Left mouse button: set-swipe
		if      (m_event->state & GDK_BUTTON1_MASK)
		{
			tile_attr_set_buffer_value
				(global_data, global_data->swipe_value);
		}
		// Right mouse button: toggle-swipe
		else if (m_event->state & GDK_BUTTON3_MASK)
		{
			if (hover_precision) { goto skip_toggle_swipe; }

			tile_attr_toggle_buffer_value(global_data, NULL);
		}
	skip_toggle_swipe:

		tileset_area_redraw_cache_tile
			(global_data, hovered_tile_old);
		tileset_area_redraw_cache_tile
			(global_data, global_data->hovered_tile);
		tileset_area_queue_tile_redraw
			(global_data, 2, hovered_tile_old,
			 global_data->hovered_tile);
	}
	else if (hover_precision)
	{
		// TileAttr requires in-tile precision: redraw on each event
		tileset_area_redraw_cache_tile
			(global_data, global_data->hovered_tile);
		tileset_area_queue_tile_redraw
			(global_data, 1, global_data->hovered_tile);
	}

	return FALSE;
}

gboolean cb_tileset_area_leave_notify
( GtkWidget *widget, GdkEventButton *b_event, gpointer data )
{
	CAST_GLOBAL_DATA

	if (!global_data->tileset) { return FALSE; }

	gint hovered_tile_old = global_data->hovered_tile;
	global_data->hovered_tile = -1;

	if (global_data->statusbar_message_id)
	{
		gtk_statusbar_remove
			(GTK_STATUSBAR( global_data->main_window->statusbar ),
			 global_data->statusbar_context_id,
			 global_data->statusbar_message_id);
		global_data->statusbar_message_id = 0;
	}

	if (!global_data->active_attribute) {return FALSE;}

	tileset_area_redraw_cache_tile
		(global_data, hovered_tile_old);
	tileset_area_queue_tile_redraw
		(global_data, 1, hovered_tile_old);

	return FALSE;
}

gboolean cb_tileset_area_drag_data_received
( GtkWidget *widget, GdkDragContext *context,
  gint x, gint y, GtkSelectionData *sdata,
  guint info, guint timestamp, gpointer data)
{
	CAST_GLOBAL_DATA

	if (!sdata->data) { return FALSE; }

	gchar **uris = g_uri_list_extract_uris((gchar*)sdata->data);
	gchar *path = g_filename_from_uri(uris[0], NULL, NULL);
	g_strfreev(uris);

	if (!path) { return FALSE; }

	if (g_str_has_suffix(path, ".png"))
	{
		if (!save_changes(global_data)) { goto clean_up; }
		new_file_dialog_run(global_data, path);
	}
	else
	{
		if (!save_changes(global_data)) { goto clean_up; }
		file_open_attempt(global_data, path);
		goto clean_up;
	}

clean_up:
	g_free(path);

	return FALSE;
}

