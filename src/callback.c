/*
 * callback.c
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

#include <stdlib.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glib/gi18n-lib.h>


#include "misc.h"
#include "callback.h"
#include "ui.h"
#include "highlighting.h"
#include "autoindent.h"
#include "filetree.h"
#include "time.h"
#include "compile.h"

#define MAX_FILEPATH_LENTH 1000
#define EXTRA_LENGTH 100

extern CWindow *window;

void
about_show_about (GtkWidget *widget, gpointer user_data)
{
	ui_about_dialog_new ();
}

void
help_goto_website (GtkWidget *widget, gpointer user_data)
{
	misc_open_homepage ();
}

void
new_create_new_tab (GtkWidget *widget, gpointer user_data)
{
	ui_editor_new ();
	ui_filetree_entry_new (1, _("Untitled"), _("Untitled"));
	ui_status_entry_new (FILE_OP_CREATE, NULL);
}

void
open_open_local_file (GtkWidget *widget, gpointer user_data)
{
	gchar *filepath;

	filepath = (gchar *) g_malloc (MAX_FILEPATH_LENTH);
	ui_get_filepath_from_dialog (filepath, TRUE, NULL);

	if (g_strcmp0 (filepath, "NULL") != 0) {
		gboolean exist;

		exist = ui_find_editor (filepath);

		if (!exist) {
			gchar *code_buf;
			gint filesize;
			gint name_offset;

			filesize = misc_get_file_size (filepath);
			code_buf = (gchar *) g_malloc (filesize + EXTRA_LENGTH);
			misc_get_file_content (filepath, &code_buf);
			ui_editor_new_with_text (filepath, code_buf);
			name_offset = misc_get_file_name_in_path (filepath);
			ui_filetree_entry_new (1, filepath + name_offset + 1, filepath);
			ui_status_entry_new (FILE_OP_OPEN, filepath);
		}
	}

	g_free (filepath);
}

void
save_save_current_code (GtkWidget *widget, gpointer user_data)
{
	gchar *filepath;
	gchar *code;

	filepath = (gchar *) g_malloc (MAX_FILEPATH_LENTH);
	ui_current_editor_filepath (filepath);

	if (g_strcmp0 (filepath, _("Untitled")) == 0) {
		ui_get_filepath_from_dialog (filepath, FALSE, _("Untitled"));

		if (g_strcmp0 (filepath, "NULL") == 0)
			return ;
	}

	code = ui_current_editor_code();
	misc_set_file_content (filepath, code);
	ui_save_code_post (filepath);
	ui_status_entry_new (FILE_OP_SAVE, filepath);
}

void
saveas_save_to_file (GtkWidget *widget, gpointer user_data)
{
	gchar *filepath;
	gchar *code;

	filepath = (gchar *) g_malloc (MAX_FILEPATH_LENTH);
	ui_get_filepath_from_dialog (filepath, FALSE, filepath);

	if (g_strcmp0 (filepath, "NULL") != 0) {
		code = ui_current_editor_code();
		misc_set_file_content (filepath, code);
		ui_save_code_post (filepath);
		ui_status_entry_new (FILE_OP_SAVE, filepath);
	}
}


void
cut_cut_code (GtkWidget *widget, gpointer user_data)
{
	ui_emit_clipboard_signal (CLIPBOARD_CUT);
}

void
copy_copy_code (GtkWidget *widget, gpointer user_data)
{
	ui_emit_clipboard_signal (CLIPBOARD_COPY);
}

void
paste_paste_code (GtkWidget *widget, gpointer user_data)
{
	ui_emit_clipboard_signal (CLIPBOARD_PASTE);
}

void
delete_delete_code (GtkWidget *widget, gpointer user_data)
{
	ui_current_editor_delete_range();
}

void
quit_quit_program (GtkWidget *widget, gpointer user_data)
{
	while (ui_have_editor ())
		ui_emit_save_signal ();
	gtk_main_quit ();
}

void
format_format_code (GtkWidget *widget, gpointer user_data)
{
	ui_current_editor_format ();
}

void
on_close_page (GtkButton *button, gpointer user_data)
{
	GSList * iterator;
	gint n_page;
	
	for (iterator = window->editor_list; iterator; iterator = iterator->next)
	{
		CEditor *editor;
		editor = (CEditor *) iterator->data;
		if (GTK_BUTTON (editor->close_button) == button)
		{
			if (editor->dirty)
			{
				GtkWidget *dialog;
				gint result;
				
				dialog = gtk_message_dialog_new (GTK_WINDOW (window->toplevel),
												 GTK_DIALOG_DESTROY_WITH_PARENT,
												 GTK_MESSAGE_WARNING,
												 GTK_BUTTONS_NONE,
												 _("%s has been modified, save to file?"),
												 editor->filepath);
				 gtk_dialog_add_buttons (GTK_DIALOG (dialog), 
										 GTK_STOCK_CANCEL,
										 GTK_RESPONSE_CANCEL,
										 GTK_STOCK_NO,
										 GTK_RESPONSE_REJECT,
										 GTK_STOCK_YES,
										 GTK_RESPONSE_ACCEPT,
										 NULL);
				result = gtk_dialog_run (GTK_DIALOG (dialog));
				gtk_widget_destroy (dialog);
				if (result == GTK_RESPONSE_CANCEL)
					return ;
				else if (result == GTK_RESPONSE_ACCEPT)
				{
					
					if (g_strcmp0 (editor->filepath, _("Untitled")) == 0)
					{
						GtkWidget *dialog;

						dialog = gtk_file_chooser_dialog_new (_("Save File"),
															  GTK_WINDOW (window->toplevel),
															  GTK_FILE_CHOOSER_ACTION_SAVE,
															  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
															  GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
															  NULL);
						gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog),
																		TRUE);
						gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog),
														   editor->filepath);

						if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
						{
							gchar *user_filename;

							user_filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
							ceditor_save_path (editor, user_filename);
							ceditor_set_dirty (editor, 0);
							filetree_set_selected_path (GTK_TREE_VIEW (window->filetree),
														user_filename,
														editor->filepath);
							ceditor_set_path (editor, user_filename);
						}
			
						gtk_widget_destroy (dialog);
					}
					else
					{
						ceditor_save_path (editor, editor->filepath);
						if (editor->dirty)
								ceditor_set_dirty (editor, 0);
					}
				}
			}
			
			n_page = gtk_notebook_page_num (GTK_NOTEBOOK (window->code_notebook),
											GTK_WIDGET (editor->scroll));
			gtk_notebook_remove_page (GTK_NOTEBOOK (window->code_notebook), n_page);
			filetree_remove (GTK_TREE_VIEW (window->filetree), editor->filepath);
			window->editor_list = g_slist_remove(window->editor_list, editor);
			break;
		}
	}

	if (g_slist_length (window->editor_list) == 0) {
		ui_disable_save_widgets ();
	}
	
}

void
build_compile (GtkWidget *widget, gpointer user_data)
{
	gint page;
	GtkWidget *current_page;
	GSList *iterator;
	gchar *output;
	CEditor *current;
	
	page = gtk_notebook_get_current_page (GTK_NOTEBOOK (window->code_notebook));	
	if (page == -1)
		return ;
	current_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (window->code_notebook),
											  page);
	
	for (iterator = window->editor_list; iterator; iterator = iterator->next)
	{
		CEditor *editor;

		editor = (CEditor *) iterator->data;
		if (GTK_WIDGET (editor->scroll) == current_page) {
			if (editor->dirty) {
				g_signal_emit_by_name (GTK_WIDGET (window->save_item), "activate");
			}
			if (editor->dirty)
				return ;
			output = compile_current_project (editor->filepath);
			current = editor;
		}
	}
	
	GtkTreeStore *store;
	GtkTreeIter  iter;
	gchar *line;
	gint p = 0;
	gint errors = 0, warnings = 0;
	
	store = gtk_tree_store_new (3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model (GTK_TREE_VIEW (window->compilertree), store);
	gtk_tree_store_append (store, &iter, NULL);
	gtk_tree_store_set(store, &iter, 0, _("Start compilation."), 1, "blue", -1);
	
	line = g_malloc (1024);
	
	ceditor_clear_notation (current);
	
	while (output[p])
	{
		gint i = 0;
		
		while (output[p] != '\n')
			line[i++] = output[p++];
		line[i] = 0;
		
		gtk_tree_store_append (store, &iter, NULL);
		gtk_tree_store_set(store, &iter, 0, line, 1, "red", -1);
		
		if (compile_is_error (line))
		{
			gchar *location;
			gchar *fil;
			gint lineno;
			
			location = g_malloc (1024);
			fil = g_malloc (1024);
			compile_get_location (line, location);
			sscanf(location, "%s%d", fil, &lineno);
			errors++;
			ceditor_add_notation (current, 1, lineno, line);
			
			g_free (location);
			g_free (fil);
		}
		else if (compile_is_warning (line))
		{
			gchar *location;
			gchar *fil;
			gint lineno;
			
			location = g_malloc (1024);
			fil = g_malloc (1024);
			compile_get_location (line, location);
			sscanf(location, "%s%d", fil, &lineno);
			warnings++;
			ceditor_add_notation (current, 0, lineno, line);
			
			g_free (location);
			g_free (fil);
		}
		
		p++;
	}
	
	gtk_tree_store_append (store, &iter, NULL);
	
	sprintf(line, _("Compilation finished with %d error(s), %d warning(s)."), errors, warnings);
	gtk_tree_store_set(store, &iter, 0, line, 1, "blue", -1);
	
	gtk_notebook_set_current_page (GTK_NOTEBOOK (window->info_notebook), 1);
	
	g_free (line);
	g_free (output);
}	

void
run_run_executable (GtkWidget *widget, gpointer user_data)
{
	gint page;
	GtkWidget *current_page;
	GSList * iterator;
	
	page = gtk_notebook_get_current_page (GTK_NOTEBOOK (window->code_notebook));	
	if (page == -1)
		return ;
	current_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (window->code_notebook),
											  page);
	
	for (iterator = window->editor_list; iterator; iterator = iterator->next)
	{
		CEditor *editor;

		editor = (CEditor *) iterator->data;
		if (GTK_WIDGET (editor->scroll) == current_page) {
			if (editor->dirty) {
				g_signal_emit_by_name (GTK_WIDGET (window->build_item), "activate");
			}
			if (editor->dirty)
				return ;
			gint len, i;
			gchar *dir, *command;
		
			dir = g_malloc (1024);
			command = g_malloc (1024);
			len = strlen (editor->filepath);
			strcpy (dir, editor->filepath);
			for (i = len - 1; dir[i] != '/'; i--)
				dir[i] = 0;
			strcpy (dir + i, "/a.out");
			
			sprintf (command, "xterm -e %s", dir);
			system (command);
			break;
		}
	}
}

void
debug_debug_executable (GtkWidget *widget, gpointer user_data)
{
	gint page;
	GtkWidget *current_page;
	GSList * iterator;
	
	page = gtk_notebook_get_current_page (GTK_NOTEBOOK (window->code_notebook));	
	if (page == -1)
		return ;
	current_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (window->code_notebook),
											  page);
	
	for (iterator = window->editor_list; iterator; iterator = iterator->next)
	{
		CEditor *editor;

		editor = (CEditor *) iterator->data;
		if (GTK_WIDGET (editor->scroll) == current_page) {
			if (editor->dirty) {
				g_signal_emit_by_name (GTK_WIDGET (window->build_item), "activate");
			}
			if (editor->dirty)
				return ;
			gint len, i;
			gchar *dir, *command;
		
			dir = g_malloc (1024);
			command = g_malloc (1024);
			len = strlen (editor->filepath);
			strcpy (dir, editor->filepath);
			for (i = len - 1; dir[i] != '/'; i--)
				dir[i] = 0;
			strcpy (dir + i, "/a.out");
			
			sprintf (command, "xterm -e gdb %s", dir);

			system (command);
			break;
		}
	}
}
	
void
on_switch_page (GtkNotebook *notebook, GtkWidget *page, guint page_num,
				gpointer user_data)
{
	ui_switch_page ();
}

void
on_editor_insert (GtkTextBuffer *textbuffer, GtkTextIter *location,
				  gchar *text, gint len, gpointer user_data)
{
	/* Highlighting changed lines. */
	GtkTextIter start, end;
	gint start_line, end_line;
	gint i;
	gint linecount = 0;
	GtkTextView *textview;
	CEditor *current;

	end_line = gtk_text_iter_get_line (location);
	start_line = end_line;
	for (i = 0; text[i]; i++)
		if (text[i] == '\n') {
			start_line--;
			linecount++;
		}

	gtk_text_buffer_get_iter_at_line (textbuffer, &start, start_line);
	gtk_text_buffer_get_iter_at_line (textbuffer, &end, end_line);
	gtk_text_iter_forward_to_line_end (&end);

	highlight_apply (textbuffer, &start, &end);
	
	if (text[0] == '\n')
		start_line++;
	
	/* Modify indents in changed lines. */
	if ((text[0] == '\n' || text[0] == '}' || text[0] == '{') && len == 1)
		autoindent_apply (textbuffer, location, start_line, end_line);
	
	/* Modify line labels and set the dirty bit. */
	GSList * iterator;
	
	for (iterator = window->editor_list; iterator; iterator = iterator->next)
	{
		CEditor *editor;
		GtkTextBuffer *buffer;

		editor = (CEditor *) iterator->data;
		buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor->textview));
		if (buffer == textbuffer)
		{
			ceditor_set_dirty (editor, 1);
			ceditor_append_line_label (editor, linecount);
			textview = GTK_TEXT_VIEW (editor->textview);
			current = editor;
			break;
		}
	}
	
	/* Update cursor location. */
	GtkTextMark *insert;
	GtkTextIter iter;
	gint line, column;
	gchar *loc;

	insert = gtk_text_buffer_get_insert (textbuffer);
	gtk_text_buffer_get_iter_at_mark (textbuffer, &iter, insert);
	gtk_text_view_place_cursor_onscreen (textview);
	line = gtk_text_iter_get_line (&iter);
	column = gtk_text_iter_get_line_offset (&iter);
	loc = g_malloc (100);
	sprintf (loc, _(" Line: %d\tColumn: %d"), line + 1, column);
	gtk_label_set_label (GTK_LABEL (window->locationlabel), loc);
	g_free (loc);
}

