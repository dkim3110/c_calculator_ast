# Based on original Makefile by Jack Wetherell, licensed under MIT. Modified by Daniel Inhoi Kim.
CC := gcc
SRCDIR := src
BUILDDIR := build
TESTBUILDDIR := $(BUILDDIR)/tests
TESTDIR := tests
TARGET := bin/calc
TESTTARGET := bin/tester
SRCEXT := c
SOURCES := $(shell find $(SRCDIR) -type f -name '*.$(SRCEXT)')
TESTSOURCES := $(wildcard $(TESTDIR)/*.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
TESTOBJECTS  := $(patsubst $(TESTDIR)/%,$(TESTBUILDDIR)/%,$(TESTSOURCES:.$(SRCEXT)=.o))
DEPS := $(OBJECTS:.o=.d)
TESTDEPS := $(TESTOBJECTS:.o=.d)
LIBOBJECTS := $(filter-out $(BUILDDIR)/main.o,$(OBJECTS))
CFLAGS := -O1 -Wall -Wextra -std=c23 -D_GNU_SOURCE
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

$(TESTBUILDDIR)/%.o: $(TESTDIR)/%.$(SRCEXT)
	@echo " Building tests..."
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(INC) -MMD -MP -c $< -o $@

$(TESTTARGET): $(LIBOBJECTS) $(TESTOBJECTS)
	@echo " Linking tests..."
	@mkdir -p $(dir $@)
	@$(CC) $^ -o $@ $(LIB)

test: $(TESTTARGET)
	@echo " Running tests..."
	@echo ""
	@./$(TESTTARGET)

-include $(DEPS)
-include $(TESTDEPS)

.PHONY: all clean debug test
