#ifndef __HAL_H_
#define __HAL_H_

#define CMD_PORT  0x9E
#define DATA_PORT 0x9F

void hal_init ();
void hal_deinit ();

bool    supports_80_column_mode ();

uint8_t getManufacturerID();
uint32_t getSDCapacity();
uint32_t getSDSerial();

void    write_command (uint8_t command)  __z88dk_fastcall __naked;
void    write_data (uint8_t data)  __z88dk_fastcall __naked;
uint8_t read_data ()  __z88dk_fastcall __naked;
uint8_t read_status ()  __z88dk_fastcall __naked;
bool    pressed_ESC() __z88dk_fastcall __naked;
void    read_data_multiple (uint8_t* buffer,uint8_t len);
void    write_data_multiple (uint8_t* buffer,uint8_t len);
void    delay_ms (uint16_t milliseconds);



#endif //__HAL_H_