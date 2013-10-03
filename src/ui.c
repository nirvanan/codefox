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
#include "time.h"
#include "staticcheck.h"
#include "symbol.h"
#include "prefix.h"
#include "editor.h"

#define TIME_BUF_SIZE 100
#define MESSAGE_BUF_SIZE 1000
#define MAX_FILEPATH_LENTH 1000

#define LOGO_SIZE 160

#define CODEFOX_STOCK_BUILD "codefox-build"
#define CODEFOX_STOCK_RUN "codefox-run"
#define CODEFOX_STOCK_DEBUG "codefox-debug"

/* Main window of codefox. */
CWindow *window;

static GtkStockItem items[] =
{
	{CODEFOX_STOCK_BUILD, "Build", 0, 0, NULL},
	{CODEFOX_STOCK_RUN, "Run", 0, 0, NULL},
	{CODEFOX_STOCK_DEBUG, "Debug", 0, 0, NULL },
	{CODEFOX_STOCK_ERROR, "Error", 0, 0, NULL },
	{CODEFOX_STOCK_WARNING, "Warning", 0, 0, NULL },
	{CODEFOX_STOCK_FILE, "File", 0, 0, NULL },
	{CODEFOX_STOCK_DIR, "Dir", 0, 0, NULL }
};

static const gchar *sig[3] = {"cut-clipboard", "copy-clipboard", 
							  "paste-clipboard"};

static void signal_connect ();
static void ui_window_init(GtkBuilder *builder, CWindow *window);
static void ui_filetree_init(CWindow *window);
static void ui_toolpad_init(CWindow *window);
static void ui_init_stock_items();
static CEditor *ui_get_current_editor();

/* Connect all callbacks widgets need. */
static void
signal_connect ()
{
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
	g_signal_connect (window->cut_item, "activate", 
					  G_CALLBACK (cut_cut_code), NULL);
	g_signal_connect (window->copy_item, "activate", 
					  G_CALLBACK (copy_copy_code), NULL);
	g_signal_connect (window->paste_item, "activate", 
					  G_CALLBACK (paste_paste_code), NULL);
	g_signal_connect (window->delete_item, "activate", 
					  G_CALLBACK (delete_delete_code), NULL);
	g_signal_connect (window->format_item, "activate", 
					  G_CALLBACK (format_format_code), NULL);
	g_signal_connect (window->build_item, "activate", 
					  G_CALLBACK (build_compile), NULL);
	g_signal_connect (window->run_item, "activate", 
					  G_CALLBACK (run_run_executable), NULL);
	g_signal_connect (window->debug_item, "activate", 
					  G_CALLBACK (debug_debug_executable), NULL);
	g_signal_connect (window->new_toolbar, "clicked", 
					  G_CALLBACK (new_create_new_tab), NULL);
	g_signal_connect (window->open_toolbar, "clicked", 
					  G_CALLBACK (open_open_local_file), NULL);
	g_signal_connect (window->save_toolbar, "clicked", 
					  G_CALLBACK (save_save_current_code), NULL);
	g_signal_connect (window->saveas_toolbar, "clicked", 
					  G_CALLBACK (saveas_save_to_file), NULL);
	g_signal_connect (window->cut_toolbar, "clicked", 
					  G_CALLBACK (cut_cut_code), NULL);
	g_signal_connect (window->copy_toolbar, "clicked", 
					  G_CALLBACK (copy_copy_code), NULL);
	g_signal_connect (window->paste_toolbar, "clicked", 
					  G_CALLBACK (paste_paste_code), NULL);
	g_signal_connect (window->delete_toolbar, "clicked", 
					  G_CALLBACK (delete_delete_code), NULL);
	g_signal_connect (window->build_toolbar, "clicked", 
					  G_CALLBACK (build_compile), NULL);
	g_signal_connect (window->run_toolbar, "clicked", 
					  G_CALLBACK (run_run_executable), NULL);
	g_signal_connect (window->debug_toolbar, "clicked", 
					  G_CALLBACK (debug_debug_executable), NULL);
	g_signal_connect (window->toplevel, "delete-event", 
					  G_CALLBACK (quit_quit_program), NULL);
						
	g_signal_connect (window->code_notebook, "switch-page", 
					  G_CALLBACK (on_switch_page), NULL);
	
	/* Load filetreeview and connect sighal handler. */
	GtkTreeSelection *select;
	
	select = gtk_tree_view_get_selection (GTK_TREE_VIEW(window->filetree));
	g_signal_connect (select, "changed", 
						G_CALLBACK (on_filetree_selection_changed), NULL);
	select = gtk_tree_view_get_selection (GTK_TREE_VIEW(window->compilertree));
	g_signal_connect (select, "changed", 
						G_CALLBACK (on_compilertree_selection_changed), NULL);
}

