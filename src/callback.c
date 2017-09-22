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
#include "debug.h"
#include "project.h"
#include "symbol.h"
#include "search.h"
#include "limits.h"

#define EXTRA_LENGTH 100

extern CWindow *window;

static void search_state_update();

static void
search_state_update ()
{
	const gchar *code;
	const gchar *token;
	gint matched;

	if (!ui_have_editor ()) {
		return;
	}

	code = ui_current_editor_code ();

	token = ui_search_entry_get_text ();

	if (code[0] == 0 || token[0] == 0) {
		g_free ((gpointer) code);

		return;
	}

	matched = search_kmp_nth (code, token, -1);
	ui_current_editor_search_init (matched);

	g_free ((gpointer) code);
}

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
	ui_status_entry_new (FILE_OP_CREATE, NULL);
	ui_disable_undo_widgets ();
	ui_disable_redo_widgets ();
}

void
open_open_local_file (GtkWidget *widget, gpointer user_data)
{
	gchar *filepath;

	filepath = (gchar *) g_malloc (MAX_FILEPATH_LENGTH + 1);
	ui_get_filepath_from_dialog (filepath, MAX_FILEPATH_LENGTH, TRUE, FALSE, NULL);

	if (g_strcmp0 (filepath, "NULL") != 0) {
		gboolean exist;

		exist = ui_find_editor (filepath);

		if (!exist) {
			gchar *code_buf;
			gint filesize;
			gint name_offset;

			filesize = misc_get_file_size (filepath);
			misc_get_file_content (filepath, &code_buf);
			ui_editor_new_with_text (filepath, code_buf);
			name_offset = misc_get_file_name_in_path (filepath);
			ui_status_entry_new (FILE_OP_OPEN, filepath);

			g_free ((gpointer) code_buf);
		}
	}

	ui_disable_undo_widgets ();
	ui_disable_redo_widgets ();

	search_state_update ();

	g_free ((gpointer) filepath);
}

void
save_save_current_code (GtkWidget *widget, gpointer user_data)
{
	gchar *filepath;
	gchar *code;

	filepath = (gchar *) g_malloc (MAX_FILEPATH_LENGTH + 1);
	ui_current_editor_filepath (filepath);

	if (g_strcmp0 (filepath, _("Untitled")) == 0) {
		ui_get_filepath_from_dialog (filepath, MAX_FILEPATH_LENGTH, FALSE, FALSE, _("Untitled"));

		if (g_strcmp0 (filepath, "NULL") == 0) {
			g_free ((gpointer) filepath);

			return;
		}
	}

	code = ui_current_editor_code();
	misc_set_file_content (filepath, code);
	ui_save_code_post (filepath);
	ui_status_entry_new (FILE_OP_SAVE, filepath);

	g_free ((gpointer) code);
	g_free ((gpointer) filepath);
}

