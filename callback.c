
#include <gtk/gtk.h>

#include "tileatteditor.h"
#include "callback.h"
#include "dialog.h"
#include "file.h"
#include "util.h"

#define CAST_BUTTON_EVENT GdkEventButton *b_event = (GdkEventButton*)


static void attr_button_box_set_expand
( struct GlobalData *global_data, gboolean expand )
{
	struct TileAttribute **tile_attr;
	for (tile_attr=global_data->tile_attributes;
	     *tile_attr; tile_attr++)
	{
		gtk_box_set_child_packing
			(GTK_BOX(global_data->main_window->attr_button_box),
			 (*tile_attr)->button, expand, expand, 8, GTK_PACK_START);
	}
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

void cb_filemenu_new
( GtkAction *action, gpointer data )
{
	CAST_GLOBAL_DATA
	cb_filemenu_close(NULL, global_data);
	new_file_dialog_run(global_data);
	global_data->buffer_changed = TRUE;
}

void cb_filemenu_open
( GtkAction *action, gpointer data )
{
	CAST_GLOBAL_DATA

	cb_filemenu_close(NULL, global_data);

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
			g_strdup(gtk_file_chooser_get_filename
				(GTK_FILE_CHOOSER(file_chooser)));
		gtk_widget_destroy(file_chooser);
		struct File *file = file_open(filename, NULL);
		if (!file)
		{
			show_error_message
				(global_data->main_window->window,
				 "The selected file could not be loaded");
			return;
		}
		else
		{
			if (!file_parse(global_data, file, NULL))
			{
				show_error_message
					(global_data->main_window->window,
					 "The selected file could not be parsed");
				return;
			}
		}

		global_data->open_file_path = filename;
		global_data->buffer_changed = FALSE;
		tileset_area_redraw_cache(global_data);
		gtk_widget_queue_draw
			(global_data->main_window->tileset_area);
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

	if (!global_data->open_file) {g_message("no filebuffer. aborting..."); return; }

	if (!global_data->buffer_changed) {g_message("no changes to save. aborting.."); return; }

	if (!global_data->open_file_path)
	{g_message("no filename specified. asking..");
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
			global_data->open_file_path = g_strdup
				(gtk_file_chooser_get_filename
					(GTK_FILE_CHOOSER(file_chooser)));
			gtk_widget_destroy(file_chooser);g_message("got filename: %s", global_data->open_file_path);
		}
		else
		{
			gtk_widget_destroy(file_chooser);g_message("no filename chosen. aborting...");
			return;
		}
	}

	gboolean suc = file_save(global_data, global_data->open_file_path);
	global_data->buffer_changed = FALSE;
	if (suc) {g_message("Successfully saved");} else {g_message("saving error.");}
}

void cb_filemenu_close
( GtkAction *action, gpointer data )
{
	CAST_GLOBAL_DATA
	if (!global_data->open_file) { return; }

	file_close(global_data);
	if (global_data->open_file_path) /* TODO: replace this with "unsaved data" dialog */
	{
		g_free(global_data->open_file_path);
		global_data->open_file_path = NULL;
	}
	gtk_widget_queue_draw
		(global_data->main_window->tileset_area);
}

void cb_filemenu_quit
( GtkAction *action, gpointer data )
{
	gtk_main_quit();
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

	if (gtk_toggle_action_get_active(GTK_TOGGLE_ACTION(action)))
	{
		gtk_orientable_set_orientation
			(GTK_ORIENTABLE(global_data->main_window->workspace_box),
			 GTK_ORIENTATION_VERTICAL);
		gtk_orientable_set_orientation
			(GTK_ORIENTABLE(global_data->main_window->workspace_separator),
			 GTK_ORIENTATION_HORIZONTAL);
		gtk_orientable_set_orientation
			(GTK_ORIENTABLE(global_data->main_window->attr_button_box),
			 GTK_ORIENTATION_HORIZONTAL);
		attr_button_box_set_expand(global_data, TRUE);
		global_data->settings->workspace_flipped = TRUE;
	}
	else
	{
		gtk_orientable_set_orientation
			(GTK_ORIENTABLE(global_data->main_window->workspace_box),
			 GTK_ORIENTATION_HORIZONTAL);
		gtk_orientable_set_orientation
			(GTK_ORIENTABLE(global_data->main_window->workspace_separator),
			 GTK_ORIENTATION_VERTICAL);
		gtk_orientable_set_orientation
			(GTK_ORIENTABLE(global_data->main_window->attr_button_box),
			 GTK_ORIENTATION_VERTICAL);
			 attr_button_box_set_expand(global_data, FALSE);
			 global_data->settings->workspace_flipped = FALSE;
	}

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
	//struct Tileset *tileset =
		//(struct Tileset*)data;

	CAST_GLOBAL_DATA

	struct Tileset *tileset = global_data->tileset;

//	if (!tileset) { return FALSE; }

//	if (!tileset->cached_composition) {return FALSE;}

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
	}

	cairo_t *cr = gdk_cairo_create(widget->window);
	gdk_cairo_region(cr, event->region);
	cairo_clip(cr);
	if (tileset)
	{
		if (tileset->cached_composition)
		{
			cairo_set_source_surface
				(cr, tileset->cached_composition, 0, 0);
		}
	}
	else
	{
		cairo_set_source_rgba(cr, 0, 0, 0, 0);
		cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	}
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
	if (global_data->hover_message_id)
	{
		gtk_statusbar_remove
			(GTK_STATUSBAR( global_data->main_window->statusbar ),
			 global_data->hover_context_id,
			 global_data->hover_message_id);
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

	global_data->hover_message_id =
		gtk_statusbar_push
			(GTK_STATUSBAR(global_data->main_window->statusbar),
			 global_data->hover_context_id,
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
	global_data->buffer_changed = TRUE;
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

	if (global_data->hover_message_id)
	{
		gtk_statusbar_remove
			(GTK_STATUSBAR( global_data->main_window->statusbar ),
			 global_data->hover_context_id,
			 global_data->hover_message_id);
		global_data->hover_message_id = 0;
	}

	if (!global_data->active_attribute) {return FALSE;}

	tileset_area_redraw_cache_tile
		(global_data, hovered_tile_old);
	tileset_area_queue_tile_redraw
		(global_data, 1, hovered_tile_old);

	return FALSE;
}






