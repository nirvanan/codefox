/*
 * debug.h
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
debug_current_file_line (const gboolean startup, gchar *filename, gint *line);

void
debug_current_locals (GList **locals);

void
debug_current_stack (GList **stack);

void
debug_expression_value (const gchar *expression, gchar *value);

gpointer 
debug_monitor (gpointer data);

void
debug_stop ();

#endif