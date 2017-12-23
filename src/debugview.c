/*
 * debugview.c
 * This file is part of codefox
 *
 * Copyright (C) 2012-2017 - Gordon Li
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */



#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glib/gi18n-lib.h>

#include "debugview.h"
#include "callback.h"

#define MAX_CELL_LENGTH 1000

static void debug_view_localtree_init (CDebugView *debug_view);
static void debug_view_calltree_init (CDebugView *debug_view);
static void debug_view_watchtree_init (CDebugView *debug_view);

static void
debug_view_localtree_init (CDebugView *debug_view)
{
	GtkListStore *store;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *select;

	store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, _("Local Variable:"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_add_attribute(column, renderer, "text", 0);
	gtk_tree_view_append_column (GTK_TREE_VIEW (debug_view->localtree), column);
	gtk_tree_view_column_set_resizable (column, TRUE);
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, _("Value:"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_add_attribute(column, renderer, "text", 1);
	gtk_tree_view_append_column (GTK_TREE_VIEW (debug_view->localtree), column);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_set_model (GTK_TREE_VIEW(debug_view->localtree), GTK_TREE_MODEL (store));
	select = gtk_tree_view_get_selection (GTK_TREE_VIEW(debug_view->localtree));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
	gtk_tree_view_columns_autosize (GTK_TREE_VIEW(debug_view->localtree));
}

static void
debug_view_calltree_init (CDebugView *debug_view)
{
	GtkListStore *store;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *select;

	store = gtk_list_store_new (3, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, _("Function:"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_add_attribute(column, renderer, "text", 0);
	gtk_tree_view_append_column (GTK_TREE_VIEW (debug_view->calltree), column);
	gtk_tree_view_column_set_resizable (column, TRUE);
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, _("Arguments:"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_add_attribute(column, renderer, "text", 1);
	gtk_tree_view_append_column (GTK_TREE_VIEW (debug_view->calltree), column);
	gtk_tree_view_column_set_resizable (column, TRUE);
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, _("Source File:"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_add_attribute(column, renderer, "text", 2);
	gtk_tree_view_append_column (GTK_TREE_VIEW (debug_view->calltree), column);
	gtk_tree_view_column_set_resizable (column, TRUE);

	gtk_tree_view_set_model (GTK_TREE_VIEW(debug_view->calltree), GTK_TREE_MODEL (store));
	select = gtk_tree_view_get_selection (GTK_TREE_VIEW(debug_view->calltree));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
	gtk_tree_view_columns_autosize (GTK_TREE_VIEW(debug_view->calltree));
}

static void
debug_view_watchtree_init (CDebugView *debug_view)
{
	GtkListStore *store;
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *select;

	store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_STRING);
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, _("Expression:"));
	renderer = gtk_cell_renderer_text_new ();
	g_object_set(renderer, "editable", TRUE, NULL);
	g_signal_connect(renderer, "edited", (GCallback) on_watchtree_edited, NULL);
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_add_attribute(column, renderer, "text", 0);
	gtk_tree_view_append_column (GTK_TREE_VIEW (debug_view->watchtree), column);
	gtk_tree_view_column_set_resizable (column, TRUE);
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, _("Value:"));
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_add_attribute(column, renderer, "text", 1);
	gtk_tree_view_append_column (GTK_TREE_VIEW (debug_view->watchtree), column);
	gtk_tree_view_column_set_resizable (column, TRUE);
	gtk_tree_view_set_model (GTK_TREE_VIEW(debug_view->watchtree), GTK_TREE_MODEL (store));
	select = gtk_tree_view_get_selection (GTK_TREE_VIEW(debug_view->watchtree));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
	gtk_tree_view_columns_autosize (GTK_TREE_VIEW(debug_view->watchtree));
}

CDebugView *
debugview_new (GObject *localtree, GObject *calltree, GObject *watchtree)
{
	CDebugView *debug_view;

	debug_view = (CDebugView *) g_malloc (sizeof (CDebugView));
	debug_view->localtree = localtree;
	debug_view->calltree = calltree;
	debug_view->watchtree = watchtree;

	debug_view_localtree_init (debug_view);
	debug_view_calltree_init (debug_view);
	debug_view_watchtree_init (debug_view);

	return debug_view;
}

void
debugview_disable (CDebugView *debug_view)
{
	GtkListStore *store;
	GtkTreeIter iter;

	store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW (debug_view->localtree)));
	gtk_list_store_clear (store);
	store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW (debug_view->calltree)));
	gtk_list_store_clear (store);
	store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW (debug_view->watchtree)));
	gtk_list_store_clear (store);
	gtk_list_store_append (store, &iter);
	gtk_list_store_set(store, &iter, 0, "", 1, "", -1);

	gtk_widget_set_sensitive (GTK_WIDGET (debug_view->localtree), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (debug_view->calltree), FALSE);
	gtk_widget_set_sensitive (GTK_WIDGET (debug_view->watchtree), FALSE);
}

