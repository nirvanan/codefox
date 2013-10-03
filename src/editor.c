/*
 * editor.c
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

#include <stdlib.h>
#include <string.h>

#ifdef HAVE_CONFIG_H
 #include "config.h"
#endif

#include <glib/gi18n-lib.h>

#include "editor.h"
#include "callback.h"

static void
ceditor_set_tabs (GtkWidget *textview, gint size)
{
	/* Set a tab's width to be equal with four white spaces. */ 
	PangoTabArray *tab_array;
	PangoLayout *layout;
	gchar *tab_string;
	gint width, height;
	PangoFontDescription *fd;
	
	fd = pango_font_description_new ();
	pango_font_description_set_family (fd, "monospace");
	tab_string = g_strnfill (size, ' ');
	layout = gtk_widget_create_pango_layout (textview, tab_string);
	pango_layout_set_font_description (layout, fd);
	pango_layout_get_pixel_size (layout, &width, &height);
	tab_array = pango_tab_array_new (1, TRUE);
	pango_tab_array_set_tab (tab_array, 0, PANGO_TAB_LEFT, width);
	gtk_text_view_set_tabs (GTK_TEXT_VIEW (textview), tab_array);
	g_free (tab_string);
}

static void
ceditor_init (CEditor *new_editor, const gchar *label)
{
	/* Init a editor. */
	GtkWidget *close_image;
	PangoAttrList *labelatt;
	PangoAttribute *att;
	
	int len = strlen (label);
	for (len = len - 1; len >= 0; len--)
		if (label[len] == '/')
			break;
	
	new_editor->label_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 1);
	new_editor->label_name = gtk_label_new (label + len + 1);
	new_editor->filepath = g_malloc (1024);
	g_strlcpy (new_editor->filepath, label, 1024);
	new_editor->close_button = gtk_button_new ();
	close_image = gtk_image_new_from_stock (GTK_STOCK_CLOSE,
											GTK_ICON_SIZE_MENU);
	gtk_button_set_image (GTK_BUTTON (new_editor->close_button), 
						  GTK_WIDGET (close_image));
	gtk_button_set_relief (GTK_BUTTON (new_editor->close_button),
						   GTK_RELIEF_NONE);
	gtk_widget_set_has_tooltip (GTK_WIDGET (new_editor->close_button), 1);
	gtk_widget_set_can_focus (GTK_WIDGET (new_editor->close_button), 0);
	gtk_widget_set_can_default (GTK_WIDGET (new_editor->close_button), 0);
	gtk_widget_set_tooltip_text (GTK_WIDGET (new_editor->close_button),
											 _("Close Tab"));
	gtk_button_set_alignment (GTK_BUTTON (new_editor->close_button), 0.5, 0.5);
	gtk_widget_set_size_request (new_editor->close_button, 18, 18);
	gtk_box_pack_start (GTK_BOX (new_editor->label_box), new_editor->label_name, 1, 1, 1);
	gtk_box_pack_end (GTK_BOX (new_editor->label_box), new_editor->close_button, 0, 0, 0);
	gtk_widget_set_has_tooltip (GTK_WIDGET (new_editor->label_box), 1);
	gtk_widget_set_tooltip_text (GTK_WIDGET (new_editor->label_box), label);
	new_editor->scroll = gtk_scrolled_window_new (NULL, NULL);
	new_editor->dirty = 0;
	new_editor->lineno = gtk_label_new (NULL);
	gtk_widget_set_valign (new_editor->lineno, GTK_ALIGN_START);
	new_editor->linecount = 0;
	labelatt = pango_attr_list_new ();
	att = pango_attr_family_new ("monospace");
	pango_attr_list_insert (labelatt, att);
	att = pango_attr_foreground_new (40000, 40000, 40000);
	pango_attr_list_insert (labelatt, att);
	gtk_label_set_attributes (GTK_LABEL (new_editor->lineno), labelatt);
	new_editor->linebox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 1);
	new_editor->textbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 1);
	new_editor->notationfixed = gtk_fixed_new ();
	gtk_widget_set_size_request (new_editor->notationfixed, 18, -1);
	gtk_box_pack_start (GTK_BOX (new_editor->linebox), new_editor->lineno, 0, 0, 0);
	gtk_box_pack_start (GTK_BOX (new_editor->linebox), new_editor->notationfixed, 0, 0, 0);
	gtk_box_pack_start (GTK_BOX (new_editor->textbox), new_editor->linebox, 0, 0, 0);
	gtk_box_pack_start (GTK_BOX (new_editor->textbox), new_editor->textview, 1, 1, 1);
	gtk_scrolled_window_add_with_viewport (GTK_SCROLLED_WINDOW (new_editor->scroll), new_editor->textbox);
	new_editor->notationlist = NULL;
	new_editor->errorlinelist = NULL;
	new_editor->typelist = NULL;
	new_editor->variblelist = NULL;
}

