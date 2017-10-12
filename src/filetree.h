/*
 * filetree.h
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

#ifndef FILETREE_H
#define FILETREE_H

#include <gtk/gtk.h>

#define CODEFOX_STOCK_FILE "codefox-file"
#define CODEFOX_STOCK_DIR "codefox-dir"

typedef enum {
	ICON,
	FILENAME,
	ISFILE,
	FILEPATH
} CFileTreeColumn;

typedef enum {
	LEVEL_FIRST = -1,
	LEVEL_SECOND = -2
} CFileTreeLevel;

/*
void
filetree_read_project (GtkTreeView *tree, const gchar *file);
*/

void
filetree_append (GtkTreeView *tree, GtkTreeIter *father, 
				 gboolean file, const gchar *filename, const gchar *filepath);

void
filetree_append_to_default (GtkTreeView *tree, gboolean file,
							const gchar *filename, const gchar *filepath);

void
filetree_get_current_store (GtkTreeView *tree, gchar **filename,
							gchar **filepath, gint *isfile, gint *child);

gboolean
filetree_foreach_select (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter,
						 gpointer data);

void
filetree_set_select (GtkTreeView *tree, const gchar *filepath);

gboolean
filetree_foreach_remove (GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter,
						 gpointer data);

void
filetree_remove (GtkTreeView *tree, const gchar *filepath);

void
filetree_set_selected_path (GtkTreeView *tree, const gchar *filepath,
							const gchar *oldpath);

void
filetree_init (GtkTreeView *tree);

void
filetree_project_init (GtkTreeView *tree, const gchar *project_name, const gchar *project_dir);

void
filetree_append_to_current (GtkTreeView *tree, const gchar *filename, const gint isfile);

void
filetree_select_second_level (GtkTreeView *tree, const gint row);

gint
filetree_get_row_second_level (GtkTreeView *tree);

void
filetree_append_to_second_level (GtkTreeView *tree, gint row, const gchar *filename, const gint isfile);

#endif /* FILETREE_H */
