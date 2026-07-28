#include <gb/gb.h>
#include <gb/cgb.h>

#include <stdbool.h>
#include <stdio.h>

#include "dialog_manager.h"

// Window size
#define WINDOW_WIDTH 640
#define WINDOW_HEIGHT 480

// Image size
#define IMAGE_WIDTH 256
#define IMAGE_HEIGHT 256

#define nullptr NULL

static void dialog_manager_test(void)
{
    struct dialog_manager dm;
    struct binary_blob data;
    dialog_manager_init(&dm, &data);

    dialog_manager_random(&dm, npc_id_empty1, scene_id_empty3);

    dialog_manager_free(&dm);
}

const unsigned char blank_tile[] = {
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0};

void main(void)
{
    dialog_manager_test();
    
    DISPLAY_OFF;

    // Load blank tile
    set_bkg_data(0, 1, blank_tile);

    // Fill BG tilemap
    for (uint8_t y = 0; y < 18; y++)
    {
        for (uint8_t x = 0; x < 20; x++)
        {
            set_bkg_tile_xy(x, y, 0);
        }
    }

    // Print text FIRST
    printf("HELLO WORLD");

    // NOW set attributes (palette index 0)
    VBK_REG = 1;
    for (uint8_t y = 0; y < 18; y++)
    {
        for (uint8_t x = 0; x < 20; x++)
        {
            set_bkg_tile_xy(x, y, 0);
        }
    }
    VBK_REG = 0;

    UWORD bg_palette[] = {
        RGB(0, 0, 31),  // color 0: background (blue)
        RGB(0, 0, 0),   // color 1: unused
        RGB(0, 0, 0),   // color 2: unused
        RGB(31, 31, 31) // color 3: text (white)
    };
    set_bkg_palette(0, 1, bg_palette);

    DISPLAY_ON;

    while (1)
    {
        wait_vbl_done();
    }
}
