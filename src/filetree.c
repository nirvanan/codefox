/*
 * filetree.c
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

#include <string.h>

#ifdef HAVE_CONFIG_H
 #include "config.h"
#endif

#include <glib/gi18n-lib.h>

#include "filetree.h"
#include "ui.h"

#define MAX_FILEPATH_LENTH 1000

typedef struct 
{
	gchar *filepath;
	GtkTreeView *tree;
} CForeachSelectData;

void
filetree_append (GtkTreeView *tree, GtkTreeIter *father, 
				 gboolean file, gchar *filename, gchar *filepath)
{
	GtkTreeIter  iter;
	GtkTreeStore *store;
	GtkTreePath *path;
	GtkTreeSelection *selection;
		
	store = GTK_TREE_STORE (gtk_tree_view_get_model (tree));
	gtk_tree_store_append (store, &iter, father);
	
	if (file)
		gtk_tree_store_set(store, &iter, ICON, CODEFOX_STOCK_FILE, 
						   FILENAME, filename,
						   ISFILE, file,
						   FILEPATH, filepath, -1);
	else
		gtk_tree_store_set(store, &iter, ICON, CODEFOX_STOCK_DIR, 
						   FILENAME, filename,
						   ISFILE, file,
						   FILEPATH, filepath, -1);
	
	selection = gtk_tree_view_get_selection (tree);
	path = gtk_tree_model_get_path (GTK_TREE_MODEL (store), &iter);
	gtk_tree_view_expand_to_path (tree, path);
	gtk_tree_selection_select_path (selection, path);
}

void
filetree_append_to_default (GtkTreeView *tree, gboolean file,
							gchar *filename, gchar *filepath)
{
	GtkTreeIter iter;
	GtkTreeStore *store;
	
	store = GTK_TREE_STORE(gtk_tree_view_get_model(tree));
	gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (store), &iter, "0");
	
	filetree_append (GTK_TREE_VIEW(tree), &iter, file, filename, filepath);
}

void
filetree_get_current_store (GtkTreeView *tree, gchar **filename,
					gchar **filepath, gint *isfile, gint *child)
{
	GtkTreeStore *store;
	GtkTreeIter iter;
	GtkTreeSelection *treeselection;
	gboolean has_node;
	
	treeselection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree));
	store = GTK_TREE_STORE(gtk_tree_view_get_model(tree));
	has_node = gtk_tree_selection_get_selected (treeselection, NULL, &iter);

	if (!has_node)
		return ;

	if (isfile != NULL)
		gtk_tree_model_get (GTK_TREE_MODEL (store), &iter,
							ISFILE, isfile, -1);
	if (filepath != NULL)
		gtk_tree_model_get (GTK_TREE_MODEL (store), &iter,
							FILEPATH, filepath, -1);
	if (filename != NULL)
		gtk_tree_model_get (GTK_TREE_MODEL (store), &iter,
							FILENAME, filename, -1);
	if (child != NULL) {
		(*child) = gtk_tree_model_iter_n_children (GTK_TREE_MODEL (store), &iter);

		if (g_strcmp0 (gtk_tree_model_get_string_from_iter (GTK_TREE_MODEL (store), &iter), "0") == 0)
			(*child) = LEVEL_FIRST;
		else if (g_strcmp0 (gtk_tree_model_get_string_from_iter (GTK_TREE_MODEL (store), &iter), "0:0") == 0 ||
				 g_strcmp0 (gtk_tree_model_get_string_from_iter (GTK_TREE_MODEL (store), &iter), "0:1") == 0 ||
				 g_strcmp0 (gtk_tree_model_get_string_from_iter (GTK_TREE_MODEL (store), &iter), "0:2") == 0)
			(*child) = LEVEL_SECOND;
	}
}

gboolean
filetree_foreach_select (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter,
                      gpointer data)
{
	CForeachSelectData *data_str = (CForeachSelectData *) data;
	gchar *filepath = data_str->filepath;
	gboolean isfile;
	
	gtk_tree_model_get (model, iter, ISFILE, &isfile, -1);
	if (isfile)
	{
		gchar *path;
		path = g_malloc (1024);
		
		gtk_tree_model_get (model, iter, FILEPATH, &path, -1);
		if (g_strcmp0 (path, filepath) == 0)
		{
			GtkTreeSelection *selection;
			
			selection = gtk_tree_view_get_selection (data_str->tree);
			gtk_tree_selection_select_iter (selection, iter);
			g_free (path);
			return 1;
		}
		else
		{
			g_free (path);
			return 0;
		}
	}
	else
		return 0;
}
	
void
filetree_set_select (GtkTreeView *tree, gchar *filepath)
{
    GtkTreeModel *model;
    CForeachSelectData data;
	
    data.filepath = filepath;
    data.tree = tree;
	model = gtk_tree_view_get_model (tree);
	gtk_tree_model_foreach (model,
							(GtkTreeModelForeachFunc) filetree_foreach_select,
							(gpointer) &data);
}

gboolean
filetree_foreach_remove (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter,
                      gpointer data)
{
	gchar *filepath = (gchar *) data;
	gboolean isfile;
	
	gtk_tree_model_get (model, iter, ISFILE, &isfile, -1);
	if (isfile)
	{
		gchar *path;
		path = g_malloc (1024);
		
		gtk_tree_model_get (model, iter, FILEPATH, &path, -1);
		if (g_strcmp0 (path, filepath) == 0)
		{
			gtk_tree_store_remove (GTK_TREE_STORE (model), iter);
			g_free (path);
			return 1;
		}
		else
		{
			g_free (path);
			return 0;
		}
	}
	else
		return 0;
}

void
filetree_remove (GtkTreeView *tree, gchar *filepath)
{
	GtkTreeModel *model;
	
	model = gtk_tree_view_get_model (tree);
	gtk_tree_model_foreach (model,
							(GtkTreeModelForeachFunc) filetree_foreach_remove,
							(gpointer) filepath);
}

void
filetree_set_selected_path (GtkTreeView *tree, gchar *filepath,
							gchar *oldpath)
{
	GtkTreeSelection *selection;
	GtkTreeIter iter;
	GtkTreeModel *model;
	
	filetree_set_select (tree, oldpath);
	selection = gtk_tree_view_get_selection (tree);
	gtk_tree_selection_get_selected (selection, NULL, &iter);	
	model = gtk_tree_view_get_model (tree);
	
	gint len;
	gint i;
	
	len = strlen (filepath);
	for (i = len - 1; filepath[i] != '/'; i--)
		;
	gtk_tree_store_set(GTK_TREE_STORE (model), &iter,
					   FILENAME, filepath + i + 1,
					   FILEPATH, filepath, -1);
}

void
filetree_init (GtkTreeView *tree)
{
	GtkTreeStore *store;	
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkTreeSelection *select;

	store = gtk_tree_store_new (4, G_TYPE_STRING, G_TYPE_STRING,
								G_TYPE_BOOLEAN, G_TYPE_STRING);
	column = gtk_tree_view_column_new ();
	gtk_tree_view_column_set_title (column, _("Project"));
	renderer = gtk_cell_renderer_pixbuf_new ();
	gtk_tree_view_column_pack_start (column, renderer, FALSE);
	gtk_tree_view_column_add_attribute(column, renderer, "stock-id", 0);
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_column_pack_start (column, renderer, TRUE);
	gtk_tree_view_column_add_attribute(column,renderer, "text", 1);
	gtk_tree_view_append_column (GTK_TREE_VIEW (tree), column);
	g_object_ref (store);
	
	gtk_tree_view_set_model (tree, GTK_TREE_MODEL (store));
}

/* Initilize three folders on filetree. */
void
filetree_project_init (GtkTreeView *tree, const gchar *project_name, const gchar *project_dir)
{
	gchar *root_dir;

	root_dir = (gchar *) g_malloc (MAX_FILEPATH_LENTH);
	g_strlcpy (root_dir, project_dir, MAX_FILEPATH_LENTH);
	g_strlcat (root_dir, "/", MAX_FILEPATH_LENTH);
	g_strlcat (root_dir, project_name, MAX_FILEPATH_LENTH);
	filetree_append (tree, NULL, 0, project_name, project_dir);
	filetree_append_to_default (tree, 0, _("header"), root_dir);
	filetree_append_to_default (tree, 0, _("source"), root_dir);
	filetree_append_to_default (tree, 0, _("resource"), root_dir);

	g_free (root_dir);
}

