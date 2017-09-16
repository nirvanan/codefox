/*
 * ui.c
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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <gtk/gtk.h>
#include <glib/gi18n-lib.h>
#include "callback.h"
#include "ui.h"
#include "filetree.h"
#include "misc.h"
#include "staticcheck.h"
#include "symbol.h"
#include "prefix.h"
#include "editor.h"
#include "editorconfig.h"
#include "project.h"
#include "env.h"
#include "limits.h"

#define PAGE_INFO 0
#define PAGE_COMPILE 1
#define PAGE_DEBUG 2

#define MESSAGE_BUF_SIZE 1000
#define MAX_TIP_LENGTH 10000

#define LOGO_SIZE 160

/* GTK stocks. */
#define CODEFOX_STOCK_OPEN "document-open"
#define CODEFOX_STOCK_NEW "document-new"
#define CODEFOX_STOCK_DELETE "edit-delete"

/* Non-GTK stocks. */
#define CODEFOX_STOCK_BUILD "codefox-build"
#define CODEFOX_STOCK_RUN "codefox-run"
#define CODEFOX_STOCK_DEBUG "codefox-debug"
#define CODEFOX_STOCK_FUNCTION "codefox-function"
#define CODEFOX_STOCK_VARIABLE "codefox-variable"
#define CODEFOX_STOCK_STATUSERROR "codefox-statuserror"
#define CODEFOX_STOCK_STATUSWARNING "codefox-statuswarning"
#define CODEFOX_STOCK_STATUSPASS "codefox-statuspass"

/* Main window of codefox. */
CWindow *window;

static CNewProjectDialog *new_project_dialog;
static CCreateFileDialog *create_file_dialog;
static CProjectSettingsDialog *project_settings_dialog;
static CFileTreeMenu *filetree_menu;
static CFunctionTip *function_tip;
static CMemberMenu *member_menu;
static CPreferencesWindow *preferences_window;

static GMutex ui_mutex;

static const gchar *license = 
"Codefox is free software: you can redistribute it and/or modify it\n"
"under the terms of the GNU General Public License as published\n"
"by the Free Software Foundation, either version 3 of the License,\n"
"or (at your option) any later version.\n\n"
"Codefox is distributed in the hope that it will be useful, but\n"
"WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
"See the GNU General Public License for more details.\n\n"
"You should have received a copy of the GNU General Public License\n"
"along with Codefox. If not, see <http://www.gnu.org/licenses/>.";
static const gchar *authors[] = {"Gordon Lee", NULL};

static GtkStockItem items[] =
{
	{CODEFOX_STOCK_BUILD, "Build", 0, 0, NULL},
	{CODEFOX_STOCK_RUN, "Run", 0, 0, NULL},
	{CODEFOX_STOCK_DEBUG, "Debug", 0, 0, NULL },
	{CODEFOX_STOCK_ERROR, "Error", 0, 0, NULL },
	{CODEFOX_STOCK_WARNING, "Warning", 0, 0, NULL },
	{CODEFOX_STOCK_FILE, "File", 0, 0, NULL },
	{CODEFOX_STOCK_DIR, "Dir", 0, 0, NULL },
	{CODEFOX_STOCK_FUNCTION, "Function", 0, 0, NULL },
	{CODEFOX_STOCK_VARIABLE, "Variable", 0, 0, NULL },
	{CODEFOX_STOCK_STATUSERROR, "Statuserror", 0, 0, NULL },
	{CODEFOX_STOCK_STATUSWARNING, "Statuswarning", 0, 0, NULL },
	{CODEFOX_STOCK_STATUSPASS, "Statuspass", 0, 0, NULL },
	{CODEFOX_STOCK_BREAKPOINT, "Breakpoint", 0, 0, NULL },
	{CODEFOX_STOCK_DEBUGPTR, "Debugptr", 0, 0, NULL }
};

static const gchar *sig[3] = {"cut-clipboard", "copy-clipboard", 
							  "paste-clipboard"};

static void signal_connect ();
static void ui_window_init(GtkBuilder *builder, CWindow *window);
static void ui_filetree_init(CWindow *window);
static void ui_toolpad_init(CWindow *window);
static void ui_debug_view_init (GtkBuilder *builder);
static void ui_new_project_dialog_init (GtkBuilder *builder);
static void ui_filetree_menu_init ();
static void ui_editorconfig_init ();
static void ui_project_label_init ();
static void ui_editors_config_update ();

/* Connect all callbacks widgets need. */
static void
signal_connect ()
{
	GtkTreeSelection *select;

	/* Connect signal handlers with widgets. */
	g_signal_connect (window->about_item, "activate", 
					  G_CALLBACK (about_show_about), NULL);
	g_signal_connect (window->help_item, "activate", 
					  G_CALLBACK (help_goto_website), NULL);
	g_signal_connect (window->new_item, "activate", 
					  G_CALLBACK (new_create_new_tab), NULL);
	g_signal_connect (window->open_item, "activate", 
					  G_CALLBACK (open_open_local_file), NULL);
	g_signal_connect (window->save_item, "activate", 
					  G_CALLBACK (save_save_current_code), NULL);
	g_signal_connect (window->saveas_item, "activate", 
					  G_CALLBACK (saveas_save_to_file), NULL);
	g_signal_connect (window->quit_item, "activate", 
					  G_CALLBACK (quit_quit_program), NULL);
	g_signal_connect (window->undo_item, "activate", 
					  G_CALLBACK (on_undo_clicked), NULL);
	g_signal_connect (window->redo_item, "activate", 
					  G_CALLBACK (on_redo_clicked), NULL);
	g_signal_connect (window->cut_item, "activate", 
					  G_CALLBACK (cut_cut_code), NULL);
	g_signal_connect (window->copy_item, "activate", 
					  G_CALLBACK (copy_copy_code), NULL);
	g_signal_connect (window->paste_item, "activate", 
					  G_CALLBACK (paste_paste_code), NULL);
	g_signal_connect (window->delete_item, "activate", 
					  G_CALLBACK (delete_delete_code), NULL);
	g_signal_connect (window->preferences_item, "activate", 
					  G_CALLBACK (on_preferences_clicked), NULL);
	g_signal_connect (window->new_project_item, "activate", 
					  G_CALLBACK (new_project_show_dialog), NULL);
	g_signal_connect (window->open_project_item, "activate", 
					  G_CALLBACK (on_open_project), NULL);
	g_signal_connect (window->format_item, "activate", 
					  G_CALLBACK (format_format_code), NULL);
	g_signal_connect (window->build_item, "activate", 
					  G_CALLBACK (build_compile), BUILD_WIDGET_COMPILE);
	g_signal_connect (window->clear_item, "activate", 
					  G_CALLBACK (build_compile), BUILD_WIDGET_CLEAR);
	g_signal_connect (window->run_item, "activate", 
					  G_CALLBACK (run_run_executable), NULL);
	g_signal_connect (window->debug_item, "activate", 
					  G_CALLBACK (on_debug_action_clicked), DEBUG_WIDGET_START);
	g_signal_connect (window->next_item, "activate", 
					  G_CALLBACK (on_debug_action_clicked), DEBUG_WIDGET_NEXT);
	g_signal_connect (window->step_item, "activate", 
					  G_CALLBACK (on_debug_action_clicked), DEBUG_WIDGET_STEP);
	g_signal_connect (window->continue_item, "activate", 
					  G_CALLBACK (on_debug_action_clicked), DEBUG_WIDGET_CONTINUE);
	g_signal_connect (window->stop_item, "activate", 
					  G_CALLBACK (on_debug_stop_clicked), NULL);
	g_signal_connect (window->settings_item, "activate", 
					  G_CALLBACK (on_project_settings_clicked), NULL);
	g_signal_connect (window->new_toolbar, "clicked", 
					  G_CALLBACK (new_create_new_tab), NULL);
	g_signal_connect (window->open_toolbar, "clicked", 
					  G_CALLBACK (open_open_local_file), NULL);
	g_signal_connect (window->save_toolbar, "clicked", 
					  G_CALLBACK (save_save_current_code), NULL);
	g_signal_connect (window->saveas_toolbar, "clicked", 
					  G_CALLBACK (saveas_save_to_file), NULL);
	g_signal_connect (window->new_project_toolbar, "clicked", 
					  G_CALLBACK (new_project_show_dialog), NULL);
	g_signal_connect (window->open_project_toolbar, "clicked", 
					  G_CALLBACK (on_open_project), NULL);
	g_signal_connect (window->build_toolbar, "clicked", 
					  G_CALLBACK (build_compile), BUILD_WIDGET_COMPILE);
	g_signal_connect (window->clear_toolbar, "clicked", 
					  G_CALLBACK (build_compile), BUILD_WIDGET_CLEAR);
	g_signal_connect (window->run_toolbar, "clicked", 
					  G_CALLBACK (run_run_executable), NULL);
	g_signal_connect (window->debug_toolbar, "clicked", 
					  G_CALLBACK (on_debug_action_clicked), DEBUG_WIDGET_START);
	g_signal_connect (window->next_toolbar, "clicked", 
					  G_CALLBACK (on_debug_action_clicked), DEBUG_WIDGET_NEXT);
	g_signal_connect (window->step_toolbar, "clicked", 
					  G_CALLBACK (on_debug_action_clicked), DEBUG_WIDGET_STEP);
	g_signal_connect (window->continue_toolbar, "clicked", 
					  G_CALLBACK (on_debug_action_clicked), DEBUG_WIDGET_CONTINUE);
	g_signal_connect (window->stop_toolbar, "clicked", 
					  G_CALLBACK (on_debug_stop_clicked), NULL);
	g_signal_connect_after (window->search_entry, "changed", 
					  G_CALLBACK (on_search_entry_changed), NULL);
	g_signal_connect_after (window->pre_search_toolbar, "clicked", 
					  G_CALLBACK (on_search_clicked), SEARCH_WIDGET_PRE); 
	g_signal_connect_after (window->next_search_toolbar, "clicked", 
					  G_CALLBACK (on_search_clicked), SEARCH_WIDGET_NEXT);
	g_signal_connect (window->toplevel, "delete-event", 
					  G_CALLBACK (quit_quit_program), NULL);
						
	g_signal_connect_after (window->code_notebook, "switch-page", 
							G_CALLBACK (on_switch_page), NULL);
	
	/* Load filetreeview and connect sighal handler. */
	
	select = gtk_tree_view_get_selection (GTK_TREE_VIEW(window->filetree));
	g_signal_connect (select, "changed", 
					  G_CALLBACK (on_filetree_selection_changed), NULL);
	select = gtk_tree_view_get_selection (GTK_TREE_VIEW(window->compilertree));
	g_signal_connect (select, "changed", 
					  G_CALLBACK (on_compilertree_selection_changed), NULL);
	g_signal_connect_after (window->filetree, "button-release-event", 
							G_CALLBACK (on_filetree_clicked), NULL);
	g_signal_connect (window->filetree, "row-activated", 
					  G_CALLBACK (on_filetree_2clicked), NULL);
}

