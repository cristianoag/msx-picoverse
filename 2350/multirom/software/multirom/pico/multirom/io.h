
#define PORT_CONTROL   0x9E
#define PORT_DATAREG   0x9F

#define SPI_CS     33
#define SPI_SCK    34
#define SPI_MOSI   35
#define SPI_MISO   36
#define SPI_PORT spi0

void spi_initialize();
uint8_t spi_handle_control_register(uint8_t write_value, bool is_write);
void io_main();
