/*
 * file.c
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


#include <cairo.h>

#include "tileatteditor.h"
#include "tileset-area.h"
#include "file.h"

#define TILE_ATTR_STRING "tileattribute"


/* private functions */
static xmlChar* xml_get_attribute_contents
( xmlNode *node, const xmlChar *name );

static xmlNode* xml_get_child_node
( xmlNode *parent, const xmlChar *name );

static xmlNode* xml_get_child_node_with_prop
( xmlNode *parent, const xmlChar *node_name,
  const xmlChar *prop_name, const xmlChar *prop_value );


static gint str_len
( const gchar *string );

static void str_clear
( gchar *str, gint len );

static void str_append_c
( gchar *str, gchar c );


static gchar* make_relative_path
( const gchar *base, const gchar *dest, gchar dlm );

static gchar* make_absolute_path
( const gchar *base, const gchar *rel_path, gchar dlm );


static guint csv_count_values
( const gchar *str );

static gchar* csv_create_string
( const gint *value_buffer, guint buffer_size, gint row_length );

static gint* csv_parse_string
( const gchar *str, guint *buffer_size,
 gint default_value, gint min_buffer_size );


static xmlNode* file_attribute_parse_node
( struct TileAttribute *tile_attr, xmlNode *root_node,
  gint min_buffer_size );
/* ----------------- */


static xmlChar* xml_get_attribute_contents
( xmlNode *node, const xmlChar *name )
{
	xmlAttr *attr;
	for (attr = node->properties; attr; attr = attr->next)
	{
		if (!xmlStrcmp(attr->name, name))
			{ return attr->children->content; }
	}
	return NULL;
}

static xmlNode* xml_get_child_node
( xmlNode *parent, const xmlChar *name )
{
	xmlNode *node;
	for (node = parent->children; node; node = node->next)
	{
		if (!xmlStrcmp(node->name, name)) { return node; }
	}
	return NULL;
}

static xmlNode* xml_get_child_node_with_prop
( xmlNode *parent, const xmlChar *node_name,
  const xmlChar *prop_name, const xmlChar *prop_value )
{
	xmlNode *node;
	for (node = parent->children; node; node = node->next)
	{
		if (!xmlStrcmp(node->name, node_name) &&
		    !xmlStrcmp(xml_get_attribute_contents(node, prop_name),
			           prop_value))
			{ return node; }
	}
	return NULL;
}


static gint str_len
( const gchar *string )
{
	gint i; for (i=0;string[i] != '\0';i++) { }
	return i;
}

static void str_clear
( gchar *str, gint len )
{
	gint i; for (i=0;i<len;i++) { str[i] = '\0'; }
}

static void str_append_c
( gchar *str, gchar c )
{
	gint i = 0;
	while (str[i]) {i++;}
	str[i] = c;
}


static gchar* make_relative_path
( const gchar *base, const gchar *dest, gchar dlm )
{
	if (!(base&&dest)) { return NULL; }	/* check if empty strings */

	const gchar *p;
	for (p=base;*(p+1);p++)				/* check for two consecutive dlms */
		{ if (*p == dlm && *(p+1) == dlm) { return NULL; } }
	for (p=dest;*(p+1);p++)
		{ if (*p == dlm && *(p+1) == dlm) { return NULL; } }

	gint i = 0, last_dlm;
	while ((base[i]&&dest[i])&&(base[i]==dest[i]))
	{
		if (base[i] == dlm) { last_dlm = i; }
		i++;
	}

	gint base_rem_dir_count = 0;
	for (p=base+last_dlm+1; *p; p++)
	{
		if (*p == dlm) { base_rem_dir_count++; }
	}

	gint dest_rem_char_count = 0;
	for (p=dest+last_dlm+1; *p; p++)
	{
		dest_rem_char_count++;
	}

	gchar *relative_path = g_malloc
		(sizeof(gchar)*((3*base_rem_dir_count)+dest_rem_char_count+1));

	int relative_path_pos = 0;
	for (i=0;i<base_rem_dir_count;i++)
	{
		relative_path[i*3] = '.';
		relative_path[i*3+1] = '.';
		relative_path[i*3+2] = '/';
	}
	relative_path_pos = i*3;
	for (i=0; dest[i+last_dlm+1]; i++)
	{
		relative_path[relative_path_pos+i] =
			dest[i+last_dlm+1] == dlm ? '/' : dest[i+last_dlm+1];
	}
	relative_path[relative_path_pos+i] = '\0';

	return relative_path;
}

