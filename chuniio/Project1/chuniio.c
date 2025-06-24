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

static HANDLE g_shm_thread = NULL;
static bool g_shm_thread_stop = false;

#define SHM_NAME "ChuniIOSharedMemory"

static HANDLE g_shm_handle = NULL;
static uint8_t* g_shm_ptr = NULL;


// reads shared memory and updates input states every frame
static unsigned __stdcall ShmThreadProc(void* arg) {
    //open up a named shared memory link
    g_shm_handle = CreateFileMappingA(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        34,
        SHM_NAME);
    //null handling is boring....
    if (!g_shm_handle) {
        OutputDebugStringA("ChuniVR: Could not create File Mapping.");
        return 1;
    }

    g_shm_ptr = (uint8_t*)MapViewOfFile(g_shm_handle, FILE_MAP_READ, 0, 0, 34);

    if (!g_shm_ptr) {
        OutputDebugStringA("ChuniVR: Failed to open Shared Memory");
        return 1;
    }

    OutputDebugStringA("ChuniVR: Shared Memory Mapped Successfully.\n");

    //every frame copy input from shared memory into their corresponding pointers
    //even if no input is there we should still send it through or chunithm thinks its dead
    while (!g_shm_thread_stop) {
        g_opbtn_state = g_shm_ptr[0];
        g_beam_state = g_shm_ptr[1];
        memcpy(g_slider_state, &g_shm_ptr[2], 32); // 2 bytes offset: first 2 are opbtn and beam
        Sleep(1); // 1ms = 1khz input
    }

    //once while loop stops we close the shared memory and stop the thread.
    UnmapViewOfFile(g_shm_ptr);
    CloseHandle(g_shm_handle);
    g_shm_ptr = NULL;
    g_shm_handle = NULL;

    return 0;
}

// runs the callback every frame with latest slider state
static unsigned __stdcall SliderThreadProc(void* arg) {
    while (!g_slider_thread_stop) {
        if (g_slider_callback) {
            g_slider_callback(g_slider_state);
        }
        Sleep(1);
    }
    return 0;
}

//latest version
uint16_t chuni_io_get_api_version(void) {
    return 0x0102;
}

HRESULT chuni_io_jvs_init(void) {
    //straight up copied from other chuniio lol
    led_init_mutex = CreateMutex(
        NULL,
        FALSE,
        NULL);

    if (led_init_mutex == NULL) {
        return E_FAIL;
    }

    return S_OK;
}

void chuni_io_jvs_poll(uint8_t* opbtn, uint8_t* beams) {
    if (opbtn) *opbtn |= g_opbtn_state;
    if (beams) *beams |= g_beam_state;
}

void chuni_io_jvs_read_coin_counter(uint16_t* total) {
    if (total) *total = 0;
}

HRESULT chuni_io_slider_init(void) {
    g_shm_thread_stop = false;
    //create thread to start reading shared memory.
    g_shm_thread = (HANDLE)_beginthreadex(NULL, 0, ShmThreadProc, NULL, 0, NULL);
    return S_OK;
}

void chuni_io_slider_start(chuni_io_slider_callback_t cb) {
    g_slider_callback = cb;
    g_slider_thread_stop = false;
    //create thread that sends input through from the shared memory read in previous thread.
    g_slider_thread = (HANDLE)_beginthreadex(NULL, 0, SliderThreadProc, NULL, 0, NULL);
}

//stop gets called in regular operation. DO NOT remove the restart functionality.
void chuni_io_slider_stop(void) {
    g_slider_thread_stop = true;
    if (g_slider_thread) {
        WaitForSingleObject(g_slider_thread, INFINITE);
        CloseHandle(g_slider_thread);
        g_slider_thread = NULL;
    }

    g_shm_thread_stop = true;
    if (g_shm_thread) {
        WaitForSingleObject(g_shm_thread, INFINITE);
        CloseHandle(g_shm_thread);
        g_shm_thread = NULL;
    }

    // restart threads if program is still running as chunithm calls the slider_stop function sometimes even when not closing
    g_slider_thread_stop = false;
    g_shm_thread_stop = false;

    g_shm_thread = (HANDLE)_beginthreadex(NULL, 0, ShmThreadProc, NULL, 0, NULL);

    if (g_slider_callback) {
        g_slider_thread = (HANDLE)_beginthreadex(NULL, 0, SliderThreadProc, NULL, 0, NULL);
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
