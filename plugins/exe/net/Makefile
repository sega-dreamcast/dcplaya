#
# Make exe plugins for net
#
# (C) COPYRIGHT 2002 Vincent Penne
#
# $Id: Makefile,v 1.4 2007-03-17 14:40:29 vincentp Exp $
#

LEZ=yes

KOS_LOCAL_INCS += -iquote . -iquote ../../../include -iquote ../../../libs
OPTIMIZE = $(OPTIMIZE_FOR_SPEED)

SUBDIRS = bba lan

include ../../../Makefile.inc

LWIPDIR = $(KOS_BASE)/../kos-ports/lwip/lwip/src
ARCHDIR = $(LWIPDIR)/../../kos

KOS_LOCAL_CFLAGS += -DDCPLAYA=1 -DIPv4 -DLWIP_DEBUG
KOS_INCS +=  \
	-DIPv4 \
	-I$(LWIPDIR)/include -I$(ARCHDIR)/include \
	-I$(LWIPDIR)/include/ipv4 

LEF_EXTRA += -L$(KOS_BASE)/addons/lib/dreamcast -llwip4
