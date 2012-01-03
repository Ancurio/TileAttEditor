

void cb_filemenu_new(GtkAction *action, gpointer data);
void cb_filemenu_open(GtkAction *action, gpointer data);
void cb_filemenu_save(GtkAction *action, gpointer data);
void cb_filemenu_close(GtkAction *action, gpointer data);
void cb_filemenu_quit(GtkAction *action, gpointer data);
void cb_editmenu_preferences(GtkAction *action, gpointer data);
void cb_editmenu_flip(GtkAction *action, gpointer data);

void cb_attr_button_toggled(GtkWidget *button, gpointer data);

gboolean cb_window_delete
( GtkWidget *widget, GdkEvent *event, gpointer data );
gboolean cb_window_configure
( GtkWidget *widget, GdkEventConfigure *event, gpointer data );
gboolean cb_tileset_area_expose
( GtkWidget *widget, GdkEventExpose *event, gpointer data );
gboolean cb_tileset_area_button_press
( GtkWidget *widget, GdkEventButton *event, gpointer data );
gboolean cb_tileset_area_motion_notify
( GtkWidget *widget, GdkEvent *b_event, gpointer data );
gboolean cb_tileset_area_leave_notify
( GtkWidget *widget, GdkEventButton *b_event, gpointer data );
gboolean cb_tileset_area_drag_data_received
( GtkWidget *widget, GdkDragContext *context,
  gint x, gint y, GtkSelectionData *sdata,
  guint info, guint timestamp, gpointer data);
