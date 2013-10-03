/*
 * main.c
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

#ifdef HAVE_CONFIG_H
 #include "config.h"
#endif

#include <glib/gi18n-lib.h>

#include "ui.h"
#include "staticcheck.h"
#include "symbol.h"
#include "prefix.h"

void
start_timer()
{
	/* Start static checking clocker and symbol parsing clocker. */
	//g_timeout_add (1000, static_check, NULL);
	//g_timeout_add (1000, symbol_parse, NULL);
}

int
main (int argc, char *argv[])
{
	gtk_init (&argc, &argv);
	
	/* Initialize i18n and i10n. */
	bindtextdomain (GETTEXT_PACKAGE, CODEFOX_LOCALEDIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

	ui_init ();
	start_timer ();

	/* Main loop. */
	gtk_main ();

	return 0;
}
