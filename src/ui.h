/*
 * ui.h
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



#define T_AREA_MIN_WIDTH  0x80
#define T_AREA_MIN_HEIGHT 0x100


struct MainWindow
{
	GtkWidget *window;
	GtkWidget *tileset_area;
	GtkWidget *tileset_viewport;
	GtkWidget *tileset_frame;
	GtkWidget *statusbar;
	GtkWidget *workspace_box;
	GtkWidget *attr_button_box;
	GtkWidget *workspace_separator;

	GtkActionGroup *action_group;

};

void ui_main_window_create
( gpointer _global_data );

void ui_main_window_destroy
( struct MainWindow *main_window );

void ui_attr_button_box_set_expand
( gpointer _global_data, gboolean expand );

void ui_attr_button_set_show_label
( gpointer _global_data, gboolean show );

void ui_workspace_box_flip_packing
( GtkWidget *workspace_box );

gchar *find_image_file_attempt
( GtkWidget *parent, const gchar *bad_path );

void ui_set_buffer_changed
( gpointer _global_data, gboolean buffer_changed );

void ui_set_open_file_path
( gpointer _global_data, const gchar *open_file_path );

void ui_update_tileset_frame
( gpointer _global_data );

void ui_filemenu_set_action_save_sensitivity
( struct MainWindow *main_window, gboolean sensitive );

void ui_filemenu_set_action_sensitivity
( struct MainWindow *main_window, gboolean sensitive );
