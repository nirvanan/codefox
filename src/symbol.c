/*
 * symbol.c
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

#include "symbol.h"
#include "highlighting.h"
#include "editor.h"
#include "ui.h"

extern CWindow *window;

void 
symbol_next_token (gchar *code, gchar *token)
{
	/* Get next token in current code. */
	gint i;
	gint p;

	p = 0;
	
	for (i = 0; code[i] != 0; i++)
		if ((code[i] <= '9' && code[i] >= '0') ||
			(code[i] <= 'z' && code[i] >= 'a') ||
			(code[i] <= 'Z' && code[i] >= 'A') ||
			code[i] == '_')
			token[p++] = code[i];
		else
			break;
	
	token[p] = 0;
	
	if (p == 0)
	{
		token[0] = code[0];
		token[1] = 0;
	}
}

gboolean
symbol_parse (gpointer data)
{
	/* Get currently under-editing code and analyse struct or class data structures. */
	GSList * iterator;
	GtkTextBuffer *textbuffer = NULL;
	GtkTextIter start, end;
	CEditor *current;
	gchar *code;
	gchar *token;
	gint len;	
	
	token = g_malloc (1024);
	
	for (iterator = window->editor_list; iterator; iterator = iterator->next)
	{
		CEditor *editor;
		
		editor = (CEditor *) iterator->data;
		if (gtk_widget_is_focus (editor->textview))
		{
			textbuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor->textview));
			current = editor;
			break;
		}			
	}
	
	/* If currently no code is been editing, return! */
	if (textbuffer == NULL)
		return TRUE;
	gtk_text_buffer_get_start_iter (textbuffer, &start);
	gtk_text_buffer_get_end_iter (textbuffer, &end);
	code = gtk_text_buffer_get_text (textbuffer, &start, &end, 1);
	len = strlen (code);
	
	/* Get next token. */
	gint offset = 0;
	
	while (offset < len)
	{
		while (SPACE (code[offset]))
			offset++;
		symbol_next_token (code + offset, token);
		offset += strlen (token);
	}
	
	g_free (token);
	
	return TRUE;
}
