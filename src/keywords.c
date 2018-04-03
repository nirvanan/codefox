/*
 * keywords.c
 * This file is part of codefox
 *
 * Copyright (C) 2012-2017 - Gordon Li
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <string.h>

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <gtk/gtk.h>

#include "keywords.h"

/* All C/C++ keywords. */
static gchar *keywords[] = 
{
	"asm", "auto", "bad_cast",
	"bad_typeid", "bool", "break", "case", "catch", "char", "class",
	"const", "const_cast", "continue", "default", "delete", "do",
	"double", "dynamic_cast", "else", "enum", "except", "explicit",
	"extern", "false", "finally", "float", "for", "friend", "goto",
	"if", "inline", "int", "long", "mutable", "namespace", "new",
	"operator", "private", "protected", "public", "register",
	"reinterpret_cast", "return", "short", "signed", "sizeof",
	"static", "static_cast", "struct", "switch", "template", "this",
	"throw", "true", "try", "type_info", "typedef", "typeid",
	"typename", "union", "unsigned", "using", "virtual", "void",
	"volatile", "wchar_t", "while", NULL,
};

typedef struct Trie {
	struct Trie *sub[256];
	gint flag;
} CTrie;

static CTrie *trie;

static CTrie *
keywords_trie_create ()
{
	CTrie *ret = (CTrie *) g_malloc (sizeof (CTrie));
	
	if (!ret) {
		g_error ("faild to create trie.");

		return NULL;
	}

	memset (ret, 0, sizeof(CTrie));
	
	return ret;
}

/** Inserts a word into the trie. */
static void
keywords_trie_insert (CTrie *obj, gchar *word)
{
	gchar *t = word;
	CTrie *h = obj;
	
	if (!obj || !word || !word[0]) {
		return;
	}
	while (t[0]) {
		CTrie *n;
		
		if (!h->sub[(gint) t[0]]) {
			n = (CTrie *) g_malloc (sizeof (CTrie));
			memset (n, 0, sizeof (CTrie));
		}
		else {
			n = h->sub[(gint) t[0]];
		};
		h->sub[(gint) t[0]] = n;
		if (!t[1]) {
			n->flag = 1;
		}
		t++;
		h = n;
	}
}

static gint
keywords_trie_search (CTrie *obj, gchar *word) {
	if (!obj || !word || !word[0]) {
		return 0;
	}
	if (!word[1]) {
		return obj->sub[(guchar) word[0]] && obj->sub[(guchar) word[0]]->flag;
	}
	return obj->sub[(guchar) word[0]]? keywords_trie_search(obj->sub[(guchar) word[0]], word + 1): 0;
}

/*
static gint
keywords_trie_starts_with (CTrie* obj, gchar *prefix) {
	if (!obj || !prefix || !prefix[0]) {
		return 0;
	}
	if (!prefix[1]) {
		return obj->sub[prefix[0] - 'a'] != NULL;
	}

	return obj->sub[prefix[0] - 'a']? keywords_trie_starts_with(obj->sub[prefix[0] - 'a'], prefix + 1): 0;
}

static void
keywords_trie_free (CTrie* obj) {
	if (!obj) {
		return;
	}

	for (gint i = 0; i < 26; i++) {
		keywords_trie_free (obj->sub[i]);
	}

	g_free (obj);
}
*/

void
keywords_init ()
{
	gchar **keyword = keywords;

	trie = keywords_trie_create ();
	if (!trie) {
		return;
	}

	while (*(keyword++)) {
		keywords_trie_insert (trie, *keyword);
	}
}

gint
keywords_is_keyword (gchar *word)
{
	if (!trie) {
		return 0;
	}

	return keywords_trie_search (trie, word);
}
