#ifndef DECRYPT_ETHORNELL_H
#define DECRYPT_ETHORNELL_H

#include <stdint.h>

uint32_t read32(uint8_t ** c);
uint16_t read16(uint8_t ** c);
uint8_t  read8 (uint8_t ** c);

uint8_t  myHIBYTE(uint16_t v);
uint8_t  myLOBYTE(uint16_t v);
uint16_t myHIWORD(uint32_t v);
uint16_t myLOWORD(uint32_t v);

uint32_t hash_update(uint32_t * hash_val);

#endif