/* Get all widgets in the builder by name*/
static void
ui_window_init (GtkBuilder *builder, CWindow *window)
{
	window->toplevel = gtk_builder_get_object (builder, "toplevel");
	window->about_item = gtk_builder_get_object (builder, "aboutmenuitem");
	window->help_item = gtk_builder_get_object (builder, "helpmenuitem");
	window->new_item =  gtk_builder_get_object (builder, "newmenuitem");
	window->open_item =  gtk_builder_get_object (builder, "openmenuitem");
	window->save_item =  gtk_builder_get_object (builder, "savemenuitem");
	window->saveas_item =  gtk_builder_get_object (builder, "saveasmenuitem");
	window->undo_item = gtk_builder_get_object (builder, "undomenuitem");
	window->redo_item = gtk_builder_get_object (builder, "redomenuitem");
	window->quit_item = gtk_builder_get_object (builder, "quitmenuitem");
	window->cut_item =  gtk_builder_get_object (builder, "cutmenuitem");
	window->copy_item =  gtk_builder_get_object (builder, "copymenuitem");
	window->paste_item =  gtk_builder_get_object (builder, "pastemenuitem");
	window->delete_item =  gtk_builder_get_object (builder, "deletemenuitem");
	window->preferences_item = gtk_builder_get_object (builder, "preferencesmenuitem");
	window->new_project_item =  gtk_builder_get_object (builder, "newprojectmenuitem");
	window->open_project_item =  gtk_builder_get_object (builder, "openprojectmenuitem");
	window->format_item =  gtk_builder_get_object (builder, "formatmenuitem");
	window->build_item =  gtk_builder_get_object (builder, "buildmenuitem");
	window->clear_item =  gtk_builder_get_object (builder, "clearmenuitem");
	window->run_item =  gtk_builder_get_object (builder, "runmenuitem");
	window->debug_item =  gtk_builder_get_object (builder, "debugmenuitem");
	window->next_item =  gtk_builder_get_object (builder, "nextmenuitem");
	window->step_item =  gtk_builder_get_object (builder, "stepmenuitem");
	window->continue_item =  gtk_builder_get_object (builder, "continuemenuitem");
	window->stop_item =  gtk_builder_get_object (builder, "stopmenuitem");
	window->settings_item =  gtk_builder_get_object (builder, "settingsmenuitem");
	window->new_toolbar =  gtk_builder_get_object (builder, "newbutton");
	window->open_toolbar =  gtk_builder_get_object (builder, "openbutton");
	window->save_toolbar =  gtk_builder_get_object (builder, "savebutton");
	window->saveas_toolbar =  gtk_builder_get_object (builder, "saveasbutton");
	window->new_project_toolbar =  gtk_builder_get_object (builder, "newprojectbutton");
	window->open_project_toolbar =  gtk_builder_get_object (builder, "openprojectbutton");
	window->build_toolbar =  gtk_builder_get_object (builder, "buildbutton");
	window->clear_toolbar =  gtk_builder_get_object (builder, "clearbutton");
	window->run_toolbar =  gtk_builder_get_object (builder, "runbutton");
	window->debug_toolbar =  gtk_builder_get_object (builder, "debugbutton");
	window->next_toolbar =  gtk_builder_get_object (builder, "nextbutton");
	window->step_toolbar =  gtk_builder_get_object (builder, "stepbutton");
	window->continue_toolbar =  gtk_builder_get_object (builder, "continuebutton");
	window->stop_toolbar =  gtk_builder_get_object (builder, "stopbutton");
	window->code_notebook = gtk_builder_get_object (builder, "codenotebook");
	window->info_notebook = gtk_builder_get_object (builder, "infonotebook");
	window->filetree = gtk_builder_get_object (builder, "filetree");
	window->horizontal_paned = gtk_builder_get_object (builder, "horpaned");
	window->statustree = gtk_builder_get_object (builder, "statustree");
	window->compilertree = gtk_builder_get_object (builder, "compilertree");
	window->notepadview = gtk_builder_get_object (builder, "notepadview");
	window->projectlabel = gtk_builder_get_object (builder, "projectlabel");
	window->locationlabel = gtk_builder_get_object (builder, "locationlabel");
	window->modelabel = gtk_builder_get_object (builder, "modelabel");
	window->status_image = gtk_builder_get_object (builder, "statusimage");
	window->search_entry = gtk_builder_get_object (builder, "searchentry");
	window->pre_search_toolbar = gtk_builder_get_object (builder, "presearchbutton");
	window->next_search_toolbar = gtk_builder_get_object (builder, "nextsearchbutton");
	window->debug_ptr = NULL;
	window->editor_list = NULL;
}

/* Init the file tree view. */
static void
ui_filetree_init (CWindow *window)
{
	filetree_init (GTK_TREE_VIEW (window->filetree));
}

/* Initialize the status view and notepad. */
static void
ui_toolpad_init (CWindow *window)
{
	GtkTextBuffer *buffer;
	gchar *time;
	GtkTreeStore *store;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *select;
	GtkTreeIter  iter;
	gchar *welcome_text;

	store = gtk_tree_store_new (3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, _("Filesystem operations:"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_add_attribute(column,renderer, "text", 0);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_add_attribute(column,renderer, "text", 1);
	gtk_tree_view_column_add_attribute(column,renderer, "foreground", 2);
	gtk_tree_view_append_column (GTK_TREE_VIEW (window->statustree), column);
	g_object_ref (store);
	gtk_tree_view_set_model (GTK_TREE_VIEW (window->statustree), GTK_TREE_MODEL (store));
	select = gtk_tree_view_get_selection (GTK_TREE_VIEW(window->statustree));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
	time = g_malloc (MAX_TIME_LENGTH + 1);
	misc_time_get_now (time);
	store = GTK_TREE_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (window->statustree)));
	gtk_tree_store_append (store, &iter, NULL);
	welcome_text = g_malloc (MESSAGE_BUF_SIZE);
	g_strlcpy (welcome_text, _("This is Codefox "), MESSAGE_BUF_SIZE);
	g_strlcat (welcome_text, PACKAGE_VERSION, MESSAGE_BUF_SIZE);
	g_strlcat (welcome_text, _(", have fun!"), MESSAGE_BUF_SIZE);
	gtk_tree_store_set(store, &iter, 0, time,
					   1, welcome_text, 
					   2, "Orange", 
					   -1);
	g_free ((gpointer) welcome_text);
	g_free ((gpointer) time);

	store = gtk_tree_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, _("Compiler outputs:"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_add_attribute(column, renderer, "text", 0);
	gtk_tree_view_column_add_attribute(column, renderer, "foreground", 1);
	gtk_tree_view_append_column (GTK_TREE_VIEW (window->compilertree), column);
	g_object_ref (store);
	gtk_tree_view_set_model (GTK_TREE_VIEW (window->compilertree), GTK_TREE_MODEL (store));
	select = gtk_tree_view_get_selection (GTK_TREE_VIEW(window->compilertree));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
	
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (window->notepadview));
	gtk_text_buffer_set_text (buffer, _("Write down any notes you want here..."), -1);
}

static void
ui_editorconfig_init ()
{
	editorconfig_default_config_new ();
	editorconfig_user_config_from_default ();
}

static void
ui_project_label_init ()
{
	gchar label[MAX_FILEPATH_LENGTH + 1];

	g_snprintf (label, MAX_FILEPATH_LENGTH, "%s: %s", _("Project"), _("None"));
	gtk_label_set_label (GTK_LABEL (window->projectlabel), label);
}

static void
ui_preferences_window_init ()
{
	GtkBuilder *builder;
	gchar *data_dir;
	gchar *template_file;
	gint response;

	preferences_window = (CPreferencesWindow *) g_malloc (sizeof (CPreferencesWindow));
	builder = gtk_builder_new ();
	data_dir = g_build_filename (CODEFOX_DATADIR, "codefox", NULL);
	template_file = g_build_filename (data_dir, "codefox-editor-settings.ui", NULL);
	gtk_builder_add_from_file (builder, template_file, NULL);

	g_free ((gpointer) data_dir);
	g_free ((gpointer) template_file);

	preferences_window->toplevel = gtk_builder_get_object (builder, "toplevel");
	preferences_window->font_button = gtk_builder_get_object (builder, "fontbutton");
	preferences_window->keyword_button = gtk_builder_get_object (builder, "keywordcolorbutton");
	preferences_window->string_button = gtk_builder_get_object (builder, "stringcolorbutton");
	preferences_window->constant_button = gtk_builder_get_object (builder, "constantcolorbutton");
	preferences_window->comment_button = gtk_builder_get_object (builder, "commentcolorbutton");
	preferences_window->preprocessor_button = gtk_builder_get_object (builder, "preprocessorcolorbutton");
	preferences_window->close_button = gtk_builder_get_object (builder, "closebutton");

	g_signal_connect (preferences_window->close_button, "clicked", 
					  G_CALLBACK (on_preferences_close_clicked), NULL);
	g_signal_connect_after (preferences_window->close_button, "clicked", 
					  G_CALLBACK (on_preferences_close_clicked_after), NULL);
}