/* Get all widgets in the builder by name*/
static void
ui_window_init(GtkBuilder *builder, CWindow *window)
{
	window->toplevel = gtk_builder_get_object (builder, "toplevel");
	window->about_item = gtk_builder_get_object (builder, "aboutmenuitem");
	window->help_item = gtk_builder_get_object (builder, "helpmenuitem");
	window->new_item =  gtk_builder_get_object (builder, "newmenuitem");
	window->open_item =  gtk_builder_get_object (builder, "openmenuitem");
	window->save_item =  gtk_builder_get_object (builder, "savemenuitem");
	window->saveas_item =  gtk_builder_get_object (builder, "saveasmenuitem");
	window->quit_item = gtk_builder_get_object (builder, "quitmenuitem");
	window->cut_item =  gtk_builder_get_object (builder, "cutmenuitem");
	window->copy_item =  gtk_builder_get_object (builder, "copymenuitem");
	window->paste_item =  gtk_builder_get_object (builder, "pastemenuitem");
	window->delete_item =  gtk_builder_get_object (builder, "deletemenuitem");
	window->format_item =  gtk_builder_get_object (builder, "formatmenuitem");
	window->build_item =  gtk_builder_get_object (builder, "buildmenuitem");
	window->run_item =  gtk_builder_get_object (builder, "runmenuitem");
	window->debug_item =  gtk_builder_get_object (builder, "debugmenuitem");
	window->new_toolbar =  gtk_builder_get_object (builder, "newbutton");
	window->open_toolbar =  gtk_builder_get_object (builder, "openbutton");
	window->save_toolbar =  gtk_builder_get_object (builder, "savebutton");
	window->saveas_toolbar =  gtk_builder_get_object (builder, "saveasbutton");
	window->cut_toolbar =  gtk_builder_get_object (builder, "cutbutton");
	window->copy_toolbar =  gtk_builder_get_object (builder, "copybutton");
	window->paste_toolbar =  gtk_builder_get_object (builder, "pastebutton");
	window->delete_toolbar =  gtk_builder_get_object (builder, "deletebutton");
	window->build_toolbar =  gtk_builder_get_object (builder, "buildbutton");
	window->run_toolbar =  gtk_builder_get_object (builder, "runbutton");
	window->debug_toolbar =  gtk_builder_get_object (builder, "debugbutton");
	window->code_notebook = gtk_builder_get_object (builder, "codenotebook");
	window->info_notebook = gtk_builder_get_object (builder, "infonotebook");
	window->filetree = gtk_builder_get_object (builder, "filetree");
	window->statustree = gtk_builder_get_object (builder, "statustree");
	window->compilertree = gtk_builder_get_object (builder, "compilertree");
	window->notepadview = gtk_builder_get_object (builder, "notepadview");
	window->locationlabel = gtk_builder_get_object (builder, "locationlabel");
	window->modelabel = gtk_builder_get_object (builder, "modelabel");
}

