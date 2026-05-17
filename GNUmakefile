#
# $Id$
#
# MAKEFILE GNU MAKE for Linux Version 1.0
# Copyright (C) 2002 by Free Software Foundation, Inc.
# Author: Antonio Tellez Flores <atellezf@yahoo.com.mx>
#
#                         COPYING
#
# This  program  is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License  as
# published by the Free Software Foundation; either version 2, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty  of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR  PURPOSE.  See  the
# GNU General Public License for more details.
#
# You  should  have  received  a  copy  of the GNU General Public
# License along with this program; if  not,  write  to  the  Free
# Software Foundation, Inc., 675  Mass  Ave, Cambridge, MA 02139,
# USA.
#
# ###################### Revision History #######################
#
# $Log$
# ###############################################################

#================================================================
# GNU C COMPILER FOR LINUX
#================================================================
#CFLAGS := -g -ggdb -I.
# Make declarations

BASE_DIR := CCode
SRC_DIR := $(BASE_DIR)/src
INC_DIR := $(BASE_DIR)/include
BUILD_DIR := $(BASE_DIR)/build

CC := gcc
LINKER := mpicc
CFLAGS := -g -ggdb -O0 -Wall -Wextra -I$(INC_DIR) -ffp-contract=off
LDFLAGS := 

CFILES := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(CFILES))

PROGRAM := LogisticDelivery
RM := rm -f
MKDIR := mkdir -p
INSTALL := cp $(PROGRAM) /usr/bin/

#----------------------------------------------------------------

# Make rules

.PHONY: all clean distclean

compile: $(BUILD_DIR) $(PROGRAM)

all:	clean compile
	@echo -en "\n"
clean:
	$(RM) *~ core $(OBJS) "#*"

distclean: clean
	$(RM) $(PROGRAM)

install_prog:
	$(INSTALL)
#----------------------------------------------------------------

# Make depend

$(BUILD_DIR):
	$(MKDIR) $(BUILD_DIR)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@
	@echo -en "\n"

$(PROGRAM): $(OBJS)
	$(LINKER) $(LDFLAGS) $(OBJS) -lm -o $(PROGRAM)

#---------------------------------------------------------------

# End Make