static void
ui_fun_tip_init ()
{
	GtkBuilder *builder;
	gchar *data_dir;
	gchar *template_file;
	gint response;

	function_tip = (CFunctionTip *) g_malloc (sizeof (CFunctionTip));
	builder = gtk_builder_new ();
	data_dir = g_build_filename (CODEFOX_DATADIR, "codefox", NULL);
	template_file = g_build_filename (data_dir, "codefox-fun-tip.ui", NULL);
	gtk_builder_add_from_file (builder, template_file, NULL);

	g_free ((gpointer) data_dir);
	g_free ((gpointer) template_file);

	function_tip->tip_window = gtk_builder_get_object (builder, "toplevel");
	function_tip->tip_label = gtk_builder_get_object (builder, "label");
}

static void
ui_debug_view_init (GtkBuilder *builder)
{
	GObject *localtree;
	GObject *calltree;
	GObject *watchtree;

	localtree = gtk_builder_get_object (builder, "localtreeview");
	calltree = gtk_builder_get_object (builder, "calltreeview");
	watchtree = gtk_builder_get_object (builder, "watchtreeview");

	window->debug_view = debugview_new (localtree, calltree, watchtree);
	debugview_disable (window->debug_view);
}

void
ui_enable_save_widgets ()
{
	/* Disable items for saving since there is no opened file! */
	gtk_widget_set_sensitive (GTK_WIDGET (window->save_item), 1);
	gtk_widget_set_sensitive (GTK_WIDGET (window->saveas_item), 1);
	gtk_widget_set_sensitive (GTK_WIDGET (window->save_toolbar), 1);
	gtk_widget_set_sensitive (GTK_WIDGET (window->saveas_toolbar), 1);
}

void
ui_disable_save_widgets ()
{
	/* Disable items for saving since there is no opened file! */
	gtk_widget_set_sensitive (GTK_WIDGET (window->save_item), 0);
	gtk_widget_set_sensitive (GTK_WIDGET (window->saveas_item), 0);
	gtk_widget_set_sensitive (GTK_WIDGET (window->save_toolbar), 0);
	gtk_widget_set_sensitive (GTK_WIDGET (window->saveas_toolbar), 0);
}

void
ui_enable_project_widgets ()
{
	if (project_get_type () == PROJECT_C && (!env_prog_exist (ENV_PROG_GCC) || !env_prog_exist (ENV_PROG_MAKE))) {
		g_warning ("gcc or make not found.");
		ui_status_entry_new (FILE_OP_WARNING, _("gcc or make not found, please check and install."));

		return;
	}
	if (project_get_type () == PROJECT_CPP && (!env_prog_exist (ENV_PROG_GPP) || !env_prog_exist (ENV_PROG_MAKE))) {
		g_warning ("g++ or make not found.");
		ui_status_entry_new (FILE_OP_WARNING, _("g++ or make not found, please check and install."));

		return;
	}

	gtk_widget_set_sensitive (GTK_WIDGET (window->build_item), 1);
	gtk_widget_set_sensitive (GTK_WIDGET (window->build_toolbar), 1);
	gtk_widget_set_sensitive (GTK_WIDGET (window->clear_item), 1);
	gtk_widget_set_sensitive (GTK_WIDGET (window->clear_toolbar), 1);
	if (!env_prog_exist (ENV_PROG_XTERM)) {
		g_warning ("xterm not found.");
		ui_status_entry_new (FILE_OP_WARNING, _("xterm not found, please check and install."));

		return;
	}	
	gtk_widget_set_sensitive (GTK_WIDGET (window->run_item), 1);
	gtk_widget_set_sensitive (GTK_WIDGET (window->run_toolbar), 1);
	if (!env_prog_exist (ENV_PROG_GDB) || !env_prog_exist (ENV_PROG_GDBSERVER)) {
		g_warning ("gdb or gdbserver not found.");
		ui_status_entry_new (FILE_OP_WARNING, _("gdb or gdbserver not found, please check and install."));

		return;
	}
	gtk_widget_set_sensitive (GTK_WIDGET (window->debug_item), 1);
	gtk_widget_set_sensitive (GTK_WIDGET (window->debug_toolbar), 1);
}

void
ui_disable_project_widgets ()
{
	gtk_widget_set_sensitive (GTK_WIDGET (window->build_item), 0);
	gtk_widget_set_sensitive (GTK_WIDGET (window->build_toolbar), 0);
	gtk_widget_set_sensitive (GTK_WIDGET (window->clear_item), 0);
	gtk_widget_set_sensitive (GTK_WIDGET (window->clear_toolbar), 0);
	gtk_widget_set_sensitive (GTK_WIDGET (window->run_item), 0);
	gtk_widget_set_sensitive (GTK_WIDGET (window->run_toolbar), 0);
	gtk_widget_set_sensitive (GTK_WIDGET (window->debug_item), 0);
	gtk_widget_set_sensitive (GTK_WIDGET (window->debug_toolbar), 0);
}

void
ui_enable_build_widgets ()
{
	if (project_get_type () == PROJECT_C && (!env_prog_exist (ENV_PROG_GCC) || !env_prog_exist (ENV_PROG_MAKE))) {
		g_warning ("gcc or make not found.");
		ui_status_entry_new (FILE_OP_WARNING, _("gcc or make not found, please check and install."));

		return;
	}
	if (project_get_type () == PROJECT_CPP && (!env_prog_exist (ENV_PROG_GPP) || !env_prog_exist (ENV_PROG_MAKE))) {
		g_warning ("g++ or make not found.");
		ui_status_entry_new (FILE_OP_WARNING, _("g++ or make not found, please check and install."));

		return;
	}
	gtk_widget_set_sensitive (GTK_WIDGET (window->build_item), 1);
	gtk_widget_set_sensitive (GTK_WIDGET (window->build_toolbar), 1);
}

void
ui_disable_build_widgets ()
{
	gtk_widget_set_sensitive (GTK_WIDGET (window->build_item), 0);
	gtk_widget_set_sensitive (GTK_WIDGET (window->build_toolbar), 0);
}

void
ui_enable_debug_widgets ()
{
	if (!env_prog_exist (ENV_PROG_XTERM) || !env_prog_exist (ENV_PROG_GDB) || !env_prog_exist (ENV_PROG_GDBSERVER)) {
		g_warning ("xterm or gdb or gdbserver not found.");
		ui_status_entry_new (FILE_OP_WARNING, _("xterm or gdb or gdbserver not found, please check and install."));

		return;
	}
	gtk_widget_set_sensitive (GTK_WIDGET (window->next_item), 1);
	gtk_widget_set_sensitive (GTK_WIDGET (window->step_item), 1);
	gtk_widget_set_sensitive (GTK_WIDGET (window->continue_item), 1);
	gtk_widget_set_sensitive (GTK_WIDGET (window->stop_item), 1);
	gtk_widget_set_sensitive (GTK_WIDGET (window->next_toolbar), 1);
	gtk_widget_set_sensitive (GTK_WIDGET (window->step_toolbar), 1);
	gtk_widget_set_sensitive (GTK_WIDGET (window->continue_toolbar), 1);
	gtk_widget_set_sensitive (GTK_WIDGET (window->stop_toolbar), 1);
}

void
ui_disable_debug_widgets ()
{
	gtk_widget_set_sensitive (GTK_WIDGET (window->next_item), 0);
	gtk_widget_set_sensitive (GTK_WIDGET (window->step_item), 0);
	gtk_widget_set_sensitive (GTK_WIDGET (window->continue_item), 0);
	gtk_widget_set_sensitive (GTK_WIDGET (window->stop_item), 0);
	gtk_widget_set_sensitive (GTK_WIDGET (window->next_toolbar), 0);
	gtk_widget_set_sensitive (GTK_WIDGET (window->step_toolbar), 0);
	gtk_widget_set_sensitive (GTK_WIDGET (window->continue_toolbar), 0);
	gtk_widget_set_sensitive (GTK_WIDGET (window->stop_toolbar), 0);
}

void
ui_enable_open_project_widgets ()
{
	gtk_widget_set_sensitive (GTK_WIDGET (window->new_project_item), 1);
	gtk_widget_set_sensitive (GTK_WIDGET (window->new_project_toolbar), 1);
	gtk_widget_set_sensitive (GTK_WIDGET (window->open_project_item), 1);
	gtk_widget_set_sensitive (GTK_WIDGET (window->open_project_toolbar), 1);
}

void
ui_disable_open_project_widgets ()
{
	gtk_widget_set_sensitive (GTK_WIDGET (window->new_project_item), 0);
	gtk_widget_set_sensitive (GTK_WIDGET (window->new_project_toolbar), 0);
	gtk_widget_set_sensitive (GTK_WIDGET (window->open_project_item), 0);
	gtk_widget_set_sensitive (GTK_WIDGET (window->open_project_toolbar), 0);
}

void
ui_enable_settings_widgets ()
{
	gtk_widget_set_sensitive (GTK_WIDGET (window->settings_item), 1);
}

void
ui_disable_settings_widgets ()
{
	gtk_widget_set_sensitive (GTK_WIDGET (window->settings_item), 0);
}

void
ui_enable_undo_widgets ()
{
	gtk_widget_set_sensitive (GTK_WIDGET (window->undo_item), 1);
}

void
ui_disable_undo_widgets ()
{
	gtk_widget_set_sensitive (GTK_WIDGET (window->undo_item), 0);
}

void
ui_enable_redo_widgets ()
{
	gtk_widget_set_sensitive (GTK_WIDGET (window->redo_item), 1);
}

void
ui_disable_redo_widgets ()
{
	gtk_widget_set_sensitive (GTK_WIDGET (window->redo_item), 0);
}

