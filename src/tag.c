/*
 * tag.c
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

#include "tag.h"

void
tag_create_tag (GtkTextBuffer *textbuffer, CTag style)
{
	/* Create a user defined tag and add it to textbuffer. */
	switch (style)
	{
		/* Tag to highlighting ordinary symbols. */
		case CODE_NONE:
			gtk_text_buffer_create_tag (textbuffer, "none",
										"family", "monospace",
										NULL);
			break;

		/* Tag to highlighting pre-processor symbols. */
		case CODE_PREPORC:
			gtk_text_buffer_create_tag (textbuffer, "preproc",
										"foreground", "#559E55",
										"family", "monospace",
										NULL);
			break;

		/* Tag to highlighting C/C++ keywords. */
		case CODE_KEYWORD:
			gtk_text_buffer_create_tag (textbuffer, "keyword",
										"weight", PANGO_WEIGHT_BOLD,
										"foreground", "blue",
										"family", "monospace",
										NULL);
			break;

		/* Tag to highlighting constant symbols. */
		case CODE_CONSTANT:
			gtk_text_buffer_create_tag (textbuffer, "constant",
										"foreground", "#A020F0",
										"family", "monospace",
										NULL);
			break;

		/* Tag to highlighting strings. */
		case CODE_STRING:
			gtk_text_buffer_create_tag (textbuffer, "string",
										"foreground", "#C05800",
										"family", "monospace",
										NULL);
			break;

		/* Tag to highlighting comments. */
		case CODE_COMMENT:
			gtk_text_buffer_create_tag (textbuffer, "comment",
										"foreground", "#888376",
										"family", "monospace",
										NULL);
			break;

		/* Tag to underline warnings in code. */
		case INFO_WARNING:
			gtk_text_buffer_create_tag (textbuffer, "warning",
										"underline", PANGO_UNDERLINE_SINGLE,
										NULL);
			break;

		/* Tag to underline errors in code. */
		case INFO_ERROR: 
			gtk_text_buffer_create_tag (textbuffer, "error",
										"family", "monospace",
										"underline", PANGO_UNDERLINE_ERROR,
										NULL);
			break;
	}
	
}
