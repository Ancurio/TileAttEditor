
#include <gtk/gtk.h>

#include "tileatteditor.h"
#include "tileset-area.h"
#include "callback.h"
#include "ui-menubar.xml"
#include "util.h"



static void ui_update_window_title
( struct GlobalData *global_data );


static GtkActionEntry action_entries[] =
{
	{ "FileMenu", NULL, "_File" },
		{ "New", GTK_STOCK_NEW, "_New", "<control>N",
		  "Create a new Attribute Map",
		  G_CALLBACK (cb_filemenu_new) },
		{ "Open", GTK_STOCK_OPEN, "_Open", "<control>O",
		  "Open an existing Attribute Map",
		  G_CALLBACK (cb_filemenu_open) },
		{ "Save", GTK_STOCK_SAVE, "_Save", "<control>S",
		  "Save current Attribute Map",
		  G_CALLBACK (cb_filemenu_save) },
		{ "Close", GTK_STOCK_CLOSE, "Close", "<control>W",
		  "Close current Attribute Map",
		  G_CALLBACK (cb_filemenu_close) },
		{ "Quit", GTK_STOCK_QUIT, "_Quit", "<control>Q",
		  "Quit program",
		  G_CALLBACK (cb_filemenu_quit) },

	{ "EditMenu", NULL, "_Edit" },
		{ "Preferences", GTK_STOCK_PREFERENCES, "_Preferences", NULL,
		  "Edit configuration",
		  G_CALLBACK (cb_editmenu_preferences) }
};

static guint action_entries_count =
	G_N_ELEMENTS (action_entries);


static GtkToggleActionEntry toggle_entry =

	{ "Flip", NULL, "_Flip Workspace", "<control>F",
	  "Flip the workspace orientation\n(Just try it out!)",
	  G_CALLBACK( cb_editmenu_flip ), FALSE };


static GtkTargetEntry dnd_target_entries[] =
{
	{ "text/uri-list", 0, 0 }
};

static guint dnd_target_entries_count =
	G_N_ELEMENTS (dnd_target_entries);



/* returns a hbox containing the menubar */
static GtkWidget* ui_menubar_create
( GtkWidget *window, struct GlobalData *global_data )
{
	GtkUIManager *ui;
	GtkWidget *ui_widget;
	GtkWidget *ui_wrap_box;
	GtkActionGroup *actions;
	GError *error = NULL;

	actions = gtk_action_group_new ("Actions");
	gtk_action_group_add_actions
		(actions, action_entries, action_entries_count, global_data);
	gtk_action_group_add_toggle_actions
		(actions, &toggle_entry, 1, global_data);

	gtk_toggle_action_set_active
		(GTK_TOGGLE_ACTION
			(gtk_action_group_get_action(actions, "Flip")),
		 global_data->settings->workspace_flipped);

	ui = gtk_ui_manager_new();
	gtk_ui_manager_insert_action_group (ui, actions, 0);
	g_object_unref (actions);

	if (!gtk_ui_manager_add_ui_from_string(ui, ui_info, -1, &error))
	{
		g_message ("building menus failed: %s", error->message);
		g_error_free (error);
		return NULL;
	}

	gtk_window_add_accel_group
		(GTK_WINDOW(window), gtk_ui_manager_get_accel_group(ui));
	ui_widget = gtk_ui_manager_get_widget(ui, "/MenuBar");
	ui_wrap_box = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(ui_wrap_box), ui_widget, TRUE, TRUE, 0);
	g_object_unref(ui);

	return ui_wrap_box;
}


/* returns the drawing area in a hbox
 * in a vbox in a scrollable window */
