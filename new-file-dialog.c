
#include <gtk/gtk.h>

#include "tileatteditor.h"
#include "tileset-area.h"
#include "callback.h"
#include "dialog.h"
#include "file.h"
#include "util.h"

struct NewFileDialog
{
	GtkWidget *window;

	GtkWidget *file_button;
	GtkWidget *spinb_tilewidth;
	GtkWidget *spinb_tileheight;
};

static void cb_button_cancel_clicked
( GtkWidget *button, gpointer data )
{
	CAST_GLOBAL_DATA
	struct NewFileDialog *dialog =
		global_data->new_file_dialog;

	gtk_widget_destroy(dialog->window);
	g_free(dialog);
}

static void cb_button_ok_clicked
( GtkWidget *button, gpointer data )
{
	CAST_GLOBAL_DATA
	struct NewFileDialog *dialog =
		global_data->new_file_dialog;

	gchar *error_message   = 0,
	      *warning_message = 0;

	gint remainder;

	gchar *filename = gtk_file_chooser_get_filename
		(GTK_FILE_CHOOSER(dialog->file_button));

	gint tile_w = gtk_spin_button_get_value_as_int
			(GTK_SPIN_BUTTON(dialog->spinb_tilewidth)),

	     tile_h = gtk_spin_button_get_value_as_int
			(GTK_SPIN_BUTTON(dialog->spinb_tileheight));

	if (!filename)
	{
		error_message = "Please select a tileset image first";
		goto error;
	}

	gint tileset_w, tileset_h;
	GdkPixbufFormat *pbf =
		gdk_pixbuf_get_file_info (filename, &tileset_w, &tileset_h);

	if (!pbf)
	{
		error_message =
			"The selected tilset image\n"
			"is <b>not a valid image file</b>";
		goto error;
	}

	gchar *format = gdk_pixbuf_format_get_name(pbf);

	if (g_strcmp0(format, "png"))
	{
		error_message =
			"Currently <b>only png files</b> are supported.\n"
			"Sorry about that =/";
		goto error;
	}

	if (tileset_w % tile_w != 0)
	{
		warning_message =
			"The tileset image <b>width</b> does not appear to be\n"
			"a multiple of the specified tile width (remainder: %d).\n"
			"Proceed anyway?";

		remainder = tileset_w % tile_w;
		goto warning;
	}

	if (tileset_h % tile_h != 0)
	{
		warning_message =
			"The tileset image <b>height</b> does not appear to be\n"
			"a multiple of the specified tile height (remainder: %d).\n"
			"Proceed anyway?";

		remainder = tileset_h % tile_h;
		goto warning;
	}

	error:
	if (error_message)
	{
		GtkWidget *error_dialog =
			gtk_message_dialog_new_with_markup
				(GTK_WINDOW(dialog->window), 0,
				 GTK_MESSAGE_ERROR,
				 GTK_BUTTONS_CLOSE,
				 "<b>Error</b>");
		gtk_message_dialog_format_secondary_markup
			(GTK_MESSAGE_DIALOG(error_dialog), error_message);

		gtk_dialog_run(GTK_DIALOG(error_dialog));
		gtk_widget_destroy(error_dialog);
		g_free(filename);

		return;
	}

	warning:
	if (warning_message)
	{
		GtkWidget *warning_dialog =
			gtk_message_dialog_new_with_markup
				(GTK_WINDOW(dialog->window), 0,
				 GTK_MESSAGE_WARNING,
				 GTK_BUTTONS_YES_NO,
				 "<b>Warning</b>");
		gtk_message_dialog_format_secondary_markup
			(GTK_MESSAGE_DIALOG(warning_dialog),
			 warning_message, remainder);

		switch (gtk_dialog_run(GTK_DIALOG(warning_dialog)))
		{
			case GTK_RESPONSE_NO   :
			case GTK_RESPONSE_NONE :
				gtk_widget_destroy(warning_dialog);
				g_free(filename);
				return;

			default :
				gtk_widget_destroy(warning_dialog);
		}

	}

	file_close(global_data);
	if (global_data->open_file_path)
	{
		g_free(global_data->open_file_path);
		global_data->open_file_path = NULL;
	}

	global_data->settings->preferred_tile_width = tile_w;
	global_data->settings->preferred_tile_height = tile_h;
	global_data->buffer_changed = TRUE;

	gtk_widget_destroy(dialog->window);
	g_free(dialog);

	file_parse
		(global_data, file_create(filename, tile_w, tile_h));

	tileset_area_update_viewport(global_data);
	tileset_area_redraw_cache(global_data);
	gtk_widget_queue_draw
		(global_data->main_window->tileset_area);
}


void new_file_dialog_run
( struct GlobalData *global_data )
{
	struct NewFileDialog *dialog =
		g_malloc( sizeof( *dialog ) );

	GtkWidget *file_dialog =
		gtk_file_chooser_dialog_new
			("Select tileset image", NULL,
			 GTK_FILE_CHOOSER_ACTION_OPEN,
			 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
			 GTK_STOCK_OPEN,   GTK_RESPONSE_ACCEPT,
			 NULL);

	GtkFileFilter *filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "PNG images");
	gtk_file_filter_add_mime_type(filter, "image/png");
