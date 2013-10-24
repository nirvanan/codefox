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
	
	filetree_append (GTK_TREE_VIEW(tree), &iter, 1, filename, filepath);
}

gboolean
filetree_get_file_path (GtkTreeView *tree, GtkTreeIter *iter, gchar *filepath)
{
	gchar *temp;
	gboolean isfile;
	GtkTreeStore *store;
	
	temp = g_malloc (1024);
	store = GTK_TREE_STORE(gtk_tree_view_get_model(tree));
	gtk_tree_model_get (GTK_TREE_MODEL (store), iter,
						ISFILE, &isfile, -1);
	if (!isfile)
	{
		filepath = NULL;
		g_free (temp);
		return 0;
	}	
	gtk_tree_model_get (GTK_TREE_MODEL (store), iter,
						FILEPATH, &temp, -1);
	g_strlcpy (filepath, temp, 1024);
	g_free (temp);
	return 1;
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
	
	filetree_append (tree, NULL, 0, _("default"), NULL);
	
	select = gtk_tree_view_get_selection (GTK_TREE_VIEW(tree));
	gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
}
