#include <ti/grlib/grlib.h>
#include "LcdDriver/Crystalfontz128x128_ST7735.h"
#include "LcdDriver/HAL_MSP_EXP432P401R_Crystalfontz128x128_ST7735.h"
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

#define MOVE 50 // 50ms move

Graphics_Context g_sContext;
extern tImage  favicon__1_4BPP_UNCOMP;
extern tImage  favicon__2_1BPP_UNCOMP;
extern tImage  tree8BPP_UNCOMP;
extern tImage  gascan4BPP_UNCOMP;
extern tImage  crash8BPP_UNCOMP;

void InitGraphics() {
    Crystalfontz128x128_Init();
    Crystalfontz128x128_SetOrientation(LCD_ORIENTATION_UP);
    Graphics_initContext(&g_sContext,
                         &g_sCrystalfontz128x128,
                         &g_sCrystalfontz128x128_funcs);
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_GREEN);
    Graphics_setBackgroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
    GrContextFontSet(&g_sContext, &g_sFontCmtt16);
    Graphics_clearDisplay(&g_sContext);
}

void LCDClearDisplay(int color) {
    Graphics_setBackgroundColor(&g_sContext, color);
    Graphics_clearDisplay(&g_sContext);
}


void LCDDrawChar(unsigned row, unsigned col, int8_t c) {
    Graphics_drawString(&g_sContext,
                        &c,
                        1,
                        8 * (col % 16),
                        16 * (row % 8),
                        OPAQUE_TEXT);
}

void drawRoad() {
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_WHITE);
    Graphics_drawLine(&g_sContext, 0, 0, 0, 127);
    Graphics_drawLine(&g_sContext, 50, 0, 50, 127);
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_GREEN);
}

void PrintString(char *str, int row, int col) {
    int i;
    for (i = 0; str[i] != '\0'; i++) {
        LCDDrawChar(row,  col, str[i]);
        col++;
        if (col >= 16) {
            col = 0;
            row++;
            if (row >= 8) {
                row = 0;
            }
        }
    }
}

void drawArrow(unsigned y) {
    PrintString(">", y, 1);
}

void clearArrow(unsigned y) {
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_BLACK);
    drawArrow(y);
    Graphics_setForegroundColor(&g_sContext, GRAPHICS_COLOR_GREEN);
}

void clearCar() {
    Graphics_drawImage(&g_sContext, &favicon__2_1BPP_UNCOMP, 1, 107);

}

void drawCar(unsigned x) {
    if(x == 1) {
        Graphics_drawImage(&g_sContext, &favicon__2_1BPP_UNCOMP, 1, 107);
        Graphics_drawImage(&g_sContext, &favicon__2_1BPP_UNCOMP, 2, 107);
    } else if(x == 33) {
        Graphics_drawImage(&g_sContext, &favicon__2_1BPP_UNCOMP, 33, 107);
        Graphics_drawImage(&g_sContext, &favicon__2_1BPP_UNCOMP, 32, 107);
    }else {
        Graphics_drawImage(&g_sContext, &favicon__2_1BPP_UNCOMP, x+1, 107);
        Graphics_drawImage(&g_sContext, &favicon__2_1BPP_UNCOMP, x-1, 107);
    }
    Graphics_drawImage(&g_sContext, &favicon__1_4BPP_UNCOMP, x, 107);
}

void clearTree(unsigned x, unsigned y) {
    if(x == 1) {
        Graphics_drawImage(&g_sContext, &favicon__2_1BPP_UNCOMP, 1, y);
        Graphics_drawImage(&g_sContext, &favicon__2_1BPP_UNCOMP, 1, y-4);
    } else if(x == 2) {
        Graphics_drawImage(&g_sContext, &favicon__2_1BPP_UNCOMP, 17, y);
        Graphics_drawImage(&g_sContext, &favicon__2_1BPP_UNCOMP, 17, y-4);
    } else if(x == 3) {
        Graphics_drawImage(&g_sContext, &favicon__2_1BPP_UNCOMP, 33, y);
        Graphics_drawImage(&g_sContext, &favicon__2_1BPP_UNCOMP, 33, y-4);
    }

}

void drawTree(unsigned x, unsigned y) {
    if(x == 1) {
        Graphics_drawImage(&g_sContext, &favicon__2_1BPP_UNCOMP, 1, y-1);
        Graphics_drawImage(&g_sContext, &tree8BPP_UNCOMP, 1, y);
    } else if(x == 2) {
        Graphics_drawImage(&g_sContext, &favicon__2_1BPP_UNCOMP, 17, y-1);
        Graphics_drawImage(&g_sContext, &tree8BPP_UNCOMP, 17, y);
    } else if(x == 3) {
        Graphics_drawImage(&g_sContext, &favicon__2_1BPP_UNCOMP, 33, y-1);
        Graphics_drawImage(&g_sContext, &tree8BPP_UNCOMP, 33, y);
    }
}

void drawGC(unsigned x, unsigned y) {
    if(x == 1) {
        Graphics_drawImage(&g_sContext, &favicon__2_1BPP_UNCOMP, 1, y-1);
        Graphics_drawImage(&g_sContext, &gascan4BPP_UNCOMP, 1, y);
    } else if(x == 2) {
        Graphics_drawImage(&g_sContext, &favicon__2_1BPP_UNCOMP, 17, y-1);
        Graphics_drawImage(&g_sContext, &gascan4BPP_UNCOMP, 17, y);
    } else if(x == 3) {
        Graphics_drawImage(&g_sContext, &favicon__2_1BPP_UNCOMP, 33, y-1);
        Graphics_drawImage(&g_sContext, &gascan4BPP_UNCOMP, 33, y);
    }
}

void drawCrash() {
    Graphics_drawImage(&g_sContext, &crash8BPP_UNCOMP, 0, 0);
}
