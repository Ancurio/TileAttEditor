/*
 * util.c
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


#include "tileatteditor.h"
#include "tileset-area.h"
#include "callback.h"
#include "util.h"


void toggle_button_disable_signal
( struct TileAttribute *tile_attr )
{
	g_signal_handler_disconnect
		(tile_attr->button, tile_attr->signal_handler_id);
	tile_attr->signal_handler_id = 0;
}

void toggle_button_enable_signal
( struct TileAttribute *tile_attr )
{
	tile_attr->signal_handler_id =
		g_signal_connect(tile_attr->button, "toggled",
			G_CALLBACK( cb_attr_button_toggled ), tile_attr);
}

void gtk_size_group_add_widgets
( GtkSizeGroup *size_group, ... )
{
	va_list widgets;
	va_start(widgets, size_group);
	GtkWidget *widget;

	for (widget=va_arg(widgets,GtkWidget*);
	     widget;
	     widget=va_arg(widgets,GtkWidget*))
	{
		gtk_size_group_add_widget(size_group, widget);
	}

	va_end(widgets);
}

void activate_zero_attribute
( struct GlobalData *global_data )
{
	if (!global_data->active_attribute) { return; }
	toggle_button_disable_signal(global_data->active_attribute);
	gtk_toggle_button_set_active
		(GTK_TOGGLE_BUTTON(global_data->active_attribute->button),
		 FALSE);
	toggle_button_enable_signal(global_data->active_attribute);
	global_data->active_attribute = NULL;
	/* A normal switch of active attribute would trigger a redraw,
	 * but if we set it to 0 this has to be done manually */
	tileset_area_redraw_cache(global_data);
	gtk_widget_queue_draw(global_data->main_window->tileset_area);
}

void activate_other_attribute
( struct GlobalData *global_data )
{
	gint i;
	for (i = 0; i < attr_store_n; i++)
	{
		if (attr_store[i]->enabled)
		{
//			toggle_button_disable_signal(*attr);
			gtk_toggle_button_set_active
				(GTK_TOGGLE_BUTTON(attr_store[i]->button), TRUE);
//			toggle_button_enable_signal(*attr);
			return;
		}
	}
	/* no enabled attribute left */
	activate_zero_attribute(global_data);
}

gint tile_attr_find_id
( const struct TileAttribute *tile_attr )
{
	CAST_GLOBAL_DATA_PTR(tile_attr->global_data)
	gint i;
	for (i=0; i < attr_store_n && attr_store[i] != tile_attr; i++)
	{}

	if (attr_store[i]) { return i;  }
	else               { return -1; }
}

