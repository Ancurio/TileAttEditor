
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

//	xmlSaveCtxt *save_context;	/* (placeholder) */

	gchar *image_filename_abs;	/* file_create/open */
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
( gchar *image_filename, gint tile_width, gint tile_height );

struct File* file_open
( gchar *filename, enum ErrorFileOpen *error );

gboolean file_check
( struct File *file, enum ErrorFileParse *error );

gboolean file_parse  /* parses doc, sets variables and inits tileset */
( struct GlobalData *global_data, struct File *file );

gboolean file_save /* updates csv strings and dumps doc to file */
( struct GlobalData *global_data, gchar *filename );

void file_destroy
( struct File *file );

gboolean file_close /* frees all file related data */
( struct GlobalData *global_data );


void file_open_attempt_noerror
( struct GlobalData *global_data, gchar *filename );


