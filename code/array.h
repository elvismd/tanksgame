#pragma once

#include "agnostic.h"

#define Static_Array(T, size)\
    struct { T a[size]; int count = size; int length; }
    
#define array_size(arr) array_count(arr.a)

#define array_add(arr, item)\
    arr.a[arr.length++] = item

#define array_item(arr, i) arr.a[i]