/*
 * editorconfig.h
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

#ifndef EDITORCONFIG_H
#define EDITORCONFIG_H

#include <gtk/gtk.h>

typedef struct {
	GdkColor keyword_color;
	GdkColor string_color;
	GdkColor constant_color;
	GdkColor comment_color;
	GdkColor preprocessor_color;
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

#endif