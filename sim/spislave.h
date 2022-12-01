#ifndef _SPI_H
#define _SPI_H

#include <stdint.h>

void spislave_init(uint8_t *_sck, uint8_t *_miso, uint8_t *_mosi, uint8_t *_ss);
int spislave_handle();
void spislave_set_miso(uint8_t dat);

#endif
