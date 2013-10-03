/*
 * highlighting.h
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
highlight_add_tag (GtkTextBuffer *buffer, GtkTextIter *startitr,
				   gint offset, gint len, gchar *tag);

void
highlight_apply (GtkTextBuffer *buffer, GtkTextIter *start,
				 GtkTextIter *end);

#endif
