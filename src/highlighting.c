/*
 * highlighting.c
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
#include "highlighting.h"
#include "keywords.h"

void 
highlight_register (GtkTextBuffer *buffer)
{  
	/* Register needed styles to a GtkTextBuffer. */
	tag_create_tag (buffer, CODE_NONE);
	tag_create_tag (buffer, CODE_PREPORC);
	tag_create_tag (buffer, CODE_KEYWORD);
	tag_create_tag (buffer, CODE_CONSTANT);
	tag_create_tag (buffer, CODE_STRING);
	tag_create_tag (buffer, CODE_COMMENT);
	tag_create_tag (buffer, INFO_WARNING);
	tag_create_tag (buffer, INFO_ERROR);
}

static gboolean
highlight_is_keyword (gchar *word)
{
	/* Check whether word is a C/C++ keyword. */
	gint i;
	
	for (i = 0; keywords[i]; i++)
	{
		gchar *keyword;
		
		keyword = keywords[i];
		
		if (g_strcmp0 (keyword, word) == 0)
			return 1;
	}
	
	return 0;
}

void
highlight_add_tag (GtkTextBuffer *buffer, GtkTextIter *startitr,
				   gint offset, gint len, gchar *tag)
{
	/* Apply highlighting. */
	gint start_offset;
	gint end_offset;
	GtkTextIter start, end;
	
	start_offset = gtk_text_iter_get_offset (startitr) + offset;
	end_offset = start_offset + len;
	gtk_text_buffer_get_iter_at_offset (buffer, &start, start_offset);
	gtk_text_buffer_get_iter_at_offset (buffer, &end, end_offset);
	/* If needed tag is a error style, we shouldn't delete other tags. */
	if (g_strcmp0 (tag, "error") != 0) 
	{
		gtk_text_buffer_remove_tag_by_name (buffer, "none", &start, &end);
		gtk_text_buffer_remove_tag_by_name (buffer, "preproc", &start, &end);
		gtk_text_buffer_remove_tag_by_name (buffer, "keyword", &start, &end);
		gtk_text_buffer_remove_tag_by_name (buffer, "constant", &start, &end);
		gtk_text_buffer_remove_tag_by_name (buffer, "string", &start, &end);
		gtk_text_buffer_remove_tag_by_name (buffer, "comment", &start, &end);
	}
	gtk_text_buffer_apply_tag_by_name (buffer, tag, &start, &end);
}

void
highlight_apply (GtkTextBuffer *buffer, GtkTextIter *start,
				 GtkTextIter *end)
{
	/* Use a finite state machine to highlighting a code. */
	gchar *text;	
	gint i;
	gchar *lex;
	gint lex_len;
	gint state;
	GtkTextIter st, ed;
	
	text = gtk_text_iter_get_text (start, end);
	lex = g_malloc (1024);
	lex_len = 0;
	state = 0;

	for (i = 0; text[i]; i++)
	{
		highlight_add_tag (buffer, start, i, 1, "none");
		
		if ((!CHAR (text[i]) && !DIGIT (text[i])) || text[i + 1] == 0)
		{			
			gint start_offset;
			gint tag;
			
			start_offset = i - lex_len;
			tag = CODE_NONE;

			switch (state)
			{
				case 0:
					if (text[i] == '\"')
					{
						state = 1;
						lex[lex_len++] = text[i];
					}
					else if (text[i] == '\'')
					{
						state = 2;
						lex[lex_len++] = text[i];
					}
					else if (text[i] == '/')
					{
						state = 3;
						lex[lex_len++] = text[i];
					}
					else if (text[i + 1] == 0 && (CHAR (text[i]) || 
							 DIGIT (text[i])))
						lex[lex_len++] = text[i];
					break;
					
				case 1:
					lex[lex_len++] = text[i];
					if (text[i] == '\"')
					{
						int s = 0;
						int j = lex_len - 2;
						
						for ( ; j >= 0; j--)
							if (lex[j] == '\\')
								s++;
							else
								break;
						
						if (s % 2 == 0) {
							state = 0;
							tag = CODE_STRING;
						}
					}
					break;
					
				case 2:
					lex[lex_len++] = text[i];
					if (text[i] == '\'')
					{
						int s = 0;
						int j = lex_len - 2;
						
						for ( ; j >= 0; j--)
							if (lex[j] == '\\')
								s++;
							else
								break;
						
						if (s % 2 == 0) {
							state = 0;
							tag = CODE_STRING;
						}
					}
					break;
				
				case 3:
					lex[lex_len++] = text[i];
					if (text[i] == '/')
						state = 4;
					else if (text[i] == '*')
						state = 5;
					else {
						state = 0;
						tag = CODE_NONE;
					}
					break;
				
				case 4:
					lex[lex_len++] = text[i];
					if (text[i] == '\n') {
						state = 0;
						tag = CODE_COMMENT;
					}
					break;
					
				case 5:
					lex[lex_len++] = text[i];
					if (text[i] == '*')
						state = 6;
					break;
					
				case 6:
					lex[lex_len++] = text[i];
					if (text[i] == '/' && text[i - 1] == '*') {
						state = 0;
						tag = CODE_COMMENT;
					}
					break;
			}
			
			lex[lex_len] = 0;
			
			if (state != 0)
				continue;
			
			if (tag == CODE_NONE) {
				if (DIGIT (lex[0]))
					tag = CODE_CONSTANT;
				else if (lex[0] == '#')
					tag = CODE_PREPORC;
				else if (CHAR (lex[0]) && lex_len <= MAX_KEYWORD_LENTH && highlight_is_keyword (lex))
					tag = CODE_KEYWORD;
			}

			/* Do highlighting. */
			
			switch (tag)
			{
				case CODE_NONE:
					break;
				case CODE_PREPORC:
					highlight_add_tag (buffer, start, start_offset, lex_len,
									   "preproc");
					break;
				case CODE_KEYWORD:
					highlight_add_tag (buffer, start, start_offset, lex_len,
									   "keyword");
					break;
				case CODE_CONSTANT:
					highlight_add_tag (buffer, start, start_offset, lex_len,
									   "constant");
					break;
				case CODE_STRING:
					highlight_add_tag (buffer, start, start_offset, lex_len,
									   "string");
					break;
				case CODE_COMMENT:
					highlight_add_tag (buffer, start, start_offset, lex_len,
									   "comment");
					break;
				default:
					break;
			}
			
			lex_len = 0;
		}
		else
			lex[lex_len++] = text[i];
		
		
	}
	
	g_free (lex);
}

