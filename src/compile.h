/*
 * compile.h
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

#ifndef COMPILE_H
#define COMPILE_H

#include <gtk/gtk.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h> 

void
compile_current_project (const gchar *path, const gboolean compile);

void
compile_getline (gchar *line, const gint size);

gboolean
compile_done ();

void
compile_static_check (const gchar *filepath, const gint type, const gchar *libs, gchar *output);

gint
compile_is_error (gchar *output);

gint
compile_is_warning (gchar *output);

void
compile_get_location (const gchar *line, gint *row, gint *column);

#endif /* COMPILE_H */
