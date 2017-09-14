/*
 * autoindent.c
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
#include <string.h>
#include "autoindent.h"
#include "callback.h"

#define IF(c) \
(c[0] == 'i' && c[1] == 'f' && \
(c[2] == ' ' || c[2] == '(' || c[2] == '\n' || \
c[2] == 0))

#define ELSE(c) \
(c[0] == 'e' && c[1] == 'l' && \
c[2] == 's' && c[3] == 'e' && \
(c[4] == ' ' || c[4] == '(' || c[4] == '\n' || \
c[4] == 0))

#define CASE(c) \
(c[0] == 'c' && c[1] == 'a' && \
c[2] == 's' && c[3] == 'e' && \
(c[4] == ' ' || c[4] == '(' || c[4] == '\n' || \
c[4] == 0))

#define FOR(c) \
(c[0] == 'f' && c[1] == 'o' && \
c[2] == 'r' && \
(c[3] == ' ' || c[3] == '(' || c[3] == '\n' || c[3] == 0))

#define WHILE(c) \
(c[0] == 'w' && c[1] == 'h' && \
c[2] == 'i' && c[3] == 'l' && c[4] == 'e' && \
(c[5] == ' ' || c[5] == '(' || c[5] == '\n' || c[5] == 0))

#define NEST(c) \
(FOR (c) || IF (c) || ELSE (c) || WHILE (c) || CASE (c))

#define MAX_BLOCK_SIZE 1000

gint block[MAX_BLOCK_SIZE];
gint block_size;

static gint
autoindent_bracket_match (GtkTextBuffer *buffer, gint line, gint offset)
{
	GtkTextIter location, start;
	gchar *text;
	gint matched_line = line;
	gint stack = 1;
	gint len;
	gint tabs = 0;
	gint find = 0;
	gint i;
	
	gtk_text_buffer_get_iter_at_line_offset (buffer, &location,
											 line, offset);
	gtk_text_buffer_get_start_iter (buffer, &start);
	text = gtk_text_buffer_get_text (buffer, &start, &location, 1);
	
	len = strlen (text);
	for (i = len - 1; i >= 0; i--) {
		if (text[i] == '{' && text[i + 1] != '\'') {
			stack--;
		}
		else if (text[i] == '}' && text[i + 1] != '\'') {
			stack++;
		}
		else if (text[i] == '\n') {
			matched_line--;
		}
		
		if (stack == 0) {
			find = 1;

			break;
		}
	}
	
	for ( ; i >= 0 && text[i] != '\n'; i--);
	for (i = i + 1; text[i] == '\t'; i++, tabs++);
	
	g_free ((gpointer) text);

	if (find) {
		return tabs;
	}
	else {
		return -1;
	}
}

static gint
autoindent_if_else_match (GtkTextBuffer *buffer, const gint line, const gint offset)
{
	GtkTextIter location, start;
	gchar *text;
	gint matched_line = line;
	gint stack = 1;
	gint len;
	gint tabs = 0;
	gint find = 0;
	gint i;
	
	gtk_text_buffer_get_iter_at_line_offset (buffer, &location, 
											 line, offset);
	gtk_text_buffer_get_start_iter (buffer, &start);
	text = gtk_text_buffer_get_text (buffer, &start, &location, 1);
	
	len = strlen (text);
	for (i = len - 1; i >= 0; i--) {
		if (IF (((gchar *)text + i))) {
			stack--;
		}
		else if (ELSE (((gchar *)text + i))) {
			stack++;
		}
		else if (text[i] == '\n') {
			matched_line--;
		}
		
		if (stack == 0) {
			find = 1;

			break;
		}
	}
	
	for ( ; i >= 0 && text[i] != '\n'; i--);
	for (i = i + 1; text[i] == '\t'; i++, tabs++);
	
	g_free ((gpointer) text);

	if (find) {
		return tabs;
	}
	else {
		return -1;
	}
}

static gint
autoindent_get_nspaces (const gchar *line1, const gchar *line2)
{
	gint i;
	gint num = 0;
	
	for (i = 0; line1[i]; i++) {
		if (line1[i] == ' ') {
			num++;
		}
		else if (line1[i] != '\t') {
			break;
		}
	}
	for (i = 0; line2[i]; i++) {
		if (line2[i] == ' ') {
			num--;
		}
		else if (line2[i] != '\t') {
			break;
		}
	}
			
	return num;
}

static gint
autoindent_get_ntabs (GtkTextBuffer *buffer, const gchar *line1,
					  const gchar* line2, const gint line, const gint format)
{
	gint num1 = 0, num2 = 0;
	gint i, j;
	gint matched_tabs;
	gint ntabs = 0;
	
	for (i = 0; line1[i]; i++) {
		if (line1[i] == '\t') {
			num1++;
		}
		else if(line1[i] != ' ') {
			break;
		}
	}
	for (j = 0; line2[j]; j++) {
		if (line2[j] == '\t') {
			num2++;
		}
		else if(line2[j] != ' ') {
			break;
		}
	}
	
	if (format) {
		gint len = strlen (line2);
		
		if ((line2[j] == '{' && line2[j + 1] != '\'') ||
			(line2[len - 1] == '{')) {
			if (block_size >= MAX_BLOCK_SIZE) {
				g_warning ("too many nested blocks.");

				return 0;
			}
			block[block_size++] = j;
		}
		else if (line2[j] == '}' && line2[j + 1] != '\'') {
			if (block_size <= 0) {
				return 0;
			}
			block_size--;
		}
		else if (!NEST (((gchar *)line1 + i)) && !NEST (((gchar *)line1 + i)) &&
				 line1[i] != '{' && line1[i + 1] != '\'' && line2[j] != '}' &&
				 line2[j + 1] != '\'') {
			return block[block_size - 1]  + 1 - num2;
		}
	}
	if (line2[j] == '}' && line2[j + 1] != '\'') {
		matched_tabs = autoindent_bracket_match (buffer, line, j);
		if (matched_tabs != -1) {
			return matched_tabs - num2;
		}
	}
	
	if (ELSE (((gchar *)line2 + j))) {
		matched_tabs = autoindent_if_else_match (buffer, line, j);
		
		if (matched_tabs != -1) {
			return matched_tabs - num2;
		}
	}
	
	if (line1[i] == '{' && line2[j] != '}' && line1[i + 1] != '\'' &&
		line2[j + 1] != '\'') {
		num1++;
	}
		
	else if (NEST (((gchar *)line1 + i)) && line2[j] != '{' && line2[j + 1] != '\'') {
		num1++;
	}

	ntabs += num1 - num2;

	return ntabs;
}

void
autoindent_apply (GtkTextBuffer *buffer, GtkTextIter *iter,
				  const gint start, const gint end)
{	
	gint i, j;
	
	block_size = 0;
	block[block_size++] = -1;
	
	for (i = (start == 0? 1: start); i <= end; i++) {
		GtkTextIter ins, ine;
		GtkTextIter st, ed;
		gchar *line1, *line2;
		gint ntabs;
		gint nspaces;
		gchar *space;

		gtk_text_buffer_get_iter_at_line (buffer, &ins, i);
		gtk_text_buffer_get_iter_at_line (buffer, &ine, i);
		gtk_text_iter_forward_to_line_end (&ine);
		gtk_text_buffer_get_iter_at_line (buffer, &st, i - 1);
		gtk_text_buffer_get_iter_at_line (buffer, &ed, i - 1);
		gtk_text_iter_forward_to_line_end (&ed);
		line1 = gtk_text_buffer_get_text (buffer, &st, &ed, 1);
		line2 = gtk_text_buffer_get_text (buffer, &ins, &ine, 1);

		if (end != start) {
			ntabs = autoindent_get_ntabs (buffer, line1, line2, i, 1);
		}
		else {
			ntabs = autoindent_get_ntabs (buffer, line1, line2, i, 0);
		}
		
		if (ntabs > 0) {
			for (j = 0; j < ntabs; j++) {
				if (iter != NULL) {
					gtk_text_buffer_insert (buffer, iter, "\t", -1);
				}
				else {
					gtk_text_buffer_insert (buffer, &ins, "\t", -1);
				}
			}
		}
		else if (ntabs < 0) {
			if (iter != NULL) {
				gtk_text_iter_set_line_offset (iter, -ntabs);
				gtk_text_buffer_delete (buffer, &ins, iter);
			}
			else {
				gint offset;
				
				gtk_text_buffer_get_iter_at_line (buffer, &ine, i);
				offset = gtk_text_iter_get_offset (&ins);
				gtk_text_iter_set_offset (&ine, offset - ntabs);
				gtk_text_buffer_delete (buffer, &ins, &ine);
			}
		}
		
		nspaces = autoindent_get_nspaces (line1, line2);
		j = 0;
		while (line2[j++] == '\t');
		j += ntabs - 1;
		if (iter != NULL) {
			if (nspaces > 0) {
				space = g_strnfill (nspaces, ' ');
				gtk_text_iter_set_line_offset (iter, j);
				gtk_text_buffer_insert (buffer, iter, space, -1);
			}
			else if (nspaces < 0) {
				gtk_text_iter_set_line_offset (&ins, j);
				gtk_text_iter_set_line_offset (iter, j - nspaces);
				gtk_text_buffer_delete (buffer, &ins, iter);
			}
		}
		else {
			if (nspaces > 0) {
				space = g_strnfill (nspaces, ' ');
				gtk_text_iter_set_line_offset (&ins, j);
				gtk_text_buffer_insert (buffer, &ins, space, -1);
			}
			else if (nspaces < 0) {
				gtk_text_iter_set_line_offset (&ins, j);
				gtk_text_buffer_get_iter_at_line_index (buffer, &ine, i, 
														j - nspaces);
				gtk_text_buffer_delete (buffer, &ins, &ine);
			}
		}

		g_free (line1);
		g_free (line2);
	}
}