/* Append a item in current folder. */
void
filetree_append_to_current (GtkTreeView *tree, const gchar *filename, const gint isfile)
{
	GtkTreeStore *store;
	GtkTreeIter iter;
	GtkTreeSelection *treeselection;
	gchar *filepath;
	
	treeselection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree));
	store = GTK_TREE_STORE(gtk_tree_view_get_model(tree));
	gtk_tree_selection_get_selected (treeselection, NULL, &iter);
	filepath = (gchar *) g_malloc (MAX_FILEPATH_LENTH);

	gtk_tree_model_get (GTK_TREE_MODEL (store), &iter, FILEPATH, &filepath, -1);
	g_strlcat (filepath, "/", MAX_FILEPATH_LENTH);
	g_strlcat (filepath, filename, MAX_FILEPATH_LENTH);
	filetree_append (GTK_TREE_VIEW (tree), &iter, isfile, filename, filepath);
}

void
filetree_select_second_level (GtkTreeView *tree, const gint row)
{
	GtkTreeIter iter;
	GtkTreeStore *store;
	GtkTreeSelection *selection;
	
	store = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW (tree)));
	switch (row) {
	case -1:
		gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (store), &iter, "0");
		break;
	case 0:
		gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (store), &iter, "0:0");
		break;
	case 1:
		gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (store), &iter, "0:1");
		break;
	case 2:
		gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (store), &iter, "0:2");
		break;
	}
			
	selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree));
	gtk_tree_selection_select_iter (selection, &iter);
}

