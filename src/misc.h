/*
 * misc.h
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

#ifndef MISC_H
#define MISC_H

#include <gtk/gtk.h>

void
misc_open_homepage();

gint
misc_get_file_size(const gchar *filepath);

void
misc_get_file_content(const gchar *filepath, gchar **file_buf);

void
misc_set_file_content(const gchar *filepath, const gchar *file_buf);

gint
misc_get_file_name_in_path(const gchar *filepath);

void
misc_time_get_now (gchar *src);

gboolean
misc_create_file (const gchar *filepath);

gboolean
misc_copy_file (const gchar *sour, const gchar *dest);

gboolean
misc_delete_file (const gchar *filepath);

void
misc_get_line_end (const gchar *line, gint *offset);

gboolean
misc_file_exist (const gchar *filepath);

void
misc_exec_file (const gchar *filepath);

#endif /* MISC_H */
