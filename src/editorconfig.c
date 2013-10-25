/*
 * editorconfig.c
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

#include "editorconfig.h"

static CEditorConfig *default_config;
static CEditorConfig *user_config;

void
editorconfig_default_config_new ()
{
	CCodeColorStyle *color_style;
	gboolean NOUSE;

	default_config = (CEditorConfig *) g_malloc (sizeof (CEditorConfig));
	default_config->code_color = (CCodeColorStyle *) g_malloc (sizeof (CCodeColorStyle));

	default_config->pfd = pango_font_description_from_string ("monospace 10");

	color_style = default_config->code_color;

	NOUSE = gdk_color_parse ("#4CA81D", &(color_style->keyword_color));
	NOUSE = gdk_color_parse ("#C05800", &(color_style->string_color));
	NOUSE = gdk_color_parse ("#A020F0", &(color_style->constant_color));
	NOUSE = gdk_color_parse ("#888376", &(color_style->comment_color));
	NOUSE = gdk_color_parse ("#BF4040", &(color_style->preprocessor_color));
}

void
editorconfig_user_config_from_default ()
{
	if (default_config == NULL)
		user_config = NULL;
	else {
		user_config = (CEditorConfig *) g_malloc (sizeof (CEditorConfig));
		user_config->pfd = default_config->pfd;
		user_config->code_color = default_config->code_color;
	}
}

const CEditorConfig *
editorconfig_config_get ()
{
	return user_config;
}

static void
editorconfig_gdkcolor_from_gdkrgba (GdkColor *color, const GdkRGBA *rgba)
{
	color->red = (guint16) (rgba->red * 65535);
	color->green = (guint16) (rgba->green * 65535);
	color->blue = (guint16) (rgba->blue * 65535);
}

void
editorconfig_config_update (const gchar *font, GdkRGBA *keyword_color,
							GdkRGBA *string_color, GdkRGBA *constant_color,
							GdkRGBA *comment_color, GdkRGBA *preprocessor_color)
{
	user_config->pfd = pango_font_description_from_string (font);
	editorconfig_gdkcolor_from_gdkrgba (&(user_config->code_color->keyword_color), keyword_color);
	editorconfig_gdkcolor_from_gdkrgba (&(user_config->code_color->string_color), string_color);
	editorconfig_gdkcolor_from_gdkrgba (&(user_config->code_color->constant_color), constant_color);
	editorconfig_gdkcolor_from_gdkrgba (&(user_config->code_color->comment_color), comment_color);
	editorconfig_gdkcolor_from_gdkrgba (&(user_config->code_color->preprocessor_color), preprocessor_color);
}