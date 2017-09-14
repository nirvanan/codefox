/*
 * symbol.c
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

#include <gtk/gtk.h>
#include <stdlib.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <glib/gi18n-lib.h>

#include "symbol.h"
#include "highlighting.h"
#include "editor.h"
#include "ui.h"
#include "misc.h"
#include "project.h"
#include "env.h"
#include "limits.h"

#define CHAR(c) ((c >= 'a' && c <= 'z') || \
 (c >= 'A' && c <= 'Z') || c == '#' || c == '_')

#define DIGIT(c) ((c >= '0' && c <= '9') || c == '.')

extern CWindow *window;

GList *namespace_list;
GList *function_list;
GList *variable_list;
GList *struct_list;
GList *class_list;

static void
symbol_clear_class(gpointer ptr);

static void
symbol_clear_struct(gpointer ptr);

static void
symbol_clear_namespace(gpointer ptr);

static void
symbol_clear_function(gpointer ptr);

static void
symbol_clear_variable(gpointer ptr);

static void
symbol_clear_class(gpointer ptr)
{
	GList *iterator;
	CSymbolClass *class_ptr = (CSymbolClass *) ptr;

	g_assert (class_ptr->metaclass == META_TYPE_CLASS);

	g_debug ("clear c:%s\n", class_ptr->name);

	for (iterator = class_ptr->public_member_list; iterator; iterator = iterator->next) {
		CSymbolMeta *meta_ptr;

		meta_ptr = (CSymbolMeta *) iterator->data;
		switch (meta_ptr->metaclass) {
			case META_TYPE_BASE:
				symbol_clear_variable ((gpointer *) meta_ptr);
				break;
			case META_TYPE_FUNCTION:
				symbol_clear_function ((gpointer *) meta_ptr);
				break;
			case META_TYPE_CLASS:
				symbol_clear_class ((gpointer *) meta_ptr);
				break;
			case META_TYPE_STRUCT:
				symbol_clear_struct ((gpointer *) meta_ptr);
				break;
			case META_TYPE_NAMESPACE:
				symbol_clear_namespace ((gpointer *) meta_ptr);
				break;
		}
	}
	g_list_free (class_ptr->public_member_list);
	g_list_free_full (class_ptr->public_function_list, g_free);

	g_free (ptr);
}

static void
symbol_clear_struct(gpointer ptr)
{
	GList *iterator;
	CSymbolStruct *struct_ptr = (CSymbolStruct *) ptr;

	g_assert (struct_ptr->metaclass == META_TYPE_STRUCT);

	g_debug ("clear s:%s\n", struct_ptr->name);

	for (iterator = struct_ptr->member_list; iterator; iterator = iterator->next) {
		CSymbolMeta *meta_ptr;

		meta_ptr = (CSymbolMeta *) iterator->data;
		switch (meta_ptr->metaclass) {
			case META_TYPE_BASE:
				symbol_clear_variable ((gpointer *) meta_ptr);
				break;
			case META_TYPE_FUNCTION:
				symbol_clear_function ((gpointer *) meta_ptr);
				break;
			case META_TYPE_CLASS:
				symbol_clear_class ((gpointer *) meta_ptr);
				break;
			case META_TYPE_STRUCT:
				symbol_clear_struct ((gpointer *) meta_ptr);
				break;
			case META_TYPE_NAMESPACE:
				symbol_clear_namespace ((gpointer *) meta_ptr);
				break;
		}
	}
	g_list_free (struct_ptr->member_list);
	g_list_free_full (struct_ptr->function_list, g_free);

	g_free (ptr);
}

static void
symbol_clear_namespace(gpointer ptr)
{
	GList *iterator;
	CSymbolNamespace *namespace_ptr = (CSymbolNamespace *) ptr;

	g_assert (namespace_ptr->metaclass == META_TYPE_NAMESPACE);

	g_debug ("clear n:%s\n", namespace_ptr->name);

	for (iterator = namespace_ptr->member_list; iterator; iterator = iterator->next) {
		CSymbolMeta *meta_ptr;

		meta_ptr = (CSymbolMeta *) iterator->data;
		switch (meta_ptr->metaclass) {
			case META_TYPE_BASE:
				symbol_clear_variable ((gpointer *) meta_ptr);
				break;
			case META_TYPE_FUNCTION:
				symbol_clear_function ((gpointer *) meta_ptr);
				break;
			case META_TYPE_CLASS:
				symbol_clear_class ((gpointer *) meta_ptr);
				break;
			case META_TYPE_STRUCT:
				symbol_clear_struct ((gpointer *) meta_ptr);
				break;
			case META_TYPE_NAMESPACE:
				symbol_clear_namespace ((gpointer *) meta_ptr);
				break;
		}
	}
	g_list_free (namespace_ptr->member_list);
	g_list_free_full (namespace_ptr->function_list, g_free);

	g_free (ptr);
}

static void
symbol_clear_function (gpointer ptr)
{
	CSymbolFunction *function_ptr = (CSymbolFunction *) ptr;

	g_debug ("clear f:%s:%s\n", function_ptr->name, function_ptr->sign);

	g_free (ptr);
}

static void
symbol_clear_variable (gpointer ptr)
{
	CSymbolVariable *variable_ptr = (CSymbolVariable *) ptr;

	g_debug ("clear v:%s:%s\n", variable_ptr->type, variable_ptr->name);

	g_free (ptr);
}

static void
symbol_debug_dump (gpointer *ptr, gint level)
{
	gchar line[MAX_LINE_LENGTH + 1];
	CSymbolMeta *meta_ptr = (CSymbolMeta *) ptr;
	gint i;
	
	for (i = 0; i < level; i++) {
		line[i] = '\t';
	}
	line[i] = '\0';

	switch (meta_ptr->metaclass) {
		case META_TYPE_BASE: {
			CSymbolVariable *variable_ptr = (CSymbolVariable *) ptr;

			g_debug ("%sv:%s:%s\n", line, variable_ptr->type, variable_ptr->name);
			break;
		}
		case META_TYPE_FUNCTION: {
			CSymbolFunction *function_ptr = (CSymbolFunction *) ptr;

			g_debug ("%sf:%s:%s\n", line, function_ptr->name, function_ptr->sign);
			break;
		}
		case META_TYPE_STRUCT: {
			CSymbolStruct *struct_ptr = (CSymbolStruct *) ptr;
			GList *iterator;

			g_debug ("%ss:%s\n", line, struct_ptr->name);
			for (iterator = struct_ptr->member_list; iterator; iterator = iterator->next) {
				symbol_debug_dump (iterator->data, level + 1);
			}
			for (iterator = struct_ptr->function_list; iterator; iterator = iterator->next) {
				symbol_debug_dump (iterator->data, level + 1);
			}
			break;
		}
	}			
}

static void
symbol_debug()
{
	GList *iterator;

	g_debug("dump class\n");
	for (iterator = class_list; iterator; iterator = iterator->next) {
		symbol_debug_dump (iterator->data, 1);
	}
	g_debug("dump struct\n");
	for (iterator = struct_list; iterator; iterator = iterator->next) {
		symbol_debug_dump (iterator->data, 1);
	}
	g_debug("dump namespace\n");
	for (iterator = namespace_list; iterator; iterator = iterator->next) {
		symbol_debug_dump (iterator->data, 1);
	}
	g_debug("dump function\n");
	for (iterator = function_list; iterator; iterator = iterator->next) {
		symbol_debug_dump (iterator->data, 1);
	}
	g_debug("dump variable\n");
	for (iterator = variable_list; iterator; iterator = iterator->next) {
		symbol_debug_dump (iterator->data, 1);
	}
}

static CSymbolClass *
symbol_find_class (const gchar *name)
{
	GList *iterator;

	for (iterator = class_list; iterator; iterator = iterator->next) {
		CSymbolClass *class_ptr;

		class_ptr = (CSymbolClass *) iterator->data;
		if (g_strcmp0 (class_ptr->name, name) == 0)
			return class_ptr;
	}

	return NULL;
}

static CSymbolStruct *
symbol_find_struct (const gchar *name)
{
	GList *iterator;

	for (iterator = struct_list; iterator; iterator = iterator->next) {
		CSymbolStruct *struct_ptr;

		struct_ptr = (CSymbolStruct *) iterator->data;
		if (g_strcmp0 (struct_ptr->name, name) == 0)
			return struct_ptr;
	}

	return NULL;
}

static CSymbolNamespace *
symbol_find_namespace (const gchar *name)
{
	GList *iterator;

	for (iterator = namespace_list; iterator; iterator = iterator->next) {
		CSymbolNamespace *namespace_ptr;

		namespace_ptr = (CSymbolNamespace *) iterator->data;
		if (g_strcmp0 (namespace_ptr->name, name) == 0)
			return namespace_ptr;
	}

	return NULL;
}

static void
symbol_read_token (const gchar *line, gchar *token, gint *offset, gint *ftoffset)
{
	sscanf (line + (*offset), "%s", token);
	for ((*ftoffset) = strlen (token) - 1; ; (*ftoffset)--)
		if (token[*ftoffset] == ':')
			break;
	(*ftoffset)++;
	(*offset) += strlen (token) + 1;
	
}

static void
symbol_parse_line (const gchar *line)
{
	gchar *name;
	gchar *type;
	gchar *acess;
	gchar *sign;
	gchar *token;
	gint offset;
	gint ftoffset;
	CSymbolClass *class_ptr;
	CSymbolStruct *struct_ptr;
	CSymbolNamespace *namespace_ptr;

	name = (gchar *) g_malloc (MAX_LINE_LENGTH);
	type = (gchar *) g_malloc (MAX_LINE_LENGTH);
	token = (gchar *) g_malloc (MAX_LINE_LENGTH);

	sscanf(line, "%s", name);
	offset = 0;
	while (line[offset] != '\"')
		offset++;
	offset++;

	sscanf (line + offset, "%s", type);
	offset += strlen (type) + 2;

	if (type[0] == 'm') {
		CSymbolVariable *variable_ptr;

		variable_ptr = (CSymbolVariable *) g_malloc (sizeof (CSymbolVariable));
		g_strlcpy (variable_ptr->type, "base", MAX_VARNAME_LENGTH);
		g_strlcpy (variable_ptr->name, name, MAX_TYPENAME_LENGTH);
		variable_ptr->metaclass = META_TYPE_BASE;

		token[0] = 0;
		symbol_read_token (line, token, &offset, &ftoffset);

		switch (token[0]) {
		case 'c':
			class_ptr = symbol_find_class (token + ftoffset);
			if (class_ptr == NULL) {
				class_ptr = (CSymbolClass *) g_malloc (sizeof (CSymbolClass));
				g_strlcpy (class_ptr->name, token + ftoffset, MAX_TYPENAME_LENGTH);
				class_ptr->metaclass = META_TYPE_CLASS;
				class_ptr->public_member_list = NULL;
				class_ptr->public_function_list = NULL;
				class_ptr->public_member_list = g_list_append (class_ptr->public_member_list, 
																variable_ptr);

				class_list = g_list_append (class_list, class_ptr);
			}
			else {
				class_ptr->public_member_list = g_list_append (class_ptr->public_member_list, 
																variable_ptr);
			}

			break;

		case 's':
			struct_ptr = symbol_find_struct (token + ftoffset);

			if (struct_ptr == NULL) {
				struct_ptr = (CSymbolStruct *) g_malloc (sizeof (CSymbolStruct));
				g_strlcpy (struct_ptr->name, token + ftoffset, MAX_TYPENAME_LENGTH);
				struct_ptr->metaclass = META_TYPE_STRUCT;
				struct_ptr->member_list = NULL;
				struct_ptr->function_list = NULL;
				struct_ptr->member_list = g_list_append (struct_ptr->member_list, 
														  variable_ptr);

				struct_list = g_list_append (struct_list, struct_ptr);
			}
			else {
				struct_ptr->member_list = g_list_append (struct_ptr->member_list, 
														  variable_ptr);
			}

			break;

		case 'n':
			namespace_ptr = symbol_find_namespace (token + ftoffset);

			if (namespace_ptr == NULL) {
				namespace_ptr = (CSymbolNamespace *) g_malloc (sizeof (CSymbolNamespace));
				g_strlcpy (namespace_ptr->name, token + ftoffset, MAX_TYPENAME_LENGTH);
				namespace_ptr->metaclass = META_TYPE_NAMESPACE;
				namespace_ptr->member_list = NULL;
				namespace_ptr->function_list = NULL;
				namespace_ptr->member_list = g_list_append (namespace_ptr->member_list, 
														  variable_ptr);

				namespace_list = g_list_append (namespace_list, namespace_ptr);
			}
			else {
				namespace_ptr->member_list = g_list_append (namespace_ptr->member_list, 
														  variable_ptr);
			}

			break;

		}
		
		token[0] = 0;
		symbol_read_token (line, token, &offset, &ftoffset);
		if (token[0] == 0) {
			g_free ((gpointer) name);
			g_free ((gpointer) type);
			g_free ((gpointer) token);

			return ;
		}

		if (token[0] == 't') {
			g_strlcpy (variable_ptr->type, token + ftoffset, MAX_TYPENAME_LENGTH);
			variable_ptr->metaclass = (token[8] == 'c')? META_TYPE_CLASS: META_TYPE_STRUCT;
		}
	}
	else if (type[0] == 'f') {
		CSymbolFunction *function_ptr;

		function_ptr = (CSymbolFunction *) g_malloc (sizeof (CSymbolFunction));
		g_strlcpy (function_ptr->name, name, MAX_TYPENAME_LENGTH);
		function_ptr->metaclass = META_TYPE_FUNCTION;
		function_ptr->sign[0] = 0;

		function_list = g_list_append (function_list, function_ptr);

		if (!line[offset] || (line[offset] == 's' && line[offset + 1] == 'i')) {
			if (line[offset])
				g_strlcpy (function_ptr->sign, line + offset + 10, MAX_SIGN_LENGTH);

			g_free ((gpointer) name);
			g_free ((gpointer) type);
			g_free ((gpointer) token);
			return ;
		}

		token[0] = 0;
		symbol_read_token (line, token, &offset, &ftoffset);

		switch (token[0]) {
		case 'c':
			class_ptr = symbol_find_class (token + ftoffset);
			if (class_ptr == NULL) {
				class_ptr = (CSymbolClass *) g_malloc (sizeof (CSymbolClass));
				g_strlcpy (class_ptr->name, token + ftoffset, MAX_TYPENAME_LENGTH);
				class_ptr->metaclass = META_TYPE_CLASS;
				class_ptr->public_member_list = NULL;
				class_ptr->public_function_list = NULL;
				class_ptr->public_function_list = g_list_append (class_ptr->public_function_list, 
																function_ptr);

				class_list = g_list_append (class_list, class_ptr);
			}
			else {
				class_ptr->public_function_list = g_list_append (class_ptr->public_function_list, 
																function_ptr);
			}

			break;

		case 's':
			struct_ptr = symbol_find_struct (token + ftoffset);

			if (struct_ptr == NULL) {
				struct_ptr = (CSymbolStruct *) g_malloc (sizeof (CSymbolStruct));
				g_strlcpy (struct_ptr->name, token + ftoffset, MAX_TYPENAME_LENGTH);
				struct_ptr->metaclass = META_TYPE_STRUCT;
				struct_ptr->member_list = NULL;
				struct_ptr->function_list = NULL;
				struct_ptr->function_list = g_list_append (struct_ptr->function_list, 
														  function_ptr);

				struct_list = g_list_append (struct_list, struct_ptr);
			}
			else {
				struct_ptr->function_list = g_list_append (struct_ptr->function_list, 
														  function_ptr);
			}

			break;

		case 'n':
			namespace_ptr = symbol_find_namespace (token + ftoffset);

			if (namespace_ptr == NULL) {
				namespace_ptr = (CSymbolNamespace *) g_malloc (sizeof (CSymbolNamespace));
				g_strlcpy (namespace_ptr->name, token + ftoffset, MAX_TYPENAME_LENGTH);
				namespace_ptr->metaclass = META_TYPE_NAMESPACE;
				namespace_ptr->member_list = NULL;
				namespace_ptr->function_list = NULL;
				namespace_ptr->function_list = g_list_append (namespace_ptr->function_list, 
														  function_ptr);

				namespace_list = g_list_append (namespace_list, namespace_ptr);
			}
			else {
				namespace_ptr->function_list = g_list_append (namespace_ptr->function_list, 
														  function_ptr);
			}

			break;
		}

		if (token[0] == 'c' || token[0] == 's') {
			token[0] = 0;
			symbol_read_token (line, token, &offset, &ftoffset);
		}

		if (token[0] == 's' && token[1] == 'i')
			offset -= strlen (token) + 1;

		if (strlen (token) > 2 && token[0] == 's' && token[1] == 'i')
			g_strlcpy (function_ptr->sign, line + offset + 10, MAX_SIGN_LENGTH);
		else
			function_ptr->sign[0] = 0;
	}
	else if (type[0] == 'v' || type[0] == 'l') {
		CSymbolVariable *variable_ptr;

		variable_ptr = (CSymbolVariable *) g_malloc (sizeof (CSymbolVariable));
		g_strlcpy (variable_ptr->type, "base", MAX_VARNAME_LENGTH);
		g_strlcpy (variable_ptr->name, name, MAX_TYPENAME_LENGTH);
		variable_ptr->metaclass = META_TYPE_BASE;

		if (line[offset] == 't') {
			token[0] = 0;
			symbol_read_token (line, token, &offset, &ftoffset);
			g_strlcpy (variable_ptr->type, token + ftoffset, MAX_TYPENAME_LENGTH);
			variable_ptr->metaclass = (token[8] == 'c'? META_TYPE_CLASS: META_TYPE_STRUCT);

		}
		variable_list = g_list_append (variable_list, variable_ptr);
	}

	g_free ((gpointer) name);
	g_free ((gpointer) type);
	g_free ((gpointer) token);
}

static void
symbol_clear ()
{
	GList *iterator;
	GList *iterator2;

	/*
	for (iterator = class_list; iterator; iterator = iterator->next) {
		CSymbolClass *class_ptr;

		class_ptr = (CSymbolClass *) iterator->data;
		g_list_free_full (class_ptr->public_member_list, g_free);
		g_list_free_full (class_ptr->public_function_list, g_free);
	}
	g_list_free_full (class_list, g_free);
	*/
	g_list_free_full (class_list, symbol_clear_class);
	class_list = NULL;
	
	/*
	for (iterator = struct_list; iterator; iterator = iterator->next) {
		CSymbolStruct *struct_ptr;
		
		struct_ptr = (CSymbolStruct *) iterator->data;
		g_list_free_full (struct_ptr->member_list, g_free);
		g_list_free_full (struct_ptr->function_list, g_free);
	}
	g_list_free_full (struct_list, g_free);
	*/
	g_list_free_full (struct_list, symbol_clear_struct);
	struct_list = NULL;
	
	/*
	for (iterator = namespace_list; iterator; iterator = iterator->next){
		CSymbolNamespace *namespace_ptr;
		
		namespace_ptr = (CSymbolNamespace *) iterator->data;
		g_list_free_full (namespace_ptr->member_list, g_free);
	}
	g_list_free_full (namespace_list, g_free);
	*/
	g_list_free_full (namespace_list, symbol_clear_namespace);
	namespace_list = NULL;

	g_list_free_full (function_list, symbol_clear_function);
	function_list = NULL;

	g_list_free_full (variable_list, symbol_clear_variable);
	variable_list = NULL;
}