void
debugview_enable (CDebugView *debug_view)
{
	gtk_widget_set_sensitive (GTK_WIDGET (debug_view->localtree), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET (debug_view->calltree), TRUE);
	gtk_widget_set_sensitive (GTK_WIDGET (debug_view->watchtree), TRUE);
}

void
debugview_watchtree_cell_change (CDebugView *debug_view, const gchar *path_string,
								 const gchar *new_text, const gchar *value)
{
	GtkListStore *store;
	GtkTreeIter iter;
	gchar old_text[MAX_CELL_LENGTH + 1];

	store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW (debug_view->watchtree)));
	gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (store), &iter, path_string);
	gtk_tree_model_get (GTK_TREE_MODEL (store), &iter, 0, &old_text, -1);
	gtk_list_store_set (store, &iter, 0, new_text, 1, value, -1);

	if (old_text[0] == 0 && new_text[0] != 0) {
		gtk_list_store_append (store, &iter);
		gtk_list_store_set(store, &iter, 0, "", 1, "", -1);
	}
}

void
debugview_watchtree_get_all (CDebugView *debug_view, GList **list)
{
	GtkListStore *store;
	GtkTreeIter iter;
	gchar *line;

	store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW (debug_view->watchtree)));
	gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (store), &iter, "0");
	gtk_tree_model_get (GTK_TREE_MODEL (store), &iter, 0, &line, -1);

	if (line[0] == 0) {
		return;
	}

	*list = g_list_append (*list, (gpointer) line);
	while (gtk_tree_model_iter_next (GTK_TREE_MODEL (store), &iter)) {
		line = (gchar *) g_malloc (MAX_CELL_LENGTH + 1);
		gtk_tree_model_get (GTK_TREE_MODEL (store), &iter, 0, &line, -1);

		if (!line[0]) {
			return;
		}
		 *list = g_list_append (*list, (gpointer) line);
	}
}

void
debugview_watchtree_set_all (CDebugView *debug_view, GList *list)
{
	GtkListStore *store;
	GtkTreeIter iter;
	GSList *iterator;
	gint row;
	gchar path[MAX_CELL_LENGTH + 1];

	store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW (debug_view->watchtree)));
	row = 0;
	for (iterator = (GSList *) list; iterator; iterator = iterator->next) {
		gchar *value;

		value = (gchar *) iterator->data;
		g_snprintf (path, MAX_CELL_LENGTH, "%d", row);
		gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (store), &iter, path);
		gtk_list_store_set (store, &iter, 1, value, -1);
		row++;
	}
}

void
debugview_clear (CDebugView *debug_view)
{
	GtkListStore *store;

	store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW (debug_view->localtree)));
	gtk_list_store_clear (store);
	store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW (debug_view->calltree)));
	gtk_list_store_clear (store);
}

void
debug_view_localtree_add (CDebugView *debug_view, const gchar *name, const gchar *value)
{
	GtkListStore *store;
	GtkTreeIter  iter;

	store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW (debug_view->localtree)));
	gtk_list_store_append (store, &iter);
	gtk_list_store_set(store, &iter, 0, name, 1, value, -1);
}

void
debug_view_calltree_add (CDebugView *debug_view, const gchar *frame_name, const gchar *frame_args,
						 const gchar *file_line)
{
	GtkListStore *store;
	GtkTreeIter  iter;

	store = GTK_LIST_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW (debug_view->calltree)));
	gtk_list_store_append (store, &iter);
	gtk_list_store_set(store, &iter, 0, frame_name, 1, frame_args, 2, file_line, -1);
}
