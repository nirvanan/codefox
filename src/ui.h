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
#include "debugview.h"

#define DEBUG_WIDGET_START "start"
#define DEBUG_WIDGET_NEXT "next"
#define DEBUG_WIDGET_STEP "step"
#define DEBUG_WIDGET_CONTINUE "continue"

#define SEARCH_WIDGET_PRE "search_pre"
#define SEARCH_WIDGET_NEXT "search_next"

#define BUILD_WIDGET_COMPILE "compile"
#define BUILD_WIDGET_CLEAR "clear"


typedef struct {
	GObject *toplevel;
	GObject *about_item;
	GObject *help_item;
	GObject *new_item;
	GObject *open_item;
	GObject *save_item;
	GObject *saveas_item;
	GObject *quit_item;
	GObject *undo_item;
	GObject *redo_item;
	GObject *cut_item;
	GObject *copy_item;
	GObject *paste_item;
	GObject *delete_item;
	GObject *preferences_item;
	GObject *new_project_item;
	GObject *open_project_item;
	GObject *format_item;
	GObject *build_item;
	GObject *run_item;
	GObject *debug_item;
	GObject *clear_item;
	GObject *next_item;
	GObject *step_item;
	GObject *continue_item;
	GObject *stop_item;
	GObject *settings_item;
	GObject *new_toolbar;
	GObject *open_toolbar;
	GObject *save_toolbar;
	GObject *saveas_toolbar;
	GObject *new_project_toolbar;
	GObject *open_project_toolbar;
	GObject *build_toolbar;
	GObject *run_toolbar;
	GObject *debug_toolbar;
	GObject *clear_toolbar;
	GObject *next_toolbar;
	GObject *step_toolbar;
	GObject *continue_toolbar;
	GObject *stop_toolbar;
	GObject *code_notebook;
	GObject *info_notebook;
	GObject *statustree;
	GObject *compilertree;
	GObject *notepadview;
	GObject *projectlabel;
	GObject *locationlabel;
	GObject *modelabel;
	GObject *filetree;
	GObject *horizontal_paned;
	GObject *vertical_paned;
	GObject *status_image;
	GObject *debug_ptr;
	GObject *search_entry;
	GObject *pre_search_toolbar;
	GObject *next_search_toolbar;
	CDebugView *debug_view;
	GList *editor_list;
} CWindow;

typedef struct {
	GObject *toplevel;
	GObject *name_entry;
	GObject *path_entry;
	GObject *type_box;
} CNewProjectDialog;

typedef struct {
	GObject *toplevel;
	GObject *name_entry;
} CCreateFileDialog;

typedef struct {
	GObject *toplevel;
	GObject *libs_entry;
	GObject *opts_entry;
} CProjectSettingsDialog;

typedef struct {
	GObject *menu;
	GObject *add_item;
	GObject *create_item;
	GObject *delete_item;
} CFileTreeMenu;

typedef struct {
	GObject *tip_window;
	GObject *tip_label;
	gboolean active;
} CFunctionTip;

typedef struct {
	GObject *menu;
	gboolean active;
	gchar *prefix;
	GList *item_list;
	gint index;
} CMemberMenu;

typedef struct {
	GObject *toplevel;
	GObject *font_button;
	GObject *keyword_button;
	GObject *string_button;
	GObject *constant_button;
	GObject *comment_button;
	GObject *preprocessor_button;
	GObject *close_button;
} CPreferencesWindow;

typedef enum {
	FILE_OP_CREATE,
	FILE_OP_SAVE,
	FILE_OP_OPEN,
	FILE_OP_NEW_PROJECT,
	FILE_OP_ADD_FILE,
	FILE_OP_CREATE_FILE,
	FILE_OP_DELETE_FILE,
	FILE_OP_WARNING,
	FILE_OP_ERROR
} CFileOp;

typedef enum {
	CLIPBOARD_CUT,
	CLIPBOARD_COPY,
	CLIPBOARD_PASTE
} CClipboradSignal;

typedef enum {
	NEW_PROJECT_CREATE,
	NEW_PROJECT_CANCEL
} CNewProjectResponse;

void
ui_disable_save_widgets ();

void
ui_enable_save_widgets ();

void
ui_enable_project_widgets ();

void
ui_disable_project_widgets ();

void
ui_enable_build_widgets ();

void
ui_disable_build_widgets ();

void
ui_enable_debug_widgets ();

void
ui_disable_debug_widgets ();

void
ui_enable_open_project_widgets ();

void
ui_disable_open_project_widgets ();

void
ui_enable_settings_widgets ();

void
ui_disable_settings_widgets ();

void
ui_enable_undo_widgets ();

void
ui_disable_undo_widgets ();

void
ui_enable_redo_widgets ();

void
ui_disable_redo_widgets ();

void
ui_init ();

void
ui_about_dialog_new ();

void
ui_editor_new ();

void
ui_editor_new_with_text (const gchar *filepath, const gchar *code_buf);

void
ui_filetree_entry_new (gboolean is_file, gchar *filename, gchar *filepath);

void
ui_status_entry_new (const gint op, const gchar *filepath);

void
ui_get_filepath_from_dialog (gchar *filepath, const gint size, const gboolean open,
                             const gboolean project, const gchar *default_path);

