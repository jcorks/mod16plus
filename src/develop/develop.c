#include "develop.h"
#include <stdlib.h>

const uint8_t * ses_develop_get_rom(uint32_t * length) {
    *length = 13;
    static char data[] = {
        'S', ' ', 'E', ' ', 'S', ' ',
        'J', ' ', 'L', ' ', 'C', ' ', 1
    };
    return data;
}