/* Init the file tree view. */
static void
ui_filetree_init(CWindow *window)
{
	filetree_init (GTK_TREE_VIEW (window->filetree));
}

/* Initialize the status view and notepad. */
static void
ui_toolpad_init(CWindow *window)
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
	gtk_tree_view_set_model (window->statustree, GTK_TREE_MODEL (store));
	select = gtk_tree_view_get_selection (GTK_TREE_VIEW(window->statustree));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
	time = g_malloc (100);
	time_get_now (time);
	store = GTK_TREE_STORE (gtk_tree_view_get_model (window->statustree));
	gtk_tree_store_append (store, &iter, NULL);
	welcome_text = g_malloc (MESSAGE_BUF_SIZE);
	g_strlcpy (welcome_text, _("This is Codefox "), MESSAGE_BUF_SIZE);
	g_strlcat (welcome_text, PACKAGE_VERSION, MESSAGE_BUF_SIZE);
	g_strlcat (welcome_text, _(", have fun!"), MESSAGE_BUF_SIZE);
	gtk_tree_store_set(store, &iter, 0, time,
					   1, welcome_text, 
					   2, "Orange", -1);
	g_free (welcome_text);
	store = gtk_tree_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, _("Compiler outputs:"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_add_attribute(column, renderer, "text", 0);
	gtk_tree_view_column_add_attribute(column, renderer, "foreground", 1);
	gtk_tree_view_append_column (GTK_TREE_VIEW (window->compilertree), column);
	g_object_ref (store);
	gtk_tree_view_set_model (window->compilertree, GTK_TREE_MODEL (store));
	select = gtk_tree_view_get_selection (GTK_TREE_VIEW(window->compilertree));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
	
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (window->notepadview));
	gtk_text_buffer_set_text (buffer, _("Write down any notes you want here..."), -1);
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

static void
ui_init_stock_items()
{
	gint i, j;
	gint icon_size[4] = {16, 24, 32, 48};

	gtk_stock_add((GtkStockItem *) items, G_N_ELEMENTS (items));
	
	for (i = 0; i < G_N_ELEMENTS (items); i++) {
		GError *error = NULL;
		GtkIconTheme *icon_theme;
		GdkPixbuf *pixbuf;
		GtkIconSet *icon_set;
		GtkIconFactory *factory;

		icon_theme = gtk_icon_theme_get_default ();
		icon_set = gtk_icon_set_new ();
		factory = gtk_icon_factory_new ();

		for (j = 0; j < G_N_ELEMENTS (icon_size); j++) {
			pixbuf = gtk_icon_theme_load_icon (icon_theme, items[i].stock_id, icon_size[j], 0, &error);
			if (!pixbuf) {
			    g_warning (_("Couldn't load icon: %s"), error->message);
			    g_error_free (error);
			} else {
				GtkIconSource *source;

				source = gtk_icon_source_new ();
				gtk_icon_source_set_pixbuf (source, pixbuf);
				gtk_icon_set_add_source (icon_set, source);
			}

			g_object_unref (pixbuf);
		}

		gtk_icon_factory_add (factory, items[i].stock_id, icon_set);
		gtk_icon_factory_add_default (factory);
		gtk_icon_set_unref (icon_set);
	}
	
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
	ui_init_stock_items ();

	builder = gtk_builder_new ();
	data_dir = g_build_filename (CODEFOX_DATADIR, "codefox", NULL);
	template_file = g_build_filename (data_dir, "codefox.ui", NULL);
	gtk_builder_add_from_file (builder, template_file, NULL);

	g_free (data_dir);
	g_free (template_file);
	g_free (icon_path);
	
	window = (CWindow *) g_malloc (sizeof(CWindow));
	
	ui_window_init(builder, window);
	ui_filetree_init(window);
	ui_toolpad_init(window);
	
	/* Disable items for saving since there is no opened file! */
	ui_disable_save_widgets ();
	
	signal_connect ();
}