void
ui_init ()
{
	/* Construct a GtkBuilder instance and load our UI description. */
	GtkBuilder *builder;
	gchar *data_dir;
	gchar *template_file;
	gchar *icon_path;
	
	/* Add our icon path in case we aren't installed in the system prefix */
	icon_path = g_build_filename(CODEFOX_DATADIR, "icons", NULL);
	gtk_icon_theme_append_search_path(gtk_icon_theme_get_default(), icon_path);

	builder = gtk_builder_new ();
	data_dir = g_build_filename (CODEFOX_DATADIR, "codefox", NULL);
	template_file = g_build_filename (data_dir, "codefox.ui", NULL);
	gtk_builder_add_from_file (builder, template_file, NULL);

	g_free ((gpointer) data_dir);
	g_free ((gpointer) template_file);
	g_free ((gpointer) icon_path);
	
	window = (CWindow *) g_malloc (sizeof(CWindow));

	member_menu = (CMemberMenu *) g_malloc (sizeof(CMemberMenu));
	member_menu->active = FALSE;
	member_menu->prefix = (gchar *) g_malloc (MAX_TIP_LENGTH + 1);
	member_menu->prefix[0] = 0;
	member_menu->item_list = NULL;
	
	ui_window_init(builder, window);
	ui_debug_view_init (builder);
	ui_filetree_init(window);
	ui_toolpad_init(window);
	ui_filetree_menu_init ();
	ui_editorconfig_init ();
	ui_project_label_init ();
	ui_preferences_window_init ();
	ui_fun_tip_init ();

	new_project_dialog = NULL;
	
	/* Disable items for saving since there is no opened file! */
	ui_disable_save_widgets ();
	ui_disable_settings_widgets ();
	ui_disable_project_widgets ();
	ui_disable_debug_widgets ();
	ui_disable_undo_widgets ();
	ui_disable_redo_widgets ();
	
	signal_connect ();
}

/* Create a new about dialog. */
void
ui_about_dialog_new ()
{
	GtkWidget *about_dialog = gtk_about_dialog_new ();
	GdkPixbuf *logo;
	GtkIconTheme *icon_theme;
	
	gtk_about_dialog_set_program_name (GTK_ABOUT_DIALOG(about_dialog), "Codefox");
    gtk_about_dialog_set_version (GTK_ABOUT_DIALOG(about_dialog), PACKAGE_VERSION);
    gtk_about_dialog_set_authors (GTK_ABOUT_DIALOG(about_dialog), authors);
    gtk_about_dialog_set_artists (GTK_ABOUT_DIALOG(about_dialog), authors);
    gtk_about_dialog_set_documenters (GTK_ABOUT_DIALOG(about_dialog), authors);
    gtk_about_dialog_set_comments (GTK_ABOUT_DIALOG(about_dialog),
    							  _("Perhaps the most lightweight C/C++ IDE..."));
    gtk_about_dialog_set_copyright (GTK_ABOUT_DIALOG(about_dialog), 
    							   "Copyright © 2012-2017 Gordon Lee");
    gtk_about_dialog_set_license (GTK_ABOUT_DIALOG(about_dialog), license);
	gtk_about_dialog_set_wrap_license (GTK_ABOUT_DIALOG (about_dialog), TRUE);
    icon_theme = gtk_icon_theme_get_default ();
    logo = gtk_icon_theme_load_icon (icon_theme, "codefox", LOGO_SIZE, 0, NULL);
    gtk_about_dialog_set_logo (GTK_ABOUT_DIALOG(about_dialog), logo);
    gtk_about_dialog_set_website (GTK_ABOUT_DIALOG(about_dialog),
    							  "https://github.com/nirvanan/Codefox");

    gtk_dialog_run (GTK_DIALOG(about_dialog));
    gtk_widget_destroy (about_dialog);
    g_object_unref (logo);
}

/* Create a new editor and show it. */
void
ui_editor_new ()
{
	
	CEditor *new_editor;
	gint index;

	new_editor = ceditor_new (_("Untitled"));
	ceditor_show (new_editor);
	ceditor_recover_breakpoint (new_editor);
	window->editor_list = g_list_append (window->editor_list, new_editor);
	index = gtk_notebook_append_page (GTK_NOTEBOOK (window->code_notebook), 
									  new_editor->textbox, 
									  new_editor->label_box);
	
	if (index != -1) {
		gtk_notebook_set_current_page (GTK_NOTEBOOK (window->code_notebook), 
									   index);
	}

	if (g_list_length (window->editor_list) != 0) {
		ui_enable_save_widgets ();
	}
	
	ui_current_editor_change_mode ();
}

/* Create a new editor with code and show it. */
void
ui_editor_new_with_text (const gchar *filepath, const gchar *code_buf)
{
	CEditor *new_editor;
	gint index;

	new_editor = ceditor_new_with_text (filepath, code_buf);
	ceditor_show (new_editor);
	ceditor_recover_breakpoint (new_editor);
	window->editor_list = g_list_append (window->editor_list, new_editor);
	index = gtk_notebook_append_page (GTK_NOTEBOOK (window->code_notebook), 
									  new_editor->textbox, 
									  new_editor->label_box);
									  
	/* Swap to show the created tab of codenotebook.*/
	if (index != -1) {
		gtk_notebook_set_current_page (GTK_NOTEBOOK (window->code_notebook), 
									   index);
	}

	if (g_list_length (window->editor_list) != 0) {
		ui_enable_save_widgets ();
	}
	
	ui_current_editor_change_mode ();
}

/* Add a new entry to filetree. */
void
ui_filetree_entry_new (gboolean is_file, gchar *filename, gchar *filepath)
{
	filetree_append_to_default (GTK_TREE_VIEW (window->filetree), is_file, 
								filename, filepath);
}

/* Append a new status entry to statustree. */
void
ui_status_entry_new (const gint op, const gchar *filepath)
{
	GtkTreeStore *store;
	GtkTreeIter  iter;
	gchar *message_buf;
	gchar *time_buf;

	time_buf = (gchar *) g_malloc (MAX_TIME_LENGTH + 1);
	message_buf = (gchar *) g_malloc(MESSAGE_BUF_SIZE + 1);
	misc_time_get_now (time_buf);
	store = GTK_TREE_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (window->statustree)));
	gtk_tree_store_append (store, &iter, NULL);

	switch (op) {
	case FILE_OP_CREATE:
		g_strlcpy (message_buf, _("An untitled file has been created."), MESSAGE_BUF_SIZE);
		break;
	case FILE_OP_SAVE:
		g_strlcpy (message_buf, filepath, MESSAGE_BUF_SIZE);
		g_strlcat (message_buf, _(" has been saved."), MESSAGE_BUF_SIZE);
		break;
	case FILE_OP_OPEN:
		g_strlcpy (message_buf, filepath, MESSAGE_BUF_SIZE);
		g_strlcat (message_buf, _(" has been opened."), MESSAGE_BUF_SIZE);
		break;
	case FILE_OP_NEW_PROJECT:
		g_strlcpy (message_buf, "A new project ", MESSAGE_BUF_SIZE);
		g_strlcat (message_buf, filepath, MESSAGE_BUF_SIZE);
		g_strlcat (message_buf, _(" has been created."), MESSAGE_BUF_SIZE);
		break;
	case FILE_OP_ADD_FILE:
		g_strlcpy (message_buf, filepath, MESSAGE_BUF_SIZE);
		g_strlcat (message_buf, _(" has been added to current project."), MESSAGE_BUF_SIZE);
		break;
	case FILE_OP_CREATE_FILE:
		g_strlcpy (message_buf, filepath, MESSAGE_BUF_SIZE);
		g_strlcat (message_buf, _(" has been created and added to current project."), MESSAGE_BUF_SIZE);
		break;
	case FILE_OP_DELETE_FILE:
		g_strlcpy (message_buf, filepath, MESSAGE_BUF_SIZE);
		g_strlcat (message_buf, _(" has been removed from project."), MESSAGE_BUF_SIZE);
		break;
	case FILE_OP_WARNING:
		g_strlcpy (message_buf, _("WARNING: "), MESSAGE_BUF_SIZE);
		g_strlcat (message_buf, filepath, MESSAGE_BUF_SIZE);
		break;
	case FILE_OP_ERROR:
		g_strlcpy (message_buf, _("ERROR: "), MESSAGE_BUF_SIZE);
		g_strlcat (message_buf, filepath, MESSAGE_BUF_SIZE);
		break;
	}

	gtk_tree_store_set (store, &iter, 0, time_buf, 1, message_buf, 2, "gray", -1);

	g_free ((gpointer) time_buf);
	g_free ((gpointer) message_buf);
}

/* Create a file choosing dialog and return the file path after user 
 * selected a local file or typed a filename. 
 */
void
ui_get_filepath_from_dialog (gchar *filepath, const gint size, const gboolean open,
                             const gboolean project, const gchar *default_path)
{
	GtkWidget *dialog;

	if (open) {
		dialog = gtk_file_chooser_dialog_new (_("Open File"),
											  GTK_WINDOW (window->toplevel),
											  GTK_FILE_CHOOSER_ACTION_OPEN,
											  _("Cancel"), GTK_RESPONSE_CANCEL,
											  _("Open"), GTK_RESPONSE_ACCEPT,
											  NULL);

		if (project) {
			GtkFileFilter* filter;

			filter = gtk_file_filter_new ();
		    gtk_file_filter_set_name (filter, _("Codefox project"));
		    gtk_file_filter_add_pattern(filter,"*.cfp");
		    gtk_file_chooser_add_filter (GTK_FILE_CHOOSER (dialog),filter);
		}
	}
	else {
		dialog = gtk_file_chooser_dialog_new (_("Save File"),
											  GTK_WINDOW (window->toplevel),
											  GTK_FILE_CHOOSER_ACTION_SAVE,
											  _("Cancel"), GTK_RESPONSE_CANCEL,
											  _("Save"), GTK_RESPONSE_ACCEPT,
											  NULL);
		gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog),
													    TRUE);
		gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog),
										   default_path);
	}

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		gchar *path;

		path = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		g_strlcpy (filepath, path, size);
		
		g_free ((gpointer) path);
	}
	else {
		g_strlcpy (filepath, "NULL", size);
	}

	gtk_widget_destroy (dialog);
}

