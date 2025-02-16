
#define PORT_CONTROL   0x9E //PORTCFG 
#define PORT_DATAREG   0x9F //PORTSPI

#define SPI_CS     33
#define SPI_SCK    34
#define SPI_MOSI   35
#define SPI_MISO   36
#define SPI_PORT spi0

void spi_initialize();
uint8_t spi_handle_control_register();
void io_main();
