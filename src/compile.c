/*
 * compile.c
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
 

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glib/gi18n-lib.h>
#include <glib.h>
#include "compile.h"
#include "env.h"
#include "limits.h"

static gchar buffer[MAX_RESULT_LENGTH + 1];
static gboolean done;
static gint offset;
static GMutex compile_mutex;

static gpointer 
compile_compile (gpointer data)
{
	FILE *pi;
	gchar *line;
	const gchar *path;
	gint NOUSE;

	if (!env_prog_exist (ENV_PROG_MAKE)) {
		g_warning ("make not found.");

		return NULL;
	}

	path = (const gchar *) data;
	fflush (stdin);
	NOUSE = chdir (path);

	line = (gchar *) g_malloc (MAX_LINE_LENGTH + 1);
	pi = popen ("make 2>&1", "r");

	if (pi == NULL) {
		g_error ("can't open pipe for make.");
		
		return NULL;
	}

	while (fgets (line, MAX_LINE_LENGTH, pi)) {
		g_mutex_lock (&compile_mutex);
		g_strlcat (buffer, line, MAX_RESULT_LENGTH);
		g_mutex_unlock (&compile_mutex);
	}

	g_mutex_lock (&compile_mutex);
	done = TRUE;
	g_mutex_unlock (&compile_mutex);

	g_free (line);

	pclose (pi);

	return NULL;
}

static gpointer 
compile_clear (gpointer data)
{
	FILE *pi;
	gchar *line;
	const gchar *path;
	gint NOUSE;

	path = (const gchar *) data;
	fflush (stdin);
	NOUSE = chdir (path);

	line = (gchar *) g_malloc (MAX_LINE_LENGTH + 1);
	pi = popen ("make clean 2>&1", "r");

	if (pi == NULL) {
		g_error ("can't open pipe for make.");
		
		return NULL;
	}

	while (fgets (line, MAX_LINE_LENGTH, pi)) {
		g_mutex_lock (&compile_mutex);

		g_strlcat (buffer, line, MAX_RESULT_LENGTH);
		g_mutex_unlock (&compile_mutex);
	}

	g_mutex_lock (&compile_mutex);
	done = TRUE;
	g_mutex_unlock (&compile_mutex);

	g_free (line);

	pclose (pi);

	return NULL;
}

gboolean
compile_done ()
{
	gboolean ret;

	g_mutex_lock (&compile_mutex);
	ret = done;
	g_mutex_unlock (&compile_mutex);

	return ret;
}

void
compile_getline (gchar *line, const gint size)
{
	gint i;

	g_mutex_lock (&compile_mutex);

	i = offset;

	if (i >= strlen (buffer)) {
		line[0] = 0;
		g_mutex_unlock (&compile_mutex);

		return;
	}

	while (i - offset < size - 1 && buffer[i] && buffer[i] != '\n') {
		line[i - offset] = buffer[i];
		i++;
	}

	line[i - offset] = 0;
	offset = i + 1;

	g_mutex_unlock (&compile_mutex);
}

void
compile_current_project (const gchar *path, const gboolean compile)
{
	buffer[0] = 0;
	done = FALSE;
	offset = 0;

	if (compile) {
		g_thread_new ("compile", compile_compile, (gpointer) path);
	}
	else {
		g_thread_new ("clear", compile_clear, (gpointer) path);
	}
}

void
compile_static_check (const gchar *filepath, const gint type, const gchar *libs, gchar *output)
{
	FILE *pi;
	gchar *line;
	gchar *command;

	fflush (stdin);

	line = (gchar *) g_malloc (MAX_LINE_LENGTH + 1);
	command = (gchar *) g_malloc (MAX_LINE_LENGTH + 1);
	g_strlcpy (command, type? "g++ -S -Wall ": "gcc -S -Wall ", MAX_LINE_LENGTH);
	if (strlen (libs) > 0) {
		g_strlcat (command, "`pkg-config --cflags ", MAX_LINE_LENGTH);
		g_strlcat (command, libs, MAX_LINE_LENGTH);
		g_strlcat (command, "` ", MAX_LINE_LENGTH);
	}
	g_strlcat (command, filepath, MAX_LINE_LENGTH);
	g_strlcat (command, " 2>&1", MAX_LINE_LENGTH);


	output[0] = 0;

	pi = popen (command, "r");

	if (pi == NULL) {
		g_error ("can't open pipe for make.");
		
		return;
	}
	while (fgets (line, MAX_LINE_LENGTH, pi))
		g_strlcat (output, line, MAX_RESULT_LENGTH);

	g_free ((gpointer) line);
	g_free ((gpointer) command);

	pclose (pi);
}

gint
compile_is_error (gchar *output)
{
	/* Check whether output is an error message. */
	gint i, j;
	gchar tag[MAX_LINE_LENGTH + 1];
	
	if (strlen (output) <= 0) {
		return 0;
	}
		
	for (i = 0; i < strlen (output) - 5; i++) {
		if (output[i] == ' ') {
			break;
		}
	}

	i++;
	for (j = 0; j < 3; j++) {
		tag[j] = output[i + j];
	}
	tag[3] = 0;

	if (g_strcmp0 (tag, _("err")) == 0 || g_strcmp0 (tag, _("fat")) == 0) {
		return 1;
	}
	
	return 0;
}

gint
compile_is_warning (gchar *output)
{
	/* check whether output is an warning message. */
	gint i, j;
	gchar tag[MAX_LINE_LENGTH + 1];
	
	if (strlen (output) <= 0)
		return 0;
		
	for (i = 0; i < strlen (output) - 5; i++) {
		if (output[i] == ' ') {
			break;
		}
	}
	
	i++;
	for (j = 0; j < 3; j++) {
		tag[j] = output[i + j];
	}
	tag[3] = 0;

	if (g_strcmp0 (tag, _("war")) == 0) {
		return 1;
	}
	
	return 0;
}

void
compile_get_location (const gchar *line, gint *row, gint *column)
{
	/* Get code location of a error or warning. */
	gint i;

	i = 0;
	while (line[i] && line[i] != ':') {
		i++;
	}
	i++;

	(*row) = 0;
	while (line[i] && line[i] != ':') {
		(*row) = (*row) * 10 + line[i] - '0';
		i++;
	}
	i++;

	(*column) = 0;
	while (line[i] && line[i] != ':') {
		(*column) = (*column) * 10 + line[i] - '0';
		i++;
	}	
}
