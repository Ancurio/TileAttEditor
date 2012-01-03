
#include <stdlib.h>
#include <glib.h>
#include <cairo.h>

#include "tileatteditor.h"

static cairo_surface_t *scale_surface
( cairo_surface_t *old_surface, gdouble scale_x, gdouble scale_y, gboolean smooth )
{
	gint old_width = cairo_image_surface_get_width(old_surface);
	gint old_height = cairo_image_surface_get_height(old_surface);

	cairo_surface_t *new_surface = cairo_surface_create_similar
		(old_surface, CAIRO_CONTENT_COLOR_ALPHA,
		(gint)(old_width*scale_x), (gint)(old_height*scale_y));
	cairo_t *cr = cairo_create (new_surface);

	cairo_scale (cr, scale_x, scale_y);
	cairo_set_source_surface (cr, old_surface, 0, 0);

//	cairo_pattern_set_extend (cairo_get_source(cr), CAIRO_EXTEND_REFLECT);
	if (!smooth)
	{
		cairo_pattern_set_filter
			(cairo_get_source(cr), CAIRO_FILTER_NEAREST);
	}
	cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint (cr);
	cairo_destroy (cr);

	return new_surface;
}

static void clear_surface
( cairo_t *cr )
{
	cairo_save(cr);
	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	cairo_set_source_rgba(cr, 0, 0, 0, 0);
	cairo_paint(cr);
	cairo_restore(cr);
}

void tileset_update_scale
( struct GlobalData *global_data )
{
	gdouble scale_ratio = global_data->settings->tileset_scale_ratio;
	struct Tileset *tileset = global_data->tileset;

	if (!tileset) { return; }

	if (tileset->cairo_scaled_surface)
		{ cairo_surface_destroy(tileset->cairo_scaled_surface); }
	tileset->cairo_scaled_surface =
		scale_surface(tileset->cairo_surface,
			scale_ratio, scale_ratio,
			global_data->settings->smooth_zoom);
	tileset->disp_width = (gint)(tileset->width * scale_ratio);
	tileset->disp_height = (gint)(tileset->height * scale_ratio);
	tileset->tile_disp_width =
		(gdouble)tileset->disp_width/(tileset->width/tileset->tile_width),
	tileset->tile_disp_height =
		(gdouble)tileset->disp_height/(tileset->height/tileset->tile_height);

	if (tileset->cached_composition)
		{ cairo_surface_destroy(tileset->cached_composition); }
	tileset->cached_composition = cairo_image_surface_create
		(CAIRO_FORMAT_ARGB32, tileset->disp_width, tileset->disp_height);
}

gboolean tileset_create_from_file
( struct GlobalData *global_data, gchar *tileset_file,
  gint tile_width, gint tile_height )
{
	cairo_surface_t *surface =
		cairo_image_surface_create_from_png(tileset_file);

	if (cairo_surface_status(surface)
	    != CAIRO_STATUS_SUCCESS)   { return FALSE; }

	if (!global_data->tileset)
	{
		global_data->tileset =
			g_malloc( sizeof (struct Tileset) );
		global_data->tileset->cairo_surface = 0;
	}

	struct Tileset *tileset = global_data->tileset;

	gdouble scale_ratio =
		global_data->settings->tileset_scale_ratio;

	if (tileset->cairo_surface)
		{ cairo_surface_destroy(tileset->cairo_surface); }

	tileset->cairo_surface = surface;
	tileset->cairo_scaled_surface = 0;
	tileset->cached_composition = 0;
	tileset->image_file = g_strdup(tileset_file);
	tileset->width = cairo_image_surface_get_width(surface);
	tileset->height = cairo_image_surface_get_height(surface);

	tileset->tile_width = tile_width;
	tileset->tile_height = tile_height;
	tileset->tile_count =
		(tileset->width/tile_width) * (tileset->height/tile_height);
	global_data->tileset = tileset;
	tileset_update_scale(global_data);

	return TRUE;

}

