


/* Colors are bit-depth independent RGBA values in range [0;1] */
struct Color
{
	gdouble r;
	gdouble g;
	gdouble b;
	gdouble a;
};

struct Color* color_new
( gdouble r, gdouble g, gdouble b, gdouble a );

void color_cairo_set_source
( cairo_t *cr, struct Color *color );

void color_set_from_button
( struct Color *color, GtkWidget *button );

void color_set_button
( struct Color *color, GtkWidget *button );

void color_destroy
( struct Color *color );
