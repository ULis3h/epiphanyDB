# EpiphanyDB Extension Makefile
# This makefile builds the EpiphanyDB extension for PostgreSQL

MODULE_big = epiphanydb
OBJS = epiphanydb.o storage_manager.o

EXTENSION = epiphanydb
DATA = epiphanydb--1.0.sql epiphanydb.control
PGFILEDESC = "EpiphanyDB - Multi-modal storage engine for PostgreSQL"

# Include directories for EpiphanyDB headers
PG_CPPFLAGS = -I$(srcdir)/../../../include

# Link with required libraries
SHLIB_LINK = -lssl -lcrypto -lxml2 -lxslt -lz

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/epiphanydb
top_builddir = ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif