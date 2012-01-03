
#include <stdlib.h>
#include <gtk/gtk.h>

#include "tileatteditor.h"
#include "callback.h"
#include "ui-menubar.xml"


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
	g_signal_connect_after( G_OBJECT (tileset_area), "scroll-event",
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

	viewport =
		gtk_container_get_children(GTK_CONTAINER(scrollarea))->data;
	gtk_viewport_set_shadow_type
		(GTK_VIEWPORT(viewport), GTK_SHADOW_NONE);
	gtk_widget_set_size_request
		(scrollarea, T_AREA_MIN_WIDTH+0x20, T_AREA_MIN_HEIGHT+0x20);

	*_tileset_area = tileset_area;
	*_tileset_viewport = viewport;
	return scrollarea;
}

static GtkWidget* ui_attribute_buttons_create
( struct GlobalData *global_data )
{
	struct TileAttribute **tile_attr = global_data->tile_attributes;

	GtkWidget *attr_box = gtk_vbox_new(FALSE, 8);

	gint i;
	for (i=0;tile_attr[i];i++)
	{
		GtkWidget *attr_button =
			gtk_toggle_button_new_with_label(tile_attr[i]->name);
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

static void destroy( GtkWidget *widget,
                     gpointer   data )
{
    gtk_main_quit ();
}

void ui_main_window_create
( struct GlobalData *global_data )
{
	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(window), "TileAttEditor");

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

	global_data->hover_context_id =
		gtk_statusbar_get_context_id
			(GTK_STATUSBAR(statusbar), "Hover information");

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
			G_CALLBACK (destroy), NULL);

	struct MainWindow *main_window =
		malloc( sizeof( struct MainWindow ) );
	main_window->window = window;
	main_window->tileset_area = tileset_area;
	main_window->tileset_viewport =
	main_window->statusbar = statusbar;
	main_window->workspace_box = workspace_box;
	main_window->attr_button_box = attribute_box;
	main_window->workspace_separator = separator;
	global_data->main_window = main_window;

	gtk_widget_show_all(tileset_frame);
	gtk_widget_show(separator);
	gtk_widget_show(attribute_box);
	gtk_widget_show(workspace_box);
	gtk_widget_show(statusbar);

	menubar_box = ui_menubar_create(window, global_data);
	gtk_widget_show_all(menubar_box);

	gtk_box_pack_start(GTK_BOX(mainbox), menubar_box, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(mainbox), workspace_box, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(mainbox), statusbar, FALSE, FALSE, 0);

	gtk_widget_show(mainbox);
//GtkAllocation alloc; gtk_widget_get_allocation(statusbar, &alloc);
//g_message("statusbar alloc: %d | %d", alloc.width, alloc.height);
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