static gchar* make_absolute_path
( const gchar *base, const gchar *rel_path, gchar dlm )
{
	if (!(base&&rel_path))  { return NULL; }	/* check if empty strings */

	gint dir_up_count = 0;
	gint rel_path_len = str_len(rel_path);

	while (TRUE)
	{
		if ((dir_up_count+1)*3 > rel_path_len)
			{ break; }

		if (!(rel_path[(dir_up_count*3)  ] == '.' &&
		      rel_path[(dir_up_count*3)+1] == '.' &&
		      rel_path[(dir_up_count*3)+2] == '/'    ))
			{ break; }

		dir_up_count++;
	}

	gint rel_path_rdhd = dir_up_count * 3;
	gint rel_path_cp_count = rel_path_len - rel_path_rdhd;

	gint base_len = str_len(base);

	gint i = 0;
	for (i = base_len; i > 0; i--)
	{
		if (base[i] == dlm)  { dir_up_count--; }
		if (dir_up_count < 0) { break; }
	}

	int base_cp_count = i + 1;

	gchar *absolute_path =
		g_malloc(sizeof(gchar)*(base_cp_count+rel_path_cp_count+1));

	for (i=0; i<base_cp_count; i++)
		{ absolute_path[i] = base[i]; }

	for (; rel_path[rel_path_rdhd]; i++)
	{
		absolute_path[i] = rel_path[rel_path_rdhd];
		rel_path_rdhd++;
	}
	absolute_path[i] = '\0';

	return absolute_path;
}

void g_realpath
( const gchar *name, gchar *resolved )
{
	gchar *current_dir = g_get_current_dir();
	gchar *dummy_base =
		g_strconcat(current_dir, G_DIR_SEPARATOR_S, "base", NULL);
	gchar *abs_filepath =
		make_absolute_path(dummy_base, name, G_DIR_SEPARATOR);

	g_stpcpy(resolved, abs_filepath);

	g_free(current_dir);
	g_free(dummy_base);
	g_free(abs_filepath);
}


static guint csv_count_values
( const gchar *str )
{
	gint i = 0;
	guint count = 0;
	while (str[i] != 0)  { if (str[i] == ',') {count++;} i++;}
	return count+1;
}

static gint* csv_parse_string
( const gchar *str, guint *buffer_size,
  gint default_value, gint min_buffer_size )
{
	*buffer_size = csv_count_values(str);
	if (*buffer_size < min_buffer_size)
		{ *buffer_size = min_buffer_size; }

	gint *val_buffer = g_malloc(sizeof(gint) * (*buffer_size));

	gchar c_buffer[16]; str_clear(c_buffer, 16);
	gboolean parsing_number = FALSE;
	gint val_count = 0;
	gint i; for (i=0; str[i]; i++)
	{
		if (g_ascii_isdigit(str[i]) || str[i] == '-')
		{
			if (parsing_number)
			{
				str_append_c(c_buffer, str[i]);
			}
			else
			{
				parsing_number = TRUE;
				c_buffer[0] = str[i];
			}
		}
		else
		{
			if (parsing_number)
			{
				parsing_number = FALSE;
				val_buffer[val_count] = g_ascii_strtod(c_buffer, NULL);
				val_count++;
				str_clear(c_buffer, 16);
			}
		}
	}

	for (; val_count<*buffer_size; val_count++)
	{
		val_buffer[val_count] = default_value;
	}
//	g_message("Successfully created buffer of size %d", *buffer_size);
	return val_buffer;
}

