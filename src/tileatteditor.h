/*
 * tileatteditor.h
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


#include <gtk/gtk.h>

#include "color.h"
#include "tileset.h"
#include "attribute.h"
#include "settings.h"
#include "ui.h"

#define DELIMITER G_DIR_SEPARATOR


struct GlobalData
{
	gchar *open_file_path;
	gboolean buffer_changed;
	gboolean settings_dirty;
	gint hovered_tile;
	gdouble hovered_offset_x;
	gdouble hovered_offset_y;
	gint swipe_value;
	guint statusbar_context_id;
	guint statusbar_message_id;
	gpointer reusable_surface;

	struct Settings *settings;
	struct Tileset *tileset;
	struct TileAttribute **tile_attributes;
	struct TileAttribute *active_attribute;
	struct MainWindow *main_window;
	struct SettingsDialog *settings_dialog;
	struct NewFileDialog *new_file_dialog;
	struct File *open_file;
};

