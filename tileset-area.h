

void tileset_update_scale
( struct GlobalData *global_data );

gboolean tileset_create_from_file
( struct GlobalData *global_data, gchar *tileset_file,
  gint tile_width, gint tile_height );

void tileset_area_update_viewport
( struct GlobalData *global_data );

void tileset_area_redraw_cache
( struct GlobalData *global_data );

void tileset_area_queue_tile_redraw
( struct GlobalData *global_data, gint n_tiles, ... );