static GtkWidget* ui_tilesetarea_create
( GtkWidget **_tileset_area, GtkWidget **_tileset_viewport,
  gpointer global_data )
{
	GtkWidget *tileset_area, *hbox, *vbox,
	          *viewport, *scrollarea;

	tileset_area = gtk_drawing_area_new();

	gtk_widget_set_size_request
		(tileset_area, T_AREA_MIN_WIDTH, T_AREA_MIN_HEIGHT);

	gtk_widget_add_events(tileset_area,
		GDK_BUTTON_PRESS_MASK | GDK_POINTER_MOTION_MASK |
		GDK_LEAVE_NOTIFY_MASK | GDK_POINTER_MOTION_HINT_MASK);

	g_signal_connect( G_OBJECT (tileset_area), "expose_event",
					  G_CALLBACK (cb_tileset_area_expose),
					  global_data );
	g_signal_connect( G_OBJECT (tileset_area), "button-press-event",
					  G_CALLBACK (cb_tileset_area_button_press),
					  global_data );
	g_signal_connect( G_OBJECT (tileset_area), "motion-notify-event",
					  G_CALLBACK (cb_tileset_area_motion_notify),
					  global_data );
	g_signal_connect( G_OBJECT (tileset_area), "scroll-event",
					  G_CALLBACK (cb_tileset_area_motion_notify),
					  global_data );
	g_signal_connect( G_OBJECT (tileset_area), "leave-notify-event",
					  G_CALLBACK (cb_tileset_area_leave_notify),
					  global_data );


	vbox = gtk_vbox_new(TRUE, 4);
	hbox = gtk_hbox_new(TRUE, 4);
	gtk_box_pack_start(GTK_BOX(hbox), tileset_area, FALSE, FALSE, 4);
	gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 4);

	scrollarea = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_add_with_viewport
		(GTK_SCROLLED_WINDOW(scrollarea), vbox);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrollarea),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

	g_signal_connect( G_OBJECT (scrollarea), "drag_data_received",
					  G_CALLBACK (cb_tileset_area_drag_data_received),
					  global_data );

	gtk_drag_dest_set
		(scrollarea, GTK_DEST_DEFAULT_ALL,
		 dnd_target_entries, dnd_target_entries_count,
		 GDK_ACTION_COPY);

	viewport =
		gtk_container_get_children(GTK_CONTAINER(scrollarea))->data;
	gtk_viewport_set_shadow_type
		(GTK_VIEWPORT(viewport), GTK_SHADOW_NONE);
	gtk_widget_set_size_request
		(scrollarea, T_AREA_MIN_WIDTH+0x20, T_AREA_MIN_HEIGHT+0x20);

	gtk_widget_show(hbox);
	gtk_widget_show(vbox);
	gtk_widget_show(viewport);
	gtk_widget_show(scrollarea);

	*_tileset_area = tileset_area;
	*_tileset_viewport = viewport;
	return scrollarea;
}

static GtkWidget* ui_attribute_buttons_create
( struct GlobalData *global_data )
{
	struct TileAttribute **tile_attr = global_data->tile_attributes;

	GtkWidget *attr_box = gtk_vbox_new(FALSE, 8);
	GtkSizeGroup *sgroup =
		gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);

	gint i;
	for (i=0;tile_attr[i];i++)
	{
		GtkWidget *attr_button_box = gtk_hbox_new(FALSE, 0);
		GtkWidget *attr_center_box = gtk_hbox_new(FALSE, 4);

		GtkWidget *attr_icon = gtk_drawing_area_new();
		gtk_widget_set_size_request(attr_icon, 32, 32);
		g_signal_connect
			(attr_icon, "expose-event",
			 G_CALLBACK( cb_attr_icon_expose ), tile_attr[i]);

		gtk_box_pack_start
			(GTK_BOX(attr_center_box), attr_icon, FALSE, FALSE, 4);
		gtk_box_pack_start
			(GTK_BOX(attr_center_box),
			 gtk_label_new(tile_attr[i]->name), FALSE, FALSE, 4);
		gtk_size_group_add_widget(sgroup, attr_center_box);

		gtk_box_pack_start
			(GTK_BOX(attr_button_box),
			 attr_center_box, TRUE, FALSE, 0);
		gtk_widget_show_all(attr_button_box);

		GtkWidget *attr_button =
			gtk_toggle_button_new();
		gtk_container_add
			(GTK_CONTAINER(attr_button), attr_button_box);

		tile_attr[i]->signal_handler_id =
			g_signal_connect(attr_button, "toggled",
			G_CALLBACK( cb_attr_button_toggled ), tile_attr[i]);
		gtk_box_pack_start(GTK_BOX( attr_box ), attr_button,
			FALSE, FALSE, 8);
		tile_attr[i]->button = attr_button;
		if (tile_attr[i]->enabled)
			{ gtk_widget_show(attr_button); }
	}

	return attr_box;
}

