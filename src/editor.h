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

#include "edithistory.h"

#define CODEFOX_STOCK_ERROR "codefox-error"
#define CODEFOX_STOCK_WARNING "codefox-warning"
#define CODEFOX_STOCK_BREAKPOINT "codefox-breakpoint"
#define CODEFOX_STOCK_DEBUGPTR "codefox-debugptr"

typedef struct Notation
{
	GtkWidget *icon;
	gint line;
	gint err;
} CNotation;

typedef struct {
	gchar *filepath;
	gint line;
	GtkWidget *icon;
} CBreakPointTag;

typedef struct {
	gchar *filepath;
	gint line;
} CBreakPointNode;

typedef struct {
	GtkWidget *label_box;
	GtkWidget *label_name;
	GtkWidget *close_button;
	GtkWidget *scroll;
	GtkWidget *event_scroll;
	GtkWidget *textview;
	GtkWidget *lineno;
	GtkWidget *linebox;
	GtkWidget *eventbox;
	GtkWidget *textbox;
	GtkWidget *notationfixed;
	gchar *filepath;
	gboolean dirty;
	gint linecount;
	GList *notationlist;
	GList *breakpoint_list;
	CEditHistory *edit_history;
	gint current_matched;
	gint total_matched;
	gint next_modify_omit;
} CEditor;

CEditor *
ceditor_new (const gchar *label);

CEditor *
ceditor_new_with_text (const gchar *label, const gchar *code_buf);

void
ceditor_remove (CEditor *editor);

void
ceditor_save_path (CEditor *editor, const gchar *filepath);

void
ceditor_set_dirty (CEditor *editor, gboolean dirty);

gboolean
ceditor_get_dirty (CEditor *editor);

void
ceditor_set_path (CEditor *editor, const gchar *filepath);

void
ceditor_show (CEditor *editor);

void
ceditor_recover_breakpoint (CEditor *editor);

gchar *
ceditor_get_text (CEditor *editor);

void
ceditor_append_line_label (CEditor *editor, gint lines);

void
ceditor_remove_line_label (CEditor *editor, gint lines);

void
ceditor_add_notation (CEditor *editor, gint err, gint line, const gchar *info);

void
ceditor_clear_notation (CEditor *editor);

void
ceditor_emit_close_signal (CEditor *editor);

void
ceditor_get_line (CEditor *editor, gchar *line, const gint size, const gint lineno);

void
ceditor_error_tag_clear(CEditor *editor);

void
ceditor_error_tag_add (CEditor *editor, const gint row, const gint column, const gint len);

void
ceditor_get_insert_location (CEditor *editor, gint *x, gint *y);

void
ceditor_insert(CEditor *editor, const gchar *text);

void
ceditor_highlighting_update (CEditor *editor);

void
ceditor_breakpoint_update (CEditor *editor, gdouble x, gdouble y, gchar *breakpoint_desc);

void
ceditor_breakpoint_tags_resize (CEditor *editor);

void
ceditor_breakpoint_tags_get (CEditor *editor, GList **list);

GtkWidget *
ceditor_icon_add (CEditor *editor, const gint line);

void
ceditor_step_add (CEditor *editor, const gboolean insert, const gint offset, const gint len,
				  const gchar *text);

gboolean
ceditor_can_undo (CEditor *editor);

gboolean
ceditor_can_redo (CEditor *editor);

void
ceditor_undo (CEditor *editor);

void
ceditor_redo (CEditor *editor);

void
ceditor_search_init (CEditor *editor, const gint matched);

gint
ceditor_search_next (CEditor *editor, const gboolean pre);

void
ceditor_select_range (CEditor *editor, const gint offset, const gint len);

#endif
