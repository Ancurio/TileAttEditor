
#include <glib.h>
#include <cairo.h>

#include "tileatteditor.h"

void tile_attr_set_primary_color
( cairo_t *cr )
{
	cairo_set_source_rgb(cr, 1, 1, 1);
}

void tile_attr_set_secondary_color
( cairo_t *cr )
{
	cairo_set_source_rgb(cr, 0, 0, 0);
}

//gint* tile_attr_create_value_buffer
//( gint buffer_size )
//{
	//return malloc(sizeof(gint) * buffer_size);
//}

struct TileAttribute** tile_attr_create
( gpointer global_data )
{
	struct TileAttribute **tile_attr;
	tile_attr =
		g_malloc(sizeof(struct TileAttribute*) * ATTRIBUTE_COUNT+1);

	tile_attr[0] = attr_passability_create();
	tile_attr[1] = attr_bushflag_create();
	tile_attr[2] = attr_dummility_create();
	tile_attr[ATTRIBUTE_COUNT] = NULL;

	//tile_attr[] =
	//{
		//attr_passability_create(),
		//attr_bushflag_create(),
		//attr_dummility_create(),
		//NULL
	//};

	gint i; for (i=0;tile_attr[i];i++)
		{ tile_attr[i]->global_data = global_data; }

	return tile_attr;
}