/* Create a new about dialog. */
void
ui_about_dialog_new()
{
	GtkWidget *about_dialog = gtk_about_dialog_new ();
	const gchar *authors[] = {"Gordon Lee"};
	GdkPixbuf *logo;
	GtkIconTheme *icon_theme;
	
	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(about_dialog), "Codefox");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(about_dialog), PACKAGE_VERSION);
    gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(about_dialog), authors);
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(about_dialog),
    							  _("Perhaps the most lightweight C/C++ IDE..."));
    gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(about_dialog), 
    							   "Copyright (c) 2012 Gordon Lee");
    gtk_about_dialog_set_license(GTK_ABOUT_DIALOG(about_dialog), "Free");
    icon_theme = gtk_icon_theme_get_default ();
    logo = gtk_icon_theme_load_icon (icon_theme, "codefox", LOGO_SIZE, 0, NULL);
    gtk_about_dialog_set_logo (GTK_ABOUT_DIALOG(about_dialog), logo);
    gtk_about_dialog_set_website (GTK_ABOUT_DIALOG(about_dialog),
    							  "http://lee75.brinkster.com");

    gtk_dialog_run (GTK_DIALOG(about_dialog));
    gtk_widget_destroy (about_dialog);
    g_object_unref (logo);
}

/* Create a new editor and show it. */
void
ui_editor_new()
{
	CEditor *new_editor;
	gint index;

	new_editor = ceditor_new (_("Untitled"));
	ceditor_show (new_editor);
	index = gtk_notebook_append_page (GTK_NOTEBOOK (window->code_notebook), 
									  new_editor->scroll, 
									  new_editor->label_box);
	
	window->editor_list = g_slist_append (window->editor_list, new_editor);
	if (index != -1)
		gtk_notebook_set_current_page (GTK_NOTEBOOK (window->code_notebook), 
									   index);
	if (g_slist_length (window->editor_list) != 0) {
		ui_enable_save_widgets ();
	}
}

/* Create a new editor with code and show it. */
void
ui_editor_new_with_text(const gchar *filepath, const gchar *code_buf)
{
	CEditor *new_editor;
	gint index;

	new_editor = ceditor_new_with_text (filepath, code_buf);
	ceditor_show (new_editor);
	index = gtk_notebook_append_page (GTK_NOTEBOOK (window->code_notebook), 
									  new_editor->scroll, 
									  new_editor->label_box);
	window->editor_list = g_slist_append (window->editor_list, new_editor);
									  
	/* Swap to show the created tab of codenotebook.*/
	if (index != -1)
		gtk_notebook_set_current_page (GTK_NOTEBOOK (window->code_notebook), 
									   index);

	if (g_slist_length (window->editor_list) != 0) {
		ui_enable_save_widgets ();
	}
}

/* Add a new entry to filetree. */
void
ui_filetree_entry_new(gboolean is_file, gchar *filename, gchar *filepath)
{
	filetree_append_to_default (GTK_TREE_VIEW (window->filetree), is_file, 
								filename, filepath);
}

/* Append a new status entry to statustree. */
void
ui_status_entry_new(const gint op, const gchar *filepath)
{
	GtkTreeStore *store;
	GtkTreeIter  iter;
	gchar *message_buf;
	gchar *time_buf;

	time_buf = g_malloc (TIME_BUF_SIZE);
	message_buf = g_malloc(MESSAGE_BUF_SIZE);
	time_get_now (time_buf);
	store = GTK_TREE_STORE (gtk_tree_view_get_model (window->statustree));
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
		g_strlcat (message_buf, _(" has been open."), MESSAGE_BUF_SIZE);
		break;
	}

	gtk_tree_store_set(store, &iter, 0, time_buf, 1, message_buf, 2, "black", -1);
	g_free (time_buf);
	g_free (message_buf);
}

/* Create a file choosing dialog and return the file path after user 
 * selected a localfile or typed a filename. 
 */
