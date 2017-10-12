/*
 * tag.h
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

#ifndef TAG_H
#define TAG_H

#include <gtk/gtk.h>

#include "editorconfig.h"

#define CODE_TAG_NONE "none"
#define CODE_TAG_PREPROCESSOR "preprocessor"
#define CODE_TAG_KEYWORD "keyword"
#define CODE_TAG_CONSTANT "constant"
#define CODE_TAG_STRING "string"
#define CODE_TAG_COMMENT "comment"
#define CODE_TAG_WARNING "warning"
#define CODE_TAG_ERROR "error"

void
tag_create_tags (GtkTextBuffer *textbuffer, const CEditorConfig *config);

void
tag_replace_tags (GtkTextBuffer *textbuffer, const CEditorConfig *config);

#endif /* TAG_H */
