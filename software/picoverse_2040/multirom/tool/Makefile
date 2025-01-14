CC = gcc

SRCDIR = src
BINDIR = build
DISDIR = dist

VERBOSE = --verbose
#CCFLAGS = -g -DDEBUG
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
	cp $(PICOBIN) $(BINDIR)/multirom.bin 
	cp $(MSXMENU) $(BINDIR)/multirom.msx
	cp $(PICOBIN) $(DISDIR)/multirom.bin 
	cp $(MSXMENU) $(DISDIR)/multirom.msx

#	cp ../../tools/uf2conv.exe $(DISDIR)/uf2conv.exe

clean:
		@echo "Cleaning ...."
		rm -f $(BINDIR)/*.exe $(BINDIR)/multirom.msx $(BINDIR)/multirom.cfg $(BINDIR)/multirom.bin $(BINDIR)/multirom.cmb $(BINDIR)/multirom.uf2
		rm -f $(DISDIR)/*.exe $(DISDIR)/multirom.msx $(DISDIR)/multirom.cfg $(DISDIR)/multirom.bin $(DISDIR)/multirom.cmb $(DISDIR)/multirom.uf2