void tileset_destroy
( struct GlobalData *global_data )
{
	struct Tileset *tileset =
		global_data->tileset;

	if (!tileset) { return; }

	if (tileset->image_file)
		{ g_free(tileset->image_file); }

	if (tileset->cairo_surface)
		{ cairo_surface_destroy(tileset->cairo_surface); }

	if (tileset->cairo_scaled_surface)
		{ cairo_surface_destroy(tileset->cairo_scaled_surface); }

	if (tileset->cached_composition)
		{ cairo_surface_destroy(tileset->cached_composition); }

	g_free(tileset);
	global_data->tileset = NULL;
}

void tileset_area_redraw_cache
( struct GlobalData *global_data )
{
	struct Tileset *tileset = global_data->tileset;
	struct Settings *settings = global_data->settings;

	if (!tileset) { return; }

	cairo_t *cr = cairo_create(tileset->cached_composition);
	if (!cr) {return;}
	clear_surface(cr);

	/* paint background color */
	color_cairo_set_source(cr, settings->bg_color);
	cairo_paint(cr);

	/* paint (scaled) tileset */
	cairo_set_source_surface(cr, tileset->cairo_scaled_surface, 0, 0);
	cairo_paint(cr);

	/* paint frame and grid */
	color_cairo_set_source(cr, settings->grid_color);
	cairo_rectangle(cr, 0, 0, tileset->disp_width, tileset->disp_height);
	gint tile_count_x = tileset->width/tileset->tile_width,
	     tile_count_y = tileset->height/tileset->tile_height,
	     i, j;

	/* vertical lines */
	for (i=1;i<tile_count_x;i++)
	{
		cairo_move_to
			(cr, i*tileset->tile_disp_width, 0);
		cairo_line_to
			(cr, i*tileset->tile_disp_width, tileset->disp_height);
	}
	/* horizontal lines */
	for (i=1;i<tile_count_y;i++)
	{
		cairo_move_to
			(cr, 0, i*tileset->tile_disp_height);
		cairo_line_to
			(cr,tileset->disp_width, i*tileset->tile_disp_height);
	}
	cairo_set_line_width(cr, 2);
	cairo_stroke(cr);

	/* paint attribute symbols */
	if (!global_data->active_attribute) {goto skip_attr;}
	cairo_surface_t *tile_surf = cairo_image_surface_create
		(CAIRO_FORMAT_ARGB32,
		 (gint)tileset->tile_disp_width,
		 (gint)tileset->tile_disp_height);
	cairo_t *tile_cr = cairo_create(tile_surf);
	cairo_scale(tile_cr,
		tileset->tile_disp_width,
		tileset->tile_disp_height);
	for (i=0;i<tile_count_y;i++)
	{
		for (j=0;j<tile_count_x;j++)
		{
			(*global_data->active_attribute->draw_attr)
				(global_data->active_attribute->
				 value_buffer[i*tile_count_x + j], tile_cr,
				 global_data->hovered_tile == (i*tile_count_x + j),
				 global_data->hovered_offset_x,
				 global_data->hovered_offset_y);
			cairo_set_source_surface
				(cr, tile_surf,
				 j*tileset->tile_disp_width,
				 i*tileset->tile_disp_height);
			cairo_paint_with_alpha(cr, settings->attribute_alpha);
			clear_surface(tile_cr);
		}
	}
	cairo_destroy(tile_cr);
skip_attr:
	cairo_destroy(cr);
}

