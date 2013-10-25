/*
 * edithistory.h
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

#ifndef EDITHISTORY_H
#define EDITHISTORY_H

#include <gtk/gtk.h>

typedef struct {
	GList *step_list;
	GList *undo_list;
} CEditHistory;

typedef struct {
	gboolean insert;
	gint offset;
	gint len;
	gchar *deleted;
	GDateTime *add_time;
} CEditStep;

CEditHistory *
edit_history_new ();

void
edit_history_step_add (CEditHistory *edit_history, const gboolean insert,
					   const gint offset, const gint len, const gchar *text);
void
edit_history_action (CEditHistory *edit_history, GtkTextBuffer *buffer, const gboolean undo);

gboolean
edit_history_can_undo (CEditHistory *edit_history);

gboolean
edit_history_can_redo (CEditHistory *edit_history);

#endif