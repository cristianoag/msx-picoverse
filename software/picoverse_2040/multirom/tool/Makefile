CC = gcc

SRCDIR = src
BINDIR = build
DISDIR = dist

VERBOSE = --verbose
CCFLAGS = -g 

SOURCES = multirom.c
OUTFILE = multirom.exe

MSXMENU = ../msx/dist/menu.rom
PICOBIN = ../pico/multirom/build/multirom.bin

all: clean compile package

compile: $(BINDIR)/$(OUTFILE)

$(BINDIR)/$(OUTFILE): $(SRCDIR)/$(SOURCES)
	@echo "Compiling $@"
	$(CC) $(CCFLAGS) $< -o $@

package:
	@echo "Packaging..."
	cp $(BINDIR)/$(OUTFILE) $(DISDIR)/$(OUTFILE)
	cp $(PICOBIN) $(DISDIR)/multirom.bin
	cp $(MSXMENU) $(DISDIR)/multirom.msx
	cp ../../tools/uf2conv.exe $(DISDIR)/uf2conv.exe
	cp $(SRCDIR)/build.bat $(DISDIR)/build.bat

clean:
		@echo "Cleaning ...."
		rm -f $(BINDIR)/*.exe $(BINDIR)/*.msx $(BINDIR)/multirom.cfg $(BINDIR)/multirom_final.bin
		rm -f $(DISDIR)/*.exe $(DISDIR)/*.msx