void
saveas_save_to_file (GtkWidget *widget, gpointer user_data)
{
	gchar *filepath;
	gchar *code;

	filepath = (gchar *) g_malloc (MAX_FILEPATH_LENGTH + 1);
	filepath[0] = '\0';
	ui_get_filepath_from_dialog (filepath, MAX_FILEPATH_LENGTH, FALSE, FALSE, filepath);

	if (g_strcmp0 (filepath, "NULL") != 0) {
		code = ui_current_editor_code();
		misc_set_file_content (filepath, code);
		ui_save_code_post (filepath);
		ui_status_entry_new (FILE_OP_SAVE, filepath);

		g_free ((gpointer) code);
	}

	g_free ((gpointer) filepath);
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

gint
quit_quit_program (GtkWidget *widget, gpointer user_data)
{
	while (ui_have_editor ()) {
		if (!ui_current_editor_close ()) {
			return TRUE;
		}
	}

	gtk_main_quit ();

	return  FALSE;
}

void
format_format_code (GtkWidget *widget, gpointer user_data)
{
	ui_current_editor_format ();
}

void
on_close_page (GtkButton *button, gpointer user_data)
{
	ui_editor_close (GTK_WIDGET (button));
	ui_undo_redo_widgets_update ();
}

void
build_compile (GtkWidget *widget, gpointer user_data)
{
	gchar *project_path;
	gchar *project_name;
	gchar exe_path[MAX_FILEPATH_LENGTH + 1];
	gchar *line;
	gint st;
	gint ed;
	gboolean suc;
	gint error_no;
	gint warning_no;
	
	line = (gchar *) g_malloc (MAX_LINE_LENGTH + 1);
	project_path = project_current_path ();
	project_name = project_current_name ();
	g_strlcpy (exe_path, project_path, MAX_FILEPATH_LENGTH);
	g_strlcat (exe_path, "/", MAX_FILEPATH_LENGTH);
	g_strlcat (exe_path, project_name, MAX_FILEPATH_LENGTH);

	ui_compiletree_clear ();
	ui_compiletree_apend (_("Start building."), 1);

	if (g_strcmp0 ((gchar *) user_data, BUILD_WIDGET_COMPILE) == 0) {
		compile_current_project (project_path, TRUE);
	}
	else if (g_strcmp0 ((gchar *) user_data, BUILD_WIDGET_CLEAR) == 0) {
		compile_current_project (project_path, FALSE);
	}
	else {
		g_free ((gpointer) line);

		return;
	}

	error_no = 0;
	warning_no = 0;
	while (!compile_done ()) {
		compile_getline (line, MAX_LINE_LENGTH);
		
		if (line[0]) {
			ui_compiletree_apend (line, 0);

			if (compile_is_error (line)) {
				error_no++;
			}
			else if (compile_is_warning (line)) {
				warning_no++;
			}
		}
	}
	ui_compiletree_apend (_("Building finished."), 1);

	if (g_strcmp0 ((gchar *) user_data, BUILD_WIDGET_COMPILE) == 0) {
		suc = misc_file_exist (exe_path);

		if (suc) {
			ui_enable_project_widgets ();
		}
	}
	else if (g_strcmp0 ((gchar *) user_data, BUILD_WIDGET_CLEAR) == 0) {
		ui_disable_project_widgets ();
		ui_enable_build_widgets ();
	}

	g_free ((gpointer) line);
}	

void
run_run_executable (GtkWidget *widget, gpointer user_data)
{
	gchar *project_path;
	gchar *project_name;
	gchar exe_path[MAX_FILEPATH_LENGTH + 1];

	project_path = project_current_path ();
	project_name = project_current_name ();
	g_strlcpy (exe_path, project_path, MAX_FILEPATH_LENGTH);
	g_strlcat (exe_path, "/", MAX_FILEPATH_LENGTH);
	g_strlcat (exe_path, project_name, MAX_FILEPATH_LENGTH);

	misc_exec_file (exe_path);
}

void
on_switch_page (GtkNotebook *notebook, GtkWidget *page, guint page_num,
				gpointer user_data)
{
	ui_switch_page ();
	ui_undo_redo_widgets_update ();

	search_state_update ();
}

void
on_editor_insert (GtkTextBuffer *textbuffer, GtkTextIter *location,
				  gchar *text, gint len, gpointer user_data)
{
	gint i;
	gint linecount;
	gboolean menu_showed;
	gint end_line;
	gint offset;

	offset = gtk_text_iter_get_offset (location);
	ui_current_editor_step_add (TRUE, offset - len, len, NULL);

	linecount = 0;
	for (i = 0; text[i]; i++) {
		if (text[i] == '\n') {
			linecount++;
		}
	}

	ui_highlight_on_insert (textbuffer, location, linecount, &end_line);

	if ((text[0] == '\n' || text[0] == '}' || text[0] == '{') && len == 1) {
		autoindent_apply (textbuffer, location, end_line, end_line);
	}

	ui_tip_window_destory ();
	if (text[0] == '(' && len == 1) {
		gchar *line;
		GList *sign;
		gint i;

		line = (gchar *) g_malloc (MAX_LINE_LENGTH + 1);
		ui_current_editor_line (line, MAX_LINE_LENGTH, end_line);

		while (! CHAR (line[strlen (line) - 1]) && ! DIGIT (line[strlen (line) - 1])) {
			line[strlen (line) - 1] = 0;
		}
		i = strlen (line) - 1;
		while ((CHAR (line[i]) || DIGIT (line[i])) && line[i] != '.' && ! SPACE (line[i])) {
			i--;
		}
		i++;

		sign = NULL;
		symbol_function_get_sign (line + i, &sign);

		if (g_list_length (sign) != 0) {
			ui_function_autocomplete (line + i, sign);
		}

		g_list_free (sign);
		g_free ((gpointer) line);
	}

	if ((text[0] == '.' || text[0] == '>' || text[0] == ':') && len == 1) {
		gchar *line;
		GList *funs;
		GList *vars;
		gint i;
		gint line_offset;

		line = (gchar *) g_malloc (MAX_LINE_LENGTH + 1);
		ui_current_editor_line (line, MAX_LINE_LENGTH, end_line);
		line_offset = gtk_text_iter_get_line_offset (location);
		line[line_offset - 1] = 0;

		if ((line[strlen (line) - 1] == '-' && text[0] == '>') ||
			(line[strlen (line) - 1] == ':' && text[0] == ':') ||
			text[0] == '.') {
			if (text[0] == '>' || text[0] == ':') {
				line[strlen (line) - 1] = 0;
			}
			while (strlen (line) > 0 && !CHAR (line[strlen (line) - 1]) &&
				   !DIGIT (line[strlen (line) - 1])) {
				line[strlen (line) - 1] = 0;
			}
			i = strlen (line) - 1;
			while (i >= 0 && (CHAR (line[i]) || DIGIT (line[i])) && line[i] != '.') {
				i--;
			}
			i++;

			funs = NULL;
			vars = NULL;

			if (text[0] == ':') {
				symbol_namespace_get_member (line + i, &funs, &vars);
			}
			else {
				symbol_variable_get_member (line + i, end_line + 1, 
											text[0] == '>', &funs, &vars);
			}

			if (g_list_length (funs) != 0 || g_list_length (vars) != 0) {
				ui_member_autocomplete (funs, vars);
				menu_showed = TRUE;
			}

			g_list_free (funs);
			g_list_free (vars);
		}

		g_free ((gpointer) line);
	}

	if ((CHAR (text[0]) || DIGIT (text[0])) && len == 1 &&
		ui_member_menu_active () && text[0] != '.') {
		ui_member_menu_update (FALSE, text[0]);
	}
	else if (len > 1 || !menu_showed) {
		ui_member_menu_destroy ();
	}
	
	ui_current_editor_set_dirty ();
	ui_update_line_number_label (TRUE, linecount, NULL, NULL);
	ui_current_editor_update_cursor();

	ui_undo_redo_widgets_update ();
}

void
on_editor_delete (GtkTextBuffer *textbuffer, GtkTextIter *start,
				  GtkTextIter *end, gpointer user_data)
{
	ui_highlight_on_delete (textbuffer, start, end);

	ui_current_editor_set_dirty ();
	
	ui_current_editor_update_cursor();
	ui_tip_window_destory ();
}

void
on_editor_delete2 (GtkTextBuffer *textbuffer, GtkTextIter *start,
				   GtkTextIter *end, gpointer user_data)
{	
	gint offset;
	const gchar *text;

	ui_update_line_number_label (FALSE, 0, start, end);

	offset = gtk_text_iter_get_offset (start);
	text = gtk_text_buffer_get_text (textbuffer, start, end, TRUE);
	if (strlen (text) == 1  && (CHAR (text[0]) || 
		DIGIT (text[0])) && ui_member_menu_active () && text[0] != '.') {
		ui_member_menu_update (TRUE, 0);
	}
	else {
		ui_member_menu_destroy ();
	}

	ui_current_editor_step_add (FALSE, offset, -1, text);
	ui_undo_redo_widgets_update ();

	g_free ((gpointer) text);
}

void
on_filetree_selection_changed (GtkTreeSelection *treeselection, gpointer user_data)
{		
	/* pending. */
}

void
on_compilertree_selection_changed (GtkTreeSelection *treeselection, gpointer user_data)
{
	/* pending. */
}

void
on_cursor_change (GtkTextView *text_view, GtkMovementStep step,
				  gint count, gboolean extend_selection,
				  gpointer user_data)
{
	ui_current_editor_update_cursor ();
	ui_tip_window_destory ();
}

void
on_textview_clicked (GtkTextBuffer *textbuffer, GtkTextIter *location,
					 GtkTextMark *mark, gpointer user_data)
{
	ui_current_editor_update_cursor ();
	//ui_tip_window_destory ();
}

void
on_mode_change (GtkTextView *text_view, gpointer user_data)
{
	ui_current_editor_change_mode ();
}

void
new_project_show_dialog (GtkWidget *widget, gpointer user_data)
{
	gchar *default_project_path;
	gchar *project_name;
	gchar *project_path;
	gint project_type;
	gint response;

	default_project_path = (gchar *) g_malloc (MAX_FILEPATH_LENGTH + 1);
	project_get_default_path (default_project_path, MAX_FILEPATH_LENGTH);
	response = ui_new_project_dialog_new (default_project_path);
	project_name = (gchar *) g_malloc (MAX_FILEPATH_LENGTH + 1);
	project_path = (gchar *) g_malloc (MAX_FILEPATH_LENGTH + 1);

	if (response == 0) {
		ui_new_project_dialog_info (project_name, MAX_LINE_LENGTH, project_path, MAX_LINE_LENGTH, &project_type);
		project_new(project_name, project_path, project_type);
		ui_start_project (project_name, project_path);
		ui_enable_build_widgets ();
		ui_enable_settings_widgets ();
		ui_disable_open_project_widgets ();

		ui_set_window_title (project_name);
		ui_set_project_label (project_name);
	}
	ui_new_project_dialog_destory ();


	g_free ((gpointer) default_project_path);
	g_free ((gpointer) project_name);
	g_free ((gpointer) project_path);
}

void
on_filetree_clicked (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GdkEventButton *bevent; 
	
	bevent = (GdkEventButton *) event;
	if (bevent->button != 3) {
		return;
	}

	ui_filetree_menu_popup ();
}

void
on_filetree_2clicked (GtkTreeView *tree_view, GtkTreePath *path, GtkTreeViewColumn *column,
					  gpointer user_data)
{
	gchar *filepath;
	gint isfile;

	ui_filetree_current_path (&filepath, &isfile);

	if (isfile) {
		gchar *code_buf;
		gint filesize;
		gint name_offset;
		gint exist;

		exist = ui_find_editor (filepath);

		if (!exist) {
			filesize = misc_get_file_size (filepath);
			misc_get_file_content (filepath, &code_buf);
			ui_editor_new_with_text (filepath, code_buf);

			g_free ((gpointer) code_buf);
		}
		else {
			ui_show_editor_by_path (filepath);
		}
	}

	g_free ((gpointer) filepath);
}

void
on_create_file_clicked (GtkWidget *widget, gpointer user_data)
{
	gint response;
	gchar *filename;
	gchar *filepath;
	gboolean suc;
	gint fold;

	response = ui_create_file_dialog_new ();

	if (response) {
		ui_create_file_dialog_destory ();

		return;
	}

	filename = (gchar *) g_malloc (MAX_FILEPATH_LENGTH + 1);
	ui_create_file_dialog_info (filename, MAX_FILEPATH_LENGTH);
	ui_filetree_current_path (&filepath, NULL);
	fold = ui_filetree_row_second_level ();
	suc = project_create_empty (filepath, filename, fold);

	if (!suc) {
		gchar *message;

		message = _("Failed to create file.");
		ui_create_file_dialog_destory ();
		ui_error_dialog_new (message);

		g_free ((gpointer) filename);
		g_free ((gpointer) filepath);

		return;
	}

	ui_filetree_append_file_to_current (filename);
	ui_status_entry_new (FILE_OP_CREATE_FILE, filename);
	ui_create_file_dialog_destory ();

	g_free ((gpointer) filename);
	g_free ((gpointer) filepath);
}

void
on_open_file_clicked (GtkWidget *widget, gpointer user_data)
{
	gchar *local_file;
	gchar *filename;
	gchar *filepath;
	gint offset;
	gboolean suc;
	gint fold;

	local_file = (gchar *) g_malloc (MAX_FILEPATH_LENGTH + 1);
	ui_get_filepath_from_dialog (local_file, MAX_FILEPATH_LENGTH, TRUE, FALSE, NULL);

	if (g_strcmp0 (local_file, "NULL") != 0) {
		filename = (gchar *) g_malloc (MAX_FILEPATH_LENGTH + 1);
		ui_filetree_current_path (&filepath, NULL);

		offset = misc_get_file_name_in_path (local_file);

		g_strlcpy (filename, local_file + offset + 1, MAX_FILEPATH_LENGTH);
		fold = ui_filetree_row_second_level ();
		suc = project_add_file (filepath, filename, local_file, fold);

		if (!suc) {
			gchar *message;

			message = _("Failed to add file to project.");
			ui_error_dialog_new (message);
		}
		else {
			ui_filetree_append_file_to_current (filename);
			ui_status_entry_new (FILE_OP_ADD_FILE, filename);
		}

		g_free ((gpointer) filename);
		g_free ((gpointer) filepath);
	}
	g_free ((gpointer) local_file);
}

void
on_delete_file_clicked (GtkWidget *widget, gpointer user_data)
{
	gchar *filepath;
	gboolean suc = 1;
	gint fold;
	gint offset;
	gchar *message;
	gint response;

	ui_filetree_current_path (&filepath, NULL);
	fold = ui_filetree_row_second_level ();

	message = _("Delete file from project and filesystem?");
	response = ui_confirm_dialog_new (message);

	if (response == GTK_RESPONSE_YES) {
		suc = project_delete_file (filepath, fold);
		if (!suc) {
			message = _("Failed to remove file from filesystem.");
			ui_error_dialog_new (message);
		}
		else {
			offset = misc_get_file_name_in_path (filepath);
			ui_filetree_remove_item (filepath);
			ui_status_entry_new (FILE_OP_DELETE_FILE, filepath + offset + 1);
			ui_editor_close_by_path (filepath);
		}
	}

	g_free ((gpointer) filepath);
}

void
on_open_project (GtkWidget *widget, gpointer user_data)
{
	gchar *filepath;
	gchar *project_name;
	gchar *project_path;
	gchar *project_root;
	gint offset;
	GList *fold1;
	GList *fold2;
	GList *fold3;

	filepath = (gchar *) g_malloc (MAX_FILEPATH_LENGTH + 1);
	ui_get_filepath_from_dialog (filepath, MAX_FILEPATH_LENGTH, TRUE, TRUE, NULL);

	if (g_strcmp0 (filepath, "NULL") != 0) {
		project_new_from_xml (filepath);
		project_get_file_lists (&fold1, &fold2, &fold3);
		project_path = project_current_path ();
		project_name = project_current_name ();
		project_root = (gchar *) g_malloc (MAX_FILEPATH_LENGTH + 1);
		g_strlcpy (project_root, project_path, MAX_FILEPATH_LENGTH);
		offset = misc_get_file_name_in_path (project_path);
		project_root[offset] = 0;
		ui_start_project (project_name, project_root);
		ui_append_files_to_second_level (fold1, 0);
		ui_append_files_to_second_level (fold2, 1);
		ui_append_files_to_second_level (fold3, 2);
		ui_enable_build_widgets ();
		ui_enable_settings_widgets ();
		ui_disable_open_project_widgets ();

		ui_set_window_title (project_name);
		ui_set_project_label (project_name);

		g_free ((gpointer) project_root);
	}


	g_free ((gpointer) filepath);
}

void
on_project_settings_clicked (GtkWidget *widget, gpointer user_data)
{
	gint response;
	gchar *libs;
	gchar *opts;

	libs = (gchar *) g_malloc (MAX_FILEPATH_LENGTH + 1);
	opts = (gchar *) g_malloc (MAX_FILEPATH_LENGTH + 1);
	project_get_settings (libs, MAX_OPTION_LENGTH, opts, MAX_OPTION_LENGTH);
	response = ui_project_settings_dialog_new (libs, opts);

	if (response) {
		ui_project_settings_dialog_destory ();

		g_free ((gpointer) libs);
		g_free ((gpointer) opts);

		return;
	}

	ui_project_settings_dialog_info (libs, opts);
	project_set_settings (libs, opts);
	ui_project_settings_dialog_destory ();

	g_free ((gpointer) libs);
	g_free ((gpointer) opts);
}

void
on_autocomplete_item_clicked (GtkWidget *widget, gpointer user_data)
{
	const gchar *text;
	const gchar *prefix;
	GtkLabel *label;

	label = GTK_LABEL (gtk_bin_get_child (GTK_BIN (widget)));
	text = gtk_label_get_text(label);
	prefix = ui_member_menu_prefix ();
	ui_current_editor_insert (text + strlen (prefix));
}

void
on_preferences_clicked (GtkWidget *widget, gpointer user_data)
{
	ui_preferences_window_show ();
}

void
on_preferences_close_clicked (GtkWidget *widget, gpointer user_data)
{
	ui_preferences_config_update ();
	ui_preferences_window_hide ();
}

void
on_preferences_close_clicked_after (GtkWidget *widget, gpointer user_data)
{
	ui_editors_breakpoint_tag_update ();
}

void
on_line_label_2clicked (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	GdkEventButton *bevent;
	gchar *breakpoint_desc;
	
	bevent = (GdkEventButton *) event;
	if (bevent->type != GDK_2BUTTON_PRESS) {
		return;
	}

	breakpoint_desc = (gchar *) g_malloc (MAX_FILEPATH_LENGTH + 1);
	ui_current_editor_breakpoint_update (bevent->x, bevent->y, breakpoint_desc);

	if (debug_is_active ()) {
		debug_breakpoint_update (breakpoint_desc);
	}

	g_free ((gpointer) breakpoint_desc);
}

void
on_watchtree_edited (GtkCellRendererText *cell, gchar *path_string,
					 gchar *new_text,  gpointer user_data)
{
	gchar *value;

	if (new_text[0] == 0) {
		return;
	}

	value = (gchar *) g_malloc (MAX_LINE_LENGTH + 1);
	debug_expression_value (new_text, value, MAX_LINE_LENGTH);
	ui_watchtree_cell_change (path_string, new_text, value);

	g_free ((gpointer) value);
}

void
on_debug_action_clicked (GtkWidget *widget, gpointer user_data)
{
	gchar *action;
	gint line;
	gchar *name;
	gchar *value;
	gchar *frame_name;
	gchar *frame_args;
	gchar *file_line;
	gchar *project_path;
	gchar *filename;
	gchar filepath[MAX_FILEPATH_LENGTH + 1];
	GList *output_list;
	GList *value_list;
	GList *iterator;

	action = (gchar *) user_data;
	if (!debug_is_active () && (g_strcmp0 (action, DEBUG_WIDGET_START) != 0)) {
		return;
	}

	if (g_strcmp0 (action, DEBUG_WIDGET_START) == 0) {
		gchar *project_name;
		GList *breakpoint_desc_list = NULL;

		project_path = project_current_path ();
		project_name = project_current_name ();

		debug_startup (project_path, project_name);
		debug_connect (project_path, project_name);

		if (!debug_is_active ()) {
			ui_status_entry_new (FILE_OP_WARNING, _("failed to start or conect to debug process."));

			return;
		}
		breakpoint_desc_list = NULL;
		ui_breakpoint_tags_get (&breakpoint_desc_list);
		debug_breakpoints_insert (breakpoint_desc_list);
		g_list_free_full (breakpoint_desc_list, g_free);

		debug_command_exec ("c", NULL, NULL);
	}
	else if (g_strcmp0 (action, DEBUG_WIDGET_NEXT) == 0) {
		debug_command_exec ("n", NULL, NULL);
	}
	else if (g_strcmp0 (action, DEBUG_WIDGET_STEP) == 0) {
		debug_command_exec ("s", NULL, NULL);
	}
	else if (g_strcmp0 (action, DEBUG_WIDGET_CONTINUE) == 0) {
		debug_command_exec ("c", NULL, NULL);
	}

	filename = (gchar *) g_malloc (MAX_FILEPATH_LENGTH + 1);
	debug_current_file_line (g_strcmp0 (action, DEBUG_WIDGET_START) == 0,
							 filename, MAX_FILEPATH_LENGTH, &line);

	project_path = project_current_path ();
	g_strlcpy (filepath, project_path, MAX_FILEPATH_LENGTH);
	g_strlcat (filepath, "/", MAX_FILEPATH_LENGTH);
	g_strlcat (filepath, filename, MAX_FILEPATH_LENGTH);

	if (!misc_file_exist (filepath)) {
		if (g_strcmp0 (action, DEBUG_WIDGET_START) == 0 ||
			g_strcmp0 (action, DEBUG_WIDGET_NEXT) == 0 ||
			g_strcmp0 (action, DEBUG_WIDGET_CONTINUE) == 0) {
			ui_debug_view_clear ();
			ui_debug_ptr_remove ();

			g_free ((gpointer) filename);

			return;
		}
		else if (g_strcmp0 (action, DEBUG_WIDGET_STEP) == 0) {
			ui_debug_view_clear ();
			ui_debug_ptr_remove ();

			debug_command_exec ("finish", NULL, NULL);
			debug_current_file_line (FALSE, filename, MAX_FILEPATH_LENGTH, &line);
			g_strlcpy (filepath, project_path, MAX_FILEPATH_LENGTH);
			g_strlcat (filepath, "/", MAX_FILEPATH_LENGTH);
			g_strlcat (filepath, filename, MAX_FILEPATH_LENGTH);
		}
	}

	if (!misc_file_exist (filepath) || filename[0] == 0) {
		debug_command_exec ("c", NULL, NULL);

		g_free ((gpointer) filename);

		return;
	}

	if (!ui_find_editor (filepath)) {
		gchar *code_buf;
		gint filesize;
		gint name_offset;

		filesize = misc_get_file_size (filepath);
		misc_get_file_content (filepath, &code_buf);
		ui_editor_new_with_text (filepath, code_buf);

		g_free ((gpointer) code_buf);
	}
	ui_select_editor_with_path (filepath);
	ui_debug_ptr_add (filepath, line);
	ui_debug_view_clear ();

	output_list = NULL;
	debug_current_locals (&output_list);
	name = (gchar *) g_malloc (MAX_LINE_LENGTH + 1);
	value = (gchar *) g_malloc (MAX_LINE_LENGTH + 1);
	for (iterator = output_list; iterator; iterator = iterator->next) {
		gchar *local;

		local = (gchar *) iterator->data;
		sscanf (local, "%s", name);
		g_strlcpy (value, local + strlen (name) + 1, MAX_LINE_LENGTH);

		ui_debug_view_locals_add (name, value);
	}

	g_list_free_full (output_list, g_free);
	output_list = NULL;

	debug_current_stack (&output_list);
	frame_name = (gchar *) g_malloc (MAX_LINE_LENGTH + 1);
	frame_args = (gchar *) g_malloc (MAX_LINE_LENGTH + 1);
	file_line = (gchar *) g_malloc (MAX_LINE_LENGTH + 1);
	for (iterator = output_list; iterator; iterator = iterator->next) {
		gchar *line;

		line = (gchar *) iterator->data;
		sscanf (line, "%s %s", frame_name, file_line);
		g_strlcpy (frame_args, line + strlen (frame_name)  + strlen (file_line) + 2, MAX_LINE_LENGTH);

		ui_debug_view_stack_add (frame_name, frame_args, file_line);
	}

	g_list_free_full (output_list, g_free);
	output_list = NULL;

	ui_debug_view_get_all_expression (&output_list);
	value_list = NULL;
	for (iterator = output_list; iterator; iterator = iterator->next) {
		gchar *line;
		gchar *value;

		line = (gchar *) iterator->data;
		value = (gchar *) g_malloc (MAX_LINE_LENGTH + 1);
		debug_expression_value (line, value, MAX_LINE_LENGTH);
		value_list = g_list_append (value_list, (gpointer) value);
	}
	ui_debug_view_set_values (value_list);

	g_list_free_full (output_list, g_free);
	g_list_free_full (value_list, g_free);

	if (g_strcmp0 (action, DEBUG_WIDGET_START) == 0) {
		ui_enable_debug_view ();
		ui_disable_project_widgets ();
		ui_enable_debug_widgets ();

		g_timeout_add (200, debug_monitor, NULL);
	}

	g_free ((gpointer) name);
	g_free ((gpointer) value);
	g_free ((gpointer) frame_name);
	g_free ((gpointer) frame_args);
	g_free ((gpointer) file_line);
	g_free ((gpointer) filename);
}

void
on_debug_stop_clicked (GtkWidget *widget, gpointer user_data)
{
	debug_stop ();

	ui_debug_ptr_remove ();
	ui_disable_debug_widgets ();
	ui_enable_project_widgets ();
}

void
on_undo_clicked (GtkWidget *widget, gpointer user_data)
{
	if (!ui_current_editor_can_undo ()) {
		return;
	}

	ui_current_editor_undo ();
	ui_undo_redo_widgets_update ();
}

void
on_redo_clicked (GtkWidget *widget, gpointer user_data)
{
	if (!ui_current_editor_can_redo ()) {
		return;
	}

	ui_current_editor_redo ();
	ui_undo_redo_widgets_update ();
}

void
on_search_entry_changed (GtkEntry *entry, gpointer user_data)
{
	search_state_update ();
}

void
on_textbuffer_changed (GtkTextBuffer *textbuffer, gpointer user_data)
{
	search_state_update ();
}

void
on_search_clicked (GtkWidget *widget, gpointer user_data)
{
	gint next;
	const gchar *code;
	const gchar *token;
	gint offset;
	gint len;

	if (!ui_have_editor ()) {
		return;
	}

	if (g_strcmp0 ((gchar *) user_data, SEARCH_WIDGET_NEXT) == 0) {
		next = ui_current_editor_search_next (FALSE);
	}
	else {
		next = ui_current_editor_search_next (TRUE);
	}

	code = ui_current_editor_code ();
	token = ui_search_entry_get_text ();
	len = strlen (token);

	if (code[0] == 0 || token[0] == 0 || next == -1) {
		g_free ((gpointer) code);

		return;
	}

	offset = search_kmp_nth (code, token, next);

	if (offset != -1) {
		ui_current_editor_select_range (offset, len);
	}

	g_free ((gpointer) code);
}

gboolean
on_key_pressed (GtkWidget *widget, GdkEventKey *event, gpointer user_data)
{
	gboolean NOUSED;

	if (event->keyval == GDK_KEY_Left || event->keyval == GDK_KEY_Right) {
		ui_member_menu_destroy ();
	}
	else if (event->keyval == GDK_KEY_Down || event->keyval == GDK_KEY_Up){
		ui_member_menu_index_change (event->keyval == GDK_KEY_Down? -1: 1);

		return TRUE;
	}
	else if (event->keyval == GDK_KEY_Return) {
		ui_member_menu_active_current ();
		ui_member_menu_destroy ();

		return TRUE;
	}
	else {
		ui_member_menu_hide ();
	}

	g_signal_emit_by_name (ui_get_current_editor ()->textview, "key-press-event", event, &NOUSED, NULL);	

	return TRUE;
}

void
do_nothing (GtkTextBuffer *textbuffer, GtkTextIter *start,
			GtkTextIter *end, gpointer user_data)
{
}
