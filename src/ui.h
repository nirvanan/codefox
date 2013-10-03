/*
 * ui.h
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

#ifndef UI_H
#define UI_H

#include <gtk/gtk.h>
#include <glib.h>

#include "editor.h"

typedef struct {
	GObject *toplevel;
	GObject *about_item;
	GObject *help_item;
	GObject *new_item;
	GObject *open_item;
	GObject *save_item;
	GObject *saveas_item;
	GObject *quit_item;
	GObject *cut_item;
	GObject *copy_item;
	GObject *paste_item;
	GObject *delete_item;
	GObject *format_item;
	GObject *build_item;
	GObject *run_item;
	GObject *debug_item;
	GObject *new_toolbar;
	GObject *open_toolbar;
	GObject *save_toolbar;
	GObject *saveas_toolbar;
	GObject *cut_toolbar;
	GObject *copy_toolbar;
	GObject *paste_toolbar;
	GObject *delete_toolbar;
	GObject *build_toolbar;
	GObject *run_toolbar;
	GObject *debug_toolbar;
	GObject *code_notebook;
	GObject *info_notebook;
	GObject *statustree;
	GObject *compilertree;
	GObject *notepadview;
	GObject *locationlabel;
	GObject *modelabel;
	GObject *filetree;
	GSList *editor_list;
} CWindow;

typedef enum {
	FILE_OP_CREATE,
	FILE_OP_SAVE,
	FILE_OP_OPEN
} CFileOp;

typedef enum {
	CLIPBOARD_CUT,
	CLIPBOARD_COPY,
	CLIPBOARD_PASTE
} CClipboradSignal;

void
ui_disable_save_widgets ();

void
ui_enable_save_widgets ();

void
ui_init ();

void
ui_about_dialog_new();

void
ui_editor_new();

void
ui_editor_new_with_text(const gchar *filepath, const gchar *code_buf);

void
ui_filetree_entry_new(gboolean is_file, gchar *filename, gchar *filepath);

void
ui_status_entry_new(const gint op, const gchar *filepath);

void
ui_get_filepath_from_dialog (gchar *filepath, const gboolean open, const gchar *default_path);

gboolean
ui_have_editor();

gboolean
ui_find_editor(const gchar *filepath);

void
ui_current_editor_filepath(gchar *filepath);

gchar *
ui_current_editor_code();

void
ui_save_code_post(const gchar *filepath);

void
ui_emit_clipboard_signal(gint clipboard_signal);

void
ui_emit_save_signal();

void
ui_current_editor_delete_range();

void
ui_current_editor_format();

void
ui_current_editor_update_cursor();

void
ui_current_editor_change_mode();

void
ui_update_line_number_label (const gboolean insert, const gint append, 
							 const GtkTextIter *start, const GtkTextIter *end);

void
ui_switch_page();

#endif
