/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-18     Md. Khairul Alam       the first version
 */

#include <pico/stdlib.h>
#include <hardware/i2c.h>
#include <pico/binary_info.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

typedef enum {
    SET_CONTRAST = 0x81,
    SET_ENTIRE_ON = 0xA4,
    SET_NORM_INV = 0xA6,
    SET_DISP = 0xAE,
    SET_MEM_ADDR = 0x20,
    SET_COL_ADDR = 0x21,
    SET_PAGE_ADDR = 0x22,
    SET_DISP_START_LINE = 0x40,
    SET_SEG_REMAP = 0xA0,
    SET_MUX_RATIO = 0xA8,
    SET_COM_OUT_DIR = 0xC0,
    SET_DISP_OFFSET = 0xD3,
    SET_COM_PIN_CFG = 0xDA,
    SET_DISP_CLK_DIV = 0xD5,
    SET_PRECHARGE = 0xD9,
    SET_VCOM_DESEL = 0xDB,
    SET_CHARGE_PUMP = 0x8D
} ssd1306_command_t;


typedef struct {
    uint8_t width;      /**< width of display */
    uint8_t height;     /**< height of display */
    uint8_t pages;      /**< stores pages of display (calculated on initialization*/
    uint8_t address;    /**< i2c address of display*/
    i2c_inst_t *i2c_i;  /**< i2c connection instance */
    bool external_vcc;  /**< whether display uses external vcc */
    uint8_t *buffer;    /**< display buffer */
    size_t bufsize;     /**< buffer size */
} ssd1306_t;

const uint8_t font_8x5[] =
{
    8, 5, 1, 32, 126,
    0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x5F, 0x00, 0x00,
    0x00, 0x07, 0x00, 0x07, 0x00,
    0x14, 0x7F, 0x14, 0x7F, 0x14,
    0x24, 0x2A, 0x7F, 0x2A, 0x12,
    0x23, 0x13, 0x08, 0x64, 0x62,
    0x36, 0x49, 0x56, 0x20, 0x50,
    0x00, 0x08, 0x07, 0x03, 0x00,
    0x00, 0x1C, 0x22, 0x41, 0x00,
    0x00, 0x41, 0x22, 0x1C, 0x00,
    0x2A, 0x1C, 0x7F, 0x1C, 0x2A,
    0x08, 0x08, 0x3E, 0x08, 0x08,
    0x00, 0x80, 0x70, 0x30, 0x00,
    0x08, 0x08, 0x08, 0x08, 0x08,
    0x00, 0x00, 0x60, 0x60, 0x00,
    0x20, 0x10, 0x08, 0x04, 0x02,
    0x3E, 0x51, 0x49, 0x45, 0x3E,
    0x00, 0x42, 0x7F, 0x40, 0x00,
    0x72, 0x49, 0x49, 0x49, 0x46,
    0x21, 0x41, 0x49, 0x4D, 0x33,
    0x18, 0x14, 0x12, 0x7F, 0x10,
    0x27, 0x45, 0x45, 0x45, 0x39,
    0x3C, 0x4A, 0x49, 0x49, 0x31,
    0x41, 0x21, 0x11, 0x09, 0x07,
    0x36, 0x49, 0x49, 0x49, 0x36,
    0x46, 0x49, 0x49, 0x29, 0x1E,
    0x00, 0x00, 0x14, 0x00, 0x00,
    0x00, 0x40, 0x34, 0x00, 0x00,
    0x00, 0x08, 0x14, 0x22, 0x41,
    0x14, 0x14, 0x14, 0x14, 0x14,
    0x00, 0x41, 0x22, 0x14, 0x08,
    0x02, 0x01, 0x59, 0x09, 0x06,
    0x3E, 0x41, 0x5D, 0x59, 0x4E,
    0x7C, 0x12, 0x11, 0x12, 0x7C,
    0x7F, 0x49, 0x49, 0x49, 0x36,
    0x3E, 0x41, 0x41, 0x41, 0x22,
    0x7F, 0x41, 0x41, 0x41, 0x3E,
    0x7F, 0x49, 0x49, 0x49, 0x41,
    0x7F, 0x09, 0x09, 0x09, 0x01,
    0x3E, 0x41, 0x41, 0x51, 0x73,
    0x7F, 0x08, 0x08, 0x08, 0x7F,
    0x00, 0x41, 0x7F, 0x41, 0x00,
    0x20, 0x40, 0x41, 0x3F, 0x01,
    0x7F, 0x08, 0x14, 0x22, 0x41,
    0x7F, 0x40, 0x40, 0x40, 0x40,
    0x7F, 0x02, 0x1C, 0x02, 0x7F,
    0x7F, 0x04, 0x08, 0x10, 0x7F,
    0x3E, 0x41, 0x41, 0x41, 0x3E,
    0x7F, 0x09, 0x09, 0x09, 0x06,
    0x3E, 0x41, 0x51, 0x21, 0x5E,
    0x7F, 0x09, 0x19, 0x29, 0x46,
    0x26, 0x49, 0x49, 0x49, 0x32,
    0x03, 0x01, 0x7F, 0x01, 0x03,
    0x3F, 0x40, 0x40, 0x40, 0x3F,
    0x1F, 0x20, 0x40, 0x20, 0x1F,
    0x3F, 0x40, 0x38, 0x40, 0x3F,
    0x63, 0x14, 0x08, 0x14, 0x63,
    0x03, 0x04, 0x78, 0x04, 0x03,
    0x61, 0x59, 0x49, 0x4D, 0x43,
    0x00, 0x7F, 0x41, 0x41, 0x41,
    0x02, 0x04, 0x08, 0x10, 0x20,
    0x00, 0x41, 0x41, 0x41, 0x7F,
    0x04, 0x02, 0x01, 0x02, 0x04,
    0x40, 0x40, 0x40, 0x40, 0x40,
    0x00, 0x03, 0x07, 0x08, 0x00,
    0x20, 0x54, 0x54, 0x78, 0x40,
    0x7F, 0x28, 0x44, 0x44, 0x38,
    0x38, 0x44, 0x44, 0x44, 0x28,
    0x38, 0x44, 0x44, 0x28, 0x7F,
    0x38, 0x54, 0x54, 0x54, 0x18,
    0x00, 0x08, 0x7E, 0x09, 0x02,
    0x18, 0xA4, 0xA4, 0x9C, 0x78,
    0x7F, 0x08, 0x04, 0x04, 0x78,
    0x00, 0x44, 0x7D, 0x40, 0x00,
    0x20, 0x40, 0x40, 0x3D, 0x00,
    0x7F, 0x10, 0x28, 0x44, 0x00,
    0x00, 0x41, 0x7F, 0x40, 0x00,
    0x7C, 0x04, 0x78, 0x04, 0x78,
    0x7C, 0x08, 0x04, 0x04, 0x78,
    0x38, 0x44, 0x44, 0x44, 0x38,
    0xFC, 0x18, 0x24, 0x24, 0x18,
    0x18, 0x24, 0x24, 0x18, 0xFC,
    0x7C, 0x08, 0x04, 0x04, 0x08,
    0x48, 0x54, 0x54, 0x54, 0x24,
    0x04, 0x04, 0x3F, 0x44, 0x24,
    0x3C, 0x40, 0x40, 0x20, 0x7C,
    0x1C, 0x20, 0x40, 0x20, 0x1C,
    0x3C, 0x40, 0x30, 0x40, 0x3C,
    0x44, 0x28, 0x10, 0x28, 0x44,
    0x4C, 0x90, 0x90, 0x90, 0x7C,
    0x44, 0x64, 0x54, 0x4C, 0x44,
    0x00, 0x08, 0x36, 0x41, 0x00,
    0x00, 0x00, 0x77, 0x00, 0x00,
    0x00, 0x41, 0x36, 0x08, 0x00,
    0x02, 0x01, 0x02, 0x04, 0x02,
};