gboolean
ui_have_editor ();

gboolean
ui_find_editor (const gchar *filepath);

CEditor *
ui_get_editor (const gchar *filepath);

void
ui_show_editor_by_path (const gchar *filepath);

void
ui_current_editor_filepath (gchar *filepath);

gchar *
ui_current_editor_code ();

void
ui_save_code_post (const gchar *filepath);

void
ui_emit_clipboard_signal (gint clipboard_signal);

void
ui_emit_close_signal ();

void
ui_emit_save_signal ();

void
ui_current_editor_delete_range ();

void
ui_current_editor_format ();

void
ui_current_editor_update_cursor ();

void
ui_current_editor_change_mode ();

void
ui_update_line_number_label (const gboolean insert, const gint append, 
							const GtkTextIter *start, const GtkTextIter *end);

void
ui_switch_page();

gint
ui_new_project_dialog_new (const gchar *default_project_path);

gint
ui_create_file_dialog_new ();

gint
ui_project_settings_dialog_new (const gchar* libs, const gchar* opts);

void
ui_new_project_dialog_destory ();

void
ui_create_file_dialog_destory ();

void
ui_project_settings_dialog_destory ();

void
ui_new_project_dialog_info (gchar *name, const gint name_size, gchar *path, const gint path_size, gint *type);

void
ui_create_file_dialog_info (gchar *name, const gint size);

void
ui_project_settings_dialog_info (gchar *libs, gchar *opts);

void
ui_start_project (const gchar *project_name, const gchar *project_path);

void
ui_filetree_menu_popup ();

void
ui_filetree_current_path (gchar **path, gint *isfile);

void
ui_filetree_append_file_to_current (const gchar *filename);

void
ui_error_dialog_new (const gchar *message);

gint
ui_confirm_dialog_new (const gchar *message);

gint
ui_filetree_row_second_level ();

void
ui_filetree_remove_item (const gchar *filepath);

void
ui_compiletree_apend (const gchar *line, gint is_message);

void
ui_compiletree_clear ();

void
ui_append_files_to_second_level (const GList *list, const gint row);

void
ui_current_editor_line (gchar *line, const gint size, const gint lineno);

void
ui_current_editor_error_tag_clear ();

void
ui_current_editor_error_tag_add (const gint row, const gint column, const gint len);

void
ui_function_autocomplete (const gchar *name, const GList *signs);

void
ui_member_autocomplete (const GList *funs, const GList *vars);

void
ui_member_menu_index_change (const gint change);

void
ui_member_menu_active_current ();

void
ui_current_editor_insert_location (gint *x, gint *y);

void
ui_current_editor_insert (const gchar *text);

void
ui_tip_window_destory ();

gboolean
ui_member_menu_active ();

void
ui_member_menu_destroy ();

void
ui_member_menu_hide ();

void
ui_member_menu_update (const gboolean del, const gchar ch);

const gchar *
ui_member_menu_prefix ();

gint
ui_editor_close (GtkWidget *button);

gint
ui_current_editor_close ();

void
ui_current_editor_set_dirty ();

void
ui_status_image_set (const gboolean error, const gboolean warning);

void
ui_highlight_on_delete (GtkTextBuffer *textbuffer, GtkTextIter *start,
						 GtkTextIter *end);

void
ui_highlight_on_insert (GtkTextBuffer *textbuffer, GtkTextIter *location, gint lines, gint *end_line_ptr);

void
ui_preferences_window_show ();

void
ui_preferences_window_hide ();

void
ui_preferences_config_update ();

void
ui_current_editor_breakpoint_update (gdouble x, gdouble y, gchar *breakpoint_desc);

void
ui_editors_breakpoint_tag_update ();

void
ui_watchtree_cell_change (const gchar *path_string, const gchar *new_text, const gchar *value);

void
ui_enable_debug_view ();

void
ui_disable_debug_view ();

void
ui_breakpoint_tags_get (GList **list);

void
ui_debug_ptr_add (const gchar *filepath, const gint line);

void
ui_debug_ptr_remove ();

void
ui_debug_view_clear ();

void
ui_debug_view_locals_add (const gchar *name, const gchar *value);

void
ui_debug_view_stack_add (const gchar *frame_name, const gchar *frame_args,
						 const gchar *file_line);

void
ui_select_editor_with_path (const gchar *filepath);

void
ui_debug_view_get_all_expression (GList **list);

void
ui_debug_view_set_values (GList *list);

CEditor *
ui_get_current_editor();

void
ui_current_editor_step_add (const gboolean insert, const gint offset, const gint len,
							const gchar *text);

gboolean
ui_current_editor_can_undo ();

gboolean
ui_current_editor_can_redo ();

void
ui_current_editor_undo ();

void
ui_current_editor_redo ();

void
ui_undo_redo_widgets_update ();

const gchar *
ui_search_entry_get_text ();

void
ui_current_editor_search_init (const gint matched);

gint
ui_current_editor_search_next (const gboolean pre);

void
ui_current_editor_select_range (const gint offset, const gint len);

void
ui_set_window_title (const gchar *project_name);

void
ui_set_project_label (const gchar *project_name);

void
ui_editor_close_by_path (const gchar *filepath);

void
ui_current_editor_move_cursor (const gint offset);

#endif
