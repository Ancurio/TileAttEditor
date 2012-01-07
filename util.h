

#define CAST_GLOBAL_DATA               \
    struct GlobalData *global_data =   \
    (struct GlobalData*)data;

#define CAST_GLOBAL_DATA_PTR(ptr)      \
    struct GlobalData *global_data =   \
    (struct GlobalData*)ptr;


void toggle_button_disable_signal
( struct TileAttribute *tile_attr );

void toggle_button_enable_signal
( struct TileAttribute *tile_attr );

void gtk_size_group_add_widgets
( GtkSizeGroup *size_group, ... );

void activate_other_attribute
( struct GlobalData *global_data );

gint tile_attr_find_id
( struct TileAttribute *tile_attr );

gchar* get_filename_from_path
( const gchar *path, gchar dlm );
