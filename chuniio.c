#include <windows.h>
#include <process.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "chuniio.h"

static uint8_t g_opbtn_state = 0;
static uint8_t g_beam_state = 0;
static uint8_t g_slider_state[32] = { 0 };

static chuni_io_slider_callback_t g_slider_callback = NULL;

static HANDLE g_slider_thread = NULL;
static bool g_slider_thread_stop = false;

static HANDLE g_pipe_thread = NULL;
static bool g_pipe_thread_stop = false;

#define PIPE_NAME "\\\\.\\pipe\\ChuniIIPipe"

SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
static unsigned __stdcall PipeThreadProc(void* arg) {
    while (!g_pipe_thread_stop) {
        HANDLE hPipe = CreateNamedPipeA(
            PIPE_NAME,
            PIPE_ACCESS_INBOUND,
            PIPE_TYPE_BYTE | PIPE_WAIT,
            1,
            0, 0, 0, &sa);

        if (hPipe == INVALID_HANDLE_VALUE) {
            Sleep(100);
            continue;
        }

        BOOL connected = ConnectNamedPipe(hPipe, NULL) ? TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);
        if (!connected) {
            CloseHandle(hPipe);
            Sleep(100);
            continue;
        }

        uint8_t buffer[34];
        DWORD bytesRead = 0;

        while (!g_pipe_thread_stop && ReadFile(hPipe, buffer, sizeof(buffer), &bytesRead, NULL)) {
            if (bytesRead == 34) {
                g_opbtn_state = buffer[0];
                g_beam_state = buffer[1];
                memcpy(g_slider_state, &buffer[2], 32);
            }
        }

        DisconnectNamedPipe(hPipe);
        CloseHandle(hPipe);
    }

    return 0;
}


static unsigned __stdcall SliderThreadProc(void* arg) {
    while (!g_slider_thread_stop) {
        if (g_slider_callback) {
            g_slider_callback(g_slider_state);
        }
        Sleep(1);
    }
    return 0;
}


uint16_t chuni_io_get_api_version(void) {
    return 0x0102;
}

HRESULT chuni_io_jvs_init(void) {
    return S_OK;
}

void chuni_io_jvs_poll(uint8_t* opbtn, uint8_t* beams) {
    if (opbtn) *opbtn = g_opbtn_state;
    if (beams) *beams = g_beam_state;
}

void chuni_io_jvs_read_coin_counter(uint16_t* total) {
    if (total) *total = 0;
}

HRESULT chuni_io_slider_init(void) {
    g_pipe_thread_stop = false;
    g_pipe_thread = (HANDLE)_beginthreadex(NULL, 0, PipeThreadProc, NULL, 0, NULL);
    return S_OK;
}

void chuni_io_slider_start(chuni_io_slider_callback_t cb) {
    g_slider_callback = cb;
    g_slider_thread_stop = false;
    g_slider_thread = (HANDLE)_beginthreadex(NULL, 0, SliderThreadProc, NULL, 0, NULL);
}

void chuni_io_slider_stop(void) {
    g_slider_thread_stop = true;
    if (g_slider_thread) {
        WaitForSingleObject(g_slider_thread, INFINITE);
        CloseHandle(g_slider_thread);
        g_slider_thread = NULL;
    }

    g_pipe_thread_stop = true;
    if (g_pipe_thread) {
        HANDLE hPipe = CreateFileA(PIPE_NAME, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (hPipe != INVALID_HANDLE_VALUE) {
            CloseHandle(hPipe);
        }
        WaitForSingleObject(g_pipe_thread, INFINITE);
        CloseHandle(g_pipe_thread);
        g_pipe_thread = NULL;
    }
}
void chuni_io_slider_set_leds(const uint8_t* rgb) {
    //TODO
    return;
}

HRESULT chuni_io_led_init(void) {
    return S_OK;
}

void chuni_io_led_set_colors(uint8_t board, uint8_t* rgb) {
    //TODO
    return;
}