gint
filetree_get_row_second_level (GtkTreeView *tree)
{
	GtkTreeStore *store;
	GtkTreeIter iter;
	GtkTreeSelection *treeselection;
	gchar *path;
	
	treeselection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree));
	store = GTK_TREE_STORE(gtk_tree_view_get_model(tree));
	gtk_tree_selection_get_selected (treeselection, NULL, &iter);
	path = gtk_tree_model_get_string_from_iter (GTK_TREE_MODEL (store), &iter);

	return path[2] - '0';
}

void
filetree_append_to_second_level (GtkTreeView *tree, gint row, const gchar *filename, const gint isfile)
{
	GtkTreeStore *store;
	GtkTreeIter iter;
	GtkTreeSelection *treeselection;
	gchar *filepath;
	
	store = GTK_TREE_STORE(gtk_tree_view_get_model(tree));
	switch (row) {
	case -1:
		gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (store), &iter, "0");
		break;
	case 0:
		gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (store), &iter, "0:0");
		break;
	case 1:
		gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (store), &iter, "0:1");
		break;
	case 2:
		gtk_tree_model_get_iter_from_string (GTK_TREE_MODEL (store), &iter, "0:2");
		break;
	}

	gtk_tree_model_get (GTK_TREE_MODEL (store), &iter, FILEPATH, &filepath, -1);
	g_strlcat (filepath, "/", MAX_FILEPATH_LENTH);
	g_strlcat (filepath, filename, MAX_FILEPATH_LENTH);
	filetree_append (GTK_TREE_VIEW (tree), &iter, isfile, filename, filepath);
}