CEditor *
ceditor_new (const gchar *label)
{
	/* Create a new empty editor, label will be "Untitled". */
	CEditor *new_editor;

	new_editor = (CEditor *) g_malloc (sizeof(CEditor));
	new_editor->textview = gtk_text_view_new ();
	ceditor_init (new_editor, label);
	ceditor_append_line_label (new_editor, 1);
	
	/* Connect signal handlers. */
	g_signal_connect_after (new_editor->textview, "move-cursor", 
						G_CALLBACK (on_cursor_change), NULL);
	g_signal_connect (new_editor->textview, "toggle-overwrite", 
						G_CALLBACK (on_mode_change), NULL);
	g_signal_connect (new_editor->textview, "button-press-event", 
						G_CALLBACK (on_textview_clicked), NULL);
	g_signal_connect (new_editor->textview, "button-release-event",
						G_CALLBACK (on_textview_clicked), NULL);
	g_signal_connect (new_editor->close_button, "clicked", 
						G_CALLBACK (on_close_page), NULL);
	g_signal_connect_after (GTK_TEXT_BUFFER (gtk_text_view_get_buffer (GTK_TEXT_VIEW (new_editor->textview))),
					  "insert-text",
					  G_CALLBACK (on_editor_insert), NULL);
	g_signal_connect_after (GTK_TEXT_BUFFER (gtk_text_view_get_buffer (GTK_TEXT_VIEW (new_editor->textview))),
					  "delete-range",
					  G_CALLBACK (on_editor_delete), NULL);
	g_signal_connect (GTK_TEXT_BUFFER (gtk_text_view_get_buffer (GTK_TEXT_VIEW (new_editor->textview))),
					  "delete-range",
					  G_CALLBACK (on_editor_delete2), NULL);
	
	highlight_register (GTK_TEXT_BUFFER (gtk_text_view_get_buffer (GTK_TEXT_VIEW (new_editor->textview))));
	
	ceditor_set_tabs (new_editor->textview, 4);	
	
	return new_editor;
}

CEditor *
ceditor_new_with_text (const gchar *label, const gchar *code_buf)
{
	/* Create a new empty editor, label will be the file name. */
	CEditor *new_editor;
	GtkTextIter start, end;
	GtkTextIter startitr, enditr;
	gint start_line, end_line;
	GtkTextTagTable *tag_table;
	GtkTextBuffer *buffer;

	new_editor = (CEditor *) g_malloc (sizeof(CEditor));
	tag_table =  gtk_text_tag_table_new ();
	buffer = gtk_text_buffer_new (tag_table);
	gtk_text_buffer_insert_at_cursor (buffer, code_buf, strlen(code_buf)); /* FIXME: use glib! */
	new_editor->textview = gtk_text_view_new_with_buffer (buffer);
	ceditor_init (new_editor, label);
	
	gtk_text_buffer_get_start_iter (buffer, &startitr);
	gtk_text_buffer_get_end_iter (buffer, &enditr);
	start_line = gtk_text_iter_get_line (&startitr);
	end_line = gtk_text_iter_get_line (&enditr);
	
	ceditor_append_line_label (new_editor, end_line - start_line + 1);
	
	/* Connect signal handlers. */
	g_signal_connect_after (new_editor->textview, "move-cursor", 
						G_CALLBACK (on_cursor_change), NULL);
	g_signal_connect (new_editor->textview, "toggle-overwrite", 
						G_CALLBACK (on_mode_change), NULL);
	g_signal_connect (new_editor->textview, "button-press-event", 
						G_CALLBACK (on_textview_clicked), NULL);
	g_signal_connect (new_editor->textview, "button-release-event",
						G_CALLBACK (on_textview_clicked), NULL);
	g_signal_connect (new_editor->close_button, "clicked", 
						G_CALLBACK (on_close_page), NULL);
	g_signal_connect_after (GTK_TEXT_BUFFER (buffer),
					  "insert-text",
					  G_CALLBACK (on_editor_insert), NULL);
	g_signal_connect_after (GTK_TEXT_BUFFER (buffer),
					  "delete-range",
					  G_CALLBACK (on_editor_delete), NULL);
	g_signal_connect (GTK_TEXT_BUFFER (buffer),
					  "delete-range",
					  G_CALLBACK (on_editor_delete2), NULL);
	
	/* Highlighting currently opened new code. */ 
	highlight_register (buffer);
	
	gtk_text_buffer_get_start_iter (buffer, &start);
	gtk_text_buffer_get_end_iter (buffer, &end);
	highlight_apply (buffer, &start, &end);
	
	ceditor_set_tabs (new_editor->textview, 4);
		
	return new_editor;
}

