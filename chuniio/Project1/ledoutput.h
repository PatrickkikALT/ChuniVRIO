/*
    LED output functions
    
    Credits:
    somewhatlurker, skogaby
*/

#pragma once

#include <windows.h>

#include <stdbool.h>
#include <stdint.h>


extern HANDLE led_init_mutex;
HRESULT led_output_init();
void led_output_update(uint8_t board, const uint8_t* rgb);
