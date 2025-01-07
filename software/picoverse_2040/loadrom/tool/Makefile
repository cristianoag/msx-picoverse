CC = gcc

SRCDIR = src
BINDIR = build
DISDIR = dist

VERBOSE = --verbose
CCFLAGS = -g 

SOURCES = loadrom.c
OUTFILE = loadrom.exe

PICOBIN = ../pico/loadrom/dist/loadrom.bin

all: clean compile package

compile: $(BINDIR)/$(OUTFILE)

$(BINDIR)/$(OUTFILE): $(SRCDIR)/$(SOURCES)
	@echo "Compiling $@"
	$(CC) $(CCFLAGS) $< -o $@

package:
	@echo "Packaging..."
	cp $(BINDIR)/$(OUTFILE) $(DISDIR)/$(OUTFILE)
	cp $(PICOBIN) $(BINDIR)/loadrom.bin 
	cp $(PICOBIN) $(DISDIR)/loadrom.bin 

clean:
		@echo "Cleaning ...."
		rm -f $(BINDIR)/*.exe $(BINDIR)/loadrom.cfg $(BINDIR)/loadrom.bin $(BINDIR)/loadrom.cmb $(BINDIR)/loadrom.uf2
		rm -f $(DISDIR)/*.*

