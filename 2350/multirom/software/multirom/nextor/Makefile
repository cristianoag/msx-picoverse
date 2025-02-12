CPP = sdcc
SJASM = n80.exe
MKNEXROM = mknexrom.exe

BUILD_DIR = build
DIST_DIR = dist
SRC_DIR = src
TOOLS_DIR = tools

NEXTOR_BASE = $(SRC_DIR)/Nextor-2.1.2.base.dat
NEXTOR_ROM =  $(BUILD_DIR)/nextor.rom

all: clean rom package
	
$(BUILD_DIR)/driver.bin: $(SRC_DIR)/driver.asm
	@echo "Assembling $<..."
	$(TOOLS_DIR)/$(SJASM) $< $@

rom: $(BUILD_DIR)/driver.bin
	@echo "Building ROM..."
	$(TOOLS_DIR)/$(MKNEXROM) $(NEXTOR_BASE) $(NEXTOR_ROM) /d:$(BUILD_DIR)/driver.bin

package:
	@echo "Creating package..."
	@cp $(NEXTOR_ROM) $(DIST_DIR)/nextor.rom

clean:
	@echo "Cleaning..."
	@rm -f $(BUILD_DIR)/driver.bin