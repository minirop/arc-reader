#include "decrypt.h"

uint32_t read32(uint8_t ** c)
{
	uint32_t i = *(uint32_t*)*c;
	(*c) += 4;
	return i;
}

uint16_t read16(uint8_t ** c)
{
	uint16_t i = *(uint16_t*)*c;
	(*c) += 2;
	return i;
}

uint8_t read8(uint8_t ** c)
{
	uint8_t i = *(uint8_t*)*c;
	(*c)++;
	return i;
}

uint8_t  myHIBYTE(uint16_t v) { return (v >> 8); }
uint8_t  myLOBYTE(uint16_t v) { return (v & 0xFFFF); }
uint16_t myHIWORD(uint32_t v) { return (v >> 16); }
uint16_t myLOWORD(uint32_t v) { return (v & 0xFFFF); }

uint32_t hash_update(uint32_t * hash_val)
{
	uint32_t eax, edx;
	edx = (20021 * myLOWORD(*hash_val));
	eax = (20021 * myHIWORD(*hash_val)) + (346 * *hash_val) + myHIWORD(edx);
	*hash_val = (myLOWORD(eax) << 16) + myLOWORD(edx) + 1;
	return eax & 0x7FFF;
}

