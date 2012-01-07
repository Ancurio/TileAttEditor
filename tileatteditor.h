
#include <gtk/gtk.h>

#include "color.h"
#include "tileset.h"
#include "attribute.h"
#include "ui.h"

#define DELIMITER '/'

struct Settings
{
	gint active_attr_id;
	gdouble tileset_scale_ratio;
	gdouble attribute_alpha;
	gboolean smooth_zoom;
	gboolean workspace_flipped;

	struct Color *bg_color;
	struct Color *grid_color;

	gint preferred_tile_width;
	gint preferred_tile_height;
	gint window_width;
	gint window_height;
	gchar *last_opened;
};

struct SettingsDialog
{
	gboolean resize_required;
	gboolean redraw_required;

	GtkWidget *window;

	GtkWidget *spinb_scale;
	GtkWidget *hscale_alpha;
	GtkWidget *checkb_smooth;
	GtkWidget *colorb_bg;
	GtkWidget *colorb_grid;
	GtkWidget *applyb;

	GtkWidget **checkb_attributes;

//	struct Settings *local_settings;

};

struct GlobalData
{
	gchar *open_file_path;
	gboolean buffer_changed;
	gint hovered_tile;
	gdouble hovered_offset_x;
	gdouble hovered_offset_y;
	guint statusbar_context_id;
	guint statusbar_message_id;

	struct Settings *settings;
	struct Tileset *tileset;
	struct TileAttribute **tile_attributes;
	struct TileAttribute *active_attribute;
	struct MainWindow *main_window;
	struct SettingsDialog *settings_dialog;
	struct NewFileDialog *new_file_dialog;
	gpointer open_file;
};

