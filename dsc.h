#ifndef DSC_ETHORNELL_H
#define DSC_ETHORNELL_H

#include <stdint.h>

/**
 * check if "data" is a "DSC FORMAT 1.00" file
 */
int dsc_is_valid(uint8_t * data, uint32_t size);

/**
 * decrypt the file.
 * WARNING: you have to check yourself if "crypted" is a valid file with "dsc_is_valid"
 *          before calling "dsc_decrypt"
 */
uint8_t * dsc_decrypt(uint8_t * crypted, uint32_t crypted_size, uint32_t * decrypted_size);

/**
 * save the file, if it's an image, save it in PNG format (and append ".png" to "filename").
 */
int dsc_save(uint8_t * data, uint32_t size, const char * filename);

#endif

