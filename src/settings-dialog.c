/*
 * settings-dialog.c
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

#include "tileatteditor.h"
#include "tileset-area.h"
#include "callback.h"
#include "dialog.h"
#include "file.h"
#include "util.h"

#define CAST_SETTINGS_DIALOG                    \
	struct SettingsDialog *settings_dialog =	\
	(struct SettingsDialog*)data;


struct SettingsDialog
{
	gboolean resize_required;
	gboolean redraw_required;

	GtkWidget *window;

	GtkWidget *spinb_scale;
	GtkWidget *hscale_alpha;
	GtkWidget *checkb_smooth;
	GtkWidget *colorb_bg;
	GtkWidget *colorb_grid;
	GtkWidget *applyb;

	GtkWidget **checkb_attributes;

	GtkWidget *checkb_show_labels;

//	struct Settings *local_settings;

};


static void apply_settings
( struct GlobalData *global_data )
{
	struct Settings *settings = global_data->settings;
	struct SettingsDialog *dialog = global_data->settings_dialog;

	settings->tileset_scale_ratio =
		gtk_spin_button_get_value
			(GTK_SPIN_BUTTON(dialog->spinb_scale));
	settings->attribute_alpha =
		gtk_range_get_value(GTK_RANGE(dialog->hscale_alpha));
	settings->smooth_zoom =
		gtk_toggle_button_get_active
			(GTK_TOGGLE_BUTTON(dialog->checkb_smooth));
	color_set_from_button(settings->bg_color, dialog->colorb_bg);
	color_set_from_button(settings->grid_color, dialog->colorb_grid);

	settings->show_button_labels =
		gtk_toggle_button_get_active
			(GTK_TOGGLE_BUTTON(dialog->checkb_show_labels));
	ui_attr_button_set_show_label
		(global_data, settings->show_button_labels);

	gboolean active_attr_disabled = FALSE;
	gint i; for (i=0;i<ATTRIBUTE_COUNT;i++)
	{
		struct TileAttribute *tile_attr =
			global_data->tile_attributes[i];

		if (!gtk_toggle_button_get_active
				(GTK_TOGGLE_BUTTON(dialog->checkb_attributes[i])))
		{
			tile_attr->enabled = FALSE;
			gtk_widget_hide(tile_attr->button);

			if (tile_attr == global_data->active_attribute)
			{
				/* Catch active attribute, but process this later
				 * after all attributes have been enabled/disabled */
				active_attr_disabled = TRUE;
			}
		}
		else
		{
			tile_attr->enabled = TRUE;
			gtk_widget_show(tile_attr->button);
			if (file_attribute_enable
					(global_data->open_file, tile_attr, i))
			{ ui_set_buffer_changed(global_data, TRUE); }
		}
	}

	if (dialog->resize_required)
	{
		tileset_update_scale(global_data);
		tileset_area_update_viewport(global_data);
	}

	if (active_attr_disabled)
	{
		/* Now we can deal with the active attribute. as this
		 * change will trigger a redraw anyway, we set the
		 * "required" flag to false */
		activate_other_attribute(global_data);
		dialog->redraw_required = FALSE;
	}

	if (dialog->redraw_required)
		{ tileset_area_redraw_cache(global_data); }
	gtk_widget_queue_draw(global_data->main_window->tileset_area);

	dialog->resize_required = FALSE;
	dialog->redraw_required = FALSE;

}

static void cb_button_cancel_clicked
( GtkWidget *button, gpointer data )
{
	CAST_GLOBAL_DATA
	gtk_widget_destroy(global_data->settings_dialog->window);
	g_free(global_data->settings_dialog->checkb_attributes);
	g_free(global_data->settings_dialog);
	global_data->settings_dialog = 0;
}

static void cb_window_deleted
( GtkWidget *window, GdkEvent *event, gpointer data )
{
	CAST_GLOBAL_DATA
	cb_button_cancel_clicked(NULL, global_data);
}


static void cb_button_ok_clicked
( GtkWidget *button, gpointer data )
{
	CAST_GLOBAL_DATA
	apply_settings(global_data);
	cb_button_cancel_clicked(NULL, global_data);
}

static void cb_button_apply_clicked
( GtkWidget *button, gpointer data )
{
	CAST_GLOBAL_DATA
	apply_settings(global_data);
	gtk_widget_set_sensitive
		(global_data->settings_dialog->applyb, FALSE);
}

static void cb_resize_required
( GtkWidget *button, gpointer data )
{
	CAST_SETTINGS_DIALOG
	settings_dialog->resize_required = TRUE;
	settings_dialog->redraw_required = TRUE;
	gtk_widget_set_sensitive(settings_dialog->applyb, TRUE);
}

static void cb_redraw_required
( GtkWidget *button, gpointer data )
{
	CAST_SETTINGS_DIALOG
	settings_dialog->redraw_required = TRUE;
	gtk_widget_set_sensitive(settings_dialog->applyb, TRUE);
}

static void cb_check_button_attr_toggled
( GtkWidget *button, gpointer data )
{
	CAST_SETTINGS_DIALOG
	gtk_widget_set_sensitive(settings_dialog->applyb, TRUE);
}


