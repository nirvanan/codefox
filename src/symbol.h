/*
 * symbol.h
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

#ifndef SYMBOL_H
#define SYMBOL_H

#include <gtk/gtk.h>

#define MAX_TYPENAME 32
#define MAX_VARNAME 255

struct __class
{
	gchar type[MAX_TYPENAME + 1];
	GList *public_data_list;
	GList *public_fun_list;
};

struct __struct
{
	gchar type[MAX_TYPENAME + 1];
	GList *member_list;
}a;
	
struct __enum
{
	gchar type[MAX_TYPENAME + 1];
	GList *member_list;
};

struct __function
{
	gchar type[MAX_TYPENAME + 1];
	gchar return_type[MAX_TYPENAME + 1];
	GList *parameter_list;
};

struct __variable
{
	gchar type[MAX_TYPENAME + 1];
	gchar name[MAX_TYPENAME + 1];
	gint codeoffset;
};

void 
symbol_next_token (gchar *code, gchar *token);

gboolean
symbol_parse (gpointer data);

#endif

