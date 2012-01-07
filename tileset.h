


struct Tileset
{
	gint width;
	gint height;
	gint disp_width;
	gint disp_height;

	gint tile_width;
	gint tile_height;
	gint tile_count;

	gdouble tile_disp_width;
	gdouble tile_disp_height;

	gchar *image_file;
	cairo_surface_t *cairo_surface;
	cairo_surface_t *cairo_scaled_surface;
	cairo_surface_t *cached_composition;
};

