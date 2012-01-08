
#include <gtk/gtk.h>

#include "tileatteditor.h"
#include "tileset-area.h"
#include "callback.h"
#include "dialog.h"
#include "file.h"
#include "util.h"

#define CAST_BUTTON_EVENT GdkEventButton *b_event = (GdkEventButton*)


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
( GtkWidget *parent, gchar *message )
{
	GtkWidget *error_dialog =
		gtk_message_dialog_new_with_markup
			(GTK_WINDOW(parent),
			 0, GTK_MESSAGE_ERROR,
			 GTK_BUTTONS_CLOSE,
			 "<b>File Loading Error</b>");
	gtk_message_dialog_format_secondary_text
		(GTK_MESSAGE_DIALOG(error_dialog), message);
	gtk_dialog_run(GTK_DIALOG(error_dialog));
	gtk_widget_destroy(error_dialog);
}

/* should always be used after 'save_changes' */
static void file_open_attempt
( struct GlobalData *global_data, const gchar *filename)
{
	gchar *_filename = g_strdup(filename);

	struct File *file = file_open(_filename, NULL);
	if (!file)
	{
		show_error_message
			(global_data->main_window->window,
			 "The selected file could not be loaded");
		g_free(_filename);
		return;
	}
	else
	{
		if (!file_check(global_data, file, NULL))
		{
			file_destroy(file);
			show_error_message
				(global_data->main_window->window,
				 "The selected file could not be parsed");
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
	ui_update_tileset_frame(global_data);
	ui_set_buffer_changed(global_data, FALSE);
	tileset_area_update_viewport(global_data);
	tileset_area_redraw_cache(global_data);
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

	ui_update_tileset_frame(global_data);
	ui_set_buffer_changed(global_data, FALSE);
	tileset_area_update_viewport(global_data);
	gtk_widget_queue_draw
		(global_data->main_window->tileset_area);
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

	workspace_box_flip_packing
		(global_data->main_window->workspace_box);

	gtk_orientable_set_orientation
		(GTK_ORIENTABLE(global_data->main_window->workspace_box),
		 workspace_orient);
	gtk_orientable_set_orientation
		(GTK_ORIENTABLE(global_data->main_window->workspace_separator),
		 children_orent);
	gtk_orientable_set_orientation
		(GTK_ORIENTABLE(global_data->main_window->attr_button_box),
		 children_orent);
	attr_button_box_set_expand(global_data, state);
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
		return;
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
( struct Tileset *tileset, GdkEventButton *b_event,
  gdouble *tile_offset_x, gdouble *tile_offset_y )
{
	gint tile_count_x = tileset->width/tileset->tile_width,
	     tile_x = (gint)(b_event->x / tileset->tile_disp_width),
	     tile_y = (gint)(b_event->y / tileset->tile_disp_height);

	if (!(tile_offset_x && tile_offset_y)) { goto skipped_offsets; }
	*tile_offset_x =
		(b_event->x / tileset->tile_disp_width) - tile_x;
	*tile_offset_y =
		(b_event->y / tileset->tile_disp_height) - tile_y;

	skipped_offsets: return tile_y * tile_count_x + tile_x;
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


gboolean cb_tileset_area_button_press
( GtkWidget *widget, GdkEventButton *b_event, gpointer data )
{
	CAST_GLOBAL_DATA

	if (b_event->type == GDK_2BUTTON_PRESS ||
	    b_event->type == GDK_3BUTTON_PRESS ||
	    !global_data->active_attribute)          {return FALSE;}

	struct Tileset *tileset = global_data->tileset;
	struct TileAttribute *active_attr = global_data->active_attribute;

	if (!tileset) { return FALSE; }

	gdouble tile_offset_x, tile_offset_y;
	gint tile_id = tileset_area_determine_tile_id
		(tileset, b_event, &tile_offset_x, &tile_offset_y);

	active_attr->value_buffer[tile_id] =
		(*active_attr->tile_clicked)
			(active_attr->value_buffer[tile_id],
			 tile_offset_x, tile_offset_y);

	tileset_area_update_statusbar_hover(global_data);
	ui_set_buffer_changed(global_data, TRUE);
	tileset_area_redraw_cache_tile(global_data, tile_id);
	tileset_area_queue_tile_redraw(global_data, 1, tile_id);

	return FALSE;
}

gboolean cb_tileset_area_motion_notify
( GtkWidget *widget, GdkEvent *b_event, gpointer data )
{
	CAST_GLOBAL_DATA

	struct Tileset *tileset = global_data->tileset;

	if (!tileset) { return FALSE; }

	if (b_event->type == GDK_SCROLL)
	{
		gdk_window_get_pointer
			(gtk_widget_get_window
				(global_data->main_window->tileset_area),
			 (gint*)&b_event->button.x, (gint*)&b_event->button.y, NULL);
	}

	gint hovered_tile_old = global_data->hovered_tile;
	global_data->hovered_tile = tileset_area_determine_tile_id
		(tileset, (GdkEventButton*)b_event,
		&global_data->hovered_offset_x,
		&global_data->hovered_offset_y);

	tileset_area_update_statusbar_hover(global_data);

	if (!global_data->active_attribute) {return FALSE;}

	if (hovered_tile_old != global_data->hovered_tile)
	{
		tileset_area_redraw_cache_tile
			(global_data, hovered_tile_old);
		tileset_area_redraw_cache_tile
			(global_data, global_data->hovered_tile);
		tileset_area_queue_tile_redraw
			(global_data, 2, hovered_tile_old,
			 global_data->hovered_tile);
	}
	else if (global_data->active_attribute->hover_precision)
	{
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

	if (!sdata->data) { return; }

	gchar **uris = g_uri_list_extract_uris(sdata->data);
	gchar *path = g_filename_from_uri(uris[0], NULL, NULL);
	g_strfreev(uris);

	if (!path) { return; }

	if (g_str_has_suffix(path, ".tsx"))
	{
		if (!save_changes(global_data)) { goto clean_up; }
		file_open_attempt(global_data, path);
		goto clean_up;
	}

	if (g_str_has_suffix(path, ".png"))
	{
		if (!save_changes(global_data)) { goto clean_up; }
		new_file_dialog_run(global_data, path);
	}

clean_up:
	g_free(path);
}
