/*
 * callback.h
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

#ifndef CALLBACK_H
#define CALLBACK_H

#include <gtk/gtk.h>

#include "ui.h" 
#include "highlighting.h"
#include "autoindent.h"

void
about_show_about (GtkWidget *widget, gpointer user_data);

void
help_goto_website (GtkWidget *widget, gpointer user_data);

void
new_create_new_tab (GtkWidget *widget, gpointer user_data);

void
open_open_local_file (GtkWidget *widget, gpointer user_data);

void
save_save_current_code (GtkWidget *widget, gpointer user_data);

void
saveas_save_to_file (GtkWidget *widget, gpointer user_data);

void
quit_quit_program (GtkWidget *widget, gpointer user_data);

void
cut_cut_code (GtkWidget *widget, gpointer user_data);

void
copy_copy_code (GtkWidget *widget, gpointer user_data);

void
paste_paste_code (GtkWidget *widget, gpointer user_data);

void
delete_delete_code (GtkWidget *widget, gpointer user_data);

void
format_format_code (GtkWidget *widget, gpointer user_data);

void
build_compile (GtkWidget *widget, gpointer user_data);

void
run_run_executable (GtkWidget *widget, gpointer user_data);

void
debug_debug_executable (GtkWidget *widget, gpointer user_data);

void
on_close_page (GtkButton *button, gpointer user_data);

void
on_switch_page (GtkNotebook *notebook, GtkWidget *page,guint page_num,
				gpointer user_data);

void
on_editor_insert (GtkTextBuffer *textbuffer, GtkTextIter *location,
				  gchar *text, gint len, gpointer user_data);

void
on_editor_delete (GtkTextBuffer *textbuffer, GtkTextIter *start,
				  GtkTextIter *end, gpointer user_data);

void
on_editor_delete2 (GtkTextBuffer *textbuffer, GtkTextIter *start,
				  GtkTextIter *end, gpointer user_data);
				  
void
on_filetree_selection_changed (GtkTreeSelection *treeselection, gpointer user_data);

void
on_compilertree_selection_changed (GtkTreeSelection *treeselection, gpointer user_data);

void
on_cursor_change (GtkTextView *text_view, GtkMovementStep step,
                    gint count, gboolean extend_selection,
                    gpointer user_data);

void
on_textview_clicked(GtkWidget *widget, GdkEvent *event, gpointer user_data);
                    
void
on_mode_change (GtkTextView *text_view, gpointer user_data);

void
do_nothing (GtkTextBuffer *textbuffer, GtkTextIter *start,
				  GtkTextIter *end, gpointer user_data);

#endif
