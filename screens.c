#include <stdio.h>
#include <stdlib.h>
#include "headers/graphics_processor.h"
#include "headers/spritesRGB.h"
#include "headers/screensRGB.h"
#include "headers/screens.h"


void draw_initial_screen() {
    int i;
    for (i = 0; i < 4800; i++) {
        int R = initialScreen[i][0];
        int G = initialScreen[i][1];
        int B = initialScreen[i][2];
        set_background_block(i, R, G, B);
    }
}

void draw_pause() {
    
}