void tileset_area_redraw_cache_tile
( struct GlobalData *global_data, gint tile_id )
{
	struct Tileset *tileset = global_data->tileset;

	if (!tileset) { return; }

	gint tile_x =
		tile_id % (tileset->width/tileset->tile_width);
	gint tile_y =
		tile_id / (tileset->width/tileset->tile_width);

	cairo_surface_t *patch_surf =
		cairo_image_surface_create
			(CAIRO_FORMAT_ARGB32,
			 (gint)tileset->tile_disp_width,
			 (gint)tileset->tile_disp_height);
	cairo_t *cr = cairo_create(patch_surf);

	color_cairo_set_source(cr, global_data->settings->bg_color);
	cairo_paint(cr);

	cairo_set_source_surface
		(cr, tileset->cairo_scaled_surface,
		 -(gint)(tile_x * tileset->tile_disp_width),
		 -(gint)(tile_y * tileset->tile_disp_height));
	cairo_paint(cr);

	gdouble grid_offset_x =
		(tile_x*tileset->tile_disp_width)
		 - (gint)(tile_x*tileset->tile_disp_width);
	gdouble grid_offset_y =
		(tile_y*tileset->tile_disp_height)
		 - (gint)(tile_y*tileset->tile_disp_height);
	color_cairo_set_source(cr, global_data->settings->grid_color);
	cairo_set_line_width(cr, 2);

	cairo_rectangle(cr, grid_offset_x, grid_offset_y,
		grid_offset_x+tileset->tile_disp_width,
		grid_offset_y+tileset->tile_disp_height);
	cairo_stroke(cr);

	if (!global_data->active_attribute) {goto skip_attr;}

	cairo_surface_t *attr_surf =
		cairo_image_surface_create
			(CAIRO_FORMAT_ARGB32,
			 (gint)tileset->tile_disp_width,
			 (gint)tileset->tile_disp_height);
	cairo_t *attr_cr = cairo_create(attr_surf);
	cairo_scale(attr_cr,
		tileset->tile_disp_width, tileset->tile_disp_height);
	(*global_data->active_attribute->draw_attr)
		(global_data->active_attribute->value_buffer[tile_id],
		 attr_cr,
		 global_data->hovered_tile == tile_id,
		 global_data->hovered_offset_x,
		 global_data->hovered_offset_y);
	cairo_destroy(attr_cr);
	cairo_set_source_surface
		(cr, attr_surf, grid_offset_x, grid_offset_y);
	cairo_paint_with_alpha
		(cr, global_data->settings->attribute_alpha);
	cairo_surface_destroy(attr_surf);
skip_attr:
	cairo_destroy(cr);
	cr = cairo_create(tileset->cached_composition);
	cairo_set_source_surface(cr, patch_surf,
		(gint)(tile_x * tileset->tile_disp_width),
		(gint)(tile_y * tileset->tile_disp_height));
	cairo_paint(cr);

	cairo_destroy(cr);
	cairo_surface_destroy(patch_surf);

}

void tileset_area_queue_tile_redraw
( struct GlobalData *global_data, gint n_tiles, ... )
{
	struct Tileset *tileset = global_data->tileset;

	GdkRectangle master_rect;

	va_list tile_ids;
	va_start(tile_ids, n_tiles);
	gint tile_id, i;

	for (i=0;i<n_tiles;i++)
	{
		tile_id = va_arg(tile_ids, gint);
		if (tile_id < 0) {continue;}
		//g_message("Invalidating tile %d", tile_id);
		gint tile_x =
			tile_id % (tileset->width/tileset->tile_width);
		gint tile_y =
			tile_id / (tileset->width/tileset->tile_width);

		GdkRectangle rect =
			{(gint)(tile_x * tileset->tile_disp_width),
			 (gint)(tile_y * tileset->tile_disp_height),
			 (gint)tileset->tile_disp_width,
			 (gint)tileset->tile_disp_height};

		gdk_rectangle_union(&rect, &master_rect, &master_rect);
	}

	va_end(tile_ids);

	GdkRegion *region = gdk_region_rectangle(&master_rect);
	gdk_window_invalidate_region
		(gtk_widget_get_window(global_data->main_window->tileset_area),
		 region, FALSE);

	gdk_region_destroy(region);
}

//void tileset_area_queue_tile_redraw
//( struct GlobalData *global_data, gint tile_id )
//{
	//struct Tileset *tileset = global_data->tileset;

	//gint tile_x =
			//tile_id % (tileset->width/tileset->tile_width);
	//gint tile_y =
		//tile_id / (tileset->width/tileset->tile_width);

	//gtk_widget_queue_draw_area
		//(global_data->main_window->tileset_area,
		 //(gint)(tile_x * tileset->tile_disp_width),
		 //(gint)(tile_y * tileset->tile_disp_height),
		 //(gint)tileset->tile_disp_width,
		 //(gint)tileset->tile_disp_height);
//}
