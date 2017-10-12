/*
 * editorconfig.c
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

#include "editorconfig.h"

#define DEFAULT_PLAIN_COLOR "#000000"
#define DEFAULT_KEYWORD_COLOR "#4CA81D"
#define DEFAULT_STRING_COLOR "#C05800"
#define DEFAULT_CONSTANT_COLOR "#A020F0"
#define DEFAULT_COMMENT_COLOR "#888376"
#define DEFAULT_PREPROCESSOR_COLOR "#BF4040"

static CEditorConfig *default_config;
static CEditorConfig *user_config;

void
editorconfig_default_config_new ()
{
	CCodeColorStyle *color_style;
	gboolean ret;

	default_config = (CEditorConfig *) g_malloc (sizeof (CEditorConfig));
	default_config->code_color = (CCodeColorStyle *) g_malloc0 (sizeof (CCodeColorStyle));

	default_config->pfd = pango_font_description_from_string ("monospace 10");

	color_style = default_config->code_color;

	ret = gdk_rgba_parse (&(color_style->plain_color), DEFAULT_PLAIN_COLOR);
	if (!ret) {
		g_error ("failed to parse default plain color %s.", DEFAULT_PLAIN_COLOR);
	}

	ret = gdk_rgba_parse (&(color_style->keyword_color), DEFAULT_KEYWORD_COLOR);
	if (!ret) {
		g_error ("failed to parse default keyword color %s.", DEFAULT_KEYWORD_COLOR);
	}

	ret = gdk_rgba_parse (&(color_style->string_color), DEFAULT_STRING_COLOR);
	if (!ret) {
		g_error ("failed to parse default string color %s.", DEFAULT_STRING_COLOR);
	}

	ret = gdk_rgba_parse (&(color_style->constant_color), DEFAULT_CONSTANT_COLOR);
	if (!ret) {
		g_error ("failed to parse default constant color %s.", DEFAULT_CONSTANT_COLOR);
	}

	ret = gdk_rgba_parse (&(color_style->comment_color), DEFAULT_COMMENT_COLOR);
	if (!ret) {
		g_error ("failed to parse default comment color %s.", DEFAULT_COMMENT_COLOR);
	}

	ret = gdk_rgba_parse (&(color_style->preprocessor_color), DEFAULT_PREPROCESSOR_COLOR);
	if (!ret) {
		g_error ("failed to parse default preprocessor color %s.", DEFAULT_PREPROCESSOR_COLOR);
	}
}

void
editorconfig_user_config_from_default ()
{
	if (default_config == NULL) {
		user_config = NULL;
	}
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
editorconfig_update_from_gdkrgba (GdkRGBA *color, const GdkRGBA *rgba)
{
	color->red = rgba->red;
	color->green = rgba->green;
	color->blue = rgba->blue;
}

void
editorconfig_config_update (const gchar *font, GdkRGBA *keyword_color,
							GdkRGBA *string_color, GdkRGBA *constant_color,
							GdkRGBA *comment_color, GdkRGBA *preprocessor_color)
{
	user_config->pfd = pango_font_description_from_string (font);
	editorconfig_update_from_gdkrgba (&(user_config->code_color->keyword_color), keyword_color);
	editorconfig_update_from_gdkrgba (&(user_config->code_color->string_color), string_color);
	editorconfig_update_from_gdkrgba (&(user_config->code_color->constant_color), constant_color);
	editorconfig_update_from_gdkrgba (&(user_config->code_color->comment_color), comment_color);
	editorconfig_update_from_gdkrgba (&(user_config->code_color->preprocessor_color), preprocessor_color);
}
