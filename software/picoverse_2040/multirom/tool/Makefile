CC = gcc

SRCDIR = src
BINDIR = build
DISDIR = dist

VERBOSE = --verbose
CCFLAGS = -g 

SOURCES = multirom.c
OUTFILE = multirom.exe

MSXMENU = ../msx/dist/menu.rom

all: clean compile package

compile: $(BINDIR)/$(OUTFILE)

$(BINDIR)/$(OUTFILE): $(SRCDIR)/$(SOURCES)
	@echo "Compiling $@"
	$(CC) $(CCFLAGS) $< -o $@

package:
	@echo "Packaging..."
	cp $(BINDIR)/$(OUTFILE) $(DISDIR)/$(OUTFILE)
	cp $(MSXMENU) $(BINDIR)/menu.msx
	cp $(MSXMENU) $(DISDIR)/menu.msx

clean:
		@echo "Cleaning ...."
		rm -f $(BINDIR)/*.exe $(BINDIR)/*.msx $(BINDIR)/config.bin $(BINDIR)/romcmb.bin
		rm -f $(DISDIR)/*.exe $(DISDIR)/*.msx

