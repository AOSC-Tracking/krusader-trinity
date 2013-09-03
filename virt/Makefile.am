## Makefile.am of tdebase/tdeioslave/tar

INCLUDES = $(all_includes)
AM_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_QT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
METASOURCES = AUTO

kde_module_LTLIBRARIES = tdeio_virt.la

tdeio_virt_la_SOURCES = virt.cc
tdeio_virt_la_LIBADD = $(LIB_TDESYCOCA)
tdeio_virt_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN) $(LIB_QT) $(LIB_TDECORE)

noinst_HEADERS = virt.h

kdelnk_DATA = virt.protocol
kdelnkdir = $(kde_servicesdir)

