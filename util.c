
#include "tileatteditor.h"
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

void activate_other_attribute
( struct GlobalData *global_data )
{
	gint i = 0;
	struct TileAttribute **attr;
	for (attr=global_data->tile_attributes; *attr; attr++)
	{
		if ((*attr)->enabled)
		{
//			toggle_button_disable_signal(*attr);
			gtk_toggle_button_set_active
				(GTK_TOGGLE_BUTTON((*attr)->button), TRUE);
//			toggle_button_enable_signal(*attr);
			return;
		}
	}
	/* no enabled attribute left */
	toggle_button_disable_signal(global_data->active_attribute);
	gtk_toggle_button_set_active
		(GTK_TOGGLE_BUTTON(global_data->active_attribute->button),
		 FALSE);
	toggle_button_enable_signal(global_data->active_attribute);
	global_data->active_attribute = 0;
	/* A normal switch of active attribute would trigger a redraw,
	 * but if we set it to 0 this has to be done manually */
	tileset_area_redraw_cache(global_data);
	gtk_widget_queue_draw(global_data->main_window->tileset_area);
}

gint tile_attr_find_id
( struct TileAttribute *tile_attr )
{
	CAST_GLOBAL_DATA_PTR(tile_attr->global_data)
	gint i;
	for (i=0; global_data->tile_attributes[i] &&
	          global_data->tile_attributes[i] != tile_attr;
	     i++)
	{}

	if (global_data->tile_attributes[i]) { return i;  }
	else                                 { return -1; }


}

gchar* get_filename_from_path
( const gchar *path, gchar dlm )
{
	if (!path) { return; }

	gint read_head, last_dlm, cp_count;

	for (read_head=0; path[read_head]; read_head++)
	{
		cp_count++;
		if (path[read_head] == dlm)
		{
			last_dlm = read_head;
			cp_count = 0;
		}
	}

	gchar *filename = g_malloc(sizeof(gchar)*cp_count);

	gint write_head;

	for (write_head=0; path[last_dlm+write_head+1]; write_head++)
	{
		filename[write_head] = path[last_dlm+write_head+1];
	}
	filename[write_head] = '\0';

	return filename;
}








