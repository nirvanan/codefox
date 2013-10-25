/*
 * debug.h
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

#include <fcntl.h>
#include <errno.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glib/gi18n-lib.h>

#include "debug.h"
#include "misc.h"

#define MAX_FILEPATH_LENTH 1000
#define MAX_LINE_LENTH 1000
#define MAX_RESULT_LENTH 100000
#define GDB_SERVER_PORT_ARG ":10086"

static gint fd1[2];
static gint fd2[2];
static FILE *out_file;
static FILE *in_file;
static GList *breakpoint_list;
static gboolean debugging;
static pid_t target_pid;
static pid_t gdb_pid;
static pid_t proc_pid;

static GMutex debug_mutex;

static void debug_breakpoint_list_clear ();
static void debug_breakpoint_add (const gchar *breakpoint_desc);
static gboolean debug_conection_broken (const gchar *output);

static void
debug_breakpoint_list_clear ()
{
	GSList * iterator;

	for (iterator = (GSList *) breakpoint_list; iterator; iterator = iterator->next) {
		CBreakPoint *breakpoint;

		breakpoint = (CBreakPoint *) iterator->data;
		
		g_free (breakpoint->filepath);
		g_free (breakpoint);
	}

	g_list_free (breakpoint_list);
	breakpoint_list = NULL;
}

static void
debug_breakpoint_add (const gchar *breakpoint_desc)
{
	CBreakPoint *breakpoint;

	breakpoint = (CBreakPoint *) g_malloc (sizeof (CBreakPoint));
	breakpoint->filepath = (gchar *) g_malloc (MAX_FILEPATH_LENTH);
	sscanf (breakpoint_desc, "%s %d", breakpoint->filepath, &(breakpoint->line));

	breakpoint_list = g_list_append (breakpoint_list, (gpointer) breakpoint);

	g_sprintf (breakpoint_desc, "%s:%d", breakpoint->filepath, breakpoint->line);
	debug_command_exec ("b", breakpoint_desc, NULL);
}

static gboolean 
debug_conection_broken (const gchar *output)
{
	return (g_strcmp0 (output, _("Remote connection closed\n")) == 0 ||
			g_strcmp0 (output, _("The program is not being run.\n")) == 0);
}

void
debug_startup (const gchar *project_path, const gchar *project_name)
{
	gchar *exe_path;
	gchar *line;
	FILE *pipe_file;
	gint NOUSE;

	exe_path = (gchar *) g_malloc (MAX_FILEPATH_LENTH);
	g_strlcpy (exe_path, project_path, MAX_FILEPATH_LENTH);
	g_strlcat (exe_path, "/", MAX_FILEPATH_LENTH);
	g_strlcat (exe_path, project_name, MAX_FILEPATH_LENTH);

	target_pid = fork ();
	if (target_pid == 0) {
		execlp ("xterm", "xterm", "-e", "gdbserver", GDB_SERVER_PORT_ARG, exe_path, NULL);
	}

	NOUSE = pipe (fd1);
	NOUSE = pipe (fd2);

	gdb_pid = fork ();
	if (gdb_pid == 0) {

		close(fd1[1]);
		close(fd2[0]);

		fflush (stdin);
		fflush (stdout);
		dup2 (fd1[0], STDIN_FILENO);
		dup2 (fd2[1], STDOUT_FILENO);
		dup2 (fd2[1], STDERR_FILENO);
		close (fd1[0]);
		close (fd2[1]);

		execlp ("gdb", "gdb", "--quiet", exe_path, NULL);

		exit (0);
	}
	else {
		gint old_fl;

		close(fd1[0]);
		close(fd2[1]);

		out_file = fdopen (fd2[0], "r");
		in_file = fdopen (fd1[1], "w");

		old_fl = fcntl (fd2[0], F_GETFL, 0);
		fcntl (fd2[0], F_SETFL, old_fl | O_NONBLOCK);
		setvbuf (in_file, NULL, _IONBF, 0);

		debug_breakpoint_list_clear ();

		g_mutex_lock (&debug_mutex);
		debugging = TRUE;
		g_mutex_unlock (&debug_mutex);
	}
	
	g_free (exe_path);
	g_free (line);
}

void
debug_command_exec (const gchar *command, const gchar *para, gchar *output)
{
	gchar *line;
	gint NOUSE;

	g_mutex_lock (&debug_mutex);
	if (!debugging) {
		g_mutex_unlock (&debug_mutex);

		return ;
	}

	fflush (out_file);

	NOUSE = write (fd1[1], command, strlen (command));
	if (para != NULL) {
		NOUSE = write (fd1[1], " ", 1);
		NOUSE = write (fd1[1], para, strlen (para));
	}
	NOUSE = write (fd1[1], "\n", 1);

	g_usleep (100000);

	line = (gchar *) g_malloc (MAX_LINE_LENTH);
	line[0] = 0;
	if (output != NULL)
		output[0] = 0;
	errno = 0;
	while (fgets (line, MAX_LINE_LENTH, out_file)) {
		if (errno == EAGAIN) {
			break;
		}

		if (output != NULL)
			g_strlcat (output, line, MAX_RESULT_LENTH);
	}

	g_free (line);

	g_mutex_unlock (&debug_mutex);
}

void
debug_breakpoints_insert (GList *list)
{
	GSList * iterator;

	if (!debugging)
		return ;

	for (iterator = (GSList *) list; iterator; iterator = iterator->next) {
		gchar *breakpoint_desc;
		CBreakPoint *breakpoint;
		gint i;

		breakpoint_desc = (gchar *) iterator->data;
		debug_breakpoint_add (breakpoint_desc);
	}
}

void
debug_connect (const gchar *project_path, const gchar *project_name)
{
	gchar *exe_path;
	gchar *output;
	gint i;
	gint pid;

	exe_path = (gchar *) g_malloc (MAX_FILEPATH_LENTH);
	output = (gchar *) g_malloc (MAX_RESULT_LENTH);
	g_strlcpy (exe_path, project_path, MAX_FILEPATH_LENTH);
	g_strlcat (exe_path, "/", MAX_FILEPATH_LENTH);
	g_strlcat (exe_path, project_name, MAX_FILEPATH_LENTH);

	debug_command_exec ("target remote", GDB_SERVER_PORT_ARG, NULL);
	g_usleep (100000);
	debug_command_exec ("info", "proc", output);
	i = 0;
	while (!g_str_has_prefix (output + i, _("process")))
		i++;
	i += strlen (_("process")) + 1;
	sscanf (output + i, "%d", &pid);
	proc_pid = (pid_t) pid;
}

void
debug_breakpoint_update (gchar *breakpoint_desc)
{
	GSList * iterator;
	CBreakPoint *breakpoint;
	gchar *filepath;
	gint line;

	if (!debugging)
		return ;

	filepath = (gchar *) g_malloc (MAX_FILEPATH_LENTH);
	sscanf (breakpoint_desc, "%s %d", filepath, &line);
	for (iterator = (GSList *) breakpoint_list; iterator; iterator = iterator->next) {
		breakpoint = (CBreakPoint *) iterator->data;

		if (g_strcmp0 (filepath, breakpoint->filepath) == 0 && line == breakpoint->line) {
			breakpoint_list = g_list_remove (breakpoint_list, (gpointer) breakpoint);

			g_free (breakpoint->filepath);
			g_free (breakpoint);

			g_sprintf (breakpoint_desc, "%s:%d", filepath, line);

			debug_command_exec ("clear", breakpoint_desc, NULL);

			g_free (filepath);

			return ;
		}
	}

	debug_breakpoint_add (breakpoint_desc);

	g_free (filepath);
}

gboolean
debug_is_active ()
{
	g_mutex_lock (&debug_mutex);

	gboolean ret;

	ret = debugging;
	g_mutex_unlock (&debug_mutex);

	return ret;
}

void
debug_current_file_line (const gboolean startup, gchar *filename, gint *line)
{
	gchar *output;
	gint i;
	gint j;

	output = (gchar *) g_malloc (MAX_RESULT_LENTH);
	debug_command_exec ("info line", NULL, output);

	if (debug_conection_broken (output)) {
		filename[0] = 0;
		*line = 0;
		g_free (output);

		return ;
	}

	i = 0;
	if (startup) {
		while (output[i] && !g_str_has_prefix (output + i, _(" at ")))
			i++;
		i += strlen (_(" at "));
		j = 0;
		while (output[i] && output[i] != ':') {
			filename[j] = output[i];
			i++;
			j++;
		}
		filename[j] = 0;

		sscanf (output + i + 1, "%d", line);
	}
	else {
		while (output[i] && output[i] != ' ')
			i++;
		i++;
		sscanf (output + i, "%d", line);
		while (output[i] && output[i] != '\"')
			i++;
		i++;
		j = 0;
		while (output[i] && output[i] != '\"') {
			filename[j] = output[i];
			j++;
			i++;
		}
		filename[j] = 0;
	}

	g_free (output);
}

void
debug_current_locals (GList **locals)
{
	gchar *output;
	gint i;
	gint j;

	output = (gchar *) g_malloc (MAX_RESULT_LENTH);
	debug_command_exec ("info locals", NULL, output);

	if (debug_conection_broken (output)) {
		*locals = NULL;
		g_free (output);

		return ;
	}

	if (!g_str_has_prefix (output, _("No locals."))) {
		gchar *name;
		gchar *value;

		name = (gchar *) g_malloc (MAX_LINE_LENTH);
		value = (gchar *) g_malloc (MAX_LINE_LENTH);
		i = 0;

		while (output[i] != 0) {
			gchar *local;

			local = (gchar *) g_malloc (MAX_LINE_LENTH);
			sscanf (output + i, "%s", name);
			i += strlen (name) + 3;
			j = 0;

			while (output[i + j] && output[i + j] != '\n') {
				value[j] = output[i + j];
				j++;
			}
			value[j] = 0;
			i += strlen (value) + 1;

			g_sprintf (local, "%s %s", name, value);

			*locals = g_list_append (*locals, (gpointer) local);
		}

		g_free (name);
		g_free (value);
	}

	g_free (output);
}

void
debug_expression_value (const gchar *expression, gchar *value)
{
	gchar *output;
	gint i;

	output = (gchar *) g_malloc (MAX_RESULT_LENTH);
	debug_command_exec ("p", expression, output);

	if (debug_conection_broken (output)) {
		g_strlcpy (value, _("Can't get the value."), MAX_LINE_LENTH);
		g_free (output);

		return ;
	}

	i = 0;
	while (output[i] && !g_str_has_prefix (output + i, " = "))
		i++;
	if (output[i] != 0) {
		i += 3;
		g_strlcpy (value, output + i, MAX_LINE_LENTH);

		if (value[strlen (value) - 1] == '\n')
			value[strlen (value) - 1] = 0;
	}
	else
		g_strlcpy (value, _("Can't get the value."), MAX_LINE_LENTH);
}

void
debug_current_stack (GList **stack)
{
	gchar *output;
	gchar *frame_name;
	gchar *frame_args;
	gchar *file_line;
	gint i;
	gchar *line;

	output = (gchar *) g_malloc (MAX_RESULT_LENTH);
	debug_command_exec ("bt", NULL, output);

	if (debug_conection_broken (output)) {
		g_free (output);
		*stack = NULL;

		return ;
	}

	i = 0;
	if (output[0] != '#') {
		while (output[i] && !g_str_has_prefix (output + i, "\n#"))
			i++;
		i++;
	}

	frame_name = (gchar *) g_malloc (MAX_LINE_LENTH);
	frame_args = (gchar *) g_malloc (MAX_LINE_LENTH);
	file_line = (gchar *) g_malloc (MAX_LINE_LENTH);
	misc_parse_gdb_bt_line (output, TRUE, frame_name, frame_args, file_line, &i);
	line = (gchar *) g_malloc (MAX_LINE_LENTH);
	g_sprintf (line, "%s %s %s", frame_name, file_line, frame_args);
	*stack = g_list_append (*stack, (gpointer) line);
	while (output[i]) {
		misc_parse_gdb_bt_line (output, FALSE, frame_name, frame_args, file_line, &i);
		line = (gchar *) g_malloc (MAX_LINE_LENTH);
		g_sprintf (line, "%s %s %s", frame_name, file_line, frame_args);

		*stack = g_list_append (*stack, (gpointer) line);
	}

	g_free (output);
	g_free (frame_name);
	g_free (frame_args);
	g_free (file_line);
}

gpointer 
debug_monitor (gpointer data)
{
	while (1) {
		if (kill (proc_pid, 0) != 0) {
			kill (gdb_pid, SIGKILL);

			ui_disable_debug_widgets ();
			ui_enable_project_widgets ();
			ui_disable_debug_view ();

			g_mutex_lock (&debug_mutex);
			debugging = FALSE;
			g_mutex_unlock (&debug_mutex);

			return NULL;
		}

		g_usleep (200000);
	}

}

void
debug_stop ()
{
	kill (target_pid, SIGKILL);
	kill (gdb_pid, SIGKILL);

	g_mutex_lock (&debug_mutex);
	debugging = FALSE;
	g_mutex_unlock (&debug_mutex);
}