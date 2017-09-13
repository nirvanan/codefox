/*
 * keywords.h
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

#ifndef KEYWORDS_H
#define KEYWORDS_H

#define MAX_KEYWORD_LENGTH 16

/* All C/C++ keywords. */
gchar *keywords[] = 
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
	"volatile", "wchar_t", "while"
} ;

#endif
