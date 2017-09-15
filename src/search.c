/*
 * search.c
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

#include <string.h>

#include "search.h"

static void search_kmp_scheme (const gchar *token, const gint len, gint *scheme);

static void
search_kmp_scheme (const gchar *token, const gint len, gint *scheme)
{
	gint i;
	gint j;

	scheme[0] = -1;
	j = -1;

	for (i = 1; i < len; i++) {
		while (j > -1 && token[j + 1] != token[i]) {
			j = scheme[j];
		}
		if (token[j + 1] == token[i]) {
			j++;
		}
		scheme[i] = j;
	}
}

gint
search_kmp_nth (const gchar *text, const gchar *token, const gint n)
{
	gint *scheme;
	gint len;
	gint text_len;
	gint i;
	gint j;
	gint nth;

	if (n == 0) {
		return -1;
	}

	len = strlen (token);
	text_len = strlen (text);
	scheme = (gint *) g_malloc (len * sizeof (gint));
	search_kmp_scheme (token, len, scheme);

	nth = 0;
	j = -1;
	for (i = 0; i < text_len; i++) {
		while (j > -1 && token[j + 1] != text[i]) {
			j = scheme[j];
		}

		if (token[j + 1] == text[i])
			j++;

		if (j == len - 1) {
			nth++;

			if (nth == n && n != -1) {
				g_free (scheme);

				return i - j;
			}
			j = -1;
		}
	}

	g_free ((gpointer) scheme);

	if (n != -1) {
		return -1;
	}
	else {
		return nth;
	}
}