static gchar* csv_create_string
( const gint *value_buffer, guint buffer_size, gint row_length )
{
	GString *csv_string =
		g_string_sized_new
			((gsize)(buffer_size*2+(buffer_size/row_length)));
	g_string_append_c(csv_string, '\n');

	gchar buffer[16];
	gint i, row_watch = 0;
	for (i=0; i<buffer_size-1; i++)
	{
		g_snprintf(buffer, 16, "%d", value_buffer[i]);
		g_string_append(csv_string, buffer);
		g_string_append_c(csv_string, ',');

		row_watch++;
		if (row_watch >= row_length)
		{
			g_string_append_c(csv_string, '\n');
			row_watch = 0;
		}
	}
	g_snprintf(buffer, 16, "%d", value_buffer[i]);
	g_string_append(csv_string, buffer);

	g_string_append(csv_string, "\n  ");

	gchar *return_string = csv_string->str;
	g_string_free(csv_string, FALSE);

	return return_string;
}


static xmlNode* file_attribute_parse_node
( struct TileAttribute *tile_attr, xmlNode *root_node,
  gint min_buffer_size )
{
	gchar buffer[16];

	xmlNode *attr_node =
		xml_get_child_node_with_prop
			(root_node, TILE_ATTR_STRING,
			 "name", tile_attr->name);
	if (!attr_node)
		{
//			g_message("Attr [%s] not found. Creating...", tile_attr->name);
			attr_node = xmlNewNode(NULL, TILE_ATTR_STRING);
			xmlSetProp(attr_node, "name", tile_attr->name);
			g_snprintf(buffer, 16, "%d", tile_attr->default_value);
			xmlSetProp(attr_node, "defaultvalue", buffer);
			xmlAddChild(root_node, attr_node);
			xmlAddPrevSibling(attr_node, xmlNewText(" "));
			xmlAddNextSibling(attr_node, xmlNewText("\n"));
		}
		if (!xml_get_child_node_with_prop
				(attr_node, "data", "encoding", "csv"))
		{
//			g_message("Attr [%s] has no csv encoded data. Creating node..", tile_attr->name);
			xmlNode *data = xmlNewNode(NULL, "data");
			xmlSetProp(data, "encoding", "csv");
			xmlAddChild(data, xmlNewText(" "));
			xmlAddChild(attr_node, data);
			xmlAddPrevSibling(data, xmlNewText("\n  "));
			xmlAddNextSibling(data, xmlNewText("\n "));
		}

		tile_attr->value_buffer =
			csv_parse_string
				(xml_get_child_node_with_prop
					(attr_node, "data",
					"encoding", "csv")
					->children->content,
				 &tile_attr->buffer_size,
				 tile_attr->default_value,
				 min_buffer_size);

	return attr_node;
}

gboolean file_attribute_enable
( struct File *file, struct TileAttribute *tile_attr, gint attr_id )
{
	if (!file) { return FALSE; }

	if (file->attr_nodes[attr_id]) { return FALSE; }

	file->attr_nodes[attr_id] =
		file_attribute_parse_node
			(tile_attr,
			 file->root_node,
			 file->min_buffer_size);
	return TRUE;
}



struct File* file_create
( gchar *image_filename, gint tile_width, gint tile_height,
  gchar *tileset_name )
{

	xmlDoc *doc = xmlNewDoc("1.0");
	xmlNode *root_node = xmlNewNode(NULL, "tileset");
	xmlDocSetRootElement(doc, root_node);

	gchar buffer[8];

	g_snprintf(buffer, 8, "%d", tile_width);
	xmlSetProp(root_node, (xmlChar*)"tilewidth", buffer);

	g_snprintf(buffer, 8, "%d", tile_height);
	xmlSetProp(root_node, (xmlChar*)"tileheight", buffer);

	xmlSetProp(root_node, "name", (xmlChar*)tileset_name);

	xmlNode *image_node = xmlNewNode(NULL, "image");
	xmlAddChild(root_node, image_node);
	xmlAddPrevSibling(image_node, xmlNewText("\n "));
	xmlAddNextSibling(image_node, xmlNewText("\n"));


	struct File *file = g_malloc( sizeof *file );

	file->doc = doc;
	file->root_node = root_node;
	file->image_node = image_node;
	file->attr_nodes = 0;
	file->image_filename_abs = g_strdup(image_filename);

	return file;

}