const uint8_t acme_font[] = {
    8, 6, 1, 32, 126,
    0x00,0x00,0x00,0x00,0x00,0x00, //
    0x7f,0x51,0x7f,0x00,0x00,0x00, // !
    0x0f,0x09,0x0f,0x09,0x0f,0x00, // "
    0x3e,0x6b,0x41,0x6b,0x41,0x6b, // #
    0x7f,0xd1,0x94,0xc5,0x7f,0x00, // $
    0x77,0x4d,0x77,0x59,0x77,0x00, // %
    0x7f,0x49,0x55,0x49,0x6f,0x54, // &
    0x0f,0x09,0x0f,0x00,0x00,0x00, // '
    0x7f,0xc1,0xbe,0xe3,0x00,0x00, // (
    0xe3,0xbe,0xc1,0x7f,0x00,0x00, // )
    0x3e,0x2a,0x77,0x41,0x77,0x2a, // *
    0x1c,0x14,0x77,0x41,0x77,0x14, // +
    0xe0,0xb0,0xd0,0x70,0x00,0x00, // ,
    0x1c,0x14,0x14,0x14,0x1c,0x00, // -
    0x70,0x50,0x70,0x00,0x00,0x00, // .
    0x78,0x4c,0x77,0x19,0x0f,0x00, // /
    0x7f,0x41,0x5d,0x41,0x7f,0x00, // 0
    0x7f,0x41,0x7f,0x00,0x00,0x00, // 1
    0x7f,0x45,0x55,0x51,0x7f,0x00, // 2
    0x7f,0x55,0x55,0x41,0x7f,0x00, // 3
    0x1f,0x11,0x77,0x41,0x7f,0x00, // 4
    0x7f,0x51,0x55,0x45,0x7f,0x00, // 5
    0x7f,0x41,0x55,0x45,0x7f,0x00, // 6
    0x07,0x7d,0x45,0x79,0x0f,0x00, // 7
    0x7f,0x49,0x55,0x49,0x7f,0x00, // 8
    0x7f,0x51,0x55,0x41,0x7f,0x00, // 9
    0x3e,0x2a,0x3e,0x00,0x00,0x00, // :
    0xe0,0xbe,0xda,0x7e,0x00,0x00, // ;
    0x1c,0x36,0x6b,0x5d,0x77,0x00, // <
    0x3e,0x2a,0x2a,0x2a,0x2a,0x3e, // =
    0x77,0x5d,0x6b,0x36,0x1c,0x00, // >
    0x07,0x7d,0x55,0x71,0x1f,0x00, // ?
    0x7f,0x41,0x5d,0x55,0x51,0x7f, // @
    0x7f,0x41,0x75,0x41,0x7f,0x00, // A
    0x7f,0x41,0x55,0x41,0x7f,0x00, // B
    0x7f,0x41,0x5d,0x55,0x77,0x00, // C
    0x7f,0x41,0x5d,0x63,0x3e,0x00, // D
    0x7f,0x41,0x55,0x55,0x7f,0x00, // E
    0x7f,0x41,0x75,0x15,0x1f,0x00, // F
    0x7f,0x41,0x5d,0x45,0x7f,0x00, // G
    0x7f,0x41,0x77,0x41,0x7f,0x00, // H
    0x7f,0x41,0x7f,0x00,0x00,0x00, // I
    0x78,0x48,0x5f,0x41,0x7f,0x00, // J
    0x7f,0x41,0x77,0x49,0x7f,0x00, // K
    0x7f,0x41,0x5f,0x50,0x70,0x00, // L
    0x7f,0x41,0x3b,0x3b,0x41,0x7f, // M
    0x7f,0x41,0x3b,0x76,0x41,0x7f, // N
    0x7f,0x41,0x5d,0x41,0x7f,0x00, // O
    0x7f,0x41,0x75,0x11,0x1f,0x00, // P
    0x7f,0x41,0x1d,0x41,0x7f,0x00, // Q
    0x7f,0x41,0x6d,0x51,0x6f,0x00, // R
    0x7f,0x51,0x55,0x45,0x7f,0x00, // S
    0x07,0x7d,0x41,0x7d,0x07,0x00, // T
    0x7f,0x41,0x5f,0x41,0x7f,0x00, // U
    0x3f,0x61,0x5f,0x61,0x3f,0x00, // V
    0x7f,0x41,0x6e,0x6e,0x41,0x7f, // W
    0x7f,0x49,0x77,0x49,0x7f,0x00, // X
    0x1f,0x71,0x47,0x71,0x1f,0x00, // Y
    0x7b,0x4d,0x55,0x59,0x6f,0x00, // Z
    0xff,0x80,0xbe,0xe3,0x00,0x00, // [
    0x0f,0x19,0x77,0x4c,0x78,0x00, // "\"
    0xe3,0xbe,0x80,0xff,0x00,0x00, // ]
    0x0e,0x0b,0x0d,0x0b,0x0e,0x00, // ^
    0xe0,0xa0,0xa0,0xa0,0xa0,0xe0, // _
    0x07,0x0d,0x0b,0x0e,0x00,0x00, // `
    0x7f,0x41,0x75,0x41,0x7f,0x00, // a
    0x7f,0x41,0x55,0x41,0x7f,0x00, // b
    0x7f,0x41,0x5d,0x55,0x77,0x00, // c
    0x7f,0x41,0x5d,0x63,0x3e,0x00, // d
    0x7f,0x41,0x55,0x55,0x7f,0x00, // e
    0x7f,0x41,0x75,0x15,0x1f,0x00, // f
    0x7f,0x41,0x5d,0x45,0x7f,0x00, // g
    0x7f,0x41,0x77,0x41,0x7f,0x00, // h
    0x7f,0x41,0x7f,0x00,0x00,0x00, // i
    0x78,0x48,0x5f,0x41,0x7f,0x00, // j
    0x7f,0x41,0x77,0x49,0x7f,0x00, // k
    0x7f,0x41,0x5f,0x50,0x70,0x00, // l
    0x7f,0x41,0x3b,0x3b,0x41,0x7f, // m
    0x7f,0x41,0x3b,0x76,0x41,0x7f, // n
    0x7f,0x41,0x5d,0x41,0x7f,0x00, // o
    0x7f,0x41,0x75,0x11,0x1f,0x00, // p
    0x7f,0x41,0x1d,0x41,0x7f,0x00, // q
    0x7f,0x41,0x6d,0x51,0x6f,0x00, // r
    0x7f,0x51,0x55,0x45,0x7f,0x00, // s
    0x07,0x7d,0x41,0x7d,0x07,0x00, // t
    0x7f,0x41,0x5f,0x41,0x7f,0x00, // u
    0x3f,0x61,0x5f,0x61,0x3f,0x00, // v
    0x7f,0x41,0x6e,0x6e,0x41,0x7f, // w
    0x7f,0x49,0x77,0x49,0x7f,0x00, // x
    0x1f,0x71,0x47,0x71,0x1f,0x00, // y
    0x7b,0x4d,0x55,0x59,0x6f,0x00, // z
    0x1c,0xf7,0x88,0xbe,0xe3,0x00, // {
    0xff,0x80,0xff,0x00,0x00,0x00, // |
    0xe3,0xbe,0x88,0xf7,0x1c,0x00, // }
    0x0f,0x09,0x0d,0x09,0x0b,0x09, // ~
    0x00,0x00,0x00,0x00,0x00,0x00
};