/* Check there is still editors in notebook. */
gboolean
ui_have_editor ()
{
	return ui_get_current_editor() != NULL;
}

/* Find editor with associated filepath. */
gboolean
ui_find_editor (const gchar *filepath)
{
	GList * iterator;

	for (iterator = window->editor_list; iterator; iterator = iterator->next) {
		CEditor *editor;

		editor = (CEditor *) iterator->data;
		if (g_strcmp0 (editor->filepath, filepath) == 0) {
			return TRUE;
		}
	}

	return FALSE;
}

/* Find editor with associated filepath. */
CEditor *
ui_get_editor (const gchar *filepath)
{
	GList * iterator;

	for (iterator = window->editor_list; iterator; iterator = iterator->next) {
		CEditor *editor;

		editor = (CEditor *) iterator->data;
		if (g_strcmp0 (editor->filepath, filepath) == 0) {
			return editor;
		}
	}

	return NULL;
}

void
ui_show_editor_by_path (const gchar *filepath)
{
	CEditor *editor;
	gint page;

	editor = ui_get_editor (filepath);
	if (editor != NULL) {
		page = gtk_notebook_page_num (GTK_NOTEBOOK (window->code_notebook), editor->textbox);
		if (page != -1) {
			gtk_notebook_set_current_page (GTK_NOTEBOOK (window->code_notebook), page);
		}
	}
}

CEditor *
ui_get_current_editor ()
{
	gint page;
	GtkWidget *current_page;
	GList * iterator;
	
	page = gtk_notebook_get_current_page (GTK_NOTEBOOK (window->code_notebook));

	if (page == -1) {
		return NULL;
	}

	current_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (window->code_notebook),
											  page);

	for (iterator = window->editor_list; iterator; iterator = iterator->next) {
		CEditor *editor;

		editor = (CEditor *) iterator->data;
		if (GTK_WIDGET (editor->textbox) == current_page) {
			return editor;
		}
	}

	return NULL;
}

/* Get current editor filepath under editing. */
void
ui_current_editor_filepath (gchar *filepath)
{
	
	CEditor *editor;

	editor = ui_get_current_editor ();

	if (editor != NULL) {
		g_strlcpy (filepath, editor->filepath, MAX_FILEPATH_LENGTH);
	}
	else {
		filepath[0] = 0;
	}
}

/* Get current editor code under editing. */
gchar *
ui_current_editor_code ()
{
	
	CEditor *editor;
	GtkTextBuffer *buffer;
	GtkTextIter start, end;
	gchar *ret;

	editor = ui_get_current_editor ();

	if (editor == NULL) {
		return NULL;
	}

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor->textview));
	gtk_text_buffer_get_start_iter (buffer, &start);
	gtk_text_buffer_get_end_iter (buffer, &end);

	ret = gtk_text_buffer_get_text (buffer, &start, &end, 1);

	return ret;
}

/* Post action after saving a file. */
void
ui_save_code_post (const gchar *filepath)
{
	CEditor *editor;

	editor = ui_get_current_editor ();
	if (ceditor_get_dirty (editor))
		ceditor_set_dirty (editor, 0);
	filetree_set_selected_path (GTK_TREE_VIEW (window->filetree),
								filepath,
								(const gchar *) editor->filepath);
	ceditor_set_path (editor, filepath);
}

/* Emit a clipboard signal. */
void
ui_emit_clipboard_signal (gint clipboard_signal)
{
	CEditor *editor;

	editor = ui_get_current_editor ();

	if (editor == NULL) {
		return;
	}

	g_signal_emit_by_name (editor->textview, sig[clipboard_signal], NULL);
}

/* Emit a click signal on save button. */
void
ui_emit_close_signal ()
{
	
	CEditor *editor;

	editor = ui_get_current_editor ();

	if (editor == NULL) {
		return;
	}
	
	ceditor_emit_close_signal (editor);
}

void
ui_emit_save_signal ()
{
	CEditor *editor;

	editor = ui_get_current_editor ();

	if (editor == NULL) {
		return;
	}

	g_signal_emit_by_name (window->save_toolbar, "clicked", NULL);
}

/* Delete selected code from current editor. */
void
ui_current_editor_delete_range ()
{
	CEditor *editor;
	GtkTextIter start, end;
	GtkTextMark *selected, *insert;
	GtkTextBuffer *buffer;

	editor = ui_get_current_editor ();

	if (editor == NULL) {
		return;
	}
			
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor->textview));
	selected = gtk_text_buffer_get_selection_bound (buffer);
	insert = gtk_text_buffer_get_insert (buffer);
	gtk_text_buffer_get_iter_at_mark (buffer, &start, selected);
	gtk_text_buffer_get_iter_at_mark (buffer, &end, insert);
	gtk_text_buffer_delete (buffer, &start, &end);	
}

/* Format current code under editting. */
void
ui_current_editor_format ()
{
	CEditor *editor;
	GtkTextIter start, end;
	GtkTextBuffer *buffer;
	gchar* text;
	gint i;
	gint end_line;

	editor = ui_get_current_editor ();

	if (editor == NULL) {
		return;
	}
	
	i = 0;
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor->textview));
	gtk_text_buffer_get_start_iter (buffer, &start);
	gtk_text_buffer_get_end_iter (buffer, &end);
	text =  gtk_text_buffer_get_text (buffer, &start, &end, 1);
	while (text[i++] == ' ');
	gtk_text_buffer_get_iter_at_offset(buffer, &end, i - 1);
	gtk_text_buffer_delete (buffer, &start, &end);
	gtk_text_buffer_get_end_iter (buffer, &end);
	end_line = gtk_text_iter_get_line (&end);

	if (end_line >= 1) {
		autoindent_apply (buffer, NULL, 0, end_line);
	}

	g_free ((gpointer) text);
}

/* Update cursor position. */
void
ui_current_editor_update_cursor ()
{
	
	GtkTextBuffer *buffer;
	GtkTextMark *insert;
	GtkTextIter iter;
	gint line, column;
	gchar *location;
	CEditor *editor;
	
	editor = ui_get_current_editor ();

	if (editor == NULL) {
		return;
	}

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor->textview));
	insert = gtk_text_buffer_get_insert (buffer);
	gtk_text_buffer_get_iter_at_mark (buffer, &iter, insert);
				
	line = gtk_text_iter_get_line (&iter);
	column = gtk_text_iter_get_line_offset (&iter);
				
	location = (gchar *) g_malloc (MAX_LINE_LENGTH + 1);
	g_snprintf (location, MAX_LINE_LENGTH, _(" Line: %d\tColumn: %d"), line + 1, column);
	gtk_label_set_label (GTK_LABEL (window->locationlabel), location);

	g_free ((gpointer) location);
}

/* Change current edit mode. */
void
ui_current_editor_change_mode ()
{
	
	CEditor *editor;
	gboolean overwrite;
	gchar *label;
	
	editor = ui_get_current_editor ();
	overwrite = gtk_text_view_get_overwrite (GTK_TEXT_VIEW (editor->textview));
	label = (gchar *) g_malloc (MAX_LINE_LENGTH + 1);

	if (overwrite) {
		g_snprintf (label, MAX_LINE_LENGTH, _(" Mode: %s"), _("Overwrite"));
	}
	else {
		g_snprintf (label, MAX_LINE_LENGTH, _(" Mode: %s"), _("Insert"));
	}
	gtk_label_set_label (GTK_LABEL (window->modelabel), label);

	g_free ((gpointer) label);
}

/* Update line number label after editing code. */
void
ui_update_line_number_label (const gboolean insert, const gint append, 
							 const GtkTextIter *start, const GtkTextIter *end)
{
	
	CEditor *editor;

	editor = ui_get_current_editor ();

	if (editor == NULL) {
		return;
	}

	if (insert) {
		ceditor_append_line_label (editor, append);
	}
	else {
		gint start_line, end_line;

		start_line = gtk_text_iter_get_line (start);
		end_line = gtk_text_iter_get_line (end);
		ceditor_remove_line_label (editor, end_line - start_line);
	}
}

/* Update editting status after switching editor. */
void
ui_switch_page ()
{
	CEditor *editor;

	editor = ui_get_current_editor ();

	if (editor == NULL) {
		return;
	}

	ui_current_editor_change_mode();
	ui_current_editor_update_cursor();
}

static void
ui_new_project_dialog_init (GtkBuilder *builder)
{
	new_project_dialog = (CNewProjectDialog *) g_malloc (sizeof (CNewProjectDialog));
	new_project_dialog->toplevel = gtk_builder_get_object (builder, "toplevel");
	new_project_dialog->name_entry = gtk_builder_get_object (builder, "nameentry");
	new_project_dialog->path_entry = gtk_builder_get_object (builder, "pathentry");
	new_project_dialog->type_box = gtk_builder_get_object (builder, "typecomboboxtext");
}

static void
ui_create_file_dialog_init (GtkBuilder *builder)
{
	create_file_dialog = (CCreateFileDialog *) g_malloc (sizeof (CCreateFileDialog));
	create_file_dialog->toplevel = gtk_builder_get_object (builder, "toplevel");
	create_file_dialog->name_entry = gtk_builder_get_object (builder, "nameentry");
}

static void
ui_project_settings_dialog_init (GtkBuilder *builder)
{
	project_settings_dialog = (CProjectSettingsDialog *) g_malloc (sizeof (CCreateFileDialog));
	project_settings_dialog->toplevel = gtk_builder_get_object (builder, "toplevel");
	project_settings_dialog->libs_entry = gtk_builder_get_object (builder, "libentry");
	project_settings_dialog->opts_entry = gtk_builder_get_object (builder, "optentry");
}