struct File* file_open
( const gchar *filename, enum ErrorFileOpen *error )
{
	if (!g_file_test(filename, G_FILE_TEST_EXISTS))
		{ ERROR(NONEXISTANT_FILE, NULL); }

	xmlDoc *doc;
	xmlNode *root_node;

	doc = xmlParseFile(filename);
	if (!doc)  { ERROR(DOCUMENT_MALFORMED, NULL); }
	root_node = xmlDocGetRootElement(doc);
	if (!root_node)  { ERROR(DOCUMENT_EMPTY, NULL); }


	if (xmlStrcmp(root_node->name, "tileset"))
		{ ERROR(NOT_TILESET_FILE, NULL); }


	if (!xml_get_attribute_contents(root_node, "tilewidth") ||
	    !xml_get_attribute_contents(root_node, "tileheight")  )
		{ ERROR(NO_TILE_SIZE_PROPS, NULL); }

	xmlNode *image_node = xml_get_child_node(root_node, "image");
	if (!image_node) { ERROR(NO_IMAGE_NODE, NULL); }

	gchar *image_filename_rel =
		xml_get_attribute_contents(image_node, "source");
	if (!image_filename_rel) { ERROR(NO_IMAGE_SOURCE, NULL); }

	gchar *image_filename_abs =
		make_absolute_path(filename, image_filename_rel, DELIMITER);


	struct File *file = g_malloc( sizeof *file );

	file->doc = doc;
	file->root_node = root_node;
	file->image_node = image_node;
	file->attr_nodes = 0;
	file->image_filename_abs = image_filename_abs;
	file->image_path_found = FALSE;

	return file;

}


gboolean file_check
( struct GlobalData *global_data, struct File *file,
  enum ErrorFileParse *error )
{
	if (!file) { ERROR(NO_FILE, FALSE); }

	xmlNode *root_node = file->root_node;

	gint tile_width = (gint)g_ascii_strtod
		(xml_get_attribute_contents(root_node, "tilewidth"), NULL);

	gint tile_height = (gint)g_ascii_strtod
		(xml_get_attribute_contents(root_node, "tileheight"), NULL);

	if (tile_width < 1 || tile_height < 1)
		{ ERROR(BAD_TILE_SIZES, FALSE); }

	if (!g_file_test(file->image_filename_abs, G_FILE_TEST_EXISTS))
	{
		if (!global_data->main_window) { goto skip_filetest; }
		file->image_filename_abs =
			find_image_file_attempt
				(global_data->main_window->window,
				 file->image_filename_abs);
		if (file->image_filename_abs)
			{ file->image_path_found = TRUE; }
skip_filetest:
		if (!file->image_filename_abs) { ERROR(BAD_IMAGE_FILE, FALSE); }
	}

	cairo_surface_t *check =
		cairo_image_surface_create_from_png(file->image_filename_abs);

	if (cairo_surface_status(check) != CAIRO_STATUS_SUCCESS)
	{
		cairo_surface_destroy(check);
		ERROR(BAD_IMAGE_FILE, FALSE);
	}
	global_data->reusable_surface = check;

	return TRUE;
}