void
ui_get_filepath_from_dialog (gchar *filepath, const gboolean open, const gchar *default_path)
{
	GtkWidget *dialog;

	if (open) {
		dialog = gtk_file_chooser_dialog_new (_("Open File"),
											  GTK_WINDOW (window->toplevel),
											  GTK_FILE_CHOOSER_ACTION_OPEN,
											  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
											  GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
											  NULL);
	}
	else {
		dialog = gtk_file_chooser_dialog_new (_("Save File"),
											  GTK_WINDOW (window->toplevel),
											  GTK_FILE_CHOOSER_ACTION_SAVE,
											  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
											  GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
											  NULL);
		gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (dialog),
													    TRUE);
		gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (dialog),
										   default_path);
	}

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		g_strlcpy (filepath, gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog)),
				   MAX_FILEPATH_LENTH);
		
	}
	else {
		g_strlcpy (filepath, "NULL", MAX_FILEPATH_LENTH);
	}

	gtk_widget_destroy (dialog);
}

/* Check there is still editors in notebook. */
gboolean
ui_have_editor()
{
	return ui_get_current_editor() != NULL;
}

/* Find editor with associated filepath. */
gboolean
ui_find_editor(const gchar *filepath)
{
	GSList * iterator;

	for (iterator = window->editor_list; iterator; iterator = iterator->next)
	{
		CEditor *editor;

		editor = (CEditor *) iterator->data;
		if (g_strcmp0 (editor->filepath, filepath) == 0)
		{
			gint index;
		
			index = gtk_notebook_page_num (GTK_NOTEBOOK (window->code_notebook),
										   editor->scroll);
			gtk_notebook_set_current_page (GTK_NOTEBOOK (window->code_notebook), 
										   index);
			return TRUE;
		}
	}

	return FALSE;
}

static CEditor *
ui_get_current_editor()
{
	gint page;
	GtkWidget *current_page;
	GSList * iterator;
	
	page = gtk_notebook_get_current_page (GTK_NOTEBOOK (window->code_notebook));

	if (page == -1)
		return NULL;

	current_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (window->code_notebook),
											  page);

	for (iterator = window->editor_list; iterator; iterator = iterator->next) {
		CEditor *editor;

		editor = (CEditor *) iterator->data;
		if (GTK_WIDGET (editor->scroll) == current_page) {
			return editor;
		}
	}

	return NULL;
}

/* Get current editor filepath under editing. */
void
ui_current_editor_filepath(gchar *filepath)
{
	CEditor *editor;

	editor = ui_get_current_editor ();
	g_strlcpy (filepath, editor->filepath, MAX_FILEPATH_LENTH);
}

/* Get current editor code under editing. */
gchar *
ui_current_editor_code()
{
	CEditor *editor;
	GtkTextBuffer *buffer;
	GtkTextIter start, end;

	editor = ui_get_current_editor ();
			
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor->textview));
	gtk_text_buffer_get_start_iter (buffer, &start);
	gtk_text_buffer_get_end_iter (buffer, &end);

	return gtk_text_buffer_get_text (buffer, &start, &end, 1);

}

/* Post action after saving a file. */
void
ui_save_code_post(const gchar *filepath)
{
	CEditor *editor;

	editor = ui_get_current_editor ();
	if (ceditor_get_dirty (editor))
		ceditor_set_dirty (editor, 0);
	filetree_set_selected_path (GTK_TREE_VIEW (window->filetree),
								filepath,
								editor->filepath);
	ceditor_set_path (editor, filepath);
}

/* Emit a clipboard signal. */
void
ui_emit_clipboard_signal(gint clipboard_signal)
{
	CEditor *editor;

	editor = ui_get_current_editor ();

	if (editor == NULL)
		return ;

	g_signal_emit_by_name (editor->textview, sig[clipboard_signal], NULL);
}