/* Create a new project creating dialog. */
gint
ui_new_project_dialog_new (const gchar *default_project_path)
{
	GtkBuilder *builder;
	gchar *data_dir;
	gchar *template_file;
	gint response;

	builder = gtk_builder_new ();
	data_dir = g_build_filename (CODEFOX_DATADIR, "codefox", NULL);
	template_file = g_build_filename (data_dir, "codefox-new-project.ui", NULL);
	gtk_builder_add_from_file (builder, template_file, NULL);

	g_free ((gpointer) data_dir);
	g_free ((gpointer) template_file);

	ui_new_project_dialog_init (builder);
	gtk_entry_set_text (GTK_ENTRY (new_project_dialog->path_entry), default_project_path);
	gtk_combo_box_set_active (GTK_COMBO_BOX (new_project_dialog->type_box), 0);
	gtk_widget_grab_focus (GTK_WIDGET (new_project_dialog->name_entry));

	response = gtk_dialog_run (GTK_DIALOG(new_project_dialog->toplevel));

	return response;
}

/* Create a new file creating dialog. */
gint
ui_create_file_dialog_new ()
{
	GtkBuilder *builder;
	gchar *data_dir;
	gchar *template_file;
	gint response;

	builder = gtk_builder_new ();
	data_dir = g_build_filename (CODEFOX_DATADIR, "codefox", NULL);
	template_file = g_build_filename (data_dir, "codefox-create-file.ui", NULL);
	gtk_builder_add_from_file (builder, template_file, NULL);

	g_free ((gpointer) data_dir);
	g_free ((gpointer) template_file);

	ui_create_file_dialog_init (builder);
	gtk_widget_grab_focus (GTK_WIDGET (create_file_dialog->name_entry));

	response = gtk_dialog_run (GTK_DIALOG(create_file_dialog->toplevel));

	return response;
}

/* Create a new project settings dialog. */
gint
ui_project_settings_dialog_new (const gchar* libs, const gchar* opts)
{
	GtkBuilder *builder;
	gchar *data_dir;
	gchar *template_file;
	gint response;

	builder = gtk_builder_new ();
	data_dir = g_build_filename (CODEFOX_DATADIR, "codefox", NULL);
	template_file = g_build_filename (data_dir, "codefox-project-settings.ui", NULL);
	gtk_builder_add_from_file (builder, template_file, NULL);

	g_free ((gpointer) data_dir);
	g_free ((gpointer) template_file);

	ui_project_settings_dialog_init (builder);
	gtk_entry_set_text (GTK_ENTRY (project_settings_dialog->libs_entry), libs);
	gtk_entry_set_text (GTK_ENTRY (project_settings_dialog->opts_entry), opts);
	gtk_widget_grab_focus (GTK_WIDGET (project_settings_dialog->libs_entry));

	response = gtk_dialog_run (GTK_DIALOG(project_settings_dialog->toplevel));

	return response;
}


/* Destory current opened new project dialog. */
void
ui_new_project_dialog_destory ()
{
	gtk_widget_destroy (GTK_WIDGET (new_project_dialog->toplevel));
	g_free (new_project_dialog);
}

/* Destory current opened create file dialog. */
void
ui_create_file_dialog_destory ()
{
	gtk_widget_destroy (GTK_WIDGET (create_file_dialog->toplevel));
	g_free (create_file_dialog);
}

void
ui_project_settings_dialog_destory ()
{
	gtk_widget_destroy (GTK_WIDGET (project_settings_dialog->toplevel));
	g_free (project_settings_dialog);
}

/* Get project info from new_project_dialog. */
void
ui_new_project_dialog_info (gchar *name, const gint name_size, gchar *path, const gint path_size, gint *type)
{
	g_strlcpy (name, gtk_entry_get_text (GTK_ENTRY (new_project_dialog->name_entry)), name_size);
	g_strlcpy (path, gtk_entry_get_text (GTK_ENTRY (new_project_dialog->path_entry)), path_size);
	(*type) = gtk_combo_box_get_active (GTK_COMBO_BOX (new_project_dialog->type_box));
}

/* Get project info from new_project_dialog. */
void
ui_create_file_dialog_info (gchar *name, const gint size)
{
	g_strlcpy (name, gtk_entry_get_text (GTK_ENTRY (create_file_dialog->name_entry)), size);
}

void
ui_project_settings_dialog_info (gchar *libs, gchar *opts)
{
	g_strlcpy (libs, gtk_entry_get_text (GTK_ENTRY (project_settings_dialog->libs_entry)), MAX_FILEPATH_LENGTH);
	g_strlcpy (opts, gtk_entry_get_text (GTK_ENTRY (project_settings_dialog->opts_entry)), MAX_FILEPATH_LENGTH);
}

/* Start project operate state on toplevel window. */
void
ui_start_project (const gchar *project_name, const gchar *project_path)
{
	gchar title[MAX_FILEPATH_LENGTH + 1];

	filetree_project_init (GTK_TREE_VIEW (window->filetree), project_name, project_path);
	g_strlcat (title, project_name, MAX_FILEPATH_LENGTH);
	g_strlcat (title, " - ", MAX_FILEPATH_LENGTH);
	g_strlcpy (title, gtk_window_get_title (GTK_WINDOW (window->toplevel)), MAX_FILEPATH_LENGTH);
	
	gtk_window_set_title (GTK_WINDOW (window->toplevel), title);
}

/* Create a popup menu for filetree. */
static void
ui_filetree_menu_init ()
{
	GtkWidget *stock_open;
	GtkWidget *stock_new;
	GtkWidget *stock_delete;

	filetree_menu = (CFileTreeMenu *) g_malloc (sizeof (CFileTreeMenu));
	filetree_menu->menu = (GObject *) gtk_menu_new ();
	filetree_menu->add_item = (GObject *) gtk_menu_item_new_with_label (_("Add local file"));
	filetree_menu->create_item = (GObject *) gtk_menu_item_new_with_label (_("Create empty file"));
	filetree_menu->delete_item = (GObject *) gtk_menu_item_new_with_label (_("Delete this item"));
	gtk_menu_shell_append (GTK_MENU_SHELL (filetree_menu->menu), GTK_WIDGET (filetree_menu->create_item));
	gtk_menu_shell_append (GTK_MENU_SHELL (filetree_menu->menu), GTK_WIDGET (filetree_menu->add_item));
	gtk_menu_shell_append (GTK_MENU_SHELL (filetree_menu->menu), GTK_WIDGET (filetree_menu->delete_item));

	g_signal_connect (filetree_menu->create_item, "activate", 
					  G_CALLBACK (on_create_file_clicked), NULL);
	g_signal_connect (filetree_menu->add_item, "activate", 
					  G_CALLBACK (on_open_file_clicked), NULL);
	g_signal_connect (filetree_menu->delete_item, "activate", 
					  G_CALLBACK (on_delete_file_clicked), NULL);
}

void
ui_filetree_menu_popup ()
{
	gchar *name;
	gchar *path;
	gint isfile;
	gint child;

	name = (gchar *) g_malloc (MAX_FILEPATH_LENGTH + 1);
	path = (gchar *) g_malloc (MAX_FILEPATH_LENGTH + 1);
	filetree_get_current_store (GTK_TREE_VIEW (window->filetree), &name, &path, &isfile, &child);
	gtk_widget_show(GTK_WIDGET (filetree_menu->add_item));
	gtk_widget_show(GTK_WIDGET (filetree_menu->create_item));
	gtk_widget_show(GTK_WIDGET (filetree_menu->delete_item));

	if (child == -1) {
		g_free ((gpointer) name);
		g_free ((gpointer) path);

		return;
	}

	if (isfile) {
		gtk_widget_hide(GTK_WIDGET (filetree_menu->add_item));
		gtk_widget_hide(GTK_WIDGET (filetree_menu->create_item));
	}

	if (child) {
		gtk_widget_hide(GTK_WIDGET (filetree_menu->delete_item));
	}

	gtk_menu_popup_at_pointer (GTK_MENU (filetree_menu->menu), NULL);

	g_free ((gpointer) name);
	g_free ((gpointer) path);
}

void
ui_filetree_current_path (gchar **path, gint *isfile)
{
	filetree_get_current_store (GTK_TREE_VIEW (window->filetree), NULL, path, isfile, NULL);
}

void
ui_filetree_append_file_to_current (const gchar *filename)
{
	filetree_append_to_current (GTK_TREE_VIEW (window->filetree), filename, 1);
}

void
ui_error_dialog_new (const gchar *message)
{
	gint ret;
	GtkWidget* dialog;

	dialog = gtk_message_dialog_new (GTK_WINDOW (window->toplevel), GTK_DIALOG_DESTROY_WITH_PARENT,
									 GTK_MESSAGE_ERROR,
									 GTK_BUTTONS_OK, "%s", message);

	ret = gtk_dialog_run (GTK_DIALOG (dialog));

	gtk_widget_destroy (dialog);
}

gint
ui_confirm_dialog_new (const gchar *message)
{
	gint ret;
	GtkWidget* dialog;

	dialog = gtk_message_dialog_new (GTK_WINDOW (window->toplevel),
									 GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_QUESTION,
									 GTK_BUTTONS_YES_NO, "%s", message);

	ret = gtk_dialog_run (GTK_DIALOG (dialog));

	gtk_widget_destroy (dialog);

	return ret;
}

gint
ui_filetree_row_second_level ()
{
	return filetree_get_row_second_level (GTK_TREE_VIEW (window->filetree));
}

void
ui_filetree_remove_item (const gchar *filepath)
{
	filetree_remove (GTK_TREE_VIEW (window->filetree), filepath);
}

void
ui_compiletree_apend (const gchar *line, gint is_message)
{
	GtkTreeIter  iter;
	GtkTreeStore *store;

	store = GTK_TREE_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (window->compilertree)));
	gtk_tree_store_append (store, &iter, NULL);
	if (is_message) {
		gtk_tree_store_set(store, &iter, 0, line, 1, "blue", -1);
	}
	else {
		gtk_tree_store_set(store, &iter, 0, line, 1, "red", -1);
	}

	gtk_notebook_set_current_page (GTK_NOTEBOOK (window->info_notebook), PAGE_COMPILE);
}

void
ui_compiletree_clear ()
{
	GtkTreeStore *store;

	store = GTK_TREE_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (window->compilertree)));
	gtk_tree_store_clear (store);
}