void
on_editor_delete (GtkTextBuffer *textbuffer, GtkTextIter *start,
				  GtkTextIter *end, gpointer user_data)
{	
	GtkTextIter startitr, enditr;
	gint start_line, end_line;
	CEditor *current;
	
	start_line = gtk_text_iter_get_line (start);
	end_line = gtk_text_iter_get_line (end);
	gtk_text_buffer_get_iter_at_line (textbuffer, &startitr, start_line);
	gtk_text_buffer_get_iter_at_line (textbuffer, &enditr, end_line);
	gtk_text_iter_forward_to_line_end (&enditr);
	
	highlight_apply (textbuffer, &startitr, &enditr);
	
	GSList * iterator;
	
	for (iterator = window->editor_list; iterator; iterator = iterator->next)
	{
		CEditor *editor;
		GtkTextBuffer *buffer;

		editor = (CEditor *) iterator->data;
		buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor->textview));
		if (buffer == textbuffer)
		{
			ceditor_set_dirty (editor, 1);
			current = editor;
		}
	}
	
	GtkTextMark *insert;
	GtkTextIter iter;

	insert = gtk_text_buffer_get_insert (textbuffer);
	gtk_text_buffer_get_iter_at_mark (textbuffer, &iter, insert);
	
	gint line, column;
	
	line = gtk_text_iter_get_line (&iter);
	column = gtk_text_iter_get_line_offset (&iter);
	
	gchar *loc;
	
	loc = g_malloc (100);
	sprintf (loc, _(" Line: %d\tColumn: %d"), line + 1, column);
	gtk_label_set_label (GTK_LABEL (window->locationlabel), loc);
	g_free (loc);
	
}

