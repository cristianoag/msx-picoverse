CPP = sdcc
N80 = n80.exe
SJASM = sjasmplus.exe
ASM = sdasz80
MKNEXROM = mknexrom.exe
HEX2BIN = hex2bin

BUILD_DIR = build
DIST_DIR = dist
SRC_DIR = src
TOOLS_DIR = tools
KERNEL_DIR = kernel

NEXTOR_BASE = $(KERNEL_DIR)/Nextor-2.1.2.base.dat
NEXTOR_ROM =  $(BUILD_DIR)/nextor.rom
NEXTOR_MAPPER = $(KERNEL_DIR)/Mapper.ASCII16.bin
IHX = $(BUILD_DIR)/driver.ihx
BIN = $(BUILD_DIR)/driver.rom

ADDR_CODE = 0x4220
ADDR_DATA = 0xc800

all: clean $(NEXTOR_ROM) package

$(BUILD_DIR)/driver.rel: $(SRC_DIR)/driver.c
	$(CPP) -mz80 -c -DNODEBUG -o $@ $< 

$(BUILD_DIR)/hal.rel: $(SRC_DIR)/hal.c
	$(CPP) -mz80 -c -DNODEBUG  -o $@ $< 

$(BUILD_DIR)/msxromcrt0.rel: $(SRC_DIR)/msxromcrt0.s
	$(ASM) -o $@ $< 


$(IHX): $(BUILD_DIR)/msxromcrt0.rel $(BUILD_DIR)/hal.rel $(BUILD_DIR)/driver.rel
	$(CPP) -mz80 --code-loc $(ADDR_CODE) --data-loc $(ADDR_DATA) --no-std-crt0 -o $@ $^

$(BIN): $(IHX)
	@echo "Building driver ROM..."
	$(HEX2BIN) -l 3fd0 -e rom $<

$(NEXTOR_ROM): $(BIN) 
	@echo "Building Nextor ROM..."
	$(TOOLS_DIR)/$(MKNEXROM) $(NEXTOR_BASE) $@ /d:$(BIN)

package:
	@echo "Creating package..."
	@cp $(NEXTOR_ROM) ../tool/build/nextor.rom
	@cp $(NEXTOR_ROM) $(DIST_DIR)/nextor.rom

clean:
	@echo "Cleaning..."
	@rm -f $(BUILD_DIR)/driver.* $(BUILD_DIR)/nextor.rom $(BUILD_DIR)/msxromcrt0.rel
	@rm -f $(DIST_DIR)/nextor.rom 