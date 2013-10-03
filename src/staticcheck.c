/*
 * staticcheck.c
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

#include "staticcheck.h"
#include "compile.h"
#include "highlighting.h"

extern CWindow *window;

gboolean static_check (gpointer data)
{	
	/* Get current code and check for errors and warnings. */
	GSList * iterator;
	GtkTextBuffer *textbuffer = NULL;
	FILE *f;
	gchar *output;
	gchar *lin;
	gint p = 0;
	GtkTextIter start, end;
	CEditor *current;
	
	/* Find current editor. */
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
	
	/* Save curent code. */
	f = fopen ("./.static.cpp", "w");
	gtk_text_buffer_get_start_iter (textbuffer, &start);
	gtk_text_buffer_get_end_iter (textbuffer, &end);
	gtk_text_buffer_remove_tag_by_name (textbuffer, "error", &start, &end);
	fprintf (f, "%s", gtk_text_buffer_get_text (textbuffer, &start, &end, 1));
	fclose(f);
	
	/* Static check. */
	output = compile_static_check ("./.static.cpp");
	lin = g_malloc (1024);
	
	g_list_free (current->errorlinelist);
	current->errorlinelist = NULL;
	
	/* Handler static check outputs. */
	while (output[p])
	{
		gint i = 0;
		
		while (output[p] != '\n')
			lin[i++] = output[p++];
		lin[i] = 0;
		
		/* Underline errors and warnings in editor. */
		if (compile_is_error (lin) || compile_is_warning (lin))
		{
			gchar *location;
			gchar *fil;
			gint lineno, colno;
			gchar *codeline;
			gint len = 0;
			
			location = g_malloc (1024);
			fil = g_malloc (1024);
			compile_get_location (lin, location);
			sscanf(location, "%s%d%d", fil, &lineno, &colno);
			gtk_text_buffer_get_iter_at_line (textbuffer, &start, lineno - 1);
			gtk_text_buffer_get_iter_at_line (textbuffer, &end, lineno - 1);
			gtk_text_iter_forward_to_line_end (&end);
			codeline = gtk_text_buffer_get_text (textbuffer, &start, &end, 1);
			
			while (CHAR (codeline[colno - 1 + len]) ||
				   (DIGIT (codeline[colno - 1 + len]) 
				   && codeline[colno - 1 + len] != '.'))
			{
				len++;
			}
			
			if (BRACKET (codeline[colno - 1]))
				len = 1;
			
			highlight_add_tag (textbuffer, &start, colno - 1, len, "error");
			
			if (compile_is_error (lin))
				current->errorlinelist = g_list_append (current->errorlinelist, GINT_TO_POINTER (lineno));

			g_free (location);
			g_free (fil);
		}
		
		p++;
	}
	
	g_free (lin);
	g_free (output);
	
	return TRUE;
}
