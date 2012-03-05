/*
 * tileset-area.h
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



void tileset_update_scale
( struct GlobalData *global_data );

gboolean tileset_create_from_file
( struct GlobalData *global_data, gchar *tileset_file,
  gint tile_width, gint tile_height );

void tileset_destroy
( struct Tileset *tileset );

void tileset_area_update_viewport
( struct GlobalData *global_data );

void tileset_area_redraw_cache
( struct GlobalData *global_data );

void tileset_area_redraw_cache_tile
( struct GlobalData *global_data, gint tile_id );

void tileset_area_queue_tile_redraw
( struct GlobalData *global_data, gint n_tiles, ... );