void
on_editor_delete2 (GtkTextBuffer *textbuffer, GtkTextIter *start,
				  GtkTextIter *end, gpointer user_data)
{	
	ui_update_line_number_label (FALSE, 0, start, end);
}

void
on_filetree_selection_changed (GtkTreeSelection *treeselection, gpointer user_data)
{		
	GtkTreeIter iter;
	GtkTreeView *treeview;
	gchar *filepath;
	gboolean isfile;
	
	treeview = gtk_tree_selection_get_tree_view (treeselection);
	if (!gtk_tree_selection_get_selected (treeselection, NULL, &iter))
		return ;
	
	filepath = g_malloc (1024);
	isfile = filetree_get_file_path (treeview, &iter, filepath);
	if (!isfile)
	{
		g_free (filepath);
		return ;
	}
	else
	{
		GSList * iterator;
	
		for (iterator = window->editor_list; iterator; iterator = iterator->next)
		{
			CEditor *editor;

			editor = (CEditor *) iterator->data;
			if (g_strcmp0 (filepath, editor->filepath) == 0)
			{
				gint index;
				
				index = gtk_notebook_page_num (GTK_NOTEBOOK (window->code_notebook),
											   GTK_WIDGET (editor->scroll));
				if (index != -1)
					gtk_notebook_set_current_page (GTK_NOTEBOOK (window->code_notebook), 
												   index);
				
				break;
			}
		}
	}
}

