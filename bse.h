#ifndef BSE_ETHORNELL_H
#define BSE_ETHORNELL_H

#include <stdint.h>

/**
 * check if "data" is a "BSE 1.0" file
 */
int bse_is_valid(uint8_t * data, uint32_t size);

/**
 * decrypt the file (only the first 64 bytes are encrypted).
 * WARNING: you have to check yourself if "crypted" is a valid file with "bse_is_valid"
 *          before calling "bse_decrypt"
 */
int bse_decrypt(uint8_t * crypted);

#endif