gboolean file_parse
( struct GlobalData *global_data, struct File *file )
{
	xmlNode *root_node = file->root_node;

	gint tile_width = (gint)g_ascii_strtod
		(xml_get_attribute_contents(root_node, "tilewidth"), NULL);

	gint tile_height = (gint)g_ascii_strtod
		(xml_get_attribute_contents(root_node, "tileheight"), NULL);

	gchar *tileset_name =
		xml_get_attribute_contents(root_node, "name");

	tileset_name = tileset_name ? tileset_name : "";

	tileset_create_from_file
		(global_data, file->image_filename_abs,
		 tile_width, tile_height);

	struct Tileset *tileset = global_data->tileset;

	file->min_buffer_size = tileset->tile_count;
	file->tileset_name = tileset_name;

	gchar buffer[16];

	if (xml_get_attribute_contents(root_node, "width"))
	{
		if (tileset->width/tile_width !=
				(gint)g_ascii_strtod
					(xml_get_attribute_contents
						(root_node, "width"), NULL))
			{ g_warning("different tileset width"); }
	}
	else
	{
		g_snprintf(buffer, 16, "%d", tileset->width/tile_width);
		xmlSetProp(root_node, "width", buffer);
	}

	if (xml_get_attribute_contents(root_node, "height"))
	{
		if (tileset->height/tile_height !=
				(gint)g_ascii_strtod
					(xml_get_attribute_contents
						(root_node, "height"), NULL))
			{ g_warning("different tileset height"); }
	}
	else
	{
		g_snprintf(buffer, 16, "%d", tileset->height/tile_height);
		xmlSetProp(root_node, "height", buffer);
	}

	file->attr_nodes =
		g_malloc( sizeof(xmlNode*)*ATTRIBUTE_COUNT );
	xmlNode **attr_nodes = file->attr_nodes;

	struct TileAttribute **tile_attr =
		global_data->tile_attributes;

	gint i;
	for (i=0; i<ATTRIBUTE_COUNT; i++)
	{
		if (!tile_attr[i]->enabled)
			{ file->attr_nodes[i] = NULL; continue; }

		file->attr_nodes[i] =
			file_attribute_parse_node
				(tile_attr[i], root_node,
				 file->min_buffer_size);
	}

	global_data->open_file = file;
	return TRUE;
}


gboolean file_save
( struct GlobalData *global_data, gchar *filename )
{
	if (!global_data->open_file) { return FALSE; }

	struct TileAttribute **tile_attr =
		global_data->tile_attributes;
	struct File *file =
		(struct File*)global_data->open_file;
	xmlNode **attr_nodes = file->attr_nodes;
	gint row_width = (global_data->tileset->width) /
	                 (global_data->tileset->tile_width);

	gint i;
	for (i=0; i<ATTRIBUTE_COUNT; i++)
	{
		if (!tile_attr[i]->enabled) { continue; }
		xmlNode *data_node =
			xml_get_child_node_with_prop
				(attr_nodes[i], "data", "encoding", "csv");
		gchar *csv_string =
			csv_create_string
				(tile_attr[i]->value_buffer,
				 tile_attr[i]->buffer_size,
				  row_width);
		xmlNodeSetContent(data_node->children, csv_string);
		g_free(csv_string);
	}

	gchar *image_rel_path =
		make_relative_path
			(filename, file->image_filename_abs, DELIMITER);
	xmlSetProp(file->image_node, "source", image_rel_path);
	g_free(image_rel_path);

	return (xmlSaveFile(filename, file->doc) != -1);

}


void file_destroy
( struct File *file )
{
	if (!file) { return; }

	if (file->attr_nodes)
		{ g_free(file->attr_nodes); }
	xmlFreeDoc(file->doc);
	g_free(file->image_filename_abs);
	g_free(file);
}


gboolean file_close
( struct GlobalData *global_data )
{
	if (!global_data->open_file) { return FALSE; }

	struct File *file =
		(struct File*)global_data->open_file;

	file_destroy(file);

	tileset_destroy(global_data->tileset);
	global_data->tileset = NULL;

	struct TileAttribute **tile_attr;
	for (tile_attr = global_data->tile_attributes;
	     *tile_attr; tile_attr++)
	{
		g_free((*tile_attr)->value_buffer);
	}

	global_data->open_file = NULL;
	return TRUE;
}


void file_open_attempt_quiet
( struct GlobalData *global_data, const gchar *filename )
{
	struct File *file = file_open(filename, NULL);
		if (file && file_check(global_data, file, NULL))
		{
			file_parse(global_data, file);
			global_data->open_file_path = g_strdup(filename);
		}
		else
			{ file_destroy(file); }
}