void
on_compilertree_selection_changed (GtkTreeSelection *treeselection, gpointer user_data)
{		
	GtkTreeIter iter;
	GtkTreeView *treeview;
	gchar *filepath;
	gboolean isfile;
	gchar *line;
	gchar *position;
	gchar *filename;
	gint row, column;
	GtkTreeStore *store;
	
	treeview = gtk_tree_selection_get_tree_view (treeselection);
	if (!gtk_tree_selection_get_selected (treeselection, NULL, &iter))
		return ;
	
	line = g_malloc (1024);
	position = g_malloc (1024);
	filename = g_malloc (1024);
	store = GTK_TREE_STORE(gtk_tree_view_get_model(treeview));
	gtk_tree_model_get (GTK_TREE_MODEL (store), &iter, 0, &line, -1);
	
	compile_get_location (line, position);
	
	printf("%s\n", position);
	
	sscanf (position, "%s%d%d", filename, &row, &column);
	
	GSList * iterator;
	
	for (iterator = window->editor_list; iterator; iterator = iterator->next)
	{
		CEditor *editor;

		editor = (CEditor *) iterator->data;
		if (g_strcmp0 (filename, editor->filepath) == 0)
		{
			gint index;
				
			index = gtk_notebook_page_num (GTK_NOTEBOOK (window->code_notebook),
											   GTK_WIDGET (editor->scroll));
			if (index != -1)
				gtk_notebook_set_current_page (GTK_NOTEBOOK (window->code_notebook), 
												   index);
				
			break;
		}
	}
	
	g_free (line);
	g_free (position);
}

void
on_cursor_change (GtkTextView *text_view, GtkMovementStep step,
                  gint count, gboolean extend_selection,
                  gpointer user_data)
{
	ui_current_editor_update_cursor ();
}

void
on_textview_clicked (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	gint page;
	GdkEventButton *bevent; 
	
	bevent = (GdkEventButton *) event;
	if (bevent->button != 1)
		return ;
	
	ui_current_editor_update_cursor ();
	/* FIXME: Just can't delete this line... */
	page = gtk_notebook_get_current_page (GTK_NOTEBOOK (window->code_notebook));

}

void
on_mode_change (GtkTextView *text_view, gpointer user_data)
{
	ui_current_editor_change_mode ();
}

void
do_nothing (GtkTextBuffer *textbuffer, GtkTextIter *start,
			GtkTextIter *end, gpointer user_data)
{
}
