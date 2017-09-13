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

#define SYMBOL_DEBUG 0

#define MAX_TYPENAME_LENGTH 255
#define MAX_VARNAME_LENGTH 255
#define MAX_SIGN_LENGTH 2550

typedef enum {
	META_TYPE_BASE,
	META_TYPE_FUNCTION,
	META_TYPE_STRUCT,
	META_TYPE_CLASS,
	META_TYPE_NAMESPACE
} CMetaType;

typedef struct {
	CMetaType metaclass;
	gint unuse;
} CSymbolMeta;

typedef struct {
	CMetaType metaclass;
	gchar name[MAX_TYPENAME_LENGTH + 1];
	GList *public_member_list;
	GList *public_function_list;
} CSymbolClass;

typedef struct {
	CMetaType metaclass;
	gchar name[MAX_TYPENAME_LENGTH + 1];
	GList *member_list;
	GList *function_list;
} CSymbolStruct;
	
typedef struct {
	CMetaType metaclass;
	gchar name[MAX_TYPENAME_LENGTH + 1];
	GList *member_list;
	GList *function_list;
} CSymbolNamespace;

typedef struct {
	CMetaType metaclass;
	gchar name[MAX_TYPENAME_LENGTH + 1];
	gchar sign[MAX_SIGN_LENGTH + 1];
} CSymbolFunction;

typedef struct {
	CMetaType metaclass;
	gchar type[MAX_TYPENAME_LENGTH + 1];
	gchar name[MAX_VARNAME_LENGTH + 1];
} CSymbolVariable;

gboolean 
symbol_parse (gpointer data);

void
symbol_init ();

void
symbol_function_get_sign (const gchar *name, GList **sign);

void
symbol_variable_get_member (const gchar *name, const gint lineno, const gboolean isptr, GList **funs, GList **vars);

void
symbol_namespace_get_member (const gchar *name, GList **funs, GList **vars);

#endif

