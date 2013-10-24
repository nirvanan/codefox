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

gchar *
compile_current_project (gchar *filepath)
{
	/* Fork a new process and use pipe to receive check outputs. */
	int fd[2];
	pid_t pid;
	gchar *buf;
	
	buf = g_malloc (4096);
	
	if(pipe(fd)<0)  
	{  
		fprintf(stderr, "[Main]pipe error!/n");
		return NULL;
	}
	
	if((pid = fork())<0)  
	{  
		fprintf(stderr,"[Main]fork error!/n");  
		return NULL;  
	} 
	/* Child process, do compile. */
	if(pid == 0)  
	{
		close (fd[0]);
		dup2 (fd[1], 2);
		close (fd[1]);
		
		gint len, i;
		gchar *dir;
		
		dir = g_malloc (1024);
		len = strlen (filepath);
		strcpy (dir, filepath);
		for (i = len - 1; dir[i] != '/'; i--)
			dir[i] = 0;
		strcpy (dir + i, "/a.out");
		
		if (filepath[len - 1] == 'c')
			execlp (C_CMD, C_CMD, "-Wall", "-g", "-o", dir, filepath, (char *) NULL);
		else
			execlp (CPP_CMD, CPP_CMD, "-Wall", "-g", "-o", dir, filepath, (char *) NULL);
		
		g_free (dir);
		g_free (buf);
		write (2, "EOF",  3);
		exit (0);
	}
	/* Main process, receive outputs from pipe. */
	else
	{
		buf[0] = 0;
		close (fd[1]);
		wait (NULL);
		read (fd[0], buf, 4096);
		
		int len, i;
		
		len = strlen (buf);
		for (i = len - 1; buf[i] != '\n'; i--)
			;
		buf[i + 1] = 0;
		return buf;
	}
}

gchar *
compile_static_check (gchar *filepath)
{
	/* Fork a new process and use pipe to receive check outputs. */
	int fd[2];
	pid_t pid;
	gchar *buf;
	
	buf = g_malloc (4096);
	
	if(pipe(fd)<0)  
	{  
		fprintf(stderr, "[Main]pipe error!/n");
		return NULL;
	}
	
	if((pid = fork())<0)  
	{  
		fprintf(stderr,"[Main]fork error!/n");  
		return NULL;  
	}  

	/* Child process, do static check. */
	if(pid == 0)  
	{
		close (fd[0]);
		dup2 (fd[1], 2);
		close (fd[1]);
		
		gint len, i;
		gchar *dir;
		
		dir = g_malloc (1024);
		len = strlen (filepath);
		strcpy (dir, filepath);
		for (i = len - 1; dir[i] != '/'; i--)
			dir[i] = 0;
		strcpy (dir + i, "/a.out");
		
		/* Use "CC -S" to do static check. */
		if (filepath[len - 1] == 'c')
			execlp (C_CMD, C_CMD, "-Wall", "-S", filepath, (char *) NULL);
		else
			execlp (CPP_CMD, CPP_CMD, "-Wall", "-S", filepath, (char *) NULL);
		
		g_free (dir);
		g_free (buf);
		write (2, "EOF",  3);
		exit (0);
	}
	/* Main process, receive outputs from pipe. */
	else
	{
		buf[0] = 0;
		close (fd[1]);
		wait (NULL);
		read (fd[0], buf, 4096);
		
		int len, i;
		
		len = strlen (buf);
		for (i = len - 1; buf[i] != '\n'; i--)
			;
		buf[i + 1] = 0;
		return buf;
	}
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
compile_get_location (gchar *line, gchar *position)
{
	/* Get code location of a error or warning. */
	gint i, len;
	sscanf (line, "%s", position);
	
	len = strlen (position);
	
	for (i = len - 1; i >= 0; i--)
		if (position[i] == ':')
		{
			position[i] = 0;
			break;
		}
	for (i = len - 2; i >= 0; i--)
		if (position[i] == ':')
		{
			position[i] = ' ';
			break;
		}
	for (i = len - 3; i >= 0; i--)
		if (position[i] == ':')
		{
			position[i] = ' ';
			break;
		}
	
}