const uint8_t bubblesstandard_font[] = {
    8, 7, 0, 32, 126,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00, //
    0xbf,0x00,0x00,0x00,0x00,0x00,0x00, // !
    0x03,0x00,0x03,0x00,0x00,0x00,0x00, // "
    0x24,0x7e,0x24,0x24,0x7e,0x24,0x00, // #
    0x44,0x4a,0xd3,0x22,0x00,0x00,0x00, // $
    0x82,0x60,0x18,0x06,0x21,0x00,0x00, // %
    0x60,0x90,0x8e,0xb9,0x46,0xb0,0x00, // &
    0x03,0x00,0x00,0x00,0x00,0x00,0x00, // '
    0x7e,0x81,0x81,0x00,0x00,0x00,0x00, // (
    0x81,0x81,0x7e,0x00,0x00,0x00,0x00, // )
    0x02,0x0e,0x05,0x0e,0x02,0x00,0x00, // *
    0x10,0x38,0x10,0x00,0x00,0x00,0x00, // +
    0x80,0x00,0x00,0x00,0x00,0x00,0x00, // ,
    0x10,0x10,0x10,0x00,0x00,0x00,0x00, // -
    0x80,0x00,0x00,0x00,0x00,0x00,0x00, // .
    0x80,0x60,0x18,0x06,0x01,0x00,0x00, // /
    0x7e,0x81,0x81,0x7e,0x00,0x00,0x00, // 0
    0x88,0x86,0x7f,0x40,0x00,0x00,0x00, // 1
    0x82,0xc5,0xb1,0x8e,0x40,0x00,0x00, // 2
    0x66,0x81,0x89,0x76,0x00,0x00,0x00, // 3
    0x10,0x18,0x16,0xff,0x08,0x00,0x00, // 4
    0x4e,0x8a,0x8a,0x91,0x61,0x00,0x00, // 5
    0x20,0x78,0x96,0x91,0x60,0x00,0x00, // 6
    0x02,0x02,0xc1,0x39,0x07,0x00,0x00, // 7
    0x76,0x89,0x89,0x76,0x00,0x00,0x00, // 8
    0x06,0x09,0x09,0xfe,0x00,0x00,0x00, // 9
    0x81,0x00,0x00,0x00,0x00,0x00,0x00, // :
    0x81,0x00,0x00,0x00,0x00,0x00,0x00, // ;
    0x10,0x28,0x00,0x00,0x00,0x00,0x00, // <
    0x28,0x28,0x28,0x00,0x00,0x00,0x00, // =
    0x28,0x10,0x00,0x00,0x00,0x00,0x00, // >
    0x06,0x01,0xb1,0x0e,0x00,0x00,0x00, // ?
    0x3c,0x42,0x99,0xa5,0xbd,0xa2,0x1c, // @
    0x50,0x30,0x1c,0x23,0x5c,0xe0,0x00, // A
    0x82,0x7f,0x49,0x36,0x00,0x00,0x00, // B
    0x3c,0x42,0x81,0x89,0x86,0x60,0x00, // C
    0x42,0xff,0xa1,0xa1,0x82,0x7c,0x00, // D
    0x76,0x89,0x81,0x8a,0x70,0x00,0x00, // E
    0x90,0x70,0x1e,0x0a,0x0a,0x01,0x01, // F
    0x3c,0x42,0x81,0xb1,0xaa,0x48,0x30, // G
    0x45,0x3e,0x08,0x1c,0x6a,0x80,0x00, // H
    0x84,0x82,0xff,0x41,0x40,0x00,0x00, // I
    0x60,0x90,0x81,0x7f,0x02,0x00,0x00, // J
    0x7e,0x08,0x16,0x61,0x80,0x00,0x00, // K
    0x02,0xff,0x80,0x40,0x40,0x20,0x00, // L
    0x01,0x7e,0x01,0x06,0x18,0x06,0x01, // M
    0x80,0x7f,0x02,0x0c,0x30,0x40,0x3e, // N
    0x3c,0x42,0x8d,0x83,0x42,0x3c,0x00, // O
    0x8a,0x7f,0x11,0x11,0x0e,0x00,0x00, // P
    0x3c,0x42,0x81,0xa3,0xa3,0x45,0xb8, // Q
    0x92,0x7f,0x09,0x09,0x36,0x40,0x00, // R
    0x60,0x8e,0x91,0xa1,0x46,0x00,0x00, // S
    0x02,0x02,0x7f,0x81,0x01,0x00,0x00, // T
    0x3e,0x41,0x80,0x80,0x80,0x42,0x3c, // U
    0x01,0x06,0x38,0xc0,0x30,0x0e,0x01, // V
    0x01,0x0e,0x30,0x40,0x20,0x18,0x60, // W
    0x82,0x80,0x41,0x32,0x0c,0x38,0x44, // X
    0x01,0x0e,0xf0,0x0c,0x03,0x00,0x00, // Y
    0x04,0xc2,0xb2,0x8d,0x43,0x40,0x20, // Z
    0xff,0x81,0x00,0x00,0x00,0x00,0x00, // [
    0x80,0x60,0x18,0x06,0x01,0x00,0x00, // "\"
    0x81,0xff,0x00,0x00,0x00,0x00,0x00, // ]
    0x08,0x06,0x01,0x0e,0x10,0x00,0x00, // ^
    0x80,0x80,0x80,0x00,0x00,0x00,0x00, // _
    0x03,0x04,0x00,0x00,0x00,0x00,0x00, // `
    0x64,0x92,0x92,0x92,0xfc,0x90,0x00, // a
    0x50,0xfe,0x89,0x88,0x70,0x00,0x00, // b
    0x78,0x84,0x82,0x82,0x84,0x60,0x00, // c
    0x70,0x88,0x88,0xfe,0x51,0x00,0x00, // d
    0x7c,0x92,0x92,0x8a,0x64,0x10,0x00, // e
    0x90,0x7e,0x09,0x01,0x00,0x00,0x00, // f
    0x00,0x78,0x84,0x84,0xfe,0x44,0x00, // g
    0x01,0x7e,0x08,0x08,0x70,0x80,0x00, // h
    0x04,0x7d,0x80,0x00,0x00,0x00,0x00, // i
    0x00,0xfd,0x04,0x00,0x00,0x00,0x00, // j
    0x81,0x7e,0x08,0x34,0x40,0x00,0x00, // k
    0x01,0x7e,0x80,0x00,0x00,0x00,0x00, // l
    0x3e,0x04,0x04,0x18,0x04,0x04,0x78, // m
    0x7e,0x04,0x04,0x78,0x80,0x00,0x00, // n
    0x7c,0x82,0x86,0x84,0x78,0x00,0x00, // o
    0x48,0xfe,0x84,0x84,0x78,0x00,0x00, // p
    0x78,0x84,0x84,0xfe,0x48,0x00,0x00, // q
    0x82,0x7c,0x02,0x02,0x04,0x00,0x00, // r
    0x60,0x8c,0x92,0x62,0x04,0x00,0x00, // s
    0x08,0x7f,0x84,0x90,0x60,0x00,0x00, // t
    0x02,0x7c,0x80,0x80,0x80,0x78,0x80, // u
    0x02,0x0c,0x70,0x80,0x78,0x04,0x00, // v
    0x02,0x0c,0x70,0x80,0x60,0x18,0x60, // w
    0x86,0x64,0x18,0x66,0x80,0x00,0x00, // x
    0x02,0x7c,0x80,0x80,0xf8,0x00,0x00, // y
    0x08,0x84,0xc4,0xb2,0x8e,0x40,0x00, // z
    0xee,0x01,0x00,0x00,0x00,0x00,0x00, // {
    0xff,0x00,0x00,0x00,0x00,0x00,0x00, // |
    0x01,0xee,0x10,0x00,0x00,0x00,0x00, // }
    0x10,0x08,0x08,0x10,0x10,0x08,0x00, // ~
    0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

const uint8_t BMSPA_font[] = {
    8, 8, 0, 32, 126,
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, //
    0x00,0x5f,0x00,0x00,0x00,0x00,0x00,0x00, // !
    0x00,0x03,0x00,0x03,0x00,0x00,0x00,0x00, // "
    0x0a,0x1f,0x0a,0x1f,0x0a,0x00,0x00,0x00, // #
    0x24,0x2a,0x2a,0x7f,0x2a,0x2a,0x12,0x00, // $
    0x00,0x47,0x25,0x17,0x08,0x74,0x52,0x71, // %
    0x00,0x36,0x49,0x49,0x49,0x41,0x41,0x38, // &
    0x00,0x03,0x00,0x00,0x00,0x00,0x00,0x00, // '
    0x00,0x3e,0x41,0x00,0x00,0x00,0x00,0x00, // (
    0x41,0x3e,0x00,0x00,0x00,0x00,0x00,0x00, // )
    0x04,0x15,0x0e,0x15,0x04,0x00,0x00,0x00, // *
    0x08,0x08,0x3e,0x08,0x08,0x00,0x00,0x00, // +
    0x00,0xc0,0x00,0x00,0x00,0x00,0x00,0x00, // ,
    0x08,0x08,0x08,0x08,0x08,0x00,0x00,0x00, // -
    0x40,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // .
    0x40,0x20,0x10,0x08,0x04,0x02,0x01,0x00, // /
    0x00,0x3e,0x61,0x51,0x49,0x45,0x43,0x3e, // 0
    0x01,0x01,0x7e,0x00,0x00,0x00,0x00,0x00, // 1
    0x00,0x71,0x49,0x49,0x49,0x49,0x49,0x46, // 2
    0x41,0x49,0x49,0x49,0x49,0x49,0x36,0x00, // 3
    0x00,0x0f,0x10,0x10,0x10,0x10,0x10,0x7f, // 4
    0x00,0x4f,0x49,0x49,0x49,0x49,0x49,0x31, // 5
    0x00,0x3e,0x49,0x49,0x49,0x49,0x49,0x30, // 6
    0x01,0x01,0x01,0x01,0x01,0x01,0x7e,0x00, // 7
    0x00,0x36,0x49,0x49,0x49,0x49,0x49,0x36, // 8
    0x00,0x06,0x49,0x49,0x49,0x49,0x49,0x3e, // 9
    0x14,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // :
    0x40,0x34,0x00,0x00,0x00,0x00,0x00,0x00, // ;
    0x08,0x14,0x22,0x00,0x00,0x00,0x00,0x00, // <
    0x14,0x14,0x14,0x14,0x14,0x00,0x00,0x00, // =
    0x22,0x14,0x08,0x00,0x00,0x00,0x00,0x00, // >
    0x00,0x06,0x01,0x01,0x59,0x09,0x09,0x06, // ?
    0x00,0x3e,0x41,0x5d,0x55,0x5d,0x51,0x5e, // @
    0x00,0x7e,0x01,0x09,0x09,0x09,0x09,0x7e, // A
    0x00,0x7f,0x41,0x49,0x49,0x49,0x49,0x36, // B
    0x00,0x3e,0x41,0x41,0x41,0x41,0x41,0x22, // C
    0x00,0x7f,0x41,0x41,0x41,0x41,0x41,0x3e, // D
    0x00,0x3e,0x49,0x49,0x49,0x49,0x49,0x41, // E
    0x00,0x7e,0x09,0x09,0x09,0x09,0x09,0x01, // F
    0x00,0x3e,0x41,0x49,0x49,0x49,0x49,0x79, // G
    0x00,0x7f,0x08,0x08,0x08,0x08,0x08,0x7f, // H
    0x00,0x7f,0x00,0x00,0x00,0x00,0x00,0x00, // I
    0x00,0x38,0x40,0x40,0x41,0x41,0x41,0x3f, // J
    0x00,0x7f,0x08,0x08,0x08,0x0c,0x0a,0x71, // K
    0x00,0x3f,0x40,0x40,0x40,0x40,0x40,0x40, // L
    0x00,0x7e,0x01,0x01,0x7e,0x01,0x01,0x7e, // M
    0x00,0x7e,0x01,0x01,0x3e,0x40,0x40,0x3f, // N
    0x00,0x3e,0x41,0x41,0x41,0x41,0x41,0x3e, // O
    0x00,0x7e,0x09,0x09,0x09,0x09,0x09,0x06, // P
    0x00,0x3e,0x41,0x41,0x71,0x51,0x51,0x7e, // Q
    0x00,0x7e,0x01,0x31,0x49,0x49,0x49,0x46, // R
    0x00,0x46,0x49,0x49,0x49,0x49,0x49,0x31, // S
    0x01,0x01,0x01,0x7f,0x01,0x01,0x01,0x00, // T
    0x00,0x3f,0x40,0x40,0x40,0x40,0x40,0x3f, // U
    0x00,0x0f,0x10,0x20,0x40,0x20,0x10,0x0f, // V
    0x00,0x3f,0x40,0x40,0x3f,0x40,0x40,0x3f, // W
    0x00,0x63,0x14,0x08,0x08,0x08,0x14,0x63, // X
    0x00,0x07,0x08,0x08,0x78,0x08,0x08,0x07, // Y
    0x00,0x71,0x49,0x49,0x49,0x49,0x49,0x47, // Z
    0x00,0x7f,0x41,0x00,0x00,0x00,0x00,0x00, // [
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // "\"
    0x41,0x7f,0x00,0x00,0x00,0x00,0x00,0x00, // ]
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // ^
    0x40,0x40,0x40,0x40,0x40,0x40,0x40,0x00, // _
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // `
    0x00,0x7e,0x01,0x09,0x09,0x09,0x09,0x7e, // A
    0x00,0x7f,0x41,0x49,0x49,0x49,0x49,0x36, // B
    0x00,0x3e,0x41,0x41,0x41,0x41,0x41,0x22, // C
    0x00,0x7f,0x41,0x41,0x41,0x41,0x41,0x3e, // D
    0x00,0x3e,0x49,0x49,0x49,0x49,0x49,0x41, // E
    0x00,0x7e,0x09,0x09,0x09,0x09,0x09,0x01, // F
    0x00,0x3e,0x41,0x49,0x49,0x49,0x49,0x79, // G
    0x00,0x7f,0x08,0x08,0x08,0x08,0x08,0x7f, // H
    0x00,0x7f,0x00,0x00,0x00,0x00,0x00,0x00, // I
    0x00,0x38,0x40,0x40,0x41,0x41,0x41,0x3f, // J
    0x00,0x7f,0x08,0x08,0x08,0x0c,0x0a,0x71, // K
    0x00,0x3f,0x40,0x40,0x40,0x40,0x40,0x40, // L
    0x00,0x7e,0x01,0x01,0x7e,0x01,0x01,0x7e, // M
    0x00,0x7e,0x01,0x01,0x3e,0x40,0x40,0x3f, // N
    0x00,0x3e,0x41,0x41,0x41,0x41,0x41,0x3e, // O
    0x00,0x7e,0x09,0x09,0x09,0x09,0x09,0x06, // P
    0x00,0x3e,0x41,0x41,0x71,0x51,0x51,0x7e, // Q
    0x00,0x7e,0x01,0x31,0x49,0x49,0x49,0x46, // R
    0x00,0x46,0x49,0x49,0x49,0x49,0x49,0x31, // S
    0x01,0x01,0x01,0x7f,0x01,0x01,0x01,0x00, // T
    0x00,0x3f,0x40,0x40,0x40,0x40,0x40,0x3f, // U
    0x00,0x0f,0x10,0x20,0x40,0x20,0x10,0x0f, // V
    0x00,0x3f,0x40,0x40,0x3f,0x40,0x40,0x3f, // W
    0x00,0x63,0x14,0x08,0x08,0x08,0x14,0x63, // X
    0x00,0x07,0x08,0x08,0x78,0x08,0x08,0x07, // Y
    0x00,0x71,0x49,0x49,0x49,0x49,0x49,0x47, // Z
    0x08,0x36,0x41,0x00,0x00,0x00,0x00,0x00, // {
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00, // |
    0x41,0x36,0x08,0x00,0x00,0x00,0x00,0x00, // }
    0x02,0x01,0x01,0x02,0x02,0x01,0x00,0x00, // ~
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};


bool ssd1306_init(ssd1306_t *p, uint16_t width, uint16_t height, uint8_t address, i2c_inst_t *i2c_instance);
void ssd1306_deinit(ssd1306_t *p);
void ssd1306_poweroff(ssd1306_t *p);
void ssd1306_poweron(ssd1306_t *p);
void ssd1306_contrast(ssd1306_t *p, uint8_t val);
void ssd1306_invert(ssd1306_t *p, uint8_t inv);
void ssd1306_show(ssd1306_t *p);
void ssd1306_clear(ssd1306_t *p);
void ssd1306_clear_pixel(ssd1306_t *p, uint32_t x, uint32_t y);
void ssd1306_draw_pixel(ssd1306_t *p, uint32_t x, uint32_t y);
void ssd1306_draw_line(ssd1306_t *p, int32_t x1, int32_t y1, int32_t x2, int32_t y2);
void ssd1306_draw_square(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
void ssd1306_draw_empty_square(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t width, uint32_t height);
void ssd1306_bmp_show_image_with_offset(ssd1306_t *p, const uint8_t *data, const long size, uint32_t x_offset, uint32_t y_offset);
void ssd1306_bmp_show_image(ssd1306_t *p, const uint8_t *data, const long size);
void ssd1306_draw_char_with_font(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t scale, const uint8_t *font, char c);
void ssd1306_draw_char(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t scale, char c);
void ssd1306_draw_string_with_font(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t scale, const uint8_t *font, const char *s );
void ssd1306_draw_string(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t scale, const char *s);
//void animation(void);


inline static void swap(int32_t *a, int32_t *b) {
    int32_t *t=a;
    *a=*b;
    *b=*t;
}

inline static void fancy_write(i2c_inst_t *i2c, uint8_t addr, const uint8_t *src, size_t len, char *name) {
    switch(i2c_write_blocking(i2c, addr, src, len, false)) {
    case PICO_ERROR_GENERIC:
        printf("[%s] addr not acknowledged!\n", name);
        break;
    case PICO_ERROR_TIMEOUT:
        printf("[%s] timeout!\n", name);
        break;
    default:
        //printf("[%s] wrote successfully %lu bytes!\n", name, len);
        break;
    }
}

inline static void ssd1306_write(ssd1306_t *p, uint8_t val) {
    uint8_t d[2]= {0x00, val};
    fancy_write(p->i2c_i, p->address, d, 2, "ssd1306_write");
}

bool ssd1306_init(ssd1306_t *p, uint16_t width, uint16_t height, uint8_t address, i2c_inst_t *i2c_instance) {
    p->width=width;
    p->height=height;
    p->pages=height/8;
    p->address=address;

    p->i2c_i=i2c_instance;


    p->bufsize=(p->pages)*(p->width);
    if((p->buffer=malloc(p->bufsize+1))==NULL) {
        p->bufsize=0;
        return false;
    }

    ++(p->buffer);

    // from https://github.com/makerportal/rpi-pico-ssd1306
    uint8_t cmds[]= {
        SET_DISP,
        // timing and driving scheme
        SET_DISP_CLK_DIV,
        0x80,
        SET_MUX_RATIO,
        height - 1,
        SET_DISP_OFFSET,
        0x00,
        // resolution and layout
        SET_DISP_START_LINE,
        // charge pump
        SET_CHARGE_PUMP,
        p->external_vcc?0x10:0x14,
        SET_SEG_REMAP | 0x01,           // column addr 127 mapped to SEG0
        SET_COM_OUT_DIR | 0x08,         // scan from COM[N] to COM0
        SET_COM_PIN_CFG,
        width>2*height?0x02:0x12,
        // display
        SET_CONTRAST,
        0xff,
        SET_PRECHARGE,
        p->external_vcc?0x22:0xF1,
        SET_VCOM_DESEL,
        0x30,                           // or 0x40?
        SET_ENTIRE_ON,                  // output follows RAM contents
        SET_NORM_INV,                   // not inverted
        SET_DISP | 0x01,
        // address setting
        SET_MEM_ADDR,
        0x00,  // horizontal
    };

    for(size_t i=0; i<sizeof(cmds); ++i)
        ssd1306_write(p, cmds[i]);

    return true;
}

inline void ssd1306_deinit(ssd1306_t *p) {
    free(p->buffer-1);
}

inline void ssd1306_poweroff(ssd1306_t *p) {
    ssd1306_write(p, SET_DISP|0x00);
}

inline void ssd1306_poweron(ssd1306_t *p) {
    ssd1306_write(p, SET_DISP|0x01);
}

inline void ssd1306_contrast(ssd1306_t *p, uint8_t val) {
    ssd1306_write(p, SET_CONTRAST);
    ssd1306_write(p, val);
}

inline void ssd1306_invert(ssd1306_t *p, uint8_t inv) {
    ssd1306_write(p, SET_NORM_INV | (inv & 1));
}

inline void ssd1306_clear(ssd1306_t *p) {
    memset(p->buffer, 0, p->bufsize);
}

void ssd1306_clear_pixel(ssd1306_t *p, uint32_t x, uint32_t y) {
    if(x>=p->width || y>=p->height) return;

    p->buffer[x+p->width*(y>>3)]&=~(0x1<<(y&0x07));
}

void ssd1306_draw_pixel(ssd1306_t *p, uint32_t x, uint32_t y) {
    if(x>=p->width || y>=p->height) return;

    p->buffer[x+p->width*(y>>3)]|=0x1<<(y&0x07); // y>>3==y/8 && y&0x7==y%8
}

void ssd1306_draw_line(ssd1306_t *p, int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
    if(x1>x2) {
        swap(&x1, &x2);
        swap(&y1, &y2);
    }

    if(x1==x2) {
        if(y1>y2)
            swap(&y1, &y2);
        for(int32_t i=y1; i<=y2; ++i)
            ssd1306_draw_pixel(p, x1, i);
        return;
    }

    float m=(float) (y2-y1) / (float) (x2-x1);

    for(int32_t i=x1; i<=x2; ++i) {
        float y=m*(float) (i-x1)+(float) y1;
        ssd1306_draw_pixel(p, i, (uint32_t) y);
    }
}

void ssd1306_draw_square(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    for(uint32_t i=0; i<width; ++i)
        for(uint32_t j=0; j<height; ++j)
            ssd1306_draw_pixel(p, x+i, y+j);

}

void ssd1306_draw_empty_square(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
    ssd1306_draw_line(p, x, y, x+width, y);
    ssd1306_draw_line(p, x, y+height, x+width, y+height);
    ssd1306_draw_line(p, x, y, x, y+height);
    ssd1306_draw_line(p, x+width, y, x+width, y+height);
}

void ssd1306_draw_char_with_font(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t scale, const uint8_t *font, char c) {
    if(c<font[3]||c>font[4])
        return;

    uint32_t parts_per_line=(font[0]>>3)+((font[0]&7)>0);
    for(uint8_t w=0; w<font[1]; ++w) { // width
        uint32_t pp=(c-font[3])*font[1]*parts_per_line+w*parts_per_line+5;
        for(uint32_t lp=0; lp<parts_per_line; ++lp) {
            uint8_t line=font[pp];

            for(int8_t j=0; j<8; ++j, line>>=1) {
                if(line & 1)
                    ssd1306_draw_square(p, x+w*scale, y+((lp<<3)+j)*scale, scale, scale);
            }

            ++pp;
        }
    }
}

void ssd1306_draw_string_with_font(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t scale, const uint8_t *font, const char *s) {
    for(int32_t x_n=x; *s; x_n+=(font[1]+font[2])*scale) {
        ssd1306_draw_char_with_font(p, x_n, y, scale, font, *(s++));
    }
}

void ssd1306_draw_char(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t scale, char c) {
    ssd1306_draw_char_with_font(p, x, y, scale, font_8x5, c);
}

void ssd1306_draw_string(ssd1306_t *p, uint32_t x, uint32_t y, uint32_t scale, const char *s) {
    ssd1306_draw_string_with_font(p, x, y, scale, font_8x5, s);
}

static inline uint32_t ssd1306_bmp_get_val(const uint8_t *data, const size_t offset, uint8_t size) {
    switch(size) {
    case 1:
        return data[offset];
    case 2:
        return data[offset]|(data[offset+1]<<8);
    case 4:
        return data[offset]|(data[offset+1]<<8)|(data[offset+2]<<16)|(data[offset+3]<<24);
    default:
        __builtin_unreachable();
    }
    __builtin_unreachable();
}

void ssd1306_bmp_show_image_with_offset(ssd1306_t *p, const uint8_t *data, const long size, uint32_t x_offset, uint32_t y_offset) {
    if(size<54) // data smaller than header
        return;

    const uint32_t bfOffBits=ssd1306_bmp_get_val(data, 10, 4);
    const uint32_t biSize=ssd1306_bmp_get_val(data, 14, 4);
    const int32_t biWidth=(int32_t) ssd1306_bmp_get_val(data, 18, 4);
    const int32_t biHeight=(int32_t) ssd1306_bmp_get_val(data, 22, 4);
    const uint16_t biBitCount=(uint16_t) ssd1306_bmp_get_val(data, 28, 2);
    const uint32_t biCompression=ssd1306_bmp_get_val(data, 30, 4);

    if(biBitCount!=1) // image not monochrome
        return;

    if(biCompression!=0) // image compressed
        return;

    const int table_start=14+biSize;
    uint8_t color_val;

    for(uint8_t i=0; i<2; ++i) {
        if(!((data[table_start+i*4]<<16)|(data[table_start+i*4+1]<<8)|data[table_start+i*4+2])) {
            color_val=i;
            break;
        }
    }

    uint32_t bytes_per_line=(biWidth/8)+(biWidth&7?1:0);
    if(bytes_per_line&3)
        bytes_per_line=(bytes_per_line^(bytes_per_line&3))+4;

    const uint8_t *img_data=data+bfOffBits;

    int step=biHeight>0?-1:1;
    int border=biHeight>0?-1:biHeight;
    for(uint32_t y=biHeight>0?biHeight-1:0; y!=border; y+=step) {
        for(uint32_t x=0; x<biWidth; ++x) {
            if(((img_data[x>>3]>>(7-(x&7)))&1)==color_val)
                ssd1306_draw_pixel(p, x_offset+x, y_offset+y);
        }
        img_data+=bytes_per_line;
    }
}

inline void ssd1306_bmp_show_image(ssd1306_t *p, const uint8_t *data, const long size) {
    ssd1306_bmp_show_image_with_offset(p, data, size, 0, 0);
}

void ssd1306_show(ssd1306_t *p) {
    uint8_t payload[]= {SET_COL_ADDR, 0, p->width-1, SET_PAGE_ADDR, 0, p->pages-1};
    if(p->width==64) {
        payload[1]+=32;
        payload[2]+=32;
    }

    for(size_t i=0; i<sizeof(payload); ++i)
        ssd1306_write(p, payload[i]);

    *(p->buffer-1)=0x40;

    fancy_write(p->i2c_i, p->address, p->buffer-1, p->bufsize+1, "ssd1306_show");
}

/*
const uint8_t num_chars_per_disp[]={7,7,7,5};
const uint8_t *fonts[3]= {acme_font, bubblesstandard_font, BMSPA_font};

#define SLEEPTIME 25

void animation(void) {

    i2c_init(i2c1, 400000);
    gpio_set_function(2, GPIO_FUNC_I2C);
    gpio_set_function(3, GPIO_FUNC_I2C);
    gpio_pull_up(2);
    gpio_pull_up(3);

    const char *words[]= {"SSD1306", "DISPLAY", "DRIVER"};

    ssd1306_t disp;
    disp.external_vcc=false;
    ssd1306_init(&disp, 128, 64, 0x3C, i2c1);
    ssd1306_clear(&disp);

    printf("ANIMATION!\n");

    char buf[8];

    for(;;) {
        for(int y=0; y<31; ++y) {
            ssd1306_draw_line(&disp, 0, y, 127, y);
            ssd1306_show(&disp);
            sleep_ms(SLEEPTIME);
            ssd1306_clear(&disp);
        }

        for(int y=0, i=1; y>=0; y+=i) {
            ssd1306_draw_line(&disp, 0, 31-y, 127, 31+y);
            ssd1306_draw_line(&disp, 0, 31+y, 127, 31-y);
            ssd1306_show(&disp);
            sleep_ms(SLEEPTIME);
            ssd1306_clear(&disp);
            if(y==32) i=-1;
        }

        for(int i=0; i<sizeof(words)/sizeof(char *); ++i) {
            ssd1306_draw_string(&disp, 8, 24, 2, words[i]);
            ssd1306_show(&disp);
            sleep_ms(800);
            ssd1306_clear(&disp);
        }

        for(int y=31; y<63; ++y) {
            ssd1306_draw_line(&disp, 0, y, 127, y);
            ssd1306_show(&disp);
            sleep_ms(SLEEPTIME);
            ssd1306_clear(&disp);
        }

        for(size_t font_i=0; font_i<sizeof(fonts)/sizeof(fonts[0]); ++font_i) {
            uint8_t c=32;
            while(c<=126) {
                uint8_t i=0;
                for(; i<num_chars_per_disp[font_i]; ++i) {
                    if(c>126)
                        break;
                    buf[i]=c++;
                }
                buf[i]=0;

                ssd1306_draw_string_with_font(&disp, 8, 24, 2, fonts[3], buf);
                ssd1306_show(&disp);
                sleep_ms(800);
                ssd1306_clear(&disp);
            }
        }

        //ssd1306_bmp_show_image(&disp, image_data, image_size);
        ssd1306_show(&disp);
        sleep_ms(2000);
    }
}

*/
