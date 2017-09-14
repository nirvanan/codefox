/*
 * edithistory.c
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

#include "edithistory.h"
#include "callback.h"
#include "highlighting.h"

#define MAX_STEP 20
#define COOLDOWN_TIME 2000000

static CEditStep * edit_step_new (const gboolean insert, const gint offset, const gint len,
								  const gchar *text);
static void edit_step_free (CEditStep *edit_step);
static void edit_history_rehighlighting (GtkTextBuffer *buffer, const gint offset, const gint len);


CEditHistory *
edit_history_new ()
{
	CEditHistory *edit_history;

	edit_history = (CEditHistory *) g_malloc (sizeof (CEditHistory));
	edit_history->step_list = NULL;
	edit_history->undo_list = NULL;

	return edit_history;
}

static CEditStep *
edit_step_new (const gboolean insert, const gint offset, const gint len,
			   const gchar *text)
{
	CEditStep *edit_step;

	edit_step = (CEditStep *) g_malloc (sizeof (CEditStep));
	if (insert) {
		edit_step->insert = TRUE;
		edit_step->offset = offset;
		edit_step->len = len;
		edit_step->deleted = NULL;
	}
	else {
		gint deleted_len;

		deleted_len = strlen (text) + 1;
		edit_step->insert = FALSE;
		edit_step->offset = offset;
		edit_step->len = -1;
		edit_step->deleted = (gchar *) g_malloc (deleted_len);
		g_strlcpy (edit_step->deleted, text, deleted_len);
	}

	edit_step->add_time = g_date_time_new_now_local ();

	return edit_step;
}

static void
edit_step_free (CEditStep *edit_step)
{
	if (!edit_step->insert)
		g_free ((gpointer) edit_step->deleted);
	g_date_time_unref (edit_step->add_time);
	g_free ((gpointer) edit_step);
}

void
edit_history_step_add (CEditHistory *edit_history, const gboolean insert,
					   const gint offset, const gint len, const gchar *text)
{
	CEditStep *edit_step;
	GList *iterator;
	GDateTime *add_time;

	if (g_list_length (edit_history->step_list) > 0) {
		iterator = g_list_last (edit_history->step_list);
		edit_step = (CEditStep *) iterator->data;

		add_time = g_date_time_new_now_local ();

		if (g_date_time_difference (add_time, edit_step->add_time) <= COOLDOWN_TIME) {
			if (edit_step->insert && edit_step->insert == insert &&
				edit_step->offset + edit_step->len == offset) {
				edit_step->len += len;

				return ;
			}
			else if (!edit_step->insert && edit_step->insert == insert &&
				offset + strlen (text) == edit_step->offset) {
				gchar *new_text;
				gint new_text_len;

				new_text_len = strlen (text) + strlen (edit_step->deleted);
				new_text = (gchar *) g_malloc (new_text_len + 1);
				g_strlcpy (new_text, text, new_text_len + 1);
				g_strlcat (new_text, edit_step->deleted, new_text_len + 1);
				g_free ((gpointer) edit_step->deleted);
				edit_step->deleted = new_text;
				edit_step->offset = offset;

				return ;
			}
		}
	}

	edit_step = edit_step_new (insert, offset, len, text);
	g_list_free_full (edit_history->undo_list, (GDestroyNotify) edit_step_free);
	edit_history->undo_list = NULL;
	edit_history->step_list = g_list_append (edit_history->step_list, (gpointer) edit_step);

	if (g_list_length (edit_history->step_list) > MAX_STEP) {
		CEditStep *edit_step;

		iterator = edit_history->step_list;
		edit_step = (CEditStep *) iterator->data;
		edit_history->step_list = g_list_remove (edit_history->step_list, iterator->data);

		edit_step_free (edit_step);
	}
}

static void
edit_history_rehighlighting (GtkTextBuffer *buffer, const gint offset, const gint len)
{
	GtkTextIter start;
	GtkTextIter end;
	gint line;

	gtk_text_buffer_get_iter_at_offset (buffer, &start, offset);
	gtk_text_buffer_get_iter_at_offset (buffer, &end, offset + len);
	gtk_text_iter_forward_to_line_end (&end);
	line = gtk_text_iter_get_line (&start);
	gtk_text_buffer_get_iter_at_line (buffer, &start, line);

	highlight_apply (buffer, &start, &end);
}

void
edit_history_action (CEditHistory *edit_history, GtkTextBuffer *buffer, const gboolean undo)
{
	GtkTextIter start;
	GtkTextIter end;
	CEditStep *edit_step;
	CEditStep *alt_step;
	GList *iterator;

	if (undo)
		iterator = g_list_last (edit_history->step_list);
	else
		iterator = g_list_last (edit_history->undo_list);
	edit_step = (CEditStep *) iterator->data;

	if (edit_step->insert) {
		const gchar *text;

		gtk_text_buffer_get_iter_at_offset (buffer, &start, edit_step->offset);
		gtk_text_buffer_get_iter_at_offset (buffer, &end, edit_step->offset + edit_step->len);

		text = gtk_text_buffer_get_text (buffer, &start, &end, TRUE);
		alt_step = edit_step_new (FALSE, edit_step->offset, -1, text);

		//g_signal_handlers_block_by_func (buffer, on_editor_delete2, NULL);
		//g_signal_handlers_block_by_func (buffer, on_editor_delete, NULL);
		gtk_text_buffer_delete (buffer, &start, &end);
		//g_signal_handlers_unblock_by_func (buffer, on_editor_delete2, NULL);
		//g_signal_handlers_unblock_by_func (buffer, on_editor_delete, NULL);
		//edit_history_rehighlighting (buffer, edit_step->offset, 0);
		
		g_free ((gpointer) text);
	}
	else {
		alt_step = edit_step_new (TRUE, edit_step->offset, strlen (edit_step->deleted), NULL);

		gtk_text_buffer_get_iter_at_offset (buffer, &start, edit_step->offset);

		//g_signal_handlers_block_by_func (buffer, on_editor_insert, NULL);
		gtk_text_buffer_insert (buffer, &start, edit_step->deleted, -1);
		//g_signal_handlers_unblock_by_func (buffer, on_editor_insert, NULL);
		//edit_history_rehighlighting (buffer, edit_step->offset, strlen (edit_step->deleted));
	}

	if (undo) {
		edit_history->step_list = g_list_remove (edit_history->step_list, (gpointer) edit_step);
		edit_step_free (edit_step);
		edit_history->undo_list = g_list_append (edit_history->undo_list, (gpointer) alt_step);
	}
	else {
		edit_history->undo_list = g_list_remove (edit_history->undo_list, (gpointer) edit_step);
		edit_step_free (edit_step);
		edit_history->step_list = g_list_append (edit_history->step_list, (gpointer) alt_step);
	}
}

gboolean
edit_history_can_undo (CEditHistory *edit_history)
{
	return g_list_length (edit_history->step_list) > 0;
}

gboolean
edit_history_can_redo (CEditHistory *edit_history)
{
	return g_list_length (edit_history->undo_list) > 0;
}
