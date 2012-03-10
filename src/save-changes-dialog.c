/*
 * save-changes-dialog.c
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
#include "dialog.h"
#include "util.h"



static void cb_button_cancel_clicked
( GtkWidget *button, gpointer dialog )
{
	gtk_dialog_response
		(GTK_DIALOG(dialog), GTK_RESPONSE_CANCEL);
}

static void cb_button_discard_clicked
( GtkWidget *button, gpointer dialog )
{
	gtk_dialog_response
		(GTK_DIALOG(dialog), GTK_RESPONSE_REJECT);
}

static void cb_button_save_clicked
( GtkWidget *button, gpointer dialog )
{
	gtk_dialog_response
		(GTK_DIALOG(dialog), GTK_RESPONSE_ACCEPT);
}



GtkWidget* save_changes_dialog_new
( GtkWidget *parent )
{
	/* Text Area */
	gchar *text[] =
		{"<big><b>Warning: There are unsaved changes!</b></big>",
		 "Do you want to save this file before continuing?"};

	GtkWidget *dialog = gtk_dialog_new();
	gtk_window_set_transient_for
		(GTK_WINDOW(dialog), GTK_WINDOW(parent));
	gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);
	gtk_window_set_title(GTK_WINDOW(dialog), "Unsaved Changes");

	GtkWidget *action_area =
		gtk_dialog_get_action_area(GTK_DIALOG(dialog));

	GtkWidget *content_area =
		gtk_dialog_get_content_area(GTK_DIALOG(dialog));

	GtkWidget *hbox = gtk_hbox_new(FALSE, 4);

	gtk_box_pack_start
		(GTK_BOX(hbox),
		 gtk_image_new_from_stock
			(GTK_STOCK_DIALOG_WARNING, GTK_ICON_SIZE_DIALOG),
		 FALSE, FALSE, 4);

	GtkWidget *label_box = gtk_vbox_new(FALSE, 4);

	GtkWidget *text_label = gtk_label_new("");
	gtk_label_set_markup(GTK_LABEL(text_label), text[0]);
	gtk_box_pack_start
		(GTK_BOX(label_box), text_label, FALSE, FALSE, 4);

	text_label = gtk_label_new(text[1]);
	gtk_box_pack_start
		(GTK_BOX(label_box), text_label, FALSE, FALSE, 4);

	gtk_box_pack_start(GTK_BOX(hbox), label_box, TRUE, TRUE, 4);

	GtkWidget *adjbox = gtk_vbox_new(FALSE, 8);
	gtk_box_pack_start(GTK_BOX(adjbox), hbox, FALSE, FALSE, 8);

	gtk_box_pack_start
		(GTK_BOX(content_area), adjbox, TRUE, FALSE, 0);

	/* Button Area */
	GtkWidget *cancelb, *discardb, *saveb;

	cancelb = gtk_button_new_from_stock(GTK_STOCK_CANCEL);

	discardb = gtk_button_new_with_mnemonic("_Discard Changes");
	gtk_button_set_image
		(GTK_BUTTON(discardb),
		 gtk_image_new_from_stock
			(GTK_STOCK_CLEAR, GTK_ICON_SIZE_BUTTON));

	saveb = gtk_button_new_from_stock(GTK_STOCK_SAVE);

	g_signal_connect(cancelb, "clicked",
		G_CALLBACK( cb_button_cancel_clicked ), dialog);

	g_signal_connect(discardb, "clicked",
		G_CALLBACK( cb_button_discard_clicked ), dialog);

	g_signal_connect(saveb, "clicked",
		G_CALLBACK( cb_button_save_clicked ), dialog);

	hbox = gtk_hbox_new(FALSE, 4);

	gtk_box_pack_start
		(GTK_BOX(hbox), cancelb, FALSE, FALSE, 4);

	gtk_box_pack_end
		(GTK_BOX(hbox), saveb, FALSE, FALSE, 4);

	gtk_box_pack_end
		(GTK_BOX(hbox), discardb, FALSE, FALSE, 4);


	gtk_box_pack_start
		(GTK_BOX(action_area), hbox, TRUE, FALSE, 0);

	gtk_size_group_add_widgets
		(gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL),
		 discardb, saveb, NULL);


	gtk_widget_show_all(content_area);
	gtk_widget_show_all(action_area);

	gtk_widget_grab_focus(saveb);

	return dialog;
}