//	gtk_file_filter_add_mime_type(filter, "image/jpeg");
//	gtk_file_filter_add_mime_type(filter, "image/gif");
	gtk_file_chooser_add_filter
		(GTK_FILE_CHOOSER(file_dialog), filter);

	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "All");
	gtk_file_filter_add_pattern(filter, "*");
	gtk_file_chooser_add_filter
		(GTK_FILE_CHOOSER(file_dialog), filter);


	GtkWidget *file_button =
		gtk_file_chooser_button_new_with_dialog(file_dialog);

	GtkWidget *spinb_tilewidth =
		gtk_spin_button_new_with_range(1, 0x100, 1);
	gtk_spin_button_set_value
		(GTK_SPIN_BUTTON(spinb_tilewidth),
		 (gdouble)global_data->settings->preferred_tile_width);

	GtkWidget *spinb_tileheight =
		gtk_spin_button_new_with_range(1, 0x100, 1);
	gtk_spin_button_set_value
		(GTK_SPIN_BUTTON(spinb_tileheight),
		 (gdouble)global_data->settings->preferred_tile_height);

	GtkSizeGroup *sgroup =
		gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	gtk_size_group_add_widgets
		(sgroup, spinb_tilewidth, spinb_tileheight, NULL);

	GtkWidget *label_width = gtk_label_new("   Width:");
	GtkWidget *label_height = gtk_label_new("Height:");

	sgroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	gtk_size_group_add_widget(sgroup, label_width);
	gtk_size_group_add_widget(sgroup, label_height);

	GtkWidget *content_area = gtk_vbox_new(FALSE, 2);
	GtkWidget *hbox;

	hbox = gtk_hbox_new(FALSE, 2);
	gtk_box_pack_start(GTK_BOX(hbox),
		gtk_label_new("Tileset image:"), FALSE, FALSE, 8);
	gtk_box_pack_end(GTK_BOX(hbox),
		file_button, TRUE, TRUE, 2);
	gtk_box_pack_start(GTK_BOX(content_area), hbox, TRUE, TRUE, 2);

	GtkWidget *tilesize_box = gtk_hbox_new(FALSE, 2);

	GtkWidget *size_label = gtk_label_new(" ");
	gtk_label_set_markup(GTK_LABEL(size_label), "<b>Tile Size</b>");
	hbox = gtk_hbox_new(FALSE, 8);
	gtk_box_pack_start(GTK_BOX(hbox), size_label, FALSE, FALSE, 8);
	gtk_box_pack_start(GTK_BOX(content_area), hbox, FALSE, FALSE, 2);

	hbox = gtk_hbox_new(FALSE, 4);
	gtk_box_pack_start(GTK_BOX(hbox),
		label_width, FALSE, FALSE, 8);
	gtk_box_pack_start(GTK_BOX(hbox),
		spinb_tilewidth, FALSE, FALSE, 4);
	gtk_box_pack_start(GTK_BOX(tilesize_box), hbox, TRUE, TRUE, 2);

	gtk_box_pack_start
		(GTK_BOX(tilesize_box), gtk_vseparator_new(), FALSE, FALSE, 2);

	hbox = gtk_hbox_new(FALSE, 4);
	gtk_box_pack_start(GTK_BOX(hbox),
		label_height, FALSE, FALSE, 8);
	gtk_box_pack_start(GTK_BOX(hbox),
		spinb_tileheight, FALSE, FALSE, 4);
	gtk_box_pack_start(GTK_BOX(tilesize_box), hbox, TRUE, TRUE, 2);

	hbox = gtk_hbox_new(FALSE, 2);
	gtk_box_pack_start(GTK_BOX(hbox), tilesize_box, FALSE, FALSE, 2);

	gtk_box_pack_start(GTK_BOX(content_area), hbox, FALSE, FALSE, 2);

	GtkWidget *content_area_adj = gtk_vbox_new(TRUE, 8);
	gtk_box_pack_start
		(GTK_BOX(content_area_adj), content_area, TRUE, TRUE, 8);

	GtkWidget *cancelb = gtk_button_new_from_stock(GTK_STOCK_CANCEL),
	          *okb     = gtk_button_new_from_stock(GTK_STOCK_OK);

	sgroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	gtk_size_group_add_widgets(sgroup, cancelb, okb, NULL);

	g_signal_connect(cancelb, "clicked",
		G_CALLBACK( cb_button_cancel_clicked ), global_data);
	g_signal_connect(okb, "clicked",
		G_CALLBACK( cb_button_ok_clicked ), global_data);

	GtkWidget *action_area = gtk_hbox_new(FALSE, 8);
	gtk_box_pack_start
		(GTK_BOX(action_area), cancelb, FALSE, FALSE, 8);
	gtk_box_pack_end
		(GTK_BOX(action_area), okb, FALSE, FALSE, 8);

	GtkWidget *dialog_box = gtk_vbox_new(FALSE, 2);
	gtk_box_pack_start
		(GTK_BOX(dialog_box), content_area_adj, TRUE, TRUE, 2);
	gtk_box_pack_end
		(GTK_BOX(dialog_box), action_area, FALSE, FALSE, 2);
	gtk_box_pack_end
		(GTK_BOX(dialog_box), gtk_hseparator_new(), FALSE, FALSE, 2);


	GtkWidget *dialog_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title
		(GTK_WINDOW(dialog_window), "New Tileset");
	gtk_window_set_resizable
		(GTK_WINDOW(dialog_window), FALSE);
	gtk_window_set_modal
		(GTK_WINDOW(dialog_window), TRUE);
	gtk_window_set_transient_for
		(GTK_WINDOW(dialog_window),
		 GTK_WINDOW(global_data->main_window->window));
	gtk_window_set_destroy_with_parent
		(GTK_WINDOW(dialog_window), TRUE);
	gtk_window_set_skip_taskbar_hint
		(GTK_WINDOW(dialog_window), TRUE);

	gtk_container_add(GTK_CONTAINER(dialog_window), dialog_box);


	dialog->window = dialog_window;
	dialog->file_button = file_button;
	dialog->spinb_tilewidth = spinb_tilewidth;
	dialog->spinb_tileheight = spinb_tileheight;
	global_data->new_file_dialog = dialog;

	gtk_widget_show_all(dialog_window);
}
