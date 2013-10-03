SUBDIRS = 16x16 24x24 32x32 48x48 scalable

gtk_update_icon_cache = gtk-update-icon-cache -f -t

install-data-hook:
	@-if test -z "$(DESTDIR)"; then					\
		echo "Updating Gtk icon cache.";			\
		$(gtk_update_icon_cache) "$(datadir)/icons/hicolor";	\
	else								\
		echo "*** Icon cache not updated. Remember to run:";	\
		echo "***";						\
		echo "***   $(gtk_update_icon_cache) '$(datadir)/icons/hicolor'";\
		echo "***";						\
	fi

uninstall-local:
	rm -f $(DESTDIR)$(datadir)/icons/hicolor/icon-theme.cache

-include $(top_srcdir)/git.mk