void
ceditor_remove (CEditor *editor)
{
	/* When closing a editor, free its memory. */
	g_free (editor->filepath);
	g_free (editor);
}

void
ceditor_save_path (CEditor *editor, gchar *filepath)
{
	/* Save current code to filepath. */
	gchar *text;
	GtkTextBuffer *buffer;
	GtkTextIter start, end;
	FILE *output;
	
	buffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (editor->textview));
	gtk_text_buffer_get_start_iter (buffer, &start);
	gtk_text_buffer_get_end_iter (buffer, &end);
	text = gtk_text_buffer_get_text (buffer, &start,
									 &end, 1);
	output = fopen (filepath, "w");
	fputs (text, output);
	fclose (output);
}

void
ceditor_set_dirty (CEditor *editor, gboolean dirty)
{
	/* If an opened file has been modified, set the dirty bit. */
	if (editor->dirty == dirty)
		return ;
	
	gchar *label;
	
	editor->dirty = dirty;
	label = (gchar *) gtk_label_get_text (GTK_LABEL (editor->label_name));
	if (!dirty)	
		gtk_label_set_text (GTK_LABEL (editor->label_name), label + 1);
	else
	{
		gchar *temp;

		temp = g_malloc (1024);
		g_strlcpy (temp + 1, label, 1023);
		temp[0] = '*';
		gtk_label_set_text (GTK_LABEL (editor->label_name), temp);
		g_free (temp);
	}
}

gboolean
ceditor_get_dirty (CEditor *editor)
{
	return editor->dirty;
}

void
ceditor_set_path (CEditor *editor, gchar *filepath)
{

	strcpy (editor->filepath, filepath);
	
	gint len;
	gint i;
	
	len = strlen (filepath);
	for (i = len - 1; filepath[i] != '/'; i--)
		;
	gtk_label_set_text (GTK_LABEL (editor->label_name),
						filepath + i + 1);
	gtk_widget_set_tooltip_text (GTK_WIDGET (editor->label_box), filepath);
}

void
ceditor_show (CEditor *editor)
{
	/* Let a widgets in editor appear. */
	gtk_widget_show (editor->label_box);
	gtk_widget_show (editor->label_name);
	gtk_widget_show (editor->close_button);
	gtk_widget_show (editor->scroll);
	gtk_widget_show (editor->textview);
	gtk_widget_show (editor->lineno);
	gtk_widget_show (editor->linebox);
	gtk_widget_show (editor->textbox);
	gtk_widget_show (editor->notationfixed);
}

void
ceditor_append_line_label (CEditor *editor, gint lines)
{
	/* Update line number column. */
	gchar *text;
	gint i;
	gint len;
	
	text = g_malloc (10240);
	g_strlcpy (text, gtk_label_get_text (editor->lineno), 10240);
	
	for (i = 1; i <= lines; i++)
	{
		gchar line[100];
		
		sprintf (line, "%d\n", i + editor->linecount);
		g_strlcat (text, line, 10240);
	}
	len = strlen (text);

	/* Update linecount. */
	editor->linecount += lines;
	
	gtk_label_set_text (editor->lineno, text);
	g_free (text);
}

