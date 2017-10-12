/*
 * highlighting.h
 * This file is part of codefox
 *
 * Copyright (C) 2012-2017 - Gordon Li
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef HIGHLIGHTING_H
#define HIGHLIGHTING_H

#include <gtk/gtk.h>
#include "tag.h"

#define CHAR(c) ((c >= 'a' && c <= 'z') || \
 (c >= 'A' && c <= 'Z') || c == '#' || c == '_')

#define DIGIT(c) ((c >= '0' && c <= '9') || c == '.')

#define BRACKET(c) (c == '{' || c == '}' || c == '[' || \
c == '(' || c == ')')

#define SPACE(c) (c == ' ' || c == '\n' || c == '\t')

void
highlight_register (GtkTextBuffer *buffer);

void
highlight_replace (GtkTextBuffer *buffer);

void
highlight_add_tag (GtkTextBuffer *buffer, GtkTextIter *startitr,
				   gint offset, gint len, gchar *tag);

void
highlight_apply (GtkTextBuffer *buffer, GtkTextIter *start,
				 GtkTextIter *end);

void
highlight_set_tab (GtkTextView *buffer);

gboolean 
highlight_parse (gpointer data);

#endif /* HIGHLIGHTING_H */
