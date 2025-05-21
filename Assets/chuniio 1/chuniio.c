#include <chuniio.h>
#include <windows.h>
#include <process.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void send_air_temp(uint_8_t index, BOOL value) {
  printf("Recieved air note %d", index)
}

void send_button_temp(uint_8_t index, BOOL value) {
  printf("Recieved button %d", index);
}

void chuni_io_slider_stop() {
  printf("Stopped ChuniIO");
}