void settings_dialog_run
( struct GlobalData *global_data )
{
	struct Settings *settings = global_data->settings;
	struct SettingsDialog *settings_dialog =
		g_malloc( sizeof( *settings_dialog ) );

	settings_dialog->resize_required = FALSE;
	settings_dialog->redraw_required = FALSE;

	/* Create first page */
	GtkWidget *spinb_scale =
		gtk_spin_button_new_with_range(0.5, 5, 0.5);
	gtk_spin_button_set_digits(GTK_SPIN_BUTTON(spinb_scale), 2);
	gtk_spin_button_set_value
		(GTK_SPIN_BUTTON(spinb_scale), settings->tileset_scale_ratio);

	GtkWidget *hscale_alpha = gtk_hscale_new_with_range(0, 1, 0.01);
	gtk_scale_set_draw_value(GTK_SCALE(hscale_alpha), FALSE);
	GtkAllocation alloc;
	gtk_widget_get_allocation(hscale_alpha, &alloc);
	gtk_widget_set_size_request(hscale_alpha, 120, alloc.height);
	gtk_range_set_value
		(GTK_RANGE(hscale_alpha), settings->attribute_alpha);

	GtkWidget *checkb_smooth = gtk_check_button_new();
	gtk_toggle_button_set_active
		(GTK_TOGGLE_BUTTON(checkb_smooth), settings->smooth_zoom);

	GtkWidget *colorb_bg = gtk_color_button_new();
	color_set_button(settings->bg_color, colorb_bg);

	GtkWidget *colorb_grid = gtk_color_button_new();
	gtk_color_button_set_use_alpha
		(GTK_COLOR_BUTTON(colorb_grid), TRUE);
	color_set_button(settings->grid_color, colorb_grid);

	GtkSizeGroup *sgroup =
		gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	gtk_size_group_add_widgets(sgroup,
		spinb_scale, hscale_alpha, checkb_smooth,
		colorb_bg, colorb_grid, NULL);

	GtkWidget *page1 = gtk_vbox_new(TRUE, 8);
	GtkWidget *vbox = gtk_vbox_new(TRUE, 8);
	GtkWidget *hbox;

	hbox = gtk_hbox_new(FALSE, 8);
	gtk_box_pack_start(GTK_BOX(hbox),
		gtk_label_new("Tileset scale:"), FALSE, FALSE, 8);
	gtk_box_pack_end(GTK_BOX(hbox),
		spinb_scale, FALSE, FALSE, 8);
	gtk_box_pack_start_defaults(GTK_BOX(vbox), hbox);

	hbox = gtk_hbox_new(FALSE, 8);
	gtk_box_pack_start(GTK_BOX(hbox),
		gtk_label_new("Attribute opacity:"), FALSE, FALSE, 8);
	gtk_box_pack_end(GTK_BOX(hbox),
		hscale_alpha, FALSE, FALSE, 8);
	gtk_box_pack_start_defaults(GTK_BOX(vbox), hbox);

	hbox = gtk_hbox_new(FALSE, 8);
	gtk_box_pack_start(GTK_BOX(hbox),
		gtk_label_new("Smooth zoom:"), FALSE, FALSE, 8);
	gtk_box_pack_end(GTK_BOX(hbox),
		checkb_smooth, FALSE, FALSE, 8);
	gtk_box_pack_start_defaults(GTK_BOX(vbox), hbox);

	hbox = gtk_hbox_new(FALSE, 8);
	gtk_box_pack_start(GTK_BOX(hbox),
		gtk_label_new("Background color:"), FALSE, FALSE, 8);
	gtk_box_pack_end(GTK_BOX(hbox),
		colorb_bg, FALSE, FALSE, 8);
	gtk_box_pack_start_defaults(GTK_BOX(vbox), hbox);

	hbox = gtk_hbox_new(FALSE, 8);
	gtk_box_pack_start(GTK_BOX(hbox),
		gtk_label_new("Grid color:"), FALSE, FALSE, 8);
	gtk_box_pack_end(GTK_BOX(hbox),
		colorb_grid, FALSE, FALSE, 8);
	gtk_box_pack_start_defaults(GTK_BOX(vbox), hbox);

	gtk_box_pack_start(GTK_BOX(page1), vbox, TRUE, TRUE, 8);

	g_signal_connect(spinb_scale, "value-changed",
		G_CALLBACK( cb_resize_required ), settings_dialog);
	g_signal_connect(hscale_alpha, "value-changed",
		G_CALLBACK( cb_redraw_required ), settings_dialog);
	g_signal_connect(checkb_smooth, "toggled",
		G_CALLBACK( cb_resize_required ), settings_dialog);
	g_signal_connect(colorb_bg, "color-set",
		G_CALLBACK( cb_redraw_required ), settings_dialog);
	g_signal_connect(colorb_grid, "color-set",
		G_CALLBACK( cb_redraw_required ), settings_dialog);

	settings_dialog->spinb_scale = spinb_scale;
	settings_dialog->hscale_alpha = hscale_alpha;
	settings_dialog->checkb_smooth = checkb_smooth;
	settings_dialog->colorb_bg = colorb_bg;
	settings_dialog->colorb_grid = colorb_grid;

	/* Create second page */
	GtkWidget *page2 = gtk_vbox_new(TRUE, 2);
	gtk_container_set_border_width(GTK_CONTAINER(page2), 8);

	settings_dialog->checkb_attributes =
		g_malloc( sizeof( GtkWidget* ) * ATTRIBUTE_COUNT);

	struct TileAttribute **attr; gint i = 0;
	for (attr=global_data->tile_attributes;
	     *attr; attr++)
	{
		GtkWidget *checkb =
			gtk_check_button_new_with_label((*attr)->name);
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON(checkb), (*attr)->enabled);
		gtk_box_pack_start(GTK_BOX(page2), checkb, TRUE, FALSE, 2);

		g_signal_connect(checkb, "toggled",
			G_CALLBACK( cb_check_button_attr_toggled ),
			settings_dialog);

		settings_dialog->checkb_attributes[i] = checkb;
		i++;
	}

	/* Create third page */
	GtkWidget *checkb_show_labels =
		gtk_check_button_new_with_label(" Show button labels");
	gtk_toggle_button_set_active
		(GTK_TOGGLE_BUTTON(checkb_show_labels),
		 settings->show_button_labels);

	g_signal_connect
		(checkb_show_labels, "toggled",
		 G_CALLBACK( cb_check_button_attr_toggled ),
		 settings_dialog);

	settings_dialog->checkb_show_labels = checkb_show_labels;

	GtkWidget *page3 = gtk_vbox_new(FALSE, 2);
	gtk_container_set_border_width(GTK_CONTAINER(page3), 8);
	gtk_box_pack_start
		(GTK_BOX(page3), checkb_show_labels, FALSE, FALSE, 2);

	/* Create notebook and dialog */
	GtkWidget *content_area = gtk_vbox_new(TRUE, 4),
	          *action_area = gtk_hbox_new(FALSE, 4),
	          *dialog_box = gtk_vbox_new(FALSE, 4);


	GtkWidget *notebook = gtk_notebook_new();
	gtk_notebook_append_page
		(GTK_NOTEBOOK(notebook), page1, gtk_label_new("Display"));
	gtk_notebook_append_page
		(GTK_NOTEBOOK(notebook), page2, gtk_label_new("Attributes"));
	gtk_notebook_append_page
		(GTK_NOTEBOOK(notebook), page3, gtk_label_new("Misc"));

	gtk_box_pack_start_defaults(GTK_BOX(content_area), notebook);

	GtkWidget *cancelb = gtk_button_new_from_stock(GTK_STOCK_CANCEL),
	          *applyb  = gtk_button_new_from_stock(GTK_STOCK_APPLY),
	          *okb     = gtk_button_new_from_stock(GTK_STOCK_OK);

	sgroup = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
	gtk_size_group_add_widgets(sgroup, cancelb, applyb, okb, NULL);

	gtk_widget_set_sensitive(applyb, FALSE);

	g_signal_connect(cancelb, "clicked",
		G_CALLBACK( cb_button_cancel_clicked ), global_data);
	g_signal_connect(applyb, "clicked",
		G_CALLBACK( cb_button_apply_clicked ), global_data);
	g_signal_connect(okb, "clicked",
		G_CALLBACK( cb_button_ok_clicked ), global_data);

	settings_dialog->applyb = applyb;


	gtk_box_pack_start
		(GTK_BOX(action_area), cancelb, FALSE, FALSE, 4);
	gtk_box_pack_end
		(GTK_BOX(action_area), okb, FALSE, FALSE, 4);
	gtk_box_pack_end
		(GTK_BOX(action_area), applyb, FALSE, FALSE, 4);

	gtk_box_pack_start(GTK_BOX(dialog_box),
		content_area, TRUE, TRUE, 4);
	gtk_box_pack_end(GTK_BOX(dialog_box),
		action_area, FALSE, FALSE, 4);

	GtkWidget *dialog_window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_container_set_border_width(GTK_CONTAINER(dialog_window), 4);
	gtk_window_set_title
		(GTK_WINDOW(dialog_window), "Preferences");
	gtk_window_set_resizable
		(GTK_WINDOW(dialog_window), FALSE);
	gtk_window_set_transient_for
		(GTK_WINDOW(dialog_window),
		 GTK_WINDOW(global_data->main_window->window));
	gtk_window_set_destroy_with_parent
		(GTK_WINDOW(dialog_window), TRUE);
	gtk_window_set_skip_taskbar_hint
		(GTK_WINDOW(dialog_window), TRUE);
	gtk_container_add(GTK_CONTAINER(dialog_window), dialog_box);

	g_signal_connect(dialog_window, "delete-event",
		G_CALLBACK( cb_window_deleted ), global_data);

	settings_dialog->window = dialog_window;
	global_data->settings_dialog = settings_dialog;

	gtk_widget_show_all(dialog_window);
}

