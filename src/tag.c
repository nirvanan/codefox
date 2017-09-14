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
tag_create_tags (GtkTextBuffer *textbuffer, const CEditorConfig *config)
{
	gtk_text_buffer_create_tag (textbuffer, CODE_TAG_NONE,
								"font-desc", config->pfd,
								NULL);
	gtk_text_buffer_create_tag (textbuffer, CODE_TAG_PREPROCESSOR,
								"font-desc", config->pfd,
								"foreground-rgba", &(config->code_color->preprocessor_color),
								NULL);
	gtk_text_buffer_create_tag (textbuffer, CODE_TAG_KEYWORD,
								"weight", PANGO_WEIGHT_BOLD,
								"font-desc", config->pfd,
								"foreground-rgba", &(config->code_color->keyword_color),
								NULL);
	gtk_text_buffer_create_tag (textbuffer, CODE_TAG_CONSTANT,
								"font-desc", config->pfd,
								"foreground-rgba", &(config->code_color->constant_color),
								NULL);
	gtk_text_buffer_create_tag (textbuffer, CODE_TAG_STRING,
								"font-desc", config->pfd,
								"foreground-rgba", &(config->code_color->string_color),
								NULL);
	gtk_text_buffer_create_tag (textbuffer, CODE_TAG_COMMENT,
								"font-desc", config->pfd,
								"foreground-rgba", &(config->code_color->comment_color),
								NULL);
	gtk_text_buffer_create_tag (textbuffer, CODE_TAG_WARNING,
								"underline", PANGO_UNDERLINE_SINGLE,
								NULL);
	gtk_text_buffer_create_tag (textbuffer, CODE_TAG_ERROR,
								"family", "monospace",
								"underline", PANGO_UNDERLINE_ERROR,
								NULL);
}

void
tag_replace_tags (GtkTextBuffer *textbuffer, const CEditorConfig *config)
{
	GtkTextTagTable *tag_table;
	GtkTextTag *tag;

	tag_table = gtk_text_buffer_get_tag_table (textbuffer);
	tag = gtk_text_tag_table_lookup (tag_table, CODE_TAG_NONE);
	g_object_set (tag, "font-desc", config->pfd, NULL);
	tag = gtk_text_tag_table_lookup (tag_table, CODE_TAG_PREPROCESSOR);
	g_object_set (tag, "font-desc", config->pfd, "foreground-rgba", 
				  &(config->code_color->preprocessor_color), NULL);
	tag = gtk_text_tag_table_lookup (tag_table, CODE_TAG_KEYWORD);
	g_object_set (tag, "weight", PANGO_WEIGHT_BOLD, "font-desc", config->pfd,
				  "foreground-rgba", &(config->code_color->keyword_color), NULL);
	tag = gtk_text_tag_table_lookup (tag_table, CODE_TAG_CONSTANT);
	g_object_set (tag, "font-desc", config->pfd, "foreground-rgba", 
				  &(config->code_color->constant_color), NULL);
	tag = gtk_text_tag_table_lookup (tag_table, CODE_TAG_STRING);
	g_object_set (tag, "font-desc", config->pfd, "foreground-rgba",
				  &(config->code_color->string_color), NULL);
	tag = gtk_text_tag_table_lookup (tag_table, CODE_TAG_COMMENT);
	g_object_set (tag, "font-desc", config->pfd, "foreground-rgba", 
				  &(config->code_color->comment_color), NULL);
}
