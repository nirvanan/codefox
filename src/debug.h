/*
 * debug.h
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

#ifndef DEBUG_H
#define DEBUG_H

#include <gtk/gtk.h>

typedef struct {
	gchar *filepath;
	gint line;
} CBreakPoint;

void
debug_startup (const gchar *project_path, const gchar *project_name);

void
debug_command_exec (const gchar *command, const gchar *para, gchar *output);

void
debug_breakpoints_insert (GList *list);

void
debug_breakpoint_update (gchar *breakpoint_desc);

gboolean
debug_is_active ();

void
debug_connect (const gchar *project_path, const gchar *project_name);

void
debug_current_file_line (const gboolean startup, gchar *filename, const gint size, gint *line);

void
debug_current_locals (GList **locals);

void
debug_current_stack (GList **stack);

void
debug_expression_value (const gchar *expression, gchar *value, const gint size);

void
debug_stop ();

gboolean 
debug_monitor (gpointer data);

#endif /* DEBUG_H */