void
symbol_init ()
{
	class_list = NULL;
	struct_list = NULL;
	namespace_list = NULL;
	function_list = NULL;
	variable_list = NULL;
}

gboolean 
symbol_parse (gpointer data)
{
	gchar *project_path;
	gchar *filepath;
	FILE *ctags;
	gchar *line;
	gint i;
	gint UNUSED;
	gchar *UNUSED2;

	if (!env_prog_exist (ENV_PROG_CTAGS) || !env_prog_exist (ENV_PROG_CSCOPE)) {
		g_warning ("ctags or cscope not found.");

		return FALSE;
	}
	symbol_clear ();
	if (1) {
		//g_usleep (500000);

		project_path = project_current_path ();

		if (project_path == NULL)
			return TRUE;

		UNUSED = chdir (project_path);
		UNUSED = system ("ctags -R --fields=ksSta --c++-kinds=+l --c-kinds=+l --exclude=Makefile *");
		ctags = fopen ("tags", "r");
		if (ctags == NULL) {
			g_warning ("can't open tag file, you might want to check whether ctags is installed.");
			ui_status_entry_new (FILE_OP_WARNING, _("can't open tag file, please check whether ctags is installed."));
			return FALSE;
		}

		line = (gchar *) g_malloc (MAX_LINE_LENGTH);

		for (i = 0; i < 6; i++)
			UNUSED2 = fgets (line, MAX_LINE_LENGTH, ctags);

		symbol_clear ();

		while (fgets (line, MAX_LINE_LENGTH, ctags)) {
			line[strlen (line) - 1] = 0;
			symbol_parse_line (line);
		}

		//symbol_debug ();

		g_free ((gpointer) line);
		fclose ((gpointer) ctags);

		UNUSED = system ("cscope -b");
	}

	return TRUE;
}

