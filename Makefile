# Based on original Makefile by Jack Wetherell, licensed under MIT. Modified by Daniel Inhoi Kim.
CC := gcc
SRCDIR := src
BUILDDIR := build
TARGET := calc
SRCEXT := c
SOURCES := $(shell find $(SRCDIR) -type f -name '*.$(SRCEXT)')
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
DEPS := $(OBJECTS:.o=.d)
CFLAGS := -O1 -Wall -Wextra
INC := -I include
LDLIBS := -lm

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@echo " Linking..."
	@$(CC) $^ -o $@ $(LDLIBS)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	@echo " Building..."
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(INC) -MMD -MP -c $< -o $@

debug:
	@$(MAKE) clean
	@$(MAKE) CFLAGS="$(CFLAGS) -g" all

clean:
	@echo " Cleaning..."
	@find $(BUILDDIR) -type f -delete
	@$(RM) $(TARGET)

-include $(DEPS)

.PHONY: all clean debug