void
ceditor_remove_line_label (CEditor *editor, gint lines)
{
	/* Update line number column. */
	gchar *text;
	gint i;
	gint len;
	gint p = -1;
	
	if (lines <= 0)
		return ;
	
	text = g_malloc (10240);
	g_strlcpy (text, gtk_label_get_text (editor->lineno), 10240);
	
	len = strlen (text);
	for (i = len - 1; i >= 0; i--)
		if (text[i] == '\n')
		{
			text[i]  = 0;
			p++;
			
			if (p == lines)
				break;
		}
	len = strlen (text);
	text[len] = '\n';
	text[len + 1] = 0;
	/* Update linecount. */
	editor->linecount -= lines;
	
	gtk_label_set_text (editor->lineno, text);
	g_free (text);
	
}

void
ceditor_add_notation (CEditor *editor, gint err, gint line, gchar *info)
{
	/* Show an error or warning icon next to the line number column. */
	GtkWidget *image;
	GSList *iterator;
	gchar *text;
	CNotation *new_notation;
	GtkIconTheme *icon_theme;
	GdkPixbuf *pixbuf;
	gint p = 0;
	
	text = g_malloc (10240);
	text[0] = 0;
	
	while (info[p] != ' ')
		p++;
	p++;
	
	for (iterator = editor->notationlist; iterator; iterator = iterator->next)
	{
		CNotation *notation;
		
		notation = (CNotation *) iterator->data;
		if (line == notation->line) 
		{
			/* If error and warning appear in the same line, show error icon, but
			   display warning messages as well. 
			 */
			if (!notation->err && err)
			{
				g_strlcat (text, gtk_widget_get_tooltip_text (notation->icon), 1024);
				g_strlcat (text, "\n", 1024);
				gtk_container_remove (GTK_CONTAINER (editor->notationfixed), notation->icon);
				g_free (notation);
				editor->notationlist = g_list_remove (editor->notationlist, (gpointer) notation);
				break;
			}
			else
			{
				g_strlcat (text, gtk_widget_get_tooltip_text (notation->icon), 1024);
				g_strlcat (text, "\n", 1024);
				g_strlcat (text, info + p, 1024);
				gtk_widget_set_tooltip_text (notation->icon, text);
				g_free (text);
				return ;
			}
		}
	}
	
	/* Add icon. */
	icon_theme = gtk_icon_theme_get_default ();
	if (err)
		pixbuf = gtk_icon_theme_load_icon (icon_theme, CODEFOX_STOCK_ERROR, 16, 0, NULL);
	else
		pixbuf = gtk_icon_theme_load_icon (icon_theme, CODEFOX_STOCK_WARNING, 16, 0, NULL);
	
	/* When mouse is on icon, display error or warning messages. */
	image = gtk_image_new_from_pixbuf (pixbuf);
	new_notation = g_malloc (sizeof (CNotation));
	new_notation->icon = image;
	new_notation->line = line;
	gtk_widget_set_has_tooltip (image, 1);
	gtk_widget_set_can_focus (image, 0);
	gtk_widget_set_can_default (image, 0);
	g_strlcat (text, info + p, 1024);
	gtk_widget_set_tooltip_text (image, text);
	gtk_widget_set_size_request (image, 14, 18);
	gtk_widget_set_size_request (editor->notationfixed, 18, 18 * editor->linecount);
	gtk_fixed_put (GTK_FIXED (editor->notationfixed), image, 2, (line - 1) * 18);
	gtk_widget_show (image);
	
	editor->notationlist = g_list_append (editor->notationlist, (gpointer) new_notation);
	
	g_object_unref (pixbuf);
	g_free (text);
}

void
ceditor_clear_notation (CEditor *editor)
{
	/* Remove all icons. */
	GSList *iterator;
	
	for (iterator = editor->notationlist; iterator; iterator = iterator->next)
	{
		CNotation *notation;
		
		notation = (CNotation *) iterator->data;
		gtk_container_remove (GTK_CONTAINER (editor->notationfixed), notation->icon);
		g_free (notation);
	}
	
	g_list_free (editor->notationlist);
	editor->notationlist = NULL;
}

void
ceditor_emit_save_signal (CEditor *editor)
{
	g_signal_emit_by_name (editor->close_button, "clicked", NULL);
}