void
ui_append_files_to_second_level (const GList *list, const gint row)
{
	
	GList *iterator;

	for (iterator = (GList *) list; iterator; iterator = iterator->next) {
		gchar *filepath;
		gint offset;

		filepath = (gchar *) iterator->data;
		offset = misc_get_file_name_in_path (filepath);
		
		filetree_append_to_second_level (GTK_TREE_VIEW (window->filetree), row, filepath + offset + 1, 1);
	}

	filetree_select_second_level (GTK_TREE_VIEW (window->filetree), -1);
}

void
ui_current_editor_line (gchar *line, const gint size, const gint lineno)
{
	CEditor *editor;

	editor = ui_get_current_editor ();

	if (editor == NULL) {
		return;
	}

	ceditor_get_line (editor, line, size, lineno);
}

void
ui_current_editor_error_tag_clear ()
{
	
	CEditor *editor;

	editor = ui_get_current_editor ();

	if (editor == NULL) {
		return;
	}

	ceditor_error_tag_clear (editor);
}

void
ui_current_editor_error_tag_add (const gint row, const gint column, const gint len)
{
	CEditor *editor;

	editor = ui_get_current_editor ();

	if (editor == NULL) {
		return;
	}

	ceditor_error_tag_add (editor, row, column, len);
}

static void
ui_memu_popup_position (GtkMenu *menu, gint *x, gint *y, gboolean *push_in,
					   gpointer user_data)
{
	*push_in = FALSE;
	ui_current_editor_insert_location (x, y);
}

void
ui_function_autocomplete (const gchar *name, const GList *signs)
{
	gchar *full;
	gint x, y;
	GList *iterator;
	

	full = (gchar *) g_malloc (MAX_TIP_LENGTH + 1);
	full[0] = 0;
	for (iterator = (GList *) signs; iterator; iterator = iterator->next) {
		g_strlcat (full, name, MAX_TIP_LENGTH);
		g_strlcat (full, " : ", MAX_TIP_LENGTH);
		g_strlcat (full, (gchar *) iterator->data, MAX_TIP_LENGTH);
		g_strlcat (full, "\n", MAX_TIP_LENGTH);
	}
	full[strlen (full) - 1] = 0;

	gtk_label_set_label (GTK_LABEL (function_tip->tip_label), full);
	ui_current_editor_insert_location (&x, &y);
	gtk_window_move (GTK_WINDOW (function_tip->tip_window), x, y);
	gtk_widget_show_all (GTK_WIDGET (function_tip->tip_window));

	function_tip->active = TRUE;

	g_free ((gpointer) full);
}

void
ui_tip_window_destory ()
{
	if (!function_tip->active) {
		return;
	}

	gtk_widget_hide (GTK_WIDGET (function_tip->tip_window));
	function_tip->active = FALSE;
}

void
ui_member_autocomplete (const GList *funs, const GList *vars)
{
	GList *iterator;
	gint x, y;
	GdkRectangle rec;

	if (member_menu->active) {
		ui_member_menu_destroy ();
	}

	member_menu->menu = (GObject *) gtk_menu_new ();
	for (iterator = (GList *) funs; iterator; iterator = iterator->next) {
		GtkWidget *item;

		item = gtk_menu_item_new_with_label ((gchar *) iterator->data);
		gtk_menu_shell_append (GTK_MENU_SHELL (member_menu->menu), item);
		g_signal_connect (item, "activate", 
					  	  G_CALLBACK (on_autocomplete_item_clicked), NULL);

		gtk_widget_show (item);
		member_menu->item_list = g_list_append (member_menu->item_list, item);
	}
	for (iterator = (GList *) vars; iterator; iterator = iterator->next) {
		GtkWidget *item;

		item = gtk_menu_item_new_with_label ((gchar *) iterator->data);
		gtk_menu_shell_append (GTK_MENU_SHELL (member_menu->menu), item);
		g_signal_connect (item, "activate", 
					  	  G_CALLBACK (on_autocomplete_item_clicked), NULL);

		gtk_widget_show (item);
		member_menu->item_list = g_list_append (member_menu->item_list, item);
	}

	gtk_widget_add_events (GTK_WIDGET (member_menu->menu), GDK_KEY_PRESS_MASK);
	g_signal_connect (member_menu->menu, "key-press-event", 
					  G_CALLBACK (on_key_pressed), NULL);

	gtk_menu_shell_set_take_focus (GTK_MENU_SHELL (member_menu->menu), FALSE);
	gtk_widget_set_can_focus (GTK_WIDGET (member_menu->menu), FALSE);

	ui_current_editor_insert_location (&x, &y);
	rec.x = x;
	rec.y = y;
	rec.width = 0;
	rec.height = 0;
	gtk_menu_popup_at_rect (GTK_MENU (member_menu->menu),
							gtk_widget_get_window (GTK_WIDGET (window->toplevel)),
							&rec, GDK_GRAVITY_NORTH_WEST,
							GDK_GRAVITY_NORTH_WEST, NULL);

	member_menu->active = TRUE;
}

void
ui_current_editor_insert_location (gint *x, gint *y)
{

	CEditor *editor;

	if (!ui_have_editor ()) {
		*x = -1;
		*y = -1;

		return;
	}

	editor = ui_get_current_editor ();
	ceditor_get_insert_location (editor, x, y);

	/* FIXME */
	*x += gtk_paned_get_position (GTK_PANED (window->horizontal_paned));
}

void
ui_current_editor_insert (const gchar *text)
{
	CEditor *editor;

	if (!ui_have_editor ()) {
		return;
	}

	editor = ui_get_current_editor ();
	ceditor_insert (editor, text);
}

gboolean
ui_member_menu_active ()
{
	return member_menu->active;
}

void
ui_member_menu_destroy ()
{
	if (!member_menu->active) {
		return;
	}

	gtk_widget_destroy (GTK_WIDGET (member_menu->menu));
	g_list_free (member_menu->item_list);
	member_menu->item_list = NULL;
	member_menu->prefix[0] = 0;
	member_menu->active = FALSE;
}

static gint
ui_member_menu_item_filter ()
{
	GList *iterator;
	gint item_showed;

	item_showed = 0;
	for (iterator = member_menu->item_list; iterator; iterator = iterator->next) {
		GtkWidget *item;
		const gchar *label;

		item = (GtkWidget *) iterator->data;
		label = gtk_menu_item_get_label (GTK_MENU_ITEM (item));

		if (g_str_has_prefix (label, member_menu->prefix) &&
			g_strcmp0 (label, member_menu->prefix) != 0) {
			gtk_widget_show (item);
			item_showed++;
		}
		else {
			gtk_widget_hide (item);
		}
	}

	return item_showed;
}

void
ui_member_menu_update (const gboolean del, const gchar ch)
{
	gint len;
	gint item_showed;

	if (del) {
		len = strlen (member_menu->prefix);
		member_menu->prefix[len - 1] = 0;

		item_showed = ui_member_menu_item_filter ();
	}
	else {
		len = strlen (member_menu->prefix);
		member_menu->prefix[len] = ch;
		member_menu->prefix[len + 1] = 0;
		item_showed = ui_member_menu_item_filter ();
	}

	if (item_showed == 0) {
		gtk_widget_hide (GTK_WIDGET (member_menu->menu));
	}
	else {
		gint x, y;
		GdkRectangle rec;

		ui_current_editor_insert_location (&x, &y);
		rec.x = x;
		rec.y = y;
		rec.width = 0;
		rec.height = 0;
		gtk_menu_popup_at_rect (GTK_MENU (member_menu->menu),
								gtk_widget_get_window (GTK_WIDGET (window->toplevel)),
								&rec, GDK_GRAVITY_NORTH_WEST,
								GDK_GRAVITY_NORTH_WEST, NULL);
	}
}

const gchar *
ui_member_menu_prefix ()
{
	return member_menu->prefix;
}

static CEditor *
ui_get_editor_by_button (GtkWidget *button)
{
	GList *iterator;
	
	for (iterator = window->editor_list; iterator; iterator = iterator->next) {
		CEditor *editor;

		editor = (CEditor *) iterator->data;
		if (editor->close_button == button) {
			return editor;
		}
	}

	return NULL;
}

gint
ui_editor_close (GtkWidget *button)
{
	CEditor *editor;
	gint n_page;

	editor = ui_get_editor_by_button (button);

	if (editor == NULL) {
		return 0;
	}

	ui_select_editor_with_path (editor->filepath);
	if (ceditor_get_dirty (editor)) {
		GtkWidget *dialog;
		gint result;
				
		dialog = gtk_message_dialog_new (GTK_WINDOW (window->toplevel),
										 GTK_DIALOG_DESTROY_WITH_PARENT,
										 GTK_MESSAGE_WARNING,
										 GTK_BUTTONS_NONE,
										 _("%s has been modified, save to file?"),
										 editor->filepath);
		gtk_dialog_add_buttons (GTK_DIALOG (dialog), 
								_("Cancel"),
								GTK_RESPONSE_CANCEL,
								_("No"),
								GTK_RESPONSE_REJECT,
								_("Yes"),
								GTK_RESPONSE_ACCEPT,
								NULL);
		result = gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);

		if (result == GTK_RESPONSE_CANCEL) {
			return 0;
		}
		else if (result == GTK_RESPONSE_ACCEPT) {
			ui_emit_save_signal ();
		}
	}
	
	n_page = gtk_notebook_page_num (GTK_NOTEBOOK (window->code_notebook),
									GTK_WIDGET (editor->textbox));
	gtk_notebook_remove_page (GTK_NOTEBOOK (window->code_notebook), n_page);

	window->editor_list = g_list_remove(window->editor_list, editor);

	if (g_list_length (window->editor_list) == 0) {
		ui_disable_save_widgets ();
	}

	return 1;
}

gint
ui_current_editor_close ()
{
	CEditor *editor;
	gint n_page;

	editor = ui_get_current_editor ();

	if (editor == NULL) {
		return 0;
	}

	return ui_editor_close (editor->close_button);
}

