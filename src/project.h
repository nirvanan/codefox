/*
 * project.h
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

#include <gtk/gtk.h>

typedef enum ProjectType
{
	PROJECT_C,
	PROJECT_CPP
} CProjectType;

typedef struct Project
{
	gchar *project_name;
	gchar *project_path;
	gint project_type;
	GList *header_list;
	GList *source_list;
	GList *resource_list;
	gchar *libs;
	gchar *opts;
} CProject;

typedef enum {
	FILE_HEADER,
	FILE_SOURCE,
	FILE_RESOURCE
} CFileFold;

void
project_path_init();

CProject *
project_new(const gchar *project_name, const gchar *project_dir, const project_type);

CProject *
project_new_from_xml(const gchar *xml_file);

void
project_get_default_path (gchar *buf, gint len);

gboolean
project_create_empty (const gchar *filepath, const gchar *filename, const gint file_type);

gboolean
project_add_file (const gchar *filepath, const gchar *filename, const gchar *local_file, const gint file_type);

gboolean
project_delete_file (const gchar *filepath, const gint file_type);

gchar *
project_current_path();

gchar *
project_current_name();

void
project_get_file_lists(GList **header_list, GList **source_list, GList **resource_list);

void
project_get_settings(gchar *libs, gchar *opts);

void
project_set_settings(const gchar *libs, const gchar *opts);

gint
project_get_type();

void
project_mutex_init ();