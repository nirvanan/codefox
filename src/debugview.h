/*
 * debugview.h
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

#ifndef DEBUGVIEW_H
#define DEBUGVIEW_H

#include <gtk/gtk.h>

typedef struct {
	GObject *localtree;
	GObject *calltree;
	GObject *watchtree;
} CDebugView;

CDebugView *
debugview_new (GObject *localtree, GObject *calltree, GObject *watchtree);

void
debugview_disable (CDebugView *debug_view);

void
debugview_enable (CDebugView *debug_view);

void
debugview_watchtree_cell_change (CDebugView *debug_view, const gchar *path_string,
								 const gchar *new_text, const gchar *value);

void
debugview_watchtree_get_all (CDebugView *debug_view, GList **list);

void
debugview_watchtree_set_all (CDebugView *debug_view, GList *list);

void
debugview_clear (CDebugView *debug_view);

void
debug_view_localtree_add (CDebugView *debug_view, const gchar *name, const gchar *value);

void
debug_view_calltree_add (CDebugView *debug_view, const gchar *frame_name, const gchar *frame_args,
						 const gchar *file_line);

#endif