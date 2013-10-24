pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_in_files = codefox.pc.in
pkgconfig_DATA = $(pkgconfig_in_files:.pc.in=.pc)

desktopdir = $(datadir)/applications
desktop_in_files = codefox.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

@INTLTOOL_DESKTOP_RULE@

appdatadir = $(datadir)/appdata
appdata_DATA = codefox.appdata.xml

EXTRA_DIST =			\
	$(desktop_in_files)	\
	codefox.pc.in		\
	codefox.appdata.xml.in


check-local: codefox.desktop
	$(DESKTOP_FILE_VALIDATE) codefox.desktop

CLEANFILES =	 			\
	$(desktop_DATA)			\
	$(pkgconfig_DATA)		\
	$(appdata_DATA)

-include $(top_srcdir)/git.mk
