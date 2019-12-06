#ifndef PROTECT_H
#define PROTECT_H

#include "type.h"

typedef struct s_descriptor {
    u16     limit_low;
    u16     base_low;
    u8      base_mid;
    u8      attr1;
    u8      limit_high_attr2;
    u8      base_high;
} DESCRIPTOR;

#endif