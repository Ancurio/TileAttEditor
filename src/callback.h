/*
 * callback.h
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



void cb_filemenu_new
( GtkAction *action, gpointer data );
void cb_filemenu_open
( GtkAction *action, gpointer data );
void cb_filemenu_save
( GtkAction *action, gpointer data );
void cb_filemenu_close
( GtkAction *action, gpointer data );
void cb_filemenu_quit
( GtkAction *action, gpointer data );
void cb_editmenu_preferences
( GtkAction *action, gpointer data );
void cb_editmenu_flip
( GtkAction *action, gpointer data );

void cb_attr_icon_expose
( GtkWidget *icon, cairo_t *cr, gpointer data );
void cb_attr_button_toggled
( GtkWidget *button, gpointer data );

gboolean cb_window_delete
( GtkWidget *widget, GdkEvent *event, gpointer data );
gboolean cb_window_configure
( GtkWidget *widget, GdkEventConfigure *event, gpointer data );
gboolean cb_tileset_area_expose
( GtkWidget *widget, cairo_t *cr, gpointer data );
gboolean cb_tileset_area_button_press
( GtkWidget *widget, GdkEventButton *event, gpointer data );
gboolean cb_tileset_area_motion_notify
( GtkWidget *widget, GdkEvent *b_event, gpointer data );
gboolean cb_tileset_area_leave_notify
( GtkWidget *widget, GdkEventButton *b_event, gpointer data );
gboolean cb_tileset_area_drag_data_received
( GtkWidget *widget, GdkDragContext *context,
  gint x, gint y, GtkSelectionData *sdata,
  guint info, guint timestamp, gpointer data);