static void gtk_window_set_limited_size
( GtkWindow *window, gint win_w, gint win_h )
{
	GdkScreen *screen = gtk_window_get_screen(window);

	gint max_w = (gdk_screen_get_width(screen)  / 4) * 3,
	     max_h = (gdk_screen_get_height(screen) / 4) * 3;

	if (win_w > max_w) { win_w = max_w; }
	if (win_h > max_h) { win_h = max_h; }

	gtk_window_set_default_size(window, win_w, win_h);
}

void ui_main_window_create
( gpointer _global_data )
{
	CAST_GLOBAL_DATA_PTR(_global_data);

	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_limited_size
		(GTK_WINDOW(window),
		 global_data->settings->window_width,
		 global_data->settings->window_height);
	ui_update_window_title(global_data);


	GtkWidget *mainbox = gtk_vbox_new(FALSE, 8);
	GtkWidget *menubar_box;
	GtkWidget *workspace_box = gtk_hbox_new(FALSE, 8);
	GtkWidget *separator = gtk_vseparator_new();
	GtkWidget *tileset_frame = gtk_frame_new("Tileset");
	GtkWidget *tileset_area, *tileset_viewport;
	GtkWidget *tilesetarea_box =
		ui_tilesetarea_create
			(&tileset_area, &tileset_viewport, global_data);
	GtkWidget *attribute_box =
		ui_attribute_buttons_create(global_data);
	GtkWidget *statusbar = gtk_statusbar_new();

	global_data->statusbar_context_id =
		gtk_statusbar_get_context_id
			(GTK_STATUSBAR(statusbar), "");

	/** [hackery] **/
	gtk_box_pack_start(GTK_BOX(attribute_box),
		gtk_button_new_with_label("DummyAttribute"), TRUE, TRUE, 8);
	gtk_box_pack_start(GTK_BOX(attribute_box),
		gtk_button_new_with_label("SecondAttribute"), TRUE, TRUE, 8);
	gtk_box_pack_start(GTK_BOX(attribute_box),
		gtk_button_new_with_label("ThirdAttribute"), TRUE, TRUE, 8);
	gtk_box_pack_start(GTK_BOX(attribute_box),
		gtk_button_new_with_label("Munyamunamisha"), TRUE, TRUE, 8);
	/** [/hackery] **/

	gtk_frame_set_shadow_type(GTK_FRAME(tileset_frame), GTK_SHADOW_IN);
	gtk_container_add(GTK_CONTAINER(tileset_frame), tilesetarea_box);

	gtk_box_pack_start
		(GTK_BOX(workspace_box), tileset_frame, TRUE, TRUE, 8);
	gtk_box_pack_start
		(GTK_BOX(workspace_box), separator, FALSE, FALSE, 8);
	gtk_box_pack_start
		(GTK_BOX(workspace_box), attribute_box, FALSE, FALSE, 8);

	gtk_container_add(GTK_CONTAINER(window), mainbox);

	g_signal_connect (window, "destroy",
			G_CALLBACK (gtk_widget_destroyed), &window);
	g_signal_connect (window, "delete-event",
			G_CALLBACK (cb_window_delete), global_data);
	g_signal_connect (window, "configure-event",
			G_CALLBACK (cb_window_configure), global_data);

	struct MainWindow *main_window =
		g_malloc( sizeof( struct MainWindow ) );
	main_window->window = window;
	main_window->tileset_area = tileset_area;
	main_window->tileset_viewport =
	main_window->statusbar = statusbar;
	main_window->workspace_box = workspace_box;
	main_window->attr_button_box = attribute_box;
	main_window->workspace_separator = separator;
	global_data->main_window = main_window;

	gtk_widget_show(tileset_frame);
	gtk_widget_show(separator);
	gtk_widget_show(attribute_box);
	gtk_widget_show(workspace_box);
	gtk_widget_show(statusbar);
	if (global_data->tileset)
	{
		gtk_widget_show(tileset_area);
		tileset_area_update_viewport(global_data);
	}

	menubar_box = ui_menubar_create(window, global_data);
	gtk_widget_show_all(menubar_box);

	gtk_box_pack_start(GTK_BOX(mainbox), menubar_box, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(mainbox), workspace_box, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(mainbox), statusbar, FALSE, FALSE, 0);

	gtk_widget_show(mainbox);

	global_data->active_attribute =
		global_data->tile_attributes
			[global_data->settings->active_attr_id];

	if (global_data->tile_attributes
			[global_data->settings->active_attr_id]->enabled)
	{
		gtk_toggle_button_set_active
			(GTK_TOGGLE_BUTTON( global_data->active_attribute->button ),
			 TRUE);
	}
	else
	{
		activate_other_attribute(global_data);
	}
}


