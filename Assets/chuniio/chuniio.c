#include <chuniio.h>
#include <windows.h>
#include <process.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static HANDLE chuni_io_slider_thread;
static bool chuni_io_slider_stop_flag;

uint8_t[32] buttonsPressed;

uint16_t chuni_io_get_api_version(void) {
  return 0x102
}

HRESULT chuni_io_slider_init(void) {
  return void;
}

void chuni_io_slider_start(chuni_io_slider_callback_t callback) {
    chuni_io_slider_thread = (HANDLE)_beginthreadex(
        NULL,
        0,
        chuni_io_slider_thread_proc,
        callback,
        0,
        NULL);
}

void chuni_io_slider_stop(void) {
  chuni_io_slider_stop_flag = true;
}

void chuni_io_send_button(uint8_t btn) {
  buttonsPressed[btn] = 1;
}

void chuni_io_release_button(uint8_t btn) {
  buttonsPressed[btn] = 0;
}

static unsigned int __stdcall chuni_io_slider_thread_proc(void *ctx) {
    chuni_io_slider_callback_t callback;
    uint8_t pressure[32];
    size_t i;

    callback = ctx;

    while (!chuni_io_slider_stop_flag) {
        for (i = 0 ; i < _countof(pressure) ; i++) {
            if (buttonsPressed[i] == 1) {
                pressure[i] = 128;
            } 
            else {
                pressure[i] = 0;
            }
        }
        callback(pressure);
        Sleep(1);
    }

    return 0;
}
