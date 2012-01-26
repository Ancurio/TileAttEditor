/*
 * file.h
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


#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlsave.h>

#define ERROR(error_type, ret_value)    \
	if (error)	                        \
		{ *error = error_type; }		\
	return (ret_value)




struct File						/* member set by:   */
{
	xmlDoc *doc;				/* file_create/open */

	xmlNode *root_node;			/* file_create/open */
	xmlNode *image_node;		/* file_create/open */

	xmlNode **attr_nodes;		/* file_parse       */

	gint min_buffer_size;		/* file_parse       */
	gchar *tileset_name;		/* file_parse       */

//	xmlSaveCtxt *save_context;	/* (placeholder)    */

	gchar *image_filename_abs;	/* file_create/open */
	gboolean image_path_found;	/* file_check       */
};

enum ErrorFileOpen
{
	NONEXISTANT_FILE,	/* Error: document doesn't exist */
	DOCUMENT_MALFORMED,	/* Error: malformed document */
	DOCUMENT_EMPTY,		/* Error: empty document(?) */
	NOT_TILESET_FILE,	/* Error: not a tileset file */
	NO_TILE_SIZE_PROPS,	/* Error: no props tile* */
	NO_IMAGE_NODE,		/* Error: no image child node */
	NO_IMAGE_SOURCE		/* Error: no image source attribute */
};

enum ErrorFileParse
{
	NO_FILE,
	BAD_TILE_SIZES,		/* Error: bad tile* values */
	BAD_IMAGE_FILE		/* Error: could not create cairo surface from file */
};


gboolean file_attribute_enable
( struct File *file, struct TileAttribute *tile_attr, gint attr_id );


struct File* file_create
( gchar *image_filename, gint tile_width, gint tile_height,
  gchar *tileset_name );

struct File* file_open
( const gchar *filename, enum ErrorFileOpen *error );

gboolean file_check
( struct GlobalData *global_data, struct File *file,
  enum ErrorFileParse *error );

gboolean file_parse  /* parses doc, sets variables and inits tileset */
( struct GlobalData *global_data, struct File *file );

gboolean file_save /* updates csv strings and dumps doc to file */
( struct GlobalData *global_data, gchar *filename );

void file_destroy
( struct File *file );

gboolean file_close /* frees all file related data */
( struct GlobalData *global_data );


void file_open_attempt_quiet
( struct GlobalData *global_data, const gchar *filename );

