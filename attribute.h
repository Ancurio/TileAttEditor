
#include <gtk/gtk.h>

#define G_TAU 2*G_PI
#define ATTRIBUTE_COUNT 6


struct TileAttribute
{
	/* defined by attribute constructor */
	gchar *name;
	gint default_value;
	gint icon_value;
	gboolean hover_precision;

	gint (*tile_clicked)
	( gint old_value, gdouble x, gdouble y );

	void (*draw_attr)
	( gint attr_value, cairo_t *cr, gboolean hovered,
	  gdouble offset_x, gdouble offset_y );

	/* defined by program */
	gboolean enabled;
	gint *value_buffer;
	guint buffer_size;
	gulong signal_handler_id;
	gpointer global_data;
	GtkWidget *button;

};

enum AttrColor
{
	ATTR_COLOR_PRI,
	ATTR_COLOR_SEC,
};

void tile_attr_set_primary_color(cairo_t *cr);
void tile_attr_set_secondary_color(cairo_t *cr);
void tile_attr_set_color
( cairo_t *cr, gboolean hovered, enum AttrColor color );

//gint* tile_attr_create_value_buffer (gint buffer_size);

struct TileAttribute** tile_attr_create
( gpointer global_data );


/* Individual attribute constructors */
struct TileAttribute* attr_passability_create();
struct TileAttribute* attr_quadpassability_create();
struct TileAttribute* attr_priority_create();
struct TileAttribute* attr_bushflag_create();
struct TileAttribute* attr_counterflag_create();
struct TileAttribute* attr_terrainflag_create();