/* Emit a click signal on save button. */
void
ui_emit_save_signal()
{
	CEditor *editor;

	editor = ui_get_current_editor ();

	if (editor == NULL)
		return ;

	ceditor_emit_save_signal (editor);
}

/* Delete selected code from current editor. */
void
ui_current_editor_delete_range()
{
	CEditor *editor;
	GtkTextIter start, end;
	GtkTextMark *selected, *insert;
	GtkTextBuffer *buffer;

	editor = ui_get_current_editor ();

	if (editor == NULL)
		return ;
			
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor->textview));
	selected = gtk_text_buffer_get_selection_bound (buffer);
	insert = gtk_text_buffer_get_insert (buffer);
	gtk_text_buffer_get_iter_at_mark (buffer, &start, selected);
	gtk_text_buffer_get_iter_at_mark (buffer, &end, insert);
	gtk_text_buffer_delete (buffer, &start, &end);	
}

/* Format current code under editting. */
void
ui_current_editor_format()
{
	CEditor *editor;
	GtkTextIter start, end;
	GtkTextBuffer *buffer;
	gchar* text;
	gint i;
	gint end_line;

	editor = ui_get_current_editor ();

	if (editor == NULL)
		return ;
	
	i = 0;
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor->textview));
	gtk_text_buffer_get_start_iter (buffer, &start);
	gtk_text_buffer_get_end_iter (buffer, &end);
	text =  gtk_text_buffer_get_text (buffer, &start, &end, 1);
	while (text[i++] == ' ')
		;
	gtk_text_buffer_get_iter_at_offset(buffer, &end, i - 1);
	gtk_text_buffer_delete (buffer, &start, &end);
	gtk_text_buffer_get_end_iter (buffer, &end);
	end_line = gtk_text_iter_get_line (&end);

	if (end_line >= 1)
		autoindent_apply (buffer, NULL, 0, end_line);
}

/* Update cursor position. */
void
ui_current_editor_update_cursor()
{
	GtkTextBuffer *buffer;
	GtkTextMark *insert;
	GtkTextIter iter;
	gint line, column;
	gchar *location;
	CEditor *editor;
	
	editor = ui_get_current_editor ();

	if (editor == NULL)
		return ;

	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor->textview));
	insert = gtk_text_buffer_get_insert (buffer);
	gtk_text_buffer_get_iter_at_mark (buffer, &iter, insert);
				
	line = gtk_text_iter_get_line (&iter);
	column = gtk_text_iter_get_line_offset (&iter);
				
	location = g_malloc (100);
	sprintf (location, _(" Line: %d\tColumn: %d"), line + 1, column);
	gtk_label_set_label (GTK_LABEL (window->locationlabel), location);
	g_free (location);
}

/* Change current edit mode. */
void
ui_current_editor_change_mode()
{
	CEditor *editor;
	gboolean overwrite;
	gchar *label;
	
	editor = ui_get_current_editor ();
	overwrite = gtk_text_view_get_overwrite (GTK_TEXT_VIEW (editor->textview));
	label = g_malloc (100);

	if (overwrite)
		sprintf (label, _(" Mode: %s"), _("Overwrite"));
	else
		sprintf (label, _(" Mode: %s"), _("Insert"));
	gtk_label_set_label (GTK_LABEL (window->modelabel), label);

	g_free (label);
}

/* Update line number label after editing code. */
void
ui_update_line_number_label (const gboolean insert, const gint append, 
							 const GtkTextIter *start, const GtkTextIter *end)
{
	CEditor *editor;

	editor = ui_get_current_editor ();

	if (editor == NULL)
		return ;

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
ui_switch_page()
{
	CEditor *editor;

	editor = ui_get_current_editor ();

	if (editor == NULL)
		return ;

	ui_current_editor_change_mode();
	ui_current_editor_update_cursor();
	filetree_set_select (GTK_TREE_VIEW (window->filetree), editor->filepath);
}
