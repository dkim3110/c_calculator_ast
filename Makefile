# Based on original Makefile by Jack Wetherell, licensed under MIT. Modified by Daniel Inhoi Kim.
CC := gcc
SRCDIR := src
BUILDDIR := build
TESTDIR := tests
TARGET := bin/calc
TESTTARGET := bin/tester
SRCEXT := c
SOURCES := $(shell find $(SRCDIR) -type f -name '*.$(SRCEXT)')
TESTSOURCES := $(wildcard $(TESTDIR)/*.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
DEPS := $(OBJECTS:.o=.d)
LIBOBJECTS := $(filter-out $(BUILDDIR)/main.o,$(OBJECTS))
CFLAGS := -O1 -Wall -Wextra
LIB := -L lib -lm
INC := -I include

all: $(TARGET)

$(TARGET): $(OBJECTS)
	@echo " Linking..."
	@mkdir -p $(dir $@)
	@$(CC) $^ -o $@ $(LIB)

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
	@$(RM) $(TARGET) $(TESTTARGET)

$(TESTTARGET): $(TESTSOURCES) $(LIBOBJECTS)
	@echo " Building tests..."
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(INC) $^ -o $@ $(LIB)

test: $(TESTTARGET)
	@echo " Running tests..."
	@echo " "
	@./$(TESTTARGET)

-include $(DEPS)

.PHONY: all clean debug test
