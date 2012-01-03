

#include <glib.h>
#include <cairo.h>

#include "tileatteditor.h"
#include "file.h"

#define DELIMITER '/'
#define TILE_ATTR_STRING "tileattribute"


static xmlChar* xml_get_attribute_contents
( xmlNode *node, xmlChar *name )
{
	xmlAttr *attr;
	for (attr = node->properties; attr; attr = attr->next)
	{
		if (!xmlStrcmp(attr->name, name))
			{ return attr->children->content; }
	}
	return NULL;
}


static xmlNodePtr xml_get_child_node
( xmlNode *parent, xmlChar *name )
{
	xmlNode *node;
	for (node = parent->children; node; node = node->next)
	{
		if (!xmlStrcmp(node->name, name)) { return node; }
	}
	return NULL;
}

static xmlNodePtr xml_get_child_node_with_prop
( xmlNode *parent, xmlChar *node_name,
  xmlChar *prop_name, xmlChar *prop_value )
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


static gint str_len(gchar *string)
{
	gint i; for (i=0;string[i] != '\0';i++) {}
	return i;
}

static gchar* make_relative_path
( gchar *base, gchar *dest, gchar dlm )
{
	if (!(base&&dest)) {return NULL;}	/* check if empty strings */

	gchar *p;
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
		relative_path[relative_path_pos+i] = dest[i+last_dlm+1];
	}
	relative_path[relative_path_pos+i] = '\0';

	return relative_path;

}

static gchar* make_absolute_path
( gchar *base, gchar *rel_path, gchar dlm )
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
		g_malloc(sizeof(gchar)*(base_cp_count+rel_path_cp_count));

	gint base_rd_end = i;

	for (i=0; i<base_cp_count; i++)
		{ absolute_path[i] = base[i]; }

	for (0; rel_path[rel_path_rdhd]; i++)
	{
		absolute_path[i] = rel_path[rel_path_rdhd];
		rel_path_rdhd++;
	}
	absolute_path[i] = '\0';

	return absolute_path;
}



void str_clear
(gchar *str, gint len)
{
	gint i; for (i=0;i<len;i++) {str[i] = '\0';}
}

void str_append_c
(gchar *str, gchar c)
{
	gint i = 0;
	while (str[i]) {i++;}
	str[i] = c;
}

guint csv_count_values
(gchar *str)
{
	gint i = 0;
	guint count = 0;
	while (str[i] != 0)  { if (str[i] == ',') {count++;} i++;}
	return count+1;
}

gint* csv_parse_string
(gchar *str, guint *buffer_size, gint default_value, gint min_buffer_size)
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

	for (0; val_count<*buffer_size; val_count++)
	{
		val_buffer[val_count] = default_value;
	}
g_message("Successfully created buffer of size %d", *buffer_size);
	return val_buffer;
}

static gchar* csv_create_string
( gint *value_buffer, guint buffer_size, gint row_length )
{
	GString *csv_string =
		g_string_sized_new
			((gsize)(buffer_size*2+(buffer_size/row_length)));
	g_string_append_c(csv_string, '\n');

	gchar buffer[16];
	gint i, row_watch = 0;
	for (i=0; i<buffer_size-1; i++)
	{
		g_ascii_dtostr(buffer, 16, (gdouble)value_buffer[i]);
		g_string_append(csv_string, buffer);
		g_string_append_c(csv_string, ',');

		row_watch++;
		if (row_watch >= row_length)
		{
			g_string_append_c(csv_string, '\n');
			row_watch = 0;
		}
	}
	g_ascii_dtostr(buffer, 16, (gdouble)value_buffer[i]);
	g_string_append(csv_string, buffer);

	g_string_append(csv_string, "\n  ");

	gchar *return_string = csv_string->str;
	g_string_free(csv_string, FALSE);

	return return_string;
}





struct File* file_create
( gchar *image_filename, gint tile_width, gint tile_height )
{

	xmlDoc *doc = xmlNewDoc("1.0");
	xmlNode *root_node = xmlNewNode(NULL, "tileset");
	xmlDocSetRootElement(doc, root_node);

	gchar buffer[8];

	xmlAttr *attr_width = xmlSetProp(root_node, "tilewidth",
	g_ascii_dtostr(buffer, 8, (gdouble)tile_width));

	xmlAttr *attr_height = xmlSetProp(root_node, "tileheight",
	g_ascii_dtostr(buffer, 8, (gdouble)tile_height));

	xmlNode *image_node = xmlNewNode(NULL, "image");
	xmlAddChild(root_node, image_node);
	xmlAddPrevSibling(image_node, xmlNewText("\n "));
	xmlAddNextSibling(image_node, xmlNewText("\n"));


	struct File *file = g_malloc( sizeof *file );

	file->doc = doc;
	file->root_node = root_node;
	file->image_node = image_node;
	file->image_filename_abs = g_strdup(image_filename);

	return file;

}


