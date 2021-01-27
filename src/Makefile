PROJDIR = $(shell git rev-parse --show-toplevel)
SRCDIR = $(PROJDIR)/src
BUILDDIR = $(PROJDIR)/build
BINDIR = $(BUILDDIR)/bin
OBJDIR = $(BUILDDIR)/obj

SRCFILES = $(wildcard $(SRCDIR)/*.c)
OBJFILES = $(notdir $(patsubst %.c,%.o, $(SRCFILES)))

EXEFILE = $(BINDIR)/interpreter

CFLAGS = -W -Wall -Wextra -fdiagnostics-color=always -Wfloat-equal -Wshadow -Wcast-align -Wcast-qual -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wredundant-decls -Wswitch-default -Wno-cast-align -g -c

# dependency generator
make-depend-c = gcc -MM -MF $3 -MP -MT $2 $(CFLAGS) $1
-include $(addprefix $(OBJDIR)/,$(OBJFILES:.o=.d))

.PHONY: clean build $(BINDIR) $(OBJDIR)

build: $(EXEFILE)

$(addprefix $(OBJDIR)/, $(OBJFILES)): | $(OBJDIR)

$(OBJDIR) $(BINDIR):
	@mkdir -p $@

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@echo "==== Auto-Generating Dependencies for $<. ===="
	@$(call make-depend-c,$<,$@,$(subst .o,.d,$@))
	@echo "==== Compiling $< into $@. ===="
	@gcc $(CFLAGS) -c -o  $@ $<

$(EXEFILE): $(addprefix $(OBJDIR)/, $(OBJFILES)) | $(BINDIR) 
	@echo "==== Linking $@. ===="
	gcc $(addprefix $(OBJDIR)/, $(OBJFILES)) -g -o $@

clean:
	@rm -rf $(OBJDIR)
	@rm -rf $(EXEFILE)

