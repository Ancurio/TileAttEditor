

#define T_AREA_MIN_WIDTH  0x80
#define T_AREA_MIN_HEIGHT 0x100


struct MainWindow
{
	GtkWidget *window;
	GtkWidget *tileset_area;
	GtkWidget *tileset_viewport;
	GtkWidget *statusbar;
	GtkWidget *workspace_box;
	GtkWidget *attr_button_box;
	GtkWidget *workspace_separator;

};

void ui_main_window_create
( gpointer *_global_data );

void attr_button_box_set_expand
( gpointer *_global_data, gboolean expand );

void workspace_box_flip_packing
( GtkWidget *workspace_box );

gchar *find_image_file_attempt
( GtkWidget *parent );