void
ui_current_editor_set_dirty ()
{
	
	CEditor *editor;
	gint n_page;

	editor = ui_get_current_editor ();

	if (editor == NULL) {
		return;
	}

	ceditor_set_dirty (editor, 1);
}

void
ui_status_image_set (const gboolean error, const gboolean warning)
{
	if (error) {
		gtk_image_set_from_icon_name (GTK_IMAGE (window->status_image), CODEFOX_STOCK_STATUSERROR, GTK_ICON_SIZE_MENU);
	}
	else if (warning) {
		gtk_image_set_from_icon_name (GTK_IMAGE (window->status_image), CODEFOX_STOCK_STATUSWARNING, GTK_ICON_SIZE_MENU);
	}
	else {
		gtk_image_set_from_icon_name (GTK_IMAGE (window->status_image), CODEFOX_STOCK_STATUSPASS, GTK_ICON_SIZE_MENU);
	}
}

void
ui_highlight_on_delete (GtkTextBuffer *textbuffer, GtkTextIter *start,
						 GtkTextIter *end)
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
}

void
ui_highlight_on_insert (GtkTextBuffer *textbuffer, GtkTextIter *location, gint lines, gint *end_line_ptr)
{
	
	GtkTextIter start, end;
	gint start_line, end_line;

	end_line = gtk_text_iter_get_line (location);
	start_line = end_line - lines;
	*end_line_ptr = end_line;

	gtk_text_buffer_get_iter_at_line (textbuffer, &start, start_line);
	gtk_text_buffer_get_iter_at_line (textbuffer, &end, end_line);
	gtk_text_iter_forward_to_line_end (&end);

	highlight_apply (textbuffer, &start, &end);
}

void
ui_preferences_window_show ()
{
	gtk_widget_show_all (GTK_WIDGET (preferences_window->toplevel));
}

void
ui_preferences_window_hide ()
{
	gtk_widget_hide (GTK_WIDGET (preferences_window->toplevel));
}

static void
ui_editors_config_update ()
{
	CEditor *editor;
	GList * iterator;

	for (iterator = window->editor_list; iterator; iterator = iterator->next) {
		CEditor *editor;

		editor = (CEditor *) iterator->data;
		ceditor_highlighting_update (editor);
	}
}

void
ui_preferences_config_update ()
{
	const gchar *font;
	GdkRGBA keyword_color;
	GdkRGBA string_color;
	GdkRGBA constant_color;
	GdkRGBA comment_color;
	GdkRGBA preprocessor_color;

	font = gtk_font_button_get_font_name (GTK_FONT_BUTTON (preferences_window->font_button));
	gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (preferences_window->keyword_button), &keyword_color);
	gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (preferences_window->string_button), &string_color);
	gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (preferences_window->constant_button), &constant_color);
	gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (preferences_window->comment_button), &comment_color);
	gtk_color_chooser_get_rgba (GTK_COLOR_CHOOSER (preferences_window->preprocessor_button), &preprocessor_color);

	editorconfig_config_update (font, &keyword_color, &string_color, &constant_color, 
								&comment_color, &preprocessor_color);
	ui_editors_config_update ();
}

void
ui_current_editor_breakpoint_update (gdouble x, gdouble y, gchar *breakpoint_desc)
{
	
	CEditor *editor;

	editor = ui_get_current_editor ();

	if (editor == NULL) {
		return;
	}

	ceditor_breakpoint_update (editor, x, y, breakpoint_desc);
}

void
ui_editors_breakpoint_tag_update ()
{
	
	CEditor *editor;
	GList * iterator;

	for (iterator = window->editor_list; iterator; iterator = iterator->next) {
		CEditor *editor;

		editor = (CEditor *) iterator->data;
		ceditor_breakpoint_tags_resize (editor);
	}
}

void
ui_watchtree_cell_change (const gchar *path_string, const gchar *new_text, const gchar *value)
{
	debugview_watchtree_cell_change (window->debug_view, path_string, new_text, value);
}

void
ui_enable_debug_view ()
{
	debugview_enable (window->debug_view);
	gtk_notebook_set_current_page (GTK_NOTEBOOK (window->info_notebook), PAGE_DEBUG);
}

void
ui_disable_debug_view ()
{
	debugview_disable (window->debug_view);
	gtk_notebook_set_current_page (GTK_NOTEBOOK (window->info_notebook), PAGE_INFO);
}

void
ui_breakpoint_tags_get (GList **list)
{
	
	CEditor *editor;
	GList * iterator;

	for (iterator = window->editor_list; iterator; iterator = iterator->next) {
		CEditor *editor;

		editor = (CEditor *) iterator->data;
		ceditor_breakpoint_tags_get (editor, list);
	}
}

void
ui_debug_ptr_add (const gchar *filepath, const gint line)
{
	
	CEditor *editor;
	GList * iterator;

	if (window->debug_ptr != NULL) {
		ui_debug_ptr_remove ();
	}

	if (filepath == NULL) {
		editor = ui_get_current_editor ();
		window->debug_ptr = (GObject *) ceditor_icon_add (editor, line);

		return;
	}

	for (iterator = window->editor_list; iterator; iterator = iterator->next) {
		editor = (CEditor *) iterator->data;
		if (g_strcmp0 (filepath, editor->filepath) == 0) {
			window->debug_ptr = (GObject *) ceditor_icon_add (editor, line);

			return;
		}
	}
}

void
ui_debug_ptr_remove ()
{
	if (window->debug_ptr != NULL) {
		gtk_widget_destroy (GTK_WIDGET (window->debug_ptr));
	}
	window->debug_ptr = NULL;
}

void
ui_debug_view_clear ()
{
	debugview_clear (window->debug_view);
}

void
ui_debug_view_locals_add (const gchar *name, const gchar *value)
{
	debug_view_localtree_add (window->debug_view, name, value);
}

void
ui_debug_view_stack_add (const gchar *frame_name, const gchar *frame_args,
						 const gchar *file_line)
{
	debug_view_calltree_add (window->debug_view, frame_name, frame_args, file_line);
}

void
ui_select_editor_with_path (const gchar *filepath)
{
	
	GList * iterator;

	for (iterator = window->editor_list; iterator; iterator = iterator->next) {
		CEditor *editor;

		editor = (CEditor *) iterator->data;
		if (g_strcmp0 (editor->filepath, filepath) == 0) {
			gint index;

			index = gtk_notebook_page_num (GTK_NOTEBOOK (window->code_notebook), 
										   editor->textbox);
			gtk_notebook_set_current_page (GTK_NOTEBOOK (window->code_notebook), index);

			return;
		}
	}
}

void
ui_debug_view_get_all_expression (GList **list)
{
	debugview_watchtree_get_all (window->debug_view, list);
}

void
ui_debug_view_set_values (GList *list)
{
	debugview_watchtree_set_all (window->debug_view, list);
}

void
ui_current_editor_step_add (const gboolean insert, const gint offset, const gint len,
							const gchar *text)
{
	CEditor *editor;

	editor = ui_get_current_editor ();

	if (editor == NULL) {
		return;
	}

	ceditor_step_add (editor, insert, offset, len, text);
}

gboolean
ui_current_editor_can_undo ()
{
	CEditor *editor;

	editor = ui_get_current_editor ();

	if (editor == NULL) {
		return FALSE;
	}

	return ceditor_can_undo (editor);
}

gboolean
ui_current_editor_can_redo ()
{
	CEditor *editor;

	editor = ui_get_current_editor ();

	if (editor == NULL) {
		return FALSE;
	}

	return ceditor_can_redo (editor);
}

void
ui_current_editor_undo ()
{
	
	CEditor *editor;

	editor = ui_get_current_editor ();

	if (editor == NULL) {
		return;
	}

	ceditor_undo (editor);
}

void
ui_current_editor_redo ()
{	
	
	CEditor *editor;

	editor = ui_get_current_editor ();

	if (editor == NULL) {
		return;
	}

	ceditor_redo (editor);
}

void
ui_undo_redo_widgets_update ()
{
	if (ui_current_editor_can_undo ()) {
		ui_enable_undo_widgets ();
	}
	else {
		ui_disable_undo_widgets ();
	}
	
	if (ui_current_editor_can_redo ()) {
		ui_enable_redo_widgets ();
	}
	else {
		ui_disable_redo_widgets ();
	}
}

const gchar *
ui_search_entry_get_text ()
{
	return gtk_entry_get_text (GTK_ENTRY (window->search_entry));
}

void
ui_current_editor_search_init (const gint matched)
{
	CEditor *editor;

	editor = ui_get_current_editor ();

	if (editor == NULL) {
		return;
	}

	ceditor_search_init (editor, matched);
}

gint
ui_current_editor_search_next (const gboolean pre)
{
	CEditor *editor;

	editor = ui_get_current_editor ();

	if (editor == NULL) {
		return 0;
	}

	return ceditor_search_next (editor, pre);
}

void
ui_current_editor_select_range (const gint offset, const gint len)
{
	CEditor *editor;

	editor = ui_get_current_editor ();

	if (editor == NULL) {
		return;
	}

	ceditor_select_range (editor, offset, len);
}

void
ui_set_window_title (const gchar *project_name)
{
	gchar title[MAX_FILEPATH_LENGTH + 1];

	g_snprintf (title, MAX_FILEPATH_LENGTH, "Codefox - %s", project_name);
	gtk_window_set_title (GTK_WINDOW (window->toplevel), title);
}

void
ui_set_project_label (const gchar *project_name)
{
	gchar label[MAX_FILEPATH_LENGTH + 1];

	g_snprintf (label, MAX_FILEPATH_LENGTH, "%s: %s", _("Project"), project_name);
	gtk_label_set_label (GTK_LABEL (window->projectlabel), label);
}

void
ui_editor_close_by_path (const gchar *filepath)
{
	CEditor *editor;

	editor = ui_get_editor (filepath);

	if (editor == NULL) {
		return;
	}

	if (ceditor_get_dirty (editor)) {
		ceditor_set_dirty (editor, 0);
	}
	ceditor_emit_close_signal (editor);
}
