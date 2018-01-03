/*
 * misc.c
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

#include <gtk/gtk.h>
#include <glib/gstdio.h>
#include <stdlib.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glib/gi18n-lib.h>

#include "misc.h"
#include "env.h"
#include "limits.h"

/* This script shall open default browser in GNOME desktop. */
#define BROWSER_PATH "/etc/alternatives/gnome-www-browser"

#define HOME_PAGE "https://github.com/nirvanan/codefox"

/* Open homepage using default browser. */
/* FIXME: find default browser. */
void
misc_open_homepage()
{
	gchar command[MAX_COMMAND_LENGTH + 1];
	gint ret;

	if (!misc_file_exist (BROWSER_PATH)) {
		g_warning ("can't find default browser.");

		return;
	}

	g_snprintf (command, MAX_COMMAND_LENGTH, "%s %s &", BROWSER_PATH, HOME_PAGE);

	ret = system(command);
	if (ret != 0) {
		g_warning ("opening default browser return %d.", ret);
	}
}

/* Get filesize. */
/* TODO: use glib instead of standard C lib. */
gint
misc_get_file_size(const gchar *filepath)
{
	gint filesize = -1;  
	FILE *fp;

	fp = g_fopen(filepath, "r");
	if (fp == NULL) {
		return filesize;
	}
	fseek(fp, 0, SEEK_END);  
	filesize = ftell(fp);
	fclose(fp);

	return filesize;
}

/* Load code file. */
void
misc_get_file_content(const gchar *filepath, gchar **file_buf)
{
	gboolean suc;

	suc = g_file_get_contents (filepath, file_buf, NULL, NULL);

	if (!suc) {
		g_error ("failed to load file %s.", filepath);
	}
}

/* Save code file. */
void
misc_set_file_content(const gchar *filepath, const gchar *file_buf)
{
	gboolean suc;

	suc = g_file_set_contents (filepath, file_buf, strlen (file_buf), NULL);

	if (!suc) {
		g_error ("failed to set file content %s.", filepath);
	}
}

/* Get filename offset in filepath. */
gint
misc_get_file_name_in_path(const gchar *filepath)
{
	gint i, len;
	
	/* FIXME: use glib! */
	len = strlen (filepath);
	for (i = len - 1; filepath[i] != '/'; i--);

	return i;
}

/* Get current time and copy to src. */
void 
misc_time_get_now (gchar *src)
{
	GDateTime *time;
	gint hr, mi, se;
	
	time = g_date_time_new_now_local ();
	hr = g_date_time_get_hour (time);
	mi = g_date_time_get_minute (time);
	se = g_date_time_get_second (time);
	
	g_snprintf (src, MAX_TIME_LENGTH, "%.2d:%.2d:%.2d: ", hr, mi, se);

	g_date_time_unref (time);
}

/* Touch. */
gboolean
misc_create_file (const gchar *filepath)
{
	GFileOutputStream *noused;
	GFile *file;

	file = g_file_new_for_path (filepath);
	noused = g_file_create (file, G_FILE_CREATE_NONE, NULL, NULL);

	g_object_unref(file);

	return noused != NULL;
}

/* Copy file. */
gboolean
misc_copy_file (const gchar *sour, const gchar *dest)
{
	gboolean suc;
	GFile *sour_file;
	GFile *dest_file;

	sour_file = g_file_new_for_path (sour);
	dest_file = g_file_new_for_path (dest);
	suc = g_file_copy (sour_file, dest_file, G_FILE_COPY_NONE, NULL, NULL, NULL, NULL);

	g_object_unref(sour_file);
	g_object_unref(dest_file);

	return suc;
}

/* Delete file. */
gboolean
misc_delete_file (const gchar *filepath)
{
	gboolean suc;
	GFile *file;

	file = g_file_new_for_path (filepath);
	suc = g_file_delete (file, NULL, NULL);

	g_object_unref(file);

	return suc;
}

void
misc_get_line_end (const gchar *line, gint *offset)
{
	while (line[*offset] != '\n' && line[*offset] != '\0') {
		(*offset)++;
	}
}

gboolean
misc_file_exist (const gchar *filepath)
{
	return filepath && strlen(filepath) && g_file_test(filepath, G_FILE_TEST_EXISTS);
}

void
misc_exec_file (const gchar *filepath)
{
	gchar command[MAX_COMMAND_LENGTH + 1];
	gint ret;

	if (!env_prog_exist (ENV_PROG_XTERM)) {
		g_warning ("xterm not found.");

		return;
	}

	g_snprintf (command, MAX_COMMAND_LENGTH, "xterm -e %s", filepath);

	ret = system (command);
	if (ret != 0) {
		g_warning ("exec returned %d.", ret);
	}
}

