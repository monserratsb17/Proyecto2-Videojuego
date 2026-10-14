#ifndef NIVEL2_ASSETS_H
#define NIVEL2_ASSETS_H
/* Nivel 2: solo hielo, bananas y uvas. Reutiliza los sprites de nivel1_assets.h
   (bloque_spr, banana_spr, uva_spr) y el tipo ObjDef_t.
   SD: fondo2.bin (fondo limpio) y n2start.bin (nivel inicial con bananas). */
#include <stdint.h>
#include "nivel1_assets.h"

#define N2_OBJ      48
#define N2_BLOQUES  16
#define N2_BANANAS  20
#define N2_UVAS     12

#define BARRA2_X 128
#define BARRA2_Y 211
#define BARRA2_W 64
#define BARRA2_H 29

extern const uint8_t barra_uvas2[BARRA2_W * BARRA2_H * 2];
extern const ObjDef_t nivel2_objs[N2_OBJ];   /* ordenado por y de base (orden de pintado) */

#endif