void
symbol_function_get_sign (const gchar *name, GList **sign)
{
	GList *iterator;

	for (iterator = function_list; iterator; iterator = iterator->next) {
		CSymbolFunction *f;

		f = iterator->data;
		if (g_strcmp0 (f->name, name) == 0) {
			*sign = g_list_append (*sign, f->sign);
			printf("%s(%s)\n", f->name, f->sign);
		}
	}
}

static void
symbol_get_type (const gchar *line, const gchar *name, const gboolean isptr, gchar *type)
{
	gint i, j;

	i = strlen (line) - 1;
	while (i >= 0 && !(g_str_has_prefix (line + i, name) && 
		   !CHAR (line[i - 1]) && !DIGIT (line[i - 1]) &&
		   !CHAR (line[i + strlen (name)]) && !DIGIT (line[i + strlen (name)])))
		i--;

	if (i == 0) {
		type[0] = 0;

		return ;
	}

	j = i - 1;
	while (line[j]== ' ')
		j--;

	if (j >= 0 && ((line[j] != '*' && !isptr) ||
		(line[j] == '*' && isptr))) {
		gint k, l;
		
		for (j = i - 1; j >= 0; j--)
			if (line[j] == ' ') {
				k = j + 1;
				while (line[k] && line[k] == ' ')
					k++;
				l = j - 1;
				while (l >= 0 && line[l] == ' ')
					l--;
				if ((CHAR (line[k]) || line[k] == '*') && (CHAR (line[l]) || DIGIT (line[l])))
					break;
			}

		if (j < 0)
			return ;

		while (CHAR (line[l]) || DIGIT (line[l]))
			l--;
		l++;
		for (k = 0; CHAR (line[l + k]) || DIGIT (line[l + k]); k++)
			type[k] = line[l + k];
		type[k] = 0;
	}
}

