/*
 * editorconfig.h
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

#ifndef EDITORCONFIG_H
#define EDITORCONFIG_H

#include <gtk/gtk.h>

typedef struct {
	GdkRGBA plain_color;
	GdkRGBA keyword_color;
	GdkRGBA string_color;
	GdkRGBA constant_color;
	GdkRGBA comment_color;
	GdkRGBA preprocessor_color;
} CCodeColorStyle;

typedef struct {
	PangoFontDescription *pfd;
	CCodeColorStyle *code_color;
} CEditorConfig;

void
editorconfig_default_config_new ();

void
editorconfig_user_config_from_default ();

const CEditorConfig *
editorconfig_config_get();

void
editorconfig_config_update (const gchar *font, GdkRGBA *keyword_color,
							GdkRGBA *string_color, GdkRGBA *constant_color,
							GdkRGBA *comment_color, GdkRGBA *preprocessor_color);

#endif /* EDITORCONFIG_H */
