/*
 * env.c
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

#include "env.h"
#include "limits.h"

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#define MAX_ENV_ARRAY_SIZE 1000

/* External programs. */
static gboolean env_extern_prog[MAX_ENV_ARRAY_SIZE + 1];

static gboolean
env_check_external_prog(const gchar *prog)
{
	gchar line[MAX_LINE_LENGTH + 1];
	gchar cmd[MAX_LINE_LENGTH + 1];
	FILE *pi;

	g_snprintf (cmd, MAX_LINE_LENGTH + 1, "which %s\n", prog);
	pi = popen (cmd, "r");

	/* One line is enough. */
	line[0] = '\0';
	fgets (line, MAX_LINE_LENGTH, pi);

	pclose (pi);

	if (strlen (line) > 0) {
		return TRUE;
	}

	return FALSE;
}

void
env_init()
{
	memset (env_extern_prog, 0, sizeof (env_extern_prog));
	env_extern_prog[ENV_PROG_XTERM] = env_check_external_prog ("xterm");
	env_extern_prog[ENV_PROG_GDBSERVER] = env_check_external_prog ("gdbserver");
	env_extern_prog[ENV_PROG_GDB] = env_check_external_prog ("gdb");
	env_extern_prog[ENV_PROG_CTAGS] = env_check_external_prog ("ctags");
	env_extern_prog[ENV_PROG_CSCOPE] = env_check_external_prog ("cscope");
	env_extern_prog[ENV_PROG_GCC] = env_check_external_prog ("gcc");
	env_extern_prog[ENV_PROG_GPP] = env_check_external_prog ("g++");
	env_extern_prog[ENV_PROG_MAKE] = env_check_external_prog ("make");
}

gboolean
env_prog_exist (const gint prog)
{
	g_assert (prog > 0 && prog <= MAX_ENV_ARRAY_SIZE);

	return env_extern_prog[prog];
}
