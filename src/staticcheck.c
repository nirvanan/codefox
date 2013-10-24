/*
 * staticcheck.c
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

#include "staticcheck.h"
#include "compile.h"
#include "highlighting.h"
#include "ui.h"
#include "project.h"

extern CWindow *window;

#define MAX_RESULT_LENTH 100000
#define MAX_LINE_LENTH 1000

gpointer 
static_check (gpointer data)
{	
	/* Get current code and check for errors and warnings. */
	gchar *output;
	gchar *line;
	gchar *code;
	gchar *project_path;
	gchar *file_path;
	gchar *code_path;
	gint project_type;
	gchar *libs;
	gint p;
	gboolean error;
	gboolean warning;

	project_path = NULL;
	file_path = (gchar *) g_malloc (MAX_LINE_LENTH);

	while (1) {
		g_usleep (500000);

		if (ui_have_editor ()) {
			ui_current_editor_filepath (file_path);

			if (project_path == NULL)
				project_path = project_current_path ();

			if (project_path == NULL || file_path[0] == 0) {

				continue;
			}

			project_type = project_get_type ();
			libs = (gchar *) g_malloc (MAX_LINE_LENTH);
			project_get_settings (libs, NULL);
			code_path = (gchar *) g_malloc (MAX_LINE_LENTH);
			g_strlcpy (code_path, project_path, MAX_LINE_LENTH);
			g_strlcat (code_path, "/", MAX_LINE_LENTH);
			g_strlcat (code_path, ".static.", MAX_LINE_LENTH);
			g_strlcat (code_path, project_type? "cpp": "c", MAX_LINE_LENTH);
			code = ui_current_editor_code ();

			if (code == NULL) {
				g_free (code_path);
				g_free (libs);

				continue;
			}

			misc_set_file_content (code_path, code);
			output = (gchar *) g_malloc (MAX_RESULT_LENTH);
			compile_static_check (code_path, project_type, libs, output);

			line = (gchar *) g_malloc (MAX_LINE_LENTH);
			p = 0;
			ui_current_editor_error_tag_clear ();

			error = FALSE;
			warning = FALSE;
			while (output[p]) {
				gint i = 0;
				
				while (output[p] && output[p] != '\n')
					line[i++] = output[p++];
				line[i] = 0;

				error = compile_is_error (line) || error;
				warning = compile_is_warning (line) || warning;
				if (error || warning) {
					gint row;
					gint column;
					gchar *code_line;
					gint len;

					compile_get_location (line, &row, &column);

					if (!ui_find_editor (file_path)) {

						continue;
					}

					code_line = (gchar *) g_malloc (MAX_LINE_LENTH);
					ui_current_editor_line (code_line, row - 1);
					len = 0;
					while (column - 1 + len >= 0 && code_line[column - 1 + len] &&
						   (code_line[column - 1 + len]) ||
						   (DIGIT (code_line[column - 1 + len]) 
						   && code_line[column - 1 + len] != '.')){
						len++;
					}

					if (BRACKET (code_line[column - 1]))
						len = 1;
					
					ui_current_editor_error_tag_add (row - 1, column - 1, len);

					g_free (code_line);
				}
				
				p++;
			}

			ui_status_image_set (error, warning);

			g_free (output);
			g_free (libs);
			g_free (code_path);
			g_free (line);
		}

	}
	
	g_free (file_path);

	return 0;
}
