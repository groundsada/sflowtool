bin_PROGRAMS=sflowtool
AM_CPPFLAGS = -I/opt/homebrew/include -I/opt/homebrew/include/cjson -I$(top_srcdir)/src/decoder/include
sflowtool_SOURCES = sflow.h sflow_v2v4.h sflow_drop.h sflowtool.c sflow_xdr.h \
	decoder/include/decoder.h \
	decoder/include/protocol.h \
	decoder/include/json_loader.h \
	decoder/src/decoder.c \
	decoder/src/protocol.c \
	decoder/src/json_loader.c \
	decoder/src/field_extract.c \
	decoder/json_protocols.c \
	decoder/output_json.c
sflowtool_LDADD = -L/opt/homebrew/lib -lcjson
