#include "aimeio.h"
#include <windows.h>
#include <process.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
HANDLE hMapFile = NULL;
LPVOID lpBaseAddress = NULL;
#define SHM_NAME "ChuniVRAime"

static HANDLE g_shm_handle = NULL;
static uint8_t* g_shm_ptr = NULL;
static HANDLE g_pipe_thread = NULL;
BOOL g_pipe_thread_stop;
static uint8_t aime_io_aime_id[10];
bool is_aime_id_valid(const uint8_t* id, size_t len) {
    if (!id) return false;

    bool all_zero = true;
    bool all_ff = true;

    for (size_t i = 0; i < len; i++) {
        if (id[i] != 0x00) {
            all_zero = false;
        }
        if (id[i] != 0xFF) {
            all_ff = false;
        }
    }

    return !(all_zero || all_ff);
}


static unsigned __stdcall ShmThreadProc(void* arg) {
    g_shm_handle = CreateFileMappingA(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        10,
        SHM_NAME);

    if (g_shm_handle == NULL) {
        DWORD error = GetLastError();
        char buffer[256];
        sprintf_s(buffer, sizeof(buffer), "ChuniVR Aime: Shared Memory Creation Failed: %lu\n", error);
        OutputDebugStringA(buffer);
        return 1;
    }

    g_shm_ptr = (uint8_t*)MapViewOfFile(g_shm_handle, FILE_MAP_READ, 0, 0, 10);

    if (g_shm_ptr == NULL) {
        DWORD error = GetLastError();
        char buffer[256];
        sprintf_s(buffer, sizeof(buffer), "ChuniVR Aime: MapViewOfFile Failed: %lu\n", error);
        OutputDebugStringA(buffer);
        CloseHandle(g_shm_handle);
        g_shm_handle = NULL;
        return 1;
    }

    OutputDebugStringA("ChuniVR Aime: Shared Memory Mapped Successfully\n");

    while (!g_pipe_thread_stop) {
        if (g_shm_ptr != NULL) {
            bool valid = false;
            for (int i = 0; i < 10; i++) {
                if (g_shm_ptr[i] != 0x00 && g_shm_ptr[i] != 0xFF) {
                    valid = true;
                    break;
                }
            }

            if (valid) {
                memcpy(aime_io_aime_id, g_shm_ptr, 10);
            }
            else {
                memset(aime_io_aime_id, 0, 10);
            }
        }
        Sleep(10);
    }


    UnmapViewOfFile(g_shm_ptr);
    CloseHandle(g_shm_handle);
    g_shm_ptr = NULL;
    g_shm_handle = NULL;

    return 0;
}

uint16_t aime_io_get_api_version(void) {
	return 0x0100;
}

HRESULT aime_io_init(void) {
    g_pipe_thread_stop = false;

    g_pipe_thread = (HANDLE)_beginthreadex(NULL, 0, ShmThreadProc, NULL, 0, NULL);
    if (g_pipe_thread == NULL) {
        return E_FAIL;
    }
    return S_OK;
}


HRESULT aime_io_nfc_poll(uint8_t unit_no) {
	if (is_aime_id_valid(aime_io_aime_id, 10)) {
        return S_OK;
    }
    else {
        return S_FALSE;
	}
}



HRESULT aime_io_nfc_get_aime_id(uint8_t unit_no, uint8_t* luid, size_t luid_size) {
    if (unit_no != 0 || luid == NULL || luid_size != 10) {
        return E_INVALIDARG;
    }

    bool valid = false;
    for (int i = 0; i < 10; i++) {
        if (aime_io_aime_id[i] != 0x00 && aime_io_aime_id[i] != 0xFF) {
            valid = true;
            break;
        }
    }

    if (!valid) {
        return S_FALSE;
    }

    memcpy(luid, aime_io_aime_id, 10);
    return S_OK;
}




HRESULT aime_io_nfc_get_felica_id(uint8_t unit_no, uint64_t* IDm) {
	return S_FALSE;
}

void aime_io_led_set_color(uint8_t unit_no, uint8_t r, uint8_t g, uint8_t b) {
	return;
}