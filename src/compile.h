/*
 * compile.h
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

#ifndef COMPILE_H
#define COMPILE_H

#include <gtk/gtk.h>
#include <sys/wait.h> 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h> 

#define C_CMD "gcc"
#define CPP_CMD "g++"

gchar *
compile_current_project (gchar *filepath);

gchar *
compile_static_check (gchar *filepath);

gint
compile_is_error (gchar *output);

gint
compile_is_warning (gchar *output);

void
compile_get_location (gchar *line, gchar *position);

#endif
