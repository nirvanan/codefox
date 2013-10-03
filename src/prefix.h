/*
 * BinReloc - a library for creating relocatable executables
 * Written by: Mike Hearn <mike@theoretic.com>
 *             Hongli Lai <h.lai@chello.nl>
 * http://autopackage.org/
 *
 * This source code is public domain. You can relicense this code
 * under whatever license you want.
 *
 * See http://autopackage.org/docs/binreloc/ for
 * more information and how to use this.
 *
 * NOTE: if you're using C++ and are getting "undefined reference
 * to br_*", try renaming prefix.c to prefix.cpp
 */

#ifndef PREFIX_H
#define PREFIX_H

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

/*
 * enrico - all the code below is only compiled and used if ENABLE_BINRELOC is set in config.h,
 *          this only happens if configure option --enable-binreloc was used
 */
#ifdef ENABLE_BINRELOC


/* WARNING, BEFORE YOU MODIFY PREFIX.C:
 *
 * If you make changes to any of the functions in prefix.c, you MUST
 * change the BR_NAMESPACE macro.
 * This way you can avoid symbol table conflicts with other libraries
 * that also happen to use BinReloc.
 *
 * Example:
 * #define BR_NAMESPACE(funcName) foobar_ ## funcName
 * --> expands br_locate to foobar_br_locate
 */
#undef BR_NAMESPACE
#define BR_NAMESPACE(funcName) funcName


#define br_thread_local_store BR_NAMESPACE(br_thread_local_store)
#define br_locate BR_NAMESPACE(br_locate)
#define br_locate_prefix BR_NAMESPACE(br_locate_prefix)
#define br_prepend_prefix BR_NAMESPACE(br_prepend_prefix)

#ifndef BR_NO_MACROS
	/* These are convience macros that replace the ones usually used
	   in Autoconf/Automake projects */
	#undef SELFPATH
	#undef PREFIX
	#undef PREFIXDIR
	#undef BINDIR
	#undef SBINDIR
	#undef DATADIR
	#undef LIBDIR
	#undef LIBEXECDIR
	#undef ETCDIR
	#undef SYSCONFDIR
	#undef CONFDIR
	#undef LOCALEDIR
	#undef CODEFOX_PREFIX
	#undef CODEFOX_DATADIR
	#undef CODEFOX_LIBDIR
	#undef CODEFOX_DOCDIR
	#undef CODEFOX_LOCALEDIR

	#define SELFPATH	(br_thread_local_store (br_locate ((void *) "")))
	#define PREFIXDIR	(br_thread_local_store (br_locate_prefix ((void *) "")))
	#define BINDIR		(br_thread_local_store (br_prepend_prefix ((void *) "", "/bin")))
	#define SBINDIR		(br_thread_local_store (br_prepend_prefix ((void *) "", "/sbin")))
	#define LIBEXECDIR	(br_thread_local_store (br_prepend_prefix ((void *) "", "/libexec")))
	#define ETCDIR		(br_thread_local_store (br_prepend_prefix ((void *) "", "/etc")))
	#define SYSCONFDIR	(br_thread_local_store (br_prepend_prefix ((void *) "", "/etc")))
	#define CONFDIR		(br_thread_local_store (br_prepend_prefix ((void *) "", "/etc")))
	#define CODEFOX_PREFIX		(br_thread_local_store (br_locate_prefix ((void *) "")))
	#define CODEFOX_DATADIR		(br_thread_local_store (br_prepend_prefix ((void *) "", "/share")))
	#define CODEFOX_LIBDIR		(br_thread_local_store (br_prepend_prefix ((void *) "", "/lib")))
	#define CODEFOX_DOCDIR		(br_thread_local_store (br_prepend_prefix ((void *) "", "/share/doc/geany")))
	#define CODEFOX_LOCALEDIR		(br_thread_local_store (br_prepend_prefix ((void *) "", "/share/locale")))
#endif /* BR_NO_MACROS */


/* The following functions are used internally by BinReloc
   and shouldn't be used directly in applications. */

const char *br_thread_local_store (char *str);
char *br_locate		(void *symbol);
char *br_locate_prefix	(void *symbol);
char *br_prepend_prefix	(void *symbol, char *path);


#endif /* ENABLE_BINRELOC */

#endif /* PREFIX_H */
