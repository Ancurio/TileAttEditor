
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
//	gtk_misc_set_alignment(GTK_MISC(text_label), 0.2, 0.5);
	gtk_label_set_markup(GTK_LABEL(text_label), text[0]);
	gtk_box_pack_start
		(GTK_BOX(label_box), text_label, FALSE, FALSE, 4);

	text_label = gtk_label_new(text[1]);
//	gtk_misc_set_alignment(GTK_MISC(text_label), 0, 0.5);
	gtk_box_pack_start
		(GTK_BOX(label_box), text_label, FALSE, FALSE, 4);

	gtk_box_pack_start(GTK_BOX(hbox), label_box, TRUE, TRUE, 4);

	GtkWidget *adjbox = gtk_vbox_new(FALSE, 8);
	gtk_box_pack_start(GTK_BOX(adjbox), hbox, FALSE, FALSE, 8);

	gtk_box_pack_start_defaults(GTK_BOX(content_area), adjbox);

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

	//gtk_dialog_add_action_widget
		//(GTK_DIALOG(dialog), cancelb, GTK_RESPONSE_CANCEL);

	//gtk_dialog_add_action_widget
		//(GTK_DIALOG(dialog), discardb, GTK_RESPONSE_REJECT);

	//gtk_dialog_add_action_widget
		//(GTK_DIALOG(dialog), saveb, GTK_RESPONSE_ACCEPT);

	gtk_box_pack_start_defaults(GTK_BOX(action_area), hbox);

	gtk_size_group_add_widgets
		(gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL),
		 discardb, saveb, NULL);

	gtk_widget_show_all(content_area);
	gtk_widget_show_all(action_area);

	gtk_widget_set_can_default(saveb, TRUE);
	gtk_window_set_default(GTK_WINDOW(dialog), saveb);

	return dialog;
}

