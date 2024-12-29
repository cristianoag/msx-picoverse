CC = gcc

SRCDIR = src
BINDIR = build
DISDIR = dist

VERBOSE = --verbose
CCFLAGS = -g 

SOURCES = multirom.c
OUTFILE = multirom.exe

all: clean compile package

compile: $(BINDIR)/$(OUTFILE)

$(BINDIR)/$(OUTFILE): $(SRCDIR)/$(SOURCES)
	@echo "Compiling $@"
	$(CC) $(CCFLAGS) $< -o $@

package:
	@echo "Packaging..."
	cp $(BINDIR)/$(OUTFILE) $(DISDIR)/$(OUTFILE)

clean:
		@echo "Cleaning ...."
		rm -f $(BINDIR)\*.exe
		rm -f $(DISDIR)\*.exe