static void
symbol_get_member_from_type (const gchar *type, GList **funs, GList **vars)
{
	CSymbolClass *class_ptr;
	CSymbolStruct *struct_ptr;

	class_ptr = symbol_find_class (type);
	if (class_ptr != NULL) {
		GList *iterator;

		for (iterator = class_ptr->public_member_list; iterator; iterator = iterator->next) {
			CSymbolVariable *v;

			v = iterator->data;
			(*vars) = g_list_append (*vars, v->name);
		}
		for (iterator = class_ptr->public_function_list; iterator; iterator = iterator->next) {
			CSymbolFunction *f;

			f = iterator->data;
			(*funs) = g_list_append (*funs, f->name);
		}
	}

	struct_ptr = symbol_find_struct (type);
	if (struct_ptr != NULL) {
		GList *iterator;

		for (iterator = struct_ptr->member_list; iterator; iterator = iterator->next) {
			CSymbolVariable *v;

			v = iterator->data;
			(*vars) = g_list_append (*vars, v->name);
		}
		for (iterator = struct_ptr->function_list; iterator; iterator = iterator->next) {
			CSymbolFunction *f;

			f = iterator->data;
			(*funs) = g_list_append (*funs, f->name);
		}
	}
}

void
symbol_variable_get_member (const gchar *name, const gint lineno, const gboolean isptr, GList **funs, GList **vars)
{
	gchar *command;
	gchar *filepath;
	gchar *project_path;
	gchar *output;
	gchar *last_line;
	FILE *pipe_file;
	gchar *type;

	project_path = project_current_path ();

	if (project_path == NULL)
		return ;
	
	command = (gchar *) g_malloc (MAX_LINE_LENGTH);
	output = (gchar *) g_malloc (MAX_LINE_LENGTH);
	last_line = (gchar *) g_malloc (MAX_LINE_LENGTH);
	type = (gchar *) g_malloc (MAX_TYPENAME_LENGTH);
	filepath = (gchar *) g_malloc (MAX_LINE_LENGTH);

	if (ui_have_editor ()) {
		const gchar *code;

		code = ui_current_editor_code ();

		if (code != NULL) {
			g_strlcpy (filepath, project_path, MAX_LINE_LENGTH);
			g_strlcat (filepath, "/", MAX_LINE_LENGTH);
			g_strlcat (filepath, ".symbol.", MAX_LINE_LENGTH);

			if (project_get_type () == PROJECT_C)
				g_strlcat (filepath, "c", MAX_LINE_LENGTH);
			else
				g_strlcat (filepath, "cpp", MAX_LINE_LENGTH);

			misc_set_file_content (filepath, code);

			g_free ((gpointer) code);
		}
	}

	g_strlcpy (command, "cscope -L0 ", MAX_LINE_LENGTH);
	g_strlcat (command, name, MAX_LINE_LENGTH);
	g_strlcat (command, " ", MAX_LINE_LENGTH);
	g_strlcat (command, filepath, MAX_LINE_LENGTH);

	pipe_file = popen (command, "r");
	last_line[0] = 0;
	type[0] = 0;
	while (fgets (output, MAX_LINE_LENGTH, pipe_file)) {
		gint i;
		gint symbol_lineno;

		i = 0;

		while (output[i] != ' ')
			i++;
		i++;
		while (output[i] != ' ')
			i++;
		i++;

		sscanf (output + i, "%d", &symbol_lineno);
		if (symbol_lineno >= lineno)
			break;

		while (output[i] != ' ')
			i++;
		i++;
		g_strlcpy (last_line, output + i, MAX_LINE_LENGTH);
		last_line[strlen (last_line) - 1] = 0;

		symbol_get_type (last_line, name, isptr, type);
	}

	if (type[0]) 
		symbol_get_member_from_type (type, funs, vars);

	g_free ((gpointer) command);
	g_free ((gpointer) output);
	g_free ((gpointer) filepath);
	g_free ((gpointer) last_line);	
	g_free ((gpointer) type);
}

void
symbol_namespace_get_member (const gchar *name, GList **funs, GList **vars)
{
	CSymbolNamespace *namespace_ptr;

	namespace_ptr = symbol_find_namespace (name);
	if (namespace_ptr != NULL) {
		GList *iterator;

		for (iterator = namespace_ptr->member_list; iterator; iterator = iterator->next) {
			CSymbolVariable *v;

			v = iterator->data;
			(*vars) = g_list_append (*vars, v->name);
		}
		for (iterator = namespace_ptr->function_list; iterator; iterator = iterator->next) {
			CSymbolFunction *f;

			f = iterator->data;
			(*funs) = g_list_append (*funs, f->name);
		}
	}
}
