/*
 * editor.h
 * This file is part of codefox
 *
 * Copyright (C) 2012-2013 - Gordon Lee
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef EDITOR_H
#define EDITOR_H

#include <gtk/gtk.h>

#define CODEFOX_STOCK_ERROR "codefox-error"
#define CODEFOX_STOCK_WARNING "codefox-warning"

typedef struct Notation
{
	GtkWidget *icon;
	gint line;
	gint err;
} CNotation;

typedef struct Editor
{
	GtkWidget *label_box;
	GtkWidget *label_name;
	GtkWidget *close_button;
	GtkWidget *scroll;
	GtkWidget *textview;
	GtkWidget *lineno;
	GtkWidget *linebox;
	GtkWidget *textbox;
	GtkWidget *notationfixed;
	gchar *filepath;
	gboolean dirty;
	gint linecount;
	GList *notationlist;
	GList *errorlinelist;
	GList *typelist;
	GList *variblelist;
} CEditor;

CEditor *
ceditor_new (const gchar *label);

CEditor *
ceditor_new_with_text (const gchar *label, const gchar *code_buf);

void
ceditor_remove (CEditor *editor);

void
ceditor_save_path (CEditor *editor, gchar *filepath);

void
ceditor_set_dirty (CEditor *editor, gboolean dirty);

gboolean
ceditor_get_dirty (CEditor *editor);

void
ceditor_set_path (CEditor *editor, gchar *filepath);

void
ceditor_show (CEditor *editor);

gchar *
ceditor_get_text (CEditor *editor);

void
ceditor_append_line_label (CEditor *editor, gint lines);

void
ceditor_remove_line_label (CEditor *editor, gint lines);

void
ceditor_add_notation (CEditor *editor, gint err, gint line, gchar *info);

void
ceditor_clear_notation (CEditor *editor);

void
ceditor_emit_save_signal (CEditor *editor);

#endif
