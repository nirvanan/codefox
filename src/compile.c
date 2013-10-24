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
 
#include "compile.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glib/gi18n-lib.h>
#include <glib.h>

#define MAX_LINE_LENTH 1000
#define MAX_RESULT_LENTH 100000

static gchar *buffer;
static gboolean done;
static gint offset;
static GMutex compile_mutex;

static gpointer 
compile_compile (gpointer data)
{
	FILE *pi;
	gchar *line;
	const gchar *path;

	path = (const gchar *) data;
	fflush (stdin);
	chdir (path);

	line = (gchar *) g_malloc (MAX_LINE_LENTH);
	pi = popen ("make 2>&1", "r");

	while (fgets (line, MAX_LINE_LENTH, pi)) {
		g_mutex_lock (&compile_mutex);

		g_strlcat (buffer, line, MAX_RESULT_LENTH);
		g_mutex_unlock (&compile_mutex);
	}

	g_mutex_lock (&compile_mutex);
	done = TRUE;
	g_mutex_unlock (&compile_mutex);

	g_free (line);
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
compile_getline (gchar *line)
{
	gint i;

	g_mutex_lock (&compile_mutex);

	i = offset;

	if (i >= strlen (buffer)) {
		line[0] = 0;
		g_mutex_unlock (&compile_mutex);

		return ;
	}

	while (buffer[i] && buffer[i] != '\n') {
		line[i - offset] = buffer[i];
		i++;
	}

	line[i - offset] = 0;
	offset = i + 1;

	g_mutex_unlock (&compile_mutex);
}

void
compile_current_project (const gchar *path)
{
	buffer = (gchar *) g_malloc (MAX_RESULT_LENTH);
	buffer[0] = 0;
	done = FALSE;
	offset = 0;

	g_thread_new ("compile", compile_compile, path);

	g_free (buffer);
}

void
compile_static_check (const gchar *filepath, const gint type, const gchar *libs, gchar *output)
{
	FILE *pi;
	gchar *line;
	gchar *command;

	fflush (stdin);

	line = (gchar *) g_malloc (MAX_LINE_LENTH);
	command = (gchar *) g_malloc (MAX_LINE_LENTH);
	g_strlcpy (command, type? "g++ -S ": "gcc -S ", MAX_LINE_LENTH);
	g_strlcat (command, "`pkg-config --cflags ", MAX_LINE_LENTH);
	g_strlcat (command, libs, MAX_LINE_LENTH);
	g_strlcat (command, "` ", MAX_LINE_LENTH);
	g_strlcat (command, filepath, MAX_LINE_LENTH);
	g_strlcat (command, " 2>&1", MAX_LINE_LENTH);

	pi = popen (command, "r");
	output[0] = 0;
	while (fgets (line, MAX_RESULT_LENTH, pi))
		g_strlcat (output, line, MAX_RESULT_LENTH);

	g_free (line);
	g_free (command);
}

gint
compile_is_error (gchar *output)
{
	/* Check whether output is an error message. */
	gint i, j;
	gchar tag[100];
	
	if (!output[0])
		return 1;
		
	for (i = 0; i < strlen (output) - 5; i++)
		if (output[i] == ' ')
			break;
	
	i++;
	for (j = 0; j < 3; j++)
		tag[j] = output[i + j];
	tag[3] = 0;

	if (g_strcmp0 (tag, _("err")) == 0 || g_strcmp0 (tag, _("fat")) == 0)
		return 1;
	
	return 0;
}

gint
compile_is_warning (gchar *output)
{
	/* check whether output is an warning message. */
	gint i, j;
	gchar tag[100];
	
	if (!output[0])
		return 1;
		
	for (i = 0; i < strlen (output) - 5; i++)
		if (output[i] == ' ')
			break;
	
	i++;
	for (j = 0; j < 3; j++)
		tag[j] = output[i + j];
	tag[3] = 0;

	if (g_strcmp0 (tag, _("war")) == 0)
		return 1;
	
	return 0;
}

void
compile_get_location (const gchar *line, gint *row, gint *column)
{
	/* Get code location of a error or warning. */
	gint i;

	i = 0;
	while (line[i] && line[i] != ':')
		i++;
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