void attr_button_box_set_expand
( gpointer _global_data, gboolean expand )
{
	struct GlobalData *global_data =
		(struct GlobalData*)_global_data;
	struct TileAttribute **tile_attr;
	for (tile_attr=global_data->tile_attributes;
	     *tile_attr; tile_attr++)
	{
		gtk_box_set_child_packing
			(GTK_BOX(global_data->main_window->attr_button_box),
			 (*tile_attr)->button, expand, expand, 8, GTK_PACK_START);
	}
}

void workspace_box_flip_packing
( GtkWidget *workspace_box )
{
	GList *children =
		gtk_container_get_children(GTK_CONTAINER(workspace_box));

	gint i;

	for (i=0; children; children = children->next)
	{
		gtk_box_reorder_child
			(GTK_BOX(workspace_box),
			 GTK_WIDGET(children->data), 2-i);
		i++;
	}

	g_free(children);
}

gchar *find_image_file_attempt
( GtkWidget *parent, const gchar *bad_path )
{
	gchar *image_file = NULL;

	GtkWidget *file_dialog =
		gtk_file_chooser_dialog_new
			("Find tileset image", NULL,
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

	gchar message[] =
		"The tileset image specified in the file you are trying to open\n"
		"(<b>%s</b>) cannot be found. Do you want to search for it manually?";

	gchar *image_filename =
		get_filename_from_path(bad_path, DELIMITER);

	GtkWidget *question_dialog =
		gtk_message_dialog_new_with_markup
			(GTK_WINDOW(parent), GTK_DIALOG_DESTROY_WITH_PARENT,
			 GTK_MESSAGE_INFO, GTK_BUTTONS_YES_NO,
			 message, image_filename);

	g_free(image_filename);

	switch (gtk_dialog_run(GTK_DIALOG(question_dialog)))
	{
		case GTK_RESPONSE_YES          : goto search_file;

		case GTK_RESPONSE_NO           :
		case GTK_RESPONSE_DELETE_EVENT : goto cancel_search;
	}

search_file:

	switch (gtk_dialog_run(GTK_DIALOG(file_dialog)))
	{
		case GTK_RESPONSE_ACCEPT       : goto file_found;

		case GTK_RESPONSE_CANCEL       :
		case GTK_RESPONSE_DELETE_EVENT : goto cancel_search;

	}

file_found:

	image_file =
		gtk_file_chooser_get_filename
			(GTK_FILE_CHOOSER(file_dialog));

cancel_search:

	gtk_widget_destroy(file_dialog);
	gtk_widget_destroy(question_dialog);

	return image_file;

}


static void ui_update_window_title
( struct GlobalData *global_data )
{
	if (!global_data->main_window) { return; }

	GString *title = g_string_new('\0');

	if (!global_data->open_file)
		{ g_string_append(title, "TileAttEditor"); goto set_title; }

	if (global_data->buffer_changed)
		{ g_string_append_c(title, '*'); }

	gchar *filename;
	if (global_data->open_file_path)
	{
		filename = get_filename_from_path
			(global_data->open_file_path, DELIMITER);
	}
	else { filename = g_strdup("untitled.tsx"); }

	g_string_append(title, filename);
	g_free(filename);
	g_string_append(title, " - TileAttEditor");

set_title:
	gtk_window_set_title
		(GTK_WINDOW(global_data->main_window->window), title->str);
	g_string_free(title, TRUE);
}


void ui_set_buffer_changed
( gpointer _global_data, gboolean buffer_changed )
{
	CAST_GLOBAL_DATA_PTR(_global_data);
	global_data->buffer_changed = buffer_changed;
	ui_update_window_title(global_data);
}

void ui_set_open_file_path
( gpointer _global_data, const gchar *open_file_path )
{
	CAST_GLOBAL_DATA_PTR(_global_data);
	global_data->open_file_path = g_strdup(open_file_path);
	ui_update_window_title(global_data);
}


