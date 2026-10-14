#ifndef NIVEL1_ASSETS_H
#define NIVEL1_ASSETS_H

#include <stdint.h>

#define BLOQUE_W   18
#define BLOQUE_H   29
#define BANANA_W   20
#define BANANA_H   20
#define UVA_W      15
#define UVA_H      20

#define TIPO_BLOQUE  0
#define TIPO_BANANA  1
#define TIPO_UVA     2

#define N_OBJ      56   /* total de objetos del nivel 1 */
#define N_BLOQUES  24
#define N_BANANAS  20
#define N_UVAS     12

/* Rectangulo de la barra de abajo en el estado "uvas seleccionadas" */
#define BARRA_X    128
#define BARRA_Y    204
#define BARRA_W    64
#define BARRA_H    36

typedef struct { int16_t x; int16_t y; uint8_t tipo; } ObjDef_t;

/* Sprites RGB565 (byte alto, byte bajo); 0x0000 = transparente. Estan en flash (const). */
extern const uint8_t bloque_spr[BLOQUE_W * BLOQUE_H * 2];
extern const uint8_t banana_spr[BANANA_W * BANANA_H * 2];
extern const uint8_t uva_spr[UVA_W * UVA_H * 2];
extern const uint8_t barra_uvas[BARRA_W * BARRA_H * 2];

/* Objetos del nivel 1 en orden de dibujo (de arriba hacia abajo), esquina superior izquierda */
extern const ObjDef_t nivel1_objs[N_OBJ];

#endif