struct File* file_open
( gchar *filename, enum ErrorFileOpen *error )
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
	file->image_filename_abs = image_filename_abs;

	return file;

}


gboolean file_parse
( struct GlobalData *global_data,
  struct File *file, enum ErrorFileParse *error )
{
	xmlNode *root_node = file->root_node;

	gint tile_width = (gint)g_ascii_strtod
		(xml_get_attribute_contents(root_node, "tilewidth"), NULL);

	gint tile_height = (gint)g_ascii_strtod
		(xml_get_attribute_contents(root_node, "tileheight"), NULL);

	if (tile_width < 1 || tile_height < 1)
		{ ERROR(BAD_TILE_SIZES, FALSE); }

	if (!tileset_create_from_file
			(global_data, file->image_filename_abs,
			 tile_width, tile_height))
	{ ERROR(BAD_IMAGE_FILE, FALSE); }

	struct Tileset *tileset = global_data->tileset;

	gchar buffer[16];

	if (xml_get_attribute_contents(root_node, "width"))
	{
		if (tileset->width/tile_width !=
				(gint)g_ascii_strtod(xml_get_attribute_contents(root_node, "width"), NULL))
		{ g_message("Warning: different tileset width"); }
	}
	else
	{
		xmlSetProp(root_node, "width",
			g_ascii_dtostr(buffer, 16,
				(gint)tileset->width/tile_width));
	}

	if (xml_get_attribute_contents(root_node, "height"))
	{
		if (tileset->height/tile_height !=
				(gint)g_ascii_strtod(xml_get_attribute_contents(root_node, "height"), NULL))
		{ g_message("Warning: different tileset height"); }
	}
	else
	{
		xmlSetProp(root_node, "height",
			g_ascii_dtostr(buffer, 16,
				(gint)tileset->height/tile_height));
	}

	file->attr_nodes =
		g_malloc( sizeof(xmlNode*)*ATTRIBUTE_COUNT );
	xmlNode **attr_nodes = file->attr_nodes;

	struct TileAttribute **tile_attr =
		global_data->tile_attributes;

	gint i;
	for (i=0; i<ATTRIBUTE_COUNT; i++)
	{
		file->attr_nodes[i] =
			xml_get_child_node_with_prop
				(root_node, TILE_ATTR_STRING,
				 "name", tile_attr[i]->name);
		if (!attr_nodes[i])
		{
			g_message("Attr [%s] not found. Creating...", tile_attr[i]->name);
			attr_nodes[i] = xmlNewNode(NULL, TILE_ATTR_STRING);
			xmlSetProp(attr_nodes[i], "name", tile_attr[i]->name);
			xmlSetProp(attr_nodes[i], "defaultvalue",
				g_ascii_dtostr(buffer, 16, (gdouble)tile_attr[i]->default_value));
			xmlAddChild(root_node, attr_nodes[i]);
			xmlAddPrevSibling(attr_nodes[i], xmlNewText(" "));
			xmlAddNextSibling(attr_nodes[i], xmlNewText("\n"));
		}
		if (!xml_get_child_node_with_prop
				(attr_nodes[i], "data", "encoding", "csv"))
		{
			g_message("Attr [%s] has no csv encoded data. Creating node..", tile_attr[i]->name);
			xmlNode *data = xmlNewNode(NULL, "data");
			xmlSetProp(data, "encoding", "csv");
			xmlAddChild(data, xmlNewText(" "));
			xmlAddChild(attr_nodes[i], data);
			xmlAddPrevSibling(data, xmlNewText("\n  "));
			xmlAddNextSibling(data, xmlNewText("\n "));
		}

		tile_attr[i]->value_buffer =
			csv_parse_string(xml_get_child_node_with_prop
						(attr_nodes[i], "data",
						 "encoding", "csv")
						->children->content,
			          &tile_attr[i]->buffer_size,
			          tile_attr[i]->default_value,
			          tileset->tile_count);
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

gboolean file_close
( struct GlobalData *global_data )
{
	if (!global_data->open_file) { return FALSE; }

	struct File *file =
		(struct File*)global_data->open_file;

	xmlFreeDoc(file->doc);
	g_free(file->attr_nodes);
	g_free(file->image_filename_abs);
	g_free(file);

	tileset_destroy(global_data);

	gint i;
	struct TileAttribute **tile_attr;
	for (tile_attr = global_data->tile_attributes;
	     *tile_attr; tile_attr++)
	{
		g_free((*tile_attr)->value_buffer);
	}

	global_data->open_file = NULL;
	return TRUE;
}

