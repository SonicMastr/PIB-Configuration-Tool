#ifndef _COMMON_H_
#define _COMMON_H_

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <psp2/common_dialog.h>
#include <psp2/ime_dialog.h>
#include <psp2/gxm.h>
#include <psp2/ctrl.h>
#include <psp2/display.h>
#include <psp2/kernel/iofilemgr.h>
#include "../common/debugScreen.h"
#include "../common/debugScreen_custom.h"

typedef enum vitaEGLNativeWindowType {
    VITA_INVALID_WINDOW = 0,
    VITA_WINDOW_960X544,
    VITA_WINDOW_720X408,
    VITA_WINDOW_640X368,
    VITA_WINDOW_480X272,
    VITA_WINDOW_1280X720,
    VITA_WINDOW_1920X1080
} VitaEGLNativeWindowType;

#define VERSION 1.00f
#define printf psvDebugScreenPrintf
#define SUCCESS(a, args...) { psvDebugScreenSetFgColor(0x00FF00); printf(a, ## args); }
#define ERROR(a, args...) { psvDebugScreenSetFgColor(0xFF0000); printf(a, ## args); }
#define WARN(a, args...) { psvDebugScreenSetFgColor(0xFFFF00); printf(a, ## args); }
#define LOG(a, args...) { psvDebugScreenSetFgColor(0xFFFFFF); printf(a, ## args); }

#endif