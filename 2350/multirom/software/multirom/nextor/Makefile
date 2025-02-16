CPP = sdcc
N80 = n80.exe
SJASM = sjasmplus.exe
MKNEXROM = mknexrom.exe

BUILD_DIR = build
DIST_DIR = dist
SRC_DIR = src
TOOLS_DIR = tools

NEXTOR_BASE = $(BUILD_DIR)/Nextor-2.1.2.base.dat
NEXTOR_ROM =  $(BUILD_DIR)/nextor.rom
NEXTOR_MAPPER = $(BUILD_DIR)/Mapper.ASCII16.bin

all: clean rom  package
	
$(BUILD_DIR)/driver.bin: $(SRC_DIR)/driver.asm
	@echo "Assembling $<..."
	$(TOOLS_DIR)/$(SJASM) --nologo --lst=build/driver.lst $<

rom: $(BUILD_DIR)/driver.bin
	@echo "Building Nextor ROM..."
	$(TOOLS_DIR)/$(MKNEXROM) $(NEXTOR_BASE) $(NEXTOR_ROM) /d:$(BUILD_DIR)/driver.bin 
#/m:$(NEXTOR_MAPPER)

package:
	@echo "Creating package..."
	@cp $(NEXTOR_ROM) ../tool/build/nextor.rom
	@cp $(NEXTOR_ROM) $(DIST_DIR)/nextor.rom

clean:
	@echo "Cleaning..."
	@rm -f $(BUILD_DIR)/driver.bin $(BUILD_DIR)/driver.lst
	@rm -f $(DIST_DIR)/nextor.rom 