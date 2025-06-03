#include <windows.h>
#include <process.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "chuniio.h"
#include "ledoutput.h"

static uint8_t g_opbtn_state = 0;
static uint8_t g_beam_state = 0;
static uint8_t g_slider_state[32] = { 0 };

static chuni_io_slider_callback_t g_slider_callback = NULL;

static HANDLE g_slider_thread = NULL;
static bool g_slider_thread_stop = false;

static HANDLE g_pipe_thread = NULL;
static bool g_pipe_thread_stop = false;

#define SHM_NAME "ChuniIOSharedMemory"

static HANDLE g_shm_handle = NULL;
static uint8_t* g_shm_ptr = NULL;


static unsigned __stdcall ShmThreadProc(void* arg) {
    g_shm_handle = CreateFileMappingA(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        34,
        SHM_NAME);

    if (g_shm_handle == NULL) {
        DWORD error = GetLastError();
        char buffer[256];
        sprintf_s(buffer, sizeof(buffer), "ChuniVR: Shared Memory Creation Failed: %lu\n", error);
        OutputDebugStringA(buffer);
        return 1;
    }

    g_shm_ptr = (uint8_t*)MapViewOfFile(g_shm_handle, FILE_MAP_READ, 0, 0, 34);
    if (g_shm_ptr == NULL) {
        DWORD error = GetLastError();
        char buffer[256];
        sprintf_s(buffer, sizeof(buffer), "ChuniVR: MapViewOfFile Failed: %lu\n", error);
        OutputDebugStringA(buffer);
        CloseHandle(g_shm_handle);
        g_shm_handle = NULL;
        return 1;
    }

    OutputDebugStringA("ChuniVR: Shared Memory Mapped Successfully.\n");

    while (!g_pipe_thread_stop) {
        g_opbtn_state = g_shm_ptr[0];
        g_beam_state = g_shm_ptr[1];
        memcpy(g_slider_state, &g_shm_ptr[2], 32);
        //char debug[128];
        //sprintf_s(debug, sizeof(debug), "Slider[0] = %d\n", g_slider_state[0]);
        //OutputDebugStringA(debug);
        Sleep(1);
    }

    UnmapViewOfFile(g_shm_ptr);
    CloseHandle(g_shm_handle);
    g_shm_ptr = NULL;
    g_shm_handle = NULL;

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
    led_init_mutex = CreateMutex(
        NULL,
        FALSE,
        NULL);

    if (led_init_mutex == NULL)
    {
        return E_FAIL;
    }

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
    g_pipe_thread = (HANDLE)_beginthreadex(NULL, 0, ShmThreadProc, NULL, 0, NULL);
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
        WaitForSingleObject(g_pipe_thread, INFINITE);
        CloseHandle(g_pipe_thread);
        g_pipe_thread = NULL;
    }


    g_slider_thread_stop = false;
    g_pipe_thread_stop = false;

    g_pipe_thread = (HANDLE)_beginthreadex(NULL, 0, ShmThreadProc, NULL, 0, NULL);
    if (!g_pipe_thread) {
        OutputDebugStringA("Failed to restart pipe thread in chuni_io_slider_stop\n");
    }

    if (g_slider_callback) {
        g_slider_thread = (HANDLE)_beginthreadex(NULL, 0, SliderThreadProc, NULL, 0, NULL);
        if (!g_slider_thread) {
            OutputDebugStringA("Failed to restart slider thread in chuni_io_slider_stop\n");
        }
    }
}

void chuni_io_slider_set_leds(const uint8_t* rgb) {
    led_output_update(2, rgb);
}

HRESULT chuni_io_led_init(void) {
	return led_output_init();
}

void chuni_io_led_set_colors(uint8_t board, uint8_t* rgb) {
    led_output_update(board, rgb);
}

