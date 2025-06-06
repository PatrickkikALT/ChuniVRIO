#include <windows.h>

#include <process.h>
#include <stdbool.h>
#include <stdint.h>

#include "leddata.h"
#include "ledoutput.h"
#include "pipeimpl.h"
#include <stdio.h>

static struct _chuni_led_data_buf_t led_unescaped_buf[LED_BOARDS_TOTAL];
static struct _chuni_led_data_buf_t led_escaped_buf[LED_BOARDS_TOTAL];

static bool led_output_is_init = false;
static bool any_outputs_enabled;

HANDLE led_init_mutex;

HRESULT led_output_init()
{
    DWORD dwWaitResult = WaitForSingleObject(led_init_mutex, INFINITE);
    if (dwWaitResult == WAIT_FAILED) {
        return HRESULT_FROM_WIN32(GetLastError());
    }
    else if (dwWaitResult != WAIT_OBJECT_0) {
        return E_FAIL;
    }
    
    if (!led_output_is_init) {
        for (int i = 0; i < LED_BOARDS_TOTAL; i++) {
            led_unescaped_buf[i].framing = LED_PACKET_FRAMING;
            led_unescaped_buf[i].board = i;
            led_unescaped_buf[i].data_len = chuni_led_board_data_lens[i];
            
            led_escaped_buf[i].framing = LED_PACKET_FRAMING;
            led_escaped_buf[i].board = i;
            led_escaped_buf[i].data_len = chuni_led_board_data_lens[i];
        }
        
        any_outputs_enabled = true;
        
        led_pipe_init();  // don't really care about errors here tbh
    }
    
    led_output_is_init = true;
    OutputDebugStringA("ChuniVR: LED Pipe Initialized \n");
    ReleaseMutex(led_init_mutex);
    return S_OK;
    // return 0;
}

struct _chuni_led_data_buf_t* escape_led_data(struct _chuni_led_data_buf_t* unescaped) {
    struct _chuni_led_data_buf_t* out_struct = &led_escaped_buf[unescaped->board];
    
    uint8_t* in_buf = unescaped->data;
    uint8_t* out_buf = out_struct->data;
    int i = 0;
    int o = 0;
    
    while (i < unescaped->data_len) {
        uint8_t b = in_buf[i++];
        if (b == LED_PACKET_FRAMING || b == LED_PACKET_ESCAPE) {
            out_buf[o++] = LED_PACKET_ESCAPE;
            b--;
        }
        out_buf[o++] = b;
    }
    
    out_struct->data_len = o;
    
    return out_struct;
}

void led_output_update(uint8_t board, const uint8_t* rgb)
{
    if (board < 0 || board > 2 || !any_outputs_enabled) {
        return;
    }
    
    memcpy(led_unescaped_buf[board].data, rgb, led_unescaped_buf[board].data_len);
    struct _chuni_led_data_buf_t* escaped_data = escape_led_data(&led_unescaped_buf[board]);
    
    led_pipe_update(escaped_data);
}
