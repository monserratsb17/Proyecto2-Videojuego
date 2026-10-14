/* USER CODE BEGIN Header */

/**

 ******************************************************************************

 * @file           : main.c

 * @brief          : Solo muestra pantalla_inicio en el ILI9341

 ******************************************************************************

 */

/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/

#include "main.h"

#include "fatfs.h"



/* Private includes ----------------------------------------------------------*/

/* USER CODE BEGIN Includes */

#include "ili9341.h"

#include "bitmaps.h"

#include "nivel1_assets.h"
#include "nivel2_assets.h"
#include "hielo_romper.h"
#include "personajes_acciones.h"
#include "sound8bit.h"

#include <string.h>

/* USER CODE END Includes */



/* Private typedef -----------------------------------------------------------*/

/* USER CODE BEGIN PTD */



/* USER CODE END PTD */



/* Private define ------------------------------------------------------------*/

/* USER CODE BEGIN PD */

#define LCD_W       320

#define LCD_H       240



#define START_X       0

#define START_Y       0

#define START_W     320

#define START_H     240

#define BG_COLOR   0x0000

#define SKY_COLOR   0x867D   // celeste

#define SPR_W       20

#define SPR_H       31

#define SPR_Y       104

#define SPR_FRAMES  5

#define CLICK_W        174

#define CLICK_H        24

#define CLICK_X        ((LCD_W - CLICK_W) / 2)   // 73

#define CLICK_Y        201   // 108

#define CLICK_FRAMES   2

#define CLICK_PERIODO  500                       // ms entre cuadros

// Color de fondo = el pixel de inicio.bin justo donde empieza el sprite (x=CLICK_X, y=CLICK_Y)

// Si cambias CLICK_X / CLICK_Y / CLICK_W / CLICK_H, hay que recalcular este valor

#define CLICK_BG  0xF7FF



#define SEL_W         14

#define SEL_H         16

#define SEL1_X        92

//#define SEL1_Y        147

#define SEL_DX        120               // desplazamiento en X de la segunda (ajustalo)

#define SEL2_X        (SEL1_X + SEL_DX)

//#define SEL2_Y        SEL1_Y

#define SEL_PERIODO   400                 // ms visible / ms oculto

#define SEL_TRANSP    0x0000              // color transparente del sprite



#define SEL_Y_POS1    147

#define SEL_Y_POS2    171



#define P1_W        35

#define P1_H        43

#define P1_X        86

#define P1_Y        50

#define P1_FRAMES   5

#define P1_PERIODO  150          // ms entre cuadros

#define P1_TRANSP   0x0000       // color transparente del sprite



#define P2_W        66

#define P2_H        48

#define P2_X        185

#define P2_Y        51

#define P2_FRAMES   5

#define P2_PERIODO  150

#define P2_TRANSP   0x0000



#define SAB_W        85

#define SAB_HMAX     115

#define SELS_W       17

#define SELS_H       40

#define SEL_UP       14           // espacio arriba del sprite para la galleta

#define GAL_GAP      4                     // separación del helado

#define GAL_DX       (SAB_W + GAL_GAP)

#define REG_W        (GAL_DX + SELS_W)     // incluye la galleta

#define REG_H        (SEL_UP + SAB_HMAX)      // 129

#define SAB_Y       35           // y del sprite; la region empieza en SAB_Y - SEL_UP

#define SAB_X_1P     117

#define SAB_X_2P_A   50

#define SAB_X_2P_B   185

#define SAB_PERIODO  150



#define NIV_W   48      // ancho de nivel_selector   <-- pon el tuyo

#define NIV_H   48      // alto de nivel_selector    <-- pon el tuyo

/* USER CODE END PD */



/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PM */



/* USER CODE END PM */



/* Private variables ---------------------------------------------------------*/

SPI_HandleTypeDef hspi1;




UART_HandleTypeDef huart1;

UART_HandleTypeDef huart2;



/* USER CODE BEGIN PV */

extern uint8_t pantalla_menu[];      // el menu sigue en flash (los selectores copian pedazos de el)



extern uint8_t helado_selector[];



typedef enum { PANT_INICIO, PANT_MENU, PANT_JUGADORES, PANT_SABOR, PANT_NIVEL, PANT_JUEGO, PANT_PAUSA, PANT_GAMEOVER, PANT_VICTORIA, PANT_CREDITOS} Pantalla_t;

volatile Pantalla_t pantalla = PANT_INICIO;



volatile uint8_t cmd_A = 0;

volatile uint8_t cmd_B = 0;

/* Estado completo enviado por cada mando: A,B,UP,DOWN,LEFT,RIGHT. */
static volatile uint8_t mando_estado[2]={0,0};
static volatile uint32_t mando_ultimo[2]={0,0};
uint8_t rx_byte;

char    rx_buf[16];

uint8_t rx_idx = 0;



uint32_t t_sel = 0;

uint8_t  sel_visible = 0;



volatile uint8_t cmd_UP = 0;

volatile uint8_t cmd_DOWN = 0;



uint8_t  sel_pos = 0;            // 0 = posicion 1, 1 = posicion 2

uint16_t sel_y = SEL_Y_POS1;



uint32_t t_p1 = 0;

uint8_t  frame_p1 = 0;

uint8_t  fondo_p1[P1_W * P1_H * 2];   // pedazo de jugador.bin que queda debajo del sprite



static uint8_t  fondo_p2[P2_W * P2_H * 2];

static uint8_t  frame_p2 = 0;

//static uint32_t t_p2 = 0;

static uint8_t  modo_jug = 0;      // 0 = 1 player, 1 = 2 players

static volatile uint8_t cmd_LEFT = 0, cmd_RIGHT = 0;



uint8_t sabor_elegido[2] = {0, 0};   // 0 choco, 1 vainilla, 2 fresa  (J1, J2)

static uint8_t  num_jug = 1, sab_turno = 0, sab_listo = 0;

static uint8_t  sab_cur[2], sab_conf[2], sab_frame[2];

static int8_t   sab_self[2];

static uint32_t t_sab = 0;





static uint8_t fondo_sab[REG_W * REG_H * 2];

static int8_t  fondo_slot = -1;       // de cual jugador es el fondo que hay en la cache

static uint8_t slot_buf[REG_W * REG_H * 2];

static uint8_t fila_tmp[SAB_W * 2];

static uint8_t sel_tmp[SELS_W * 2];

static const char    *sab_file[3] = {"choco.bin", "vainilla.bin", "fresa.bin"};

static const uint8_t  sab_h[3]    = {108, 108, 115};



// Posición de la galleta dentro del grupo de helados:

// chocolate, vainilla, fresa

// Orden: chocolate, vainilla, fresa

// Chocolate, vainilla, fresa

static const uint8_t gal_x_sabor[3] = {0, 25, 48};

static const uint8_t gal_y_sabor[3] = {0, 8, 12};



extern const uint8_t nivel_selector[];



// posicion (esquina superior izquierda) del selector en nivel 1, 2 y 3  <-- ajusta

static const uint16_t niv_x[3] = {35, 85, 135};

static const uint16_t niv_y[3] = {90, 90, 90};



static uint8_t niv_buf[NIV_W * NIV_H * 2];

static uint8_t nivel_sel = 0;

uint8_t nivel_elegido = 0;            // 0 = nivel 1, 1 = nivel 2, 2 = nivel 3 (para el juego)



#define MAX_OBJ ((N_OBJ>N2_OBJ)?N_OBJ:N2_OBJ)
static const ObjDef_t *mapa_objs=nivel1_objs;
static uint8_t mapa_n=N_OBJ,mapa_bananas=N_BANANAS,mapa_uvas=N_UVAS,mapa_nivel=0;
static const char *mapa_fondo="fondo1.bin",*mapa_inicio="n1start.bin";
static uint8_t obj_activo[MAX_OBJ];
static uint8_t hielo_estado[MAX_OBJ];
static uint32_t hielo_inicio[MAX_OBJ];
static volatile uint8_t cmd_A2=0;

static uint8_t frutas_restantes = 0;

static uint8_t fase = 0;                 // 0 = bananas, 1 = uvas

static uint8_t rect_buf[64 * 64 * 2];





/* PWM del buzzer pasivo: PB10 AF1, TIM2 canal 3; contador a 1 MHz. */
static void Juego_Audio_Init(void) {
 __HAL_RCC_GPIOB_CLK_ENABLE();
 __HAL_RCC_TIM2_CLK_ENABLE();
 GPIO_InitTypeDef gpio={0};
 gpio.Pin=GPIO_PIN_10;gpio.Mode=GPIO_MODE_AF_PP;gpio.Pull=GPIO_NOPULL;
 gpio.Speed=GPIO_SPEED_FREQ_LOW;gpio.Alternate=GPIO_AF1_TIM2;
 HAL_GPIO_Init(GPIOB,&gpio);
 uint32_t reloj=HAL_RCC_GetPCLK1Freq();
 if((RCC->CFGR & RCC_CFGR_PPRE1)!=0)reloj*=2;
 TIM2->CR1=0;TIM2->CCER=0;TIM2->DIER=0;
 TIM2->PSC=reloj/1000000u-1;
 TIM2->ARR=999;TIM2->CCR3=0;
 TIM2->CCMR2=(6u<<4)|TIM_CCMR2_OC3PE;
 TIM2->EGR=TIM_EGR_UG;
 TIM2->CCER=TIM_CCER_CC3E;
 TIM2->CR1=TIM_CR1_ARPE|TIM_CR1_CEN;
 Sound8Bit_Init();
}

static void DebugPC(const char* s)

{

  HAL_UART_Transmit(&huart2, (uint8_t*)s, (uint16_t)strlen(s), HAL_MAX_DELAY);

}



// Dibuja una pantalla completa desde la SD. Si falla, avisa por UART y pinta la pantalla roja

static void Pantalla_SD(const char *nombre)

{

  if (!LCD_Bitmap_SD(nombre)) {

    DebugPC("ERROR leyendo: ");

    DebugPC(nombre);

    DebugPC("\r\n");

    LCD_Clear(0xF800);

  }

}



// Se llama cada vez que llega 1 byte por USART1

static int Mando_Hex(char c) {
 if(c>='0' && c<='9')return c-'0';
 if(c>='A' && c<='F')return c-'A'+10;
 if(c>='a' && c<='f')return c-'a'+10;
 return -1;
}
static void Mando_Linea(void) {
 if(rx_idx!=5 || rx_buf[0]!='P' || rx_buf[2]!=':' ||
    (rx_buf[1]!='1' && rx_buf[1]!='2'))return;
 int h=Mando_Hex(rx_buf[3]),l=Mando_Hex(rx_buf[4]);
 if(h<0 || l<0)return;
 uint8_t id=(uint8_t)(rx_buf[1]-'1'),estado=(uint8_t)((h<<4)|l);
 if(estado&0xC0)return;
 uint8_t antes=mando_estado[id];
 if(HAL_GetTick()-mando_ultimo[id]>350)antes=0;
 mando_estado[id]=estado;mando_ultimo[id]=HAL_GetTick();
 uint8_t nuevos=estado & (uint8_t)~antes;
 if(pantalla==PANT_JUEGO && id==1 && (nuevos&1))cmd_A2=1;
 /* J1 controla menus. En sabores, acepta el mando del turno actual. */
 uint8_t acepta=(pantalla==PANT_SABOR)?(id==sab_turno):(id==0);
 if(acepta) {
  if (nuevos & 1) { cmd_A = 1; }
  if (nuevos & 2) { cmd_B = 1; }

  if (nuevos & 4) { cmd_UP = 1; }
  if (nuevos & 8) { cmd_DOWN = 1; }

  if (nuevos & 16) { cmd_LEFT = 1; }
  if (nuevos & 32) { cmd_RIGHT = 1; }

 }
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
 if(huart->Instance==USART1) {
  char c=(char)rx_byte;
  if(c=='\r' || c=='\n') {
   rx_buf[rx_idx]='\0';Mando_Linea();rx_idx=0;
  } else if(rx_idx<sizeof(rx_buf)-1)rx_buf[rx_idx++]=c;
  else rx_idx=0;
  HAL_UART_Receive_IT(&huart1,&rx_byte,1);
 }
}

// Si hay error (overrun, ruido) se vuelve a armar la recepcion

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)

{

  if (huart->Instance == USART1) {

    HAL_UART_Receive_IT(&huart1, &rx_byte, 1);

  }

}



static void Selectores_Dibujar(void)

{

  LCD_SpriteSobreFondo(SEL1_X, sel_y, SEL_W, SEL_H, helado_selector, SEL_TRANSP, pantalla_menu, LCD_W);

  LCD_SpriteSobreFondo(SEL2_X, sel_y, SEL_W, SEL_H, helado_selector, SEL_TRANSP, pantalla_menu, LCD_W);

}



static void Selectores_Borrar(void)

{

  LCD_RestoreFondo(SEL1_X, sel_y, SEL_W, SEL_H, pantalla_menu, LCD_W);

  LCD_RestoreFondo(SEL2_X, sel_y, SEL_W, SEL_H, pantalla_menu, LCD_W);

}



// Mueve los selectores a la posicion 0 o 1 (quedan visibles al moverse)

static void Selector_Mover(uint8_t pos)

{

  Selectores_Borrar();

  sel_pos = pos;

  sel_y = (pos == 0) ? SEL_Y_POS1 : SEL_Y_POS2;

  Selectores_Dibujar();

  sel_visible = 1;

  t_sel = HAL_GetTick();

}



// Dibuja el menu completo con los selectores en su posicion actual

static void Menu_Mostrar(void)

{

  cmd_A = 0; cmd_B = 0; cmd_UP = 0; cmd_DOWN = 0;   // limpia comandos viejos

  LCD_Bitmap(START_X, START_Y, START_W, START_H, pantalla_menu);

  Selectores_Dibujar();

  sel_visible = 1;

  t_sel = HAL_GetTick();

  pantalla = PANT_MENU;

}



// Lee de jugador.bin el fondo que queda debajo del sprite (se hace una vez al entrar a la pantalla)

static void P1_CargarFondo(void)

{

  FIL f;

  UINT br;

  if (f_open(&f, "imagen.bin", FA_READ) != FR_OK) return;

  for (int j = 0; j < P1_H; j++) {

    f_lseek(&f, ((P1_Y + j) * LCD_W + P1_X) * 2);

    f_read(&f, &fondo_p1[j * P1_W * 2], P1_W * 2, &br);

  }

  f_close(&f);

}





static void P2_CargarFondo(void)

{

  FIL f;

  UINT br;

  if (f_open(&f, "imagen.bin", FA_READ) != FR_OK) return;

  for (int j = 0; j < P2_H; j++) {

    f_lseek(&f, ((P2_Y + j) * LCD_W + P2_X) * 2);

    f_read(&f, &fondo_p2[j * P2_W * 2], P2_W * 2, &br);

  }

  f_close(&f);

}



void Jugadores_Dibujar(void) {

  LCD_SpritePatch(P1_X, P1_Y, P1_W, P1_H, (const unsigned char*)player1_seleccion, P1_FRAMES,

                  (modo_jug == 0) ? frame_p1 : 0, P1_TRANSP, fondo_p1);

  LCD_SpritePatch(P2_X, P2_Y, P2_W, P2_H, (const unsigned char*)player2_seleccion, P2_FRAMES,

                  (modo_jug == 1) ? frame_p2 : 0, P2_TRANSP, fondo_p2);

}



static uint8_t SD_LeerFila(FIL *f, uint32_t off, uint8_t *dst, uint32_t n) {

  UINT br;

  if (f_lseek(f, off) != FR_OK) return 0;

  if (f_read(f, dst, n, &br) != FR_OK || br != n) return 0;

  return 1;

}



void Jugadores_Mostrar(void) {

  pantalla = PANT_JUGADORES;

  Pantalla_SD("imagen.bin");          // el nombre que uses para la imagen de jugadores

  P1_CargarFondo(); P2_CargarFondo();

  modo_jug = 0; frame_p1 = 0; frame_p2 = 0;

  cmd_LEFT = 0; cmd_RIGHT = 0;

  Jugadores_Dibujar();

  t_p1 = HAL_GetTick();

  DebugPC("JUGADORES\r\n");

}



static uint16_t Sab_X(uint8_t slot) {

  if (num_jug == 1) return SAB_X_1P;

  return slot ? SAB_X_2P_B : SAB_X_2P_A;

}



// Slot_CargarFondo: ya no usa el indice [slot], y avisa de quien es la cache

static uint8_t Slot_CargarFondo(uint8_t slot) {

  FIL f; uint16_t r;

  uint16_t x = Sab_X(slot), y = SAB_Y - SEL_UP;

  if (f_open(&f, "sabor.bin", FA_READ) != FR_OK) return 0;

  for (r = 0; r < REG_H; r++) {

    if (!SD_LeerFila(&f, ((uint32_t)(y + r) * LCD_W + x) * 2, &fondo_sab[r * REG_W * 2], REG_W * 2)) { f_close(&f); return 0; }

  }

  f_close(&f);

  fondo_slot = slot;

  return 1;

}



static uint8_t Slot_Dibujar(uint8_t slot, uint8_t sabor, uint8_t frame, int8_t selframe) {

  FIL f; UINT br; uint16_t r, c;

  uint8_t h = sab_h[sabor];

  uint8_t yoff = SAB_HMAX - h;                       // alineado abajo

  // Centra la galleta verticalmente junto al sabor actual

  uint16_t gal_x = gal_x_sabor[sabor];

  uint16_t gal_y = SEL_UP + yoff + gal_y_sabor[sabor];

  if (fondo_slot != slot) { if (!Slot_CargarFondo(slot)) return 0; }

  memcpy(slot_buf, fondo_sab, sizeof(slot_buf));



  // galleta (detras)

  if (selframe >= 0) {

    if (f_open(&f, "sabsel.bin", FA_READ) != FR_OK) return 0;

    f_lseek(&f, (uint32_t)selframe * SELS_W * SELS_H * 2);

    for (r = 0; r < SELS_H; r++) {

      if (f_read(&f, sel_tmp, SELS_W * 2, &br) != FR_OK || br != SELS_W * 2) { f_close(&f); return 0; }

      for (c = 0; c < SELS_W; c++) {

        if (sel_tmp[2 * c] || sel_tmp[2 * c + 1]) {

        	uint8_t *d = &slot_buf[

				  ((gal_y + r) * REG_W + gal_x + c) * 2

        	];

          d[0] = sel_tmp[2 * c]; d[1] = sel_tmp[2 * c + 1];

        }

      }

    }

    f_close(&f);

  }



  // sabor (adelante)

  if (f_open(&f, sab_file[sabor], FA_READ) != FR_OK) return 0;

  f_lseek(&f, (uint32_t)frame * SAB_W * h * 2);

  for (r = 0; r < h; r++) {

    if (f_read(&f, fila_tmp, SAB_W * 2, &br) != FR_OK || br != SAB_W * 2) { f_close(&f); return 0; }

    for (c = 0; c < SAB_W; c++) {

      if (fila_tmp[2 * c] || fila_tmp[2 * c + 1]) {

        uint8_t *d = &slot_buf[((SEL_UP + yoff + r) * REG_W + c) * 2];

        d[0] = fila_tmp[2 * c]; d[1] = fila_tmp[2 * c + 1];

      }

    }

  }

  f_close(&f);



  LCD_Bitmap(Sab_X(slot), SAB_Y - SEL_UP, REG_W, REG_H, slot_buf);

  return 1;

}



void Sab_Redibujar(uint8_t slot) {

  if (!Slot_Dibujar(slot, sab_cur[slot], sab_frame[slot], sab_self[slot]))

    DebugPC("ERROR SD sabor\r\n");

}



void Sabor_Mostrar(void) {

  pantalla = PANT_SABOR;

  Pantalla_SD("sabor.bin");

  num_jug = modo_jug + 1;

  sab_turno = 0; sab_listo = 0;

  for (uint8_t i = 0; i < 2; i++) { sab_cur[i] = 0; sab_conf[i] = 0; sab_frame[i] = 0; sab_self[i] = -1; }

  cmd_UP = 0; cmd_DOWN = 0; cmd_LEFT = 0; cmd_RIGHT = 0; cmd_A = 0; cmd_B = 0;

  if (!Slot_CargarFondo(0)) DebugPC("ERROR fondo sabor\r\n");

  if (num_jug == 2 && !Slot_CargarFondo(1)) DebugPC("ERROR fondo sabor\r\n");

  Sab_Redibujar(0);

  if (num_jug == 2) Sab_Redibujar(1);

  t_sab = HAL_GetTick();

  DebugPC("SABOR\r\n");

}



static uint8_t niv_buf[NIV_W * NIV_H * 2];





// lee de levelsel.bin el pedazo de fondo que queda en la posicion n

static uint8_t Nivel_LeerFondo(uint8_t n) {

  FIL f; uint16_t r;

  if (f_open(&f, "levelsel.bin", FA_READ) != FR_OK) return 0;

  for (r = 0; r < NIV_H; r++) {

    if (!SD_LeerFila(&f, ((uint32_t)(niv_y[n] + r) * LCD_W + niv_x[n]) * 2, &niv_buf[r * NIV_W * 2], NIV_W * 2)) { f_close(&f); return 0; }

  }

  f_close(&f);

  return 1;

}



static void Nivel_Borrar(uint8_t n) {

  if (Nivel_LeerFondo(n)) LCD_Bitmap(niv_x[n], niv_y[n], NIV_W, NIV_H, niv_buf);

}



static void Nivel_Dibujar(uint8_t n) {

  if (!Nivel_LeerFondo(n)) { DebugPC("ERROR levelsel\r\n"); return; }

  for (uint16_t i = 0; i < NIV_W * NIV_H; i++) {

    if (nivel_selector[2 * i] || nivel_selector[2 * i + 1]) {      // 0x0000 = transparente

      niv_buf[2 * i] = nivel_selector[2 * i];

      niv_buf[2 * i + 1] = nivel_selector[2 * i + 1];

    }

  }

  LCD_Bitmap(niv_x[n], niv_y[n], NIV_W, NIV_H, niv_buf);

}



static void Nivel_Mover(uint8_t n) {

  Nivel_Borrar(nivel_sel);

  nivel_sel = n;

  Nivel_Dibujar(n);

}



void Nivel_Mostrar(void) {

  pantalla = PANT_NIVEL;

  Pantalla_SD("levelsel.bin");

  nivel_sel = 0;

  cmd_UP = 0; cmd_DOWN = 0; cmd_LEFT = 0; cmd_RIGHT = 0; cmd_A = 0; cmd_B = 0;

  Nivel_Dibujar(0);

  DebugPC("NIVEL\r\n");

}



static const uint8_t *Spr_De(uint8_t tipo, int16_t *w, int16_t *h)

{

  if (tipo == TIPO_BLOQUE) { *w = BLOQUE_W; *h = BLOQUE_H; return bloque_spr; }

  if (tipo == TIPO_BANANA) { *w = BANANA_W; *h = BANANA_H; return banana_spr; }

  *w = UVA_W; *h = UVA_H; return uva_spr;

}



// Copia un sprite dentro de rect_buf (recortado al rectangulo); 0x0000 es transparente

static void Blit_Rect(const uint8_t *spr, int16_t sw, int16_t sh, int16_t sx, int16_t sy,

                      int16_t rx, int16_t ry, int16_t rw, int16_t rh)

{

  for (int16_t j = 0; j < sh; j++) {

    int16_t py = sy + j - ry;

    if (py < 0 || py >= rh) continue;

    for (int16_t i = 0; i < sw; i++) {

      int16_t px = sx + i - rx;

      if (px < 0 || px >= rw) continue;

      uint8_t hi = spr[(j * sw + i) * 2], lo = spr[(j * sw + i) * 2 + 1];

      if (hi == 0 && lo == 0) continue;

      rect_buf[(py * rw + px) * 2]     = hi;

      rect_buf[(py * rw + px) * 2 + 1] = lo;

    }

  }

}



// Redibuja un rectangulo: fondo limpio (SD) + objetos activos en orden

static FIL fondo_cache;
static const char *fondo_cache_nombre=0;
static uint8_t fondo_cache_abierto=0;
static void Juego_ActoresRect(int rx,int ry,int rw,int rh);

static uint8_t Nivel_RedibujarRect(int16_t rx, int16_t ry, int16_t rw, int16_t rh)

{

  int16_t r, w, h;

  if ((uint32_t)(rw * rh * 2) > sizeof(rect_buf)) return 0;

  if(!fondo_cache_abierto || fondo_cache_nombre!=mapa_fondo){
    if(fondo_cache_abierto)f_close(&fondo_cache);
    fondo_cache_abierto=0;
    if(f_open(&fondo_cache,mapa_fondo,FA_READ)!=FR_OK)return 0;
    fondo_cache_abierto=1;fondo_cache_nombre=mapa_fondo;
  }

  for (r = 0; r < rh; r++) {

    if (!SD_LeerFila(&fondo_cache, ((uint32_t)(ry + r) * LCD_W + rx) * 2, &rect_buf[r * rw * 2], rw * 2)) { f_close(&fondo_cache); fondo_cache_abierto=0; return 0; }

  }



  for (uint8_t i = 0; i < mapa_n; i++) {

    if (!obj_activo[i]) continue;

    const uint8_t *s = Spr_De(mapa_objs[i].tipo, &w, &h);

    if(mapa_objs[i].tipo==TIPO_BLOQUE && hielo_estado[i])s=hielo_romper_spr[hielo_estado[i]-1];
    Blit_Rect(s, w, h, mapa_objs[i].x, mapa_objs[i].y, rx, ry, rw, rh);

  }

  Juego_ActoresRect(rx, ry, rw, rh);
  LCD_Bitmap(rx, ry, rw, rh, rect_buf);

  return 1;

}



static void Obj_Redibujar(uint8_t i)

{

  int16_t w, h;

  Spr_De(mapa_objs[i].tipo, &w, &h);

  Nivel_RedibujarRect(mapa_objs[i].x, mapa_objs[i].y, w, h);

}



/* Motor de personajes y colisiones */
/* Incluir en main.c despues de Obj_Redibujar, antes de Nivel1_Mostrar. */
typedef struct { const uint8_t *p; uint8_t w,h,n; } JuegoSprite;
typedef struct { int16_t x,y; uint8_t dir,frame,sabor; } JuegoActor;
/* x,y = centro de los pies. dir: derecha, abajo, izquierda, arriba. */
static JuegoActor jugador, jugador2, monstruo;
static uint8_t juego_dos;
static uint32_t paso_jugador[2];
static uint32_t accion_inicio[2],muerte_inicio;
static uint8_t accion_activa[2],muriendo;
static uint8_t juego_fin; /* 0 jugando, 1 derrota, 2 victoria */
static uint32_t juego_tick, juego_anim,pausa_inicio;
static uint32_t vaca_busqueda;
static int8_t vaca_ruta=-1;
static uint32_t juego_rng = 0x194731u;
void Fruta_Comer(uint8_t i);
static const JuegoSprite juego_sprites[3][3] = {
 {{(const uint8_t*)choco_ladoSprite,20,31,5},{(const uint8_t*)choco_frenteSprite,20,31,4},{(const uint8_t*)choco_espaldaSprite,20,31,4}},
 {{(const uint8_t*)vainilla_ladoSprite,20,29,5},{(const uint8_t*)vainilla_frenteSprite,21,31,4},{(const uint8_t*)vainilla_espaldaSprite,21,31,4}},
 {{(const uint8_t*)fresa_ladoSprite,20,28,5},{(const uint8_t*)fresa_frenteSprite,21,31,4},{(const uint8_t*)fresa_espaldaSprite,21,31,4}}
};
static const JuegoSprite troll_sprites[3] = {
 {(const uint8_t*)lado_Troll,22,26,4},
 {(const uint8_t*)frente_Troll,31,25,4},
 {(const uint8_t*)espalda_Troll,32,25,4}
};
static const JuegoSprite vaca_sprites[3] = {
 {(const uint8_t*)lado_Vaca,21,33,4},
 {(const uint8_t*)frente_Vaca,20,35,4},
 {(const uint8_t*)espalda_Vaca,20,35,4}
};
_Static_assert(sizeof(lado_Vaca)==21*33*4*2,"Tamano lado Vaca");
_Static_assert(sizeof(frente_Vaca)==20*35*4*2,"Tamano frente Vaca");
_Static_assert(sizeof(espalda_Vaca)==20*35*4*2,"Tamano espalda Vaca");
static const JuegoSprite romper_sprites[3][3]={
 {{lado_HieloC,34,41,5},{frente_HieloC,31,41,6},{espalda_HieloC,29,41,6}},
 {{lado_HieloV,34,41,5},{frente_HieloV,31,41,6},{espalda_HieloV,29,41,6}},
 {{lado_HieloF,34,41,5},{frente_HieloF,31,41,6},{espalda_HieloF,29,41,6}}
};
static const JuegoSprite muerte_sprites[3]={
 {chocolate_Muerto,27,32,5},
 {vainilla_Muerto,27,34,5},{fresa_Muerto,27,34,5}
};
static const int8_t juego_dx[4]={1,0,-1,0}, juego_dy[4]={0,1,0,-1};
static uint8_t Juego_Solapa(int x,int y,int w,int h,int a,int b,int c,int d) {
 return x<a+c && x+w>a && y<b+d && y+h>b;
}
static uint8_t Juego_Libre(int x,int y) {
 if(mapa_nivel==1) {
  /* Nivel 2: bordes fijos del fondo, independientes de los hielos rompibles.
     Interior x=31..290, y=46..191; margen para el sprite completo. */
  if(x-11<31 || x+11>290 || y-35<46 || y>191)return 0;
  /* Figuras fijas en las cuatro esquinas. */
  if(Juego_Solapa(x-6,y-7,12,7,31,46,32,31) ||
     Juego_Solapa(x-6,y-7,12,7,250,46,40,31) ||
     Juego_Solapa(x-6,y-7,12,7,31,154,32,37) ||
     Juego_Solapa(x-6,y-7,12,7,250,154,40,37))return 0;
  /* Iglu del nivel 2, ajustado a preview_inicio(2).png. */
  if(Juego_Solapa(x-6,y-7,12,7,148,84,22,20) ||
     Juego_Solapa(x-6,y-7,12,7,141,94,38,10) ||
     Juego_Solapa(x-6,y-7,12,7,127,104,64,30) ||
     Juego_Solapa(x-6,y-7,12,7,134,134,52,16) ||
     Juego_Solapa(x-6,y-7,12,7,147,150,27,6))return 0;
 } else {
  if(x-6<17 || x+6>302 || y<37 || y>200)return 0;
  if(Juego_Solapa(x-6,y-7,12,7,143,64,34,15) ||
     Juego_Solapa(x-6,y-7,12,7,132,79,55,16) ||
     Juego_Solapa(x-6,y-7,12,7,125,95,70,25) ||
     Juego_Solapa(x-6,y-7,12,7,139,120,43,15))return 0;
 }
 for(uint8_t i=0;i<mapa_n;i++) {
  if(obj_activo[i] && mapa_objs[i].tipo==TIPO_BLOQUE &&
     Juego_Solapa(x-6,y-7,12,7,mapa_objs[i].x,mapa_objs[i].y+9,BLOQUE_W,BLOQUE_H-9)) return 0;
 }
 return 1;
}
static JuegoSprite Juego_Sprite(const JuegoActor *a,uint8_t troll) {
 uint8_t vista=a->dir==3?2:(a->dir==1?1:0);
 if(!troll){
  uint8_t id=(a==&jugador2);
  if(muriendo)return muerte_sprites[a->sabor];
  if(accion_activa[id])return romper_sprites[a->sabor][vista];
 }
 return troll?(mapa_nivel?vaca_sprites[vista]:troll_sprites[vista]):juego_sprites[a->sabor][vista];
}
static void Juego_Blit(const JuegoActor *a,uint8_t troll,int rx,int ry,int rw,int rh) {
 JuegoSprite s=Juego_Sprite(a,troll);
 int sx=a->x-s.w/2,sy=a->y-s.h;
 for(int j=0;j<s.h;j++) for(int i=0;i<s.w;i++) {
  int px=sx+i-rx,py=sy+j-ry;
  if(px<0 || px>=rw || py<0 || py>=rh) continue;
  /* La Vaca lateral mira a la derecha; los otros sprites, a la izquierda. */
  uint8_t flip=(troll && mapa_nivel==1)?(a->dir==2):(a->dir==0);
  int col=flip?s.w-1-i:i;
  uint8_t frame=a->frame%s.n;
  if(!troll){
   if(muriendo){frame=(HAL_GetTick()-muerte_inicio)/110;if(frame>=s.n)frame=s.n-1;}
   else if(accion_activa[a==&jugador2]){frame=(HAL_GetTick()-accion_inicio[a==&jugador2])/70;if(frame>=s.n)frame=s.n-1;}
  }
  uint32_t k=((uint32_t)j*s.w*s.n+frame*s.w+col)*2;
  uint8_t hi=s.p[k],lo=s.p[k+1];
  if(!(hi|lo)) continue;
  rect_buf[(py*rw+px)*2]=hi;rect_buf[(py*rw+px)*2+1]=lo;
 }
}
static void Juego_ActoresRect(int rx,int ry,int rw,int rh) {
 const JuegoActor *actores[3]={&jugador,&monstruo,&jugador2};
 uint8_t n=juego_dos?3:2;
 for(uint8_t i=0;i<n;i++)for(uint8_t j=i+1;j<n;j++)
  if(actores[i]->y>actores[j]->y){const JuegoActor *tmp=actores[i];actores[i]=actores[j];actores[j]=tmp;}
 for(uint8_t i=0;i<n;i++)Juego_Blit(actores[i],actores[i]==&monstruo,rx,ry,rw,rh);
}
static void Juego_ParcheMovimiento(int ox,int oy,int nx,int ny) {
 int rx=(ox<nx?ox:nx)-17, ry=(oy<ny?oy:ny)-43;
 int rw=34+(ox>nx?ox-nx:nx-ox), rh=44+(oy>ny?oy-ny:ny-oy);
 if(rx<17){rw-=17-rx;rx=17;}
 if(ry<0){rh+=ry;ry=0;}
 if(rx+rw>303)rw=303-rx;
 if(ry+rh>(mapa_nivel?208:201))rh=(mapa_nivel?208:201)-ry;
 if(rw<=0||rh<=0)return;
 /* Un solo parche: fondo + objetos + ambos actores en su posicion final. */
 Nivel_RedibujarRect(rx,ry,rw,rh);
}
static void Juego_Parche(int x,int y) {
 Juego_ParcheMovimiento(x,y,x,y);
}
static void Juego_Cartel(const char *nombre,uint16_t w,uint16_t h){
 FIL f;uint8_t fila[242*2];
 if(w>242 || f_open(&f,nombre,FA_READ)!=FR_OK){DebugPC("ERROR cartel SD\r\n");return;}
 if(f_size(&f)!=(uint32_t)w*h*2){f_close(&f);DebugPC("ERROR tamano cartel\r\n");return;}
 for(uint16_t y=0;y<h;y++){
  if(!SD_LeerFila(&f,(uint32_t)y*w*2,fila,w*2)){DebugPC("ERROR fila cartel\r\n");break;}
  LCD_Bitmap((LCD_W-w)/2,(LCD_H-h)/2+y,w,1,fila);
 }
 f_close(&f);
}
static void Juego_Terminar(uint8_t victoria) {
 if(!victoria && !muriendo){
  juego_fin=1;muriendo=1;muerte_inicio=HAL_GetTick();accion_activa[0]=accion_activa[1]=0;
  Sound8Bit_Stop();
  Juego_Parche(jugador.x,jugador.y);if(juego_dos)Juego_Parche(jugador2.x,jugador2.y);
  return;
 }
 muriendo=0;juego_fin=victoria?2:1;Sound8Bit_Stop();
 DebugPC(victoria?"VICTORIA\r\n":"GAME OVER\r\n");
 pantalla=victoria?PANT_VICTORIA:PANT_GAMEOVER;
 cmd_A=cmd_A2=cmd_B=cmd_UP=cmd_DOWN=cmd_LEFT=cmd_RIGHT=0;
 Juego_Cartel(victoria?"winner.bin":"gameover.bin",242,148);
}
static uint8_t Juego_Contacto(void) {
 return Juego_Solapa(jugador.x-6,jugador.y-9,12,9,monstruo.x-8,monstruo.y-10,16,10) ||
 (juego_dos && Juego_Solapa(jugador2.x-6,jugador2.y-9,12,9,monstruo.x-8,monstruo.y-10,16,10));
}
static void Juego_Recoger(const JuegoActor *a) {
 for(uint8_t i=0;i<mapa_n && !juego_fin;i++) {
  if(obj_activo[i] && mapa_objs[i].tipo==(fase?TIPO_UVA:TIPO_BANANA)) {
   int16_t w,h;Spr_De(mapa_objs[i].tipo,&w,&h);
   if(Juego_Solapa(a->x-6,a->y-10,12,10,mapa_objs[i].x,mapa_objs[i].y,w,h))Fruta_Comer(i);
  }
 }
}
static uint8_t Juego_Mover(JuegoActor *a,uint8_t dir,uint8_t pasos,uint8_t troll) {
 uint8_t movio=0;a->dir=dir;
 for(uint8_t k=0;k<pasos;k++) {
  int x=a->x+juego_dx[dir],y=a->y+juego_dy[dir];
  if(!Juego_Libre(x,y))break;
  a->x=x;a->y=y;movio=1;
  if(Juego_Contacto()){Juego_Terminar(0);break;}
  if(!troll)Juego_Recoger(a);
  if(juego_fin)break;
 }
 return movio;
}
static uint8_t Juego_Buscar(JuegoActor *a,int cx,int cy) {
 int mejor=32767;uint8_t ok=0;
 for(int y=37;y<=(mapa_nivel?207:199);y++)for(int x=24;x<=295;x++)if(Juego_Libre(x,y)) {
  int dist=(x-cx)*(x-cx)+(y-cy)*(y-cy);
  if(dist<mejor){mejor=dist;a->x=x;a->y=y;ok=1;}
 }
 return ok;
}
static void Juego_Iniciar(void) {
 for (uint8_t i = 0; i < mapa_n; i++) { hielo_estado[i] = 0; }
 cmd_A2 = 0;
 muriendo=0;accion_activa[0]=accion_activa[1]=0;Sound8Bit_StartBackground();
 juego_fin=0;juego_dos=(modo_jug!=0);jugador2=(JuegoActor){0,0,1,0,0};jugador=(JuegoActor){0,0,1,0,0};monstruo=(JuegoActor){0,0,0,0,0};
 jugador.sabor=sabor_elegido[0]<3?sabor_elegido[0]:0;
 if(!Juego_Buscar(&jugador,160,158)||!Juego_Buscar(&monstruo,40,90)){Juego_Terminar(0);return;}
 jugador2.sabor=sabor_elegido[1]<3?sabor_elegido[1]:0;
 if(juego_dos && !Juego_Buscar(&jugador2,183,180)){Juego_Terminar(0);return;}
 vaca_ruta=-1;vaca_busqueda=HAL_GetTick()-180;
 juego_tick=juego_anim=HAL_GetTick();paso_jugador[0]=paso_jugador[1]=juego_tick-120;juego_rng^=juego_tick;
 if(juego_dos)Juego_Parche(jugador2.x,jugador2.y);
 Juego_Parche(jugador.x,jugador.y);Juego_Parche(monstruo.x,monstruo.y);
}
static void Hielo_Atacar(const JuegoActor *a) {
 int x=a->x-6,y=a->y-7,w=12,h=7;
 /* Extiende la zona de contacto 18 pixeles en la direccion de la mirada. */
 if(a->dir==0)w+=18;
 else if(a->dir==2){x-=18;w+=18;}
 else if(a->dir==1)h+=18;
 else {y-=18;h+=18;}
 int mejor=-1,distancia=32767;
 for(uint8_t i=0;i<mapa_n;i++) {
  if(!obj_activo[i] || mapa_objs[i].tipo!=TIPO_BLOQUE || hielo_estado[i])continue;
  if(!Juego_Solapa(x,y,w,h,mapa_objs[i].x,mapa_objs[i].y+9,BLOQUE_W,BLOQUE_H-9))continue;
  int dx=mapa_objs[i].x+BLOQUE_W/2-a->x,dy=mapa_objs[i].y+BLOQUE_H-a->y;
  int d=dx*dx+dy*dy;if(d<distancia){distancia=d;mejor=i;}
 }
 if(mejor>=0){
  uint8_t id=(a==&jugador2);accion_activa[id]=1;accion_inicio[id]=HAL_GetTick();
  hielo_estado[mejor]=1;hielo_inicio[mejor]=HAL_GetTick();Obj_Redibujar((uint8_t)mejor);}
}
static void Hielo_Actualizar(uint32_t now) {
 for(uint8_t i=0;i<mapa_n;i++)if(hielo_estado[i]) {
  uint32_t edad=now-hielo_inicio[i];
  if(edad>=180){hielo_estado[i]=0;obj_activo[i]=0;Obj_Redibujar(i);}
  else if(edad>=90 && hielo_estado[i]==1){hielo_estado[i]=2;Obj_Redibujar(i);}
 }
}
static int8_t Vaca_Perseguir(void) {
 /* Busca al jugador mas cercano dentro de 90 pixeles. */
 const JuegoActor *objetivo=&jugador;
 int dx=jugador.x-monstruo.x,dy=jugador.y-monstruo.y,dist=dx*dx+dy*dy;
 if(juego_dos){int x=jugador2.x-monstruo.x,y=jugador2.y-monstruo.y,d=x*x+y*y;if(d<dist){objetivo=&jugador2;dist=d;}}
 if(dist>90*90)return -1;
 /* BFS local: rutas de 6 px, comprobadas pixel por pixel. */
 static int8_t primera[25*25];static uint16_t cola[25*25];
 memset(primera,-2,sizeof(primera));
 uint16_t ini=12*25+12,cabeza=0,fin=0;cola[fin++]=ini;primera[ini]=-1;
 int mejor=dist;int8_t salida=-1;
 while(cabeza<fin){
  uint16_t celda=cola[cabeza++];int cx=celda%25,cy=celda/25;
  int x=monstruo.x+(cx-12)*6,y=monstruo.y+(cy-12)*6;
  int tx=x-objetivo->x,ty=y-objetivo->y,d=tx*tx+ty*ty;
  if(d<mejor){mejor=d;salida=primera[celda];if(d<=9)break;}
  for(uint8_t dir=0;dir<4;dir++){
   int nx=cx+juego_dx[dir],ny=cy+juego_dy[dir];
   if(nx<0||nx>=25||ny<0||ny>=25)continue;
   uint16_t siguiente=ny*25+nx;if(primera[siguiente]!=-2)continue;
   uint8_t libre=1;
   for(uint8_t k=1;k<=6;k++)if(!Juego_Libre(x+juego_dx[dir]*k,y+juego_dy[dir]*k)){libre=0;break;}
   if(!libre)continue;
   primera[siguiente]=(celda==ini)?(int8_t)dir:primera[celda];cola[fin++]=siguiente;
  }
 }
 return salida;
}
static void Juego_Pausar(void){
 Sound8Bit_Stop();
 pausa_inicio=HAL_GetTick();pantalla=PANT_PAUSA;
 cmd_A=cmd_A2=cmd_B=cmd_UP=cmd_DOWN=cmd_LEFT=cmd_RIGHT=0;
 Juego_Cartel("pausa.bin",196,98);
}
static void Juego_Reanudar(void){
 uint32_t now=HAL_GetTick(),duracion=now-pausa_inicio;
 for(uint8_t i=0;i<mapa_n;i++)if(hielo_estado[i])hielo_inicio[i]+=duracion;
 for(uint8_t id=0;id<2;id++)if(accion_activa[id])accion_inicio[id]+=duracion;
 /* Reconstruye desde fondo limpio y el estado actual, sin reiniciar objetos. */
 pantalla=PANT_JUEGO;
 for(int y=0;y<LCD_H;y+=24)for(int x=0;x<LCD_W;x+=32)
  Nivel_RedibujarRect(x,y,32,(LCD_H-y<24)?LCD_H-y:24);
 if(fase){
  if(mapa_nivel)LCD_Bitmap(BARRA2_X,BARRA2_Y,BARRA2_W,BARRA2_H,(unsigned char*)barra_uvas2);
  else LCD_Bitmap(BARRA_X,BARRA_Y,BARRA_W,BARRA_H,(unsigned char*)barra_uvas);
 }
 now=HAL_GetTick();juego_tick=juego_anim=now;paso_jugador[0]=paso_jugador[1]=now;
 vaca_busqueda=now-180;vaca_ruta=-1;Sound8Bit_StartBackground();
 cmd_A=cmd_A2=cmd_B=cmd_UP=cmd_DOWN=cmd_LEFT=cmd_RIGHT=0;
}
static void Juego_Actualizar(void) {
 if(muriendo){
  cmd_A=cmd_A2=cmd_B=cmd_UP=cmd_DOWN=cmd_LEFT=cmd_RIGHT=0;
  uint32_t now=HAL_GetTick();
  if(now-muerte_inicio>=650){Juego_Terminar(0);return;}
  if(now-juego_anim>=100){juego_anim=now;Juego_Parche(jugador.x,jugador.y);if(juego_dos)Juego_Parche(jugador2.x,jugador2.y);}
  return;
 }
 if(cmd_B){cmd_B=0;Juego_Pausar();return;}
 if(juego_fin){cmd_UP=cmd_DOWN=cmd_LEFT=cmd_RIGHT=cmd_A=cmd_A2=0;return;}
 uint32_t now=HAL_GetTick();if(now-juego_tick<35)return;juego_tick=now;
 Hielo_Actualizar(now);
 for(uint8_t id=0;id<2;id++)if(accion_activa[id] && now-accion_inicio[id]>=420){
  accion_activa[id]=0;JuegoActor *a=id?&jugador2:&jugador;Juego_Parche(a->x,a->y);
 }
 if(cmd_A){cmd_A=0;Hielo_Atacar(&jugador);}
 if(cmd_A2){cmd_A2=0;if(juego_dos)Hielo_Atacar(&jugador2);}
 int jx=jugador.x,jy=jugador.y,mx=monstruo.x,my=monstruo.y;
 int j2x=jugador2.x,j2y=jugador2.y;
 cmd_UP=cmd_DOWN=cmd_LEFT=cmd_RIGHT=cmd_A=0;
 for(uint8_t id=0;id<(juego_dos?2:1) && !juego_fin;id++) {
  uint8_t estado=mando_estado[id];
  if(now-mando_ultimo[id]>350)estado=0;
  int dir=-1;
  if(estado&4)dir=3;else if(estado&8)dir=1;else if(estado&16)dir=2;else if(estado&32)dir=0;
  if(dir>=0 && !accion_activa[id] && now-paso_jugador[id]>=120) {
   paso_jugador[id]=now;Juego_Mover(id?&jugador2:&jugador,(uint8_t)dir,16,0);
  }
 }
 if(!juego_fin) {
  /* Patrulla; al bloquearse elige otra salida, sin borrar frutas. */
  juego_rng^=juego_rng<<13;juego_rng^=juego_rng>>17;juego_rng^=juego_rng<<5;
  uint8_t dir_mon=monstruo.dir;
  /* Decide en cruces y espacios abiertos; evita regresar si hay otra salida. */
  if(mapa_nivel && now-vaca_busqueda>=180){vaca_busqueda=now;vaca_ruta=Vaca_Perseguir();}
  if(mapa_nivel && vaca_ruta>=0)dir_mon=(uint8_t)vaca_ruta;
  if((!mapa_nivel || vaca_ruta<0) && (juego_rng&7)==0) {
   uint8_t opciones[4],n=0;
   for(uint8_t d=0;d<4;d++)
    if(d!=((monstruo.dir+2)&3) &&
       Juego_Libre(monstruo.x+juego_dx[d]*3,monstruo.y+juego_dy[d]*3))opciones[n++]=d;
   if(n)dir_mon=opciones[(juego_rng>>8)%n];
  }
  if(!Juego_Mover(&monstruo,dir_mon,3,1) && !juego_fin)
   for(uint8_t k=0;k<4;k++) {
    uint8_t d=((juego_rng>>16)+k)&3;
    if(Juego_Mover(&monstruo,d,3,1)||juego_fin)break;
   }
 }
 if(juego_fin)return; /* No dibujar el mapa encima de winner/gameover. */
 uint8_t animar=(now-juego_anim>=140);
 if(animar){juego_anim=now;jugador.frame++;jugador2.frame++;monstruo.frame++;}
 if(animar || jx!=jugador.x || jy!=jugador.y)Juego_ParcheMovimiento(jx,jy,jugador.x,jugador.y);
 if(animar || mx!=monstruo.x || my!=monstruo.y)Juego_ParcheMovimiento(mx,my,monstruo.x,monstruo.y);
 if(juego_dos && (animar || j2x!=jugador2.x || j2y!=jugador2.y))Juego_ParcheMovimiento(j2x,j2y,jugador2.x,jugador2.y);
}


static void Nivel_JuegoMostrar(uint8_t nivel)

{

  mapa_nivel=nivel;
  mapa_objs=nivel?nivel2_objs:nivel1_objs;
  mapa_n=nivel?N2_OBJ:N_OBJ;
  mapa_bananas=nivel?N2_BANANAS:N_BANANAS;
  mapa_uvas=nivel?N2_UVAS:N_UVAS;
  mapa_fondo=nivel?"fondo2.bin":"fondo1.bin";
  mapa_inicio=nivel?"n2start.bin":"n1start.bin";
  pantalla = PANT_JUEGO;

  Pantalla_SD(mapa_inicio);                     // nivel completo con bananas

  for (uint8_t i = 0; i < mapa_n; i++) obj_activo[i] = (mapa_objs[i].tipo != TIPO_UVA);

  fase = 0; frutas_restantes = mapa_bananas;

  cmd_UP = 0; cmd_DOWN = 0; cmd_LEFT = 0; cmd_RIGHT = 0; cmd_A = 0; cmd_B = 0;

  Juego_Iniciar();
  DebugPC(mapa_nivel?"NIVEL 2\r\n":"NIVEL 1\r\n");

}



// Se acabaron las bananas: pasa el selector a las uvas y aparecen las uvas

static void Fase_Uvas(void)

{

  fase = 1; frutas_restantes = mapa_uvas;

  if(mapa_nivel)LCD_Bitmap(BARRA2_X,BARRA2_Y,BARRA2_W,BARRA2_H,(unsigned char*)barra_uvas2);
  else LCD_Bitmap(BARRA_X,BARRA_Y,BARRA_W,BARRA_H,(unsigned char*)barra_uvas);

  for (uint8_t i = 0; i < mapa_n; i++)

    if (mapa_objs[i].tipo == TIPO_UVA) { obj_activo[i] = 1; Obj_Redibujar(i); }

  DebugPC("FASE UVAS\r\n");

}



// Para cuando el jugador se coma una fruta (i = indice en nivel1_objs) o rompa un bloque

void Fruta_Comer(uint8_t i)

{

  if (i>=mapa_n || !obj_activo[i] || mapa_objs[i].tipo == TIPO_BLOQUE) return;

  obj_activo[i] = 0;

  Obj_Redibujar(i);

  if (--frutas_restantes == 0) {

    if (fase == 0) Fase_Uvas();

    else Juego_Terminar(1);

  }

}



void Bloque_Romper(uint8_t i)

{

  if (i>=mapa_n || !obj_activo[i] || mapa_objs[i].tipo != TIPO_BLOQUE) return;

  obj_activo[i] = 0;

  Obj_Redibujar(i);

}



/* USER CODE END PV */



/* Private function prototypes -----------------------------------------------*/

void SystemClock_Config(void);

static void MX_GPIO_Init(void);

static void MX_USART2_UART_Init(void);

static void MX_SPI1_Init(void);

static void MX_USART1_UART_Init(void);

/* USER CODE BEGIN PFP */



/* USER CODE END PFP */



/* Private user code ---------------------------------------------------------*/

/* USER CODE BEGIN 0 */



/* USER CODE END 0 */



/**

  * @brief  The application entry point.

  * @retval int

  */

int main(void)

{



  /* USER CODE BEGIN 1 */



  /* USER CODE END 1 */



  /* MCU Configuration--------------------------------------------------------*/



  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */

  HAL_Init();



  /* USER CODE BEGIN Init */



  /* USER CODE END Init */



  /* Configure the system clock */

  SystemClock_Config();



  /* USER CODE BEGIN SysInit */



  /* USER CODE END SysInit */



  /* Initialize all configured peripherals */

  MX_GPIO_Init();
  Juego_Audio_Init();

  MX_USART2_UART_Init();

  MX_SPI1_Init();

  MX_USART1_UART_Init();

  MX_FATFS_Init();



  /* USER CODE BEGIN 2 */

  HAL_GPIO_WritePin(GPIOB, LCD_CS_Pin | SD_CS_Pin, GPIO_PIN_SET);   // LCD y SD deseleccionados



  if (f_mount(&USERFatFS, USERPath, 1) == FR_OK) DebugPC("SD OK\r\n");

  else                                           DebugPC("SD ERROR\r\n");



  LCD_Init();



  // Pantalla de inicio (desde la SD)

  Pantalla_SD("inicio.bin");

  DebugPC("\r\n=== Pantalla de inicio dibujada ===\r\n");



  // Sprite "click" centrado, empieza en el cuadro 0

  uint32_t t_click = HAL_GetTick();

  uint8_t frame_click = 0;

  LCD_SpriteBG(CLICK_X, CLICK_Y, CLICK_W, CLICK_H, click_Sprite, CLICK_FRAMES, frame_click, 0, 0x0000, CLICK_BG);



  HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);

  HAL_NVIC_EnableIRQ(USART1_IRQn);

  HAL_UART_Receive_IT(&huart1, &rx_byte, 1);

  /* USER CODE END 2 */



  /* Infinite loop */

  /* USER CODE BEGIN WHILE */

  while (1)

  {
    Sound8Bit_Update();

    /* USER CODE END WHILE */



    /* USER CODE BEGIN 3 */

	    if (pantalla == PANT_INICIO) {



	      if (cmd_A || cmd_B) {

	        Menu_Mostrar();

	        DebugPC("MENU\r\n");

	      }

	      else if (HAL_GetTick() - t_click >= CLICK_PERIODO) {

	        t_click = HAL_GetTick();

	        frame_click = (frame_click + 1) % CLICK_FRAMES;

	        LCD_SpriteBG(CLICK_X, CLICK_Y, CLICK_W, CLICK_H, click_Sprite, CLICK_FRAMES, frame_click, 0, 0x0000, CLICK_BG);

	      }

	    }

	    else if (pantalla == PANT_MENU) {



	      // Mover el selector

	      if (cmd_UP) {

	        cmd_UP = 0;

	        if (sel_pos != 0) Selector_Mover(0);

	      }

	      if (cmd_DOWN) {

	        cmd_DOWN = 0;

	        if (sel_pos != 1) Selector_Mover(1);

	      }



	      // Seleccionar con A o B

	      if (cmd_A || cmd_B) {

	        cmd_A = 0;

	        cmd_B = 0;



	        if (sel_pos == 0) {

	            Jugadores_Mostrar();

	        }else {

	          pantalla = PANT_CREDITOS;

	          Pantalla_SD("creditos.bin");

	          DebugPC("CREDITOS\r\n");

	        }

	      }

	      // Blink de los selectores

	      else if (HAL_GetTick() - t_sel >= SEL_PERIODO) {

	        t_sel = HAL_GetTick();

	        sel_visible ^= 1;

	        if (sel_visible) Selectores_Dibujar();

	        else             Selectores_Borrar();

	      }

	    }

	    else if (pantalla == PANT_JUGADORES) {

	      cmd_UP = 0; cmd_DOWN = 0;



	      if (cmd_LEFT)  { cmd_LEFT = 0;  if (modo_jug != 0) { modo_jug = 0; frame_p1 = 0; frame_p2 = 0; Jugadores_Dibujar(); } }

	      if (cmd_RIGHT) { cmd_RIGHT = 0; if (modo_jug != 1) { modo_jug = 1; frame_p1 = 0; frame_p2 = 0; Jugadores_Dibujar(); } }



	      if (HAL_GetTick() - t_p1 >= P1_PERIODO) {

	        t_p1 = HAL_GetTick();

	        if (modo_jug == 0) {

	          frame_p1 = (frame_p1 + 1) % P1_FRAMES;

	          LCD_SpritePatch(P1_X, P1_Y, P1_W, P1_H, (const unsigned char*)player1_seleccion, P1_FRAMES, frame_p1, P1_TRANSP, fondo_p1);

	        } else {

	          frame_p2 = (frame_p2 + 1) % P2_FRAMES;

	          LCD_SpritePatch(P2_X, P2_Y, P2_W, P2_H, (const unsigned char*)player2_seleccion, P2_FRAMES, frame_p2, P2_TRANSP, fondo_p2);

	        }

	      }



	      if (cmd_A) {

	          cmd_A = 0;

	          Sabor_Mostrar();

	      }



	      if (cmd_B) { cmd_B = 0; Menu_Mostrar(); }

	    }

	    else if (pantalla == PANT_CREDITOS) {

	      cmd_UP = 0; cmd_DOWN = 0;

	      if (cmd_A || cmd_B) Menu_Mostrar();   // A o B regresan al menu

	    }



	    else if (pantalla == PANT_SABOR) {

	      cmd_UP = 0; cmd_DOWN = 0;

	      uint8_t t = sab_turno;



	      if (cmd_LEFT)  { cmd_LEFT = 0;  if (!sab_conf[t] && sab_cur[t] > 0) { sab_cur[t]--; sab_frame[t] = 0; Sab_Redibujar(t); } }

	      if (cmd_RIGHT) { cmd_RIGHT = 0; if (!sab_conf[t] && sab_cur[t] < 2) { sab_cur[t]++; sab_frame[t] = 0; Sab_Redibujar(t); } }



	      if (cmd_A) {

	        cmd_A = 0;

	        if (!sab_conf[t] && !(t == 1 && sab_cur[1] == sab_cur[0])) {

	          sab_conf[t] = 1; sabor_elegido[t] = sab_cur[t];

	          sab_self[t] = 0; Sab_Redibujar(t); t_sab = HAL_GetTick();

	        }

	      }



	      if (cmd_B) {

	        cmd_B = 0;

	        if (sab_listo) {                       // cancelar la ultima eleccion

	          sab_listo = 0; sab_conf[t] = 0; sab_self[t] = -1; sab_frame[t] = 0; Sab_Redibujar(t);

	        } else if (t == 1) {                   // J2 regresa a J1

	          sab_conf[1] = 0; sab_self[1] = -1; sab_frame[1] = 0; Sab_Redibujar(1);

	          sab_turno = 0; sab_conf[0] = 0; sab_self[0] = -1; Sab_Redibujar(0);

	        } else {

	          Jugadores_Mostrar();

	        }

	      }



	      if (pantalla == PANT_SABOR && !sab_listo && HAL_GetTick() - t_sab >= SAB_PERIODO) {

	        t_sab = HAL_GetTick();

	        t = sab_turno;

	        if (!sab_conf[t]) { sab_frame[t] = (sab_frame[t] + 1) % 3; Sab_Redibujar(t); }

	        else if (sab_self[t] < 2) { sab_self[t]++; Sab_Redibujar(t); }

	        else if (t == 0 && num_jug == 2) { sab_turno = 1; }

	        else { sab_listo = 1; t_sab = HAL_GetTick(); DebugPC("SABORES LISTOS\r\n"); }

	      }

	      if (pantalla == PANT_SABOR && sab_listo && HAL_GetTick() - t_sab >= 600) Nivel_Mostrar();

	    }



	    else if (pantalla == PANT_NIVEL) {

	    	  cmd_UP = 0; cmd_DOWN = 0;



	    	  if (cmd_LEFT)  { cmd_LEFT = 0;  if (nivel_sel > 0) Nivel_Mover(nivel_sel - 1); }

	    	  if (cmd_RIGHT) { cmd_RIGHT = 0; if (nivel_sel < 2) Nivel_Mover(nivel_sel + 1); }



	    	  if (cmd_A) {

	    	    cmd_A = 0;

	    	    nivel_elegido = nivel_sel;

	    	    if (nivel_sel <= 1) Nivel_JuegoMostrar(nivel_sel);

	    	    else DebugPC("Nivel aun no listo\r\n");

	    	  }

	      if (cmd_B) { cmd_B = 0; Sabor_Mostrar(); }    // regresa a elegir sabor

	    }



        else if (pantalla == PANT_JUEGO) {
            Juego_Actualizar();
        }
        else if(pantalla==PANT_PAUSA){
            if(cmd_B){cmd_B=0;Juego_Reanudar();}
            cmd_A=cmd_A2=cmd_UP=cmd_DOWN=cmd_LEFT=cmd_RIGHT=0;
        }
        else if(pantalla==PANT_GAMEOVER || pantalla==PANT_VICTORIA){
            if(cmd_A || cmd_B){cmd_A=cmd_B=0;Nivel_Mostrar();}
            cmd_A2=cmd_UP=cmd_DOWN=cmd_LEFT=cmd_RIGHT=0;
        }



  }

  /* USER CODE END 3 */

}



/**

  * @brief System Clock Configuration

  * @retval None

  */

void SystemClock_Config(void)

{

  RCC_OscInitTypeDef RCC_OscInitStruct = {0};

  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};



  /** Configure the main internal regulator output voltage

  */

  __HAL_RCC_PWR_CLK_ENABLE();

  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);



  /** Initializes the RCC Oscillators according to the specified parameters

  * in the RCC_OscInitTypeDef structure.

  */

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;

  RCC_OscInitStruct.HSIState = RCC_HSI_ON;

  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;

  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;

  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;

  RCC_OscInitStruct.PLL.PLLM = 16;

  RCC_OscInitStruct.PLL.PLLN = 336;

  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;

  RCC_OscInitStruct.PLL.PLLQ = 2;

  RCC_OscInitStruct.PLL.PLLR = 2;

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)

  {

    Error_Handler();

  }



  /** Initializes the CPU, AHB and APB buses clocks

  */

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK

                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;

  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;

  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;

  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;

  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;



  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)

  {

    Error_Handler();

  }

}



/**

  * @brief SPI1 Initialization Function

  * @param None

  * @retval None

  */

static void MX_SPI1_Init(void)

{



  /* USER CODE BEGIN SPI1_Init 0 */



  /* USER CODE END SPI1_Init 0 */



  /* USER CODE BEGIN SPI1_Init 1 */



  /* USER CODE END SPI1_Init 1 */

  /* SPI1 parameter configuration*/

  hspi1.Instance = SPI1;

  hspi1.Init.Mode = SPI_MODE_MASTER;

  hspi1.Init.Direction = SPI_DIRECTION_2LINES;

  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;

  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;

  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;

  hspi1.Init.NSS = SPI_NSS_SOFT;

  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;

  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;

  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;

  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;

  hspi1.Init.CRCPolynomial = 10;

  if (HAL_SPI_Init(&hspi1) != HAL_OK)

  {

    Error_Handler();

  }

  /* USER CODE BEGIN SPI1_Init 2 */



  /* USER CODE END SPI1_Init 2 */



}



/**

  * @brief USART1 Initialization Function

  * @param None

  * @retval None

  */

static void MX_USART1_UART_Init(void)

{



  /* USER CODE BEGIN USART1_Init 0 */



  /* USER CODE END USART1_Init 0 */



  /* USER CODE BEGIN USART1_Init 1 */



  /* USER CODE END USART1_Init 1 */

  huart1.Instance = USART1;

  huart1.Init.BaudRate = 115200;

  huart1.Init.WordLength = UART_WORDLENGTH_8B;

  huart1.Init.StopBits = UART_STOPBITS_1;

  huart1.Init.Parity = UART_PARITY_NONE;

  huart1.Init.Mode = UART_MODE_TX_RX;

  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;

  huart1.Init.OverSampling = UART_OVERSAMPLING_16;

  if (HAL_UART_Init(&huart1) != HAL_OK)

  {

    Error_Handler();

  }

  /* USER CODE BEGIN USART1_Init 2 */



  /* USER CODE END USART1_Init 2 */



}



/**

  * @brief USART2 Initialization Function

  * @param None

  * @retval None

  */

static void MX_USART2_UART_Init(void)

{



  /* USER CODE BEGIN USART2_Init 0 */



  /* USER CODE END USART2_Init 0 */



  /* USER CODE BEGIN USART2_Init 1 */



  /* USER CODE END USART2_Init 1 */

  huart2.Instance = USART2;

  huart2.Init.BaudRate = 115200;

  huart2.Init.WordLength = UART_WORDLENGTH_8B;

  huart2.Init.StopBits = UART_STOPBITS_1;

  huart2.Init.Parity = UART_PARITY_NONE;

  huart2.Init.Mode = UART_MODE_TX_RX;

  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;

  huart2.Init.OverSampling = UART_OVERSAMPLING_16;

  if (HAL_UART_Init(&huart2) != HAL_OK)

  {

    Error_Handler();

  }

  /* USER CODE BEGIN USART2_Init 2 */



  /* USER CODE END USART2_Init 2 */



}



/**

  * @brief GPIO Initialization Function

  * @param None

  * @retval None

  */

static void MX_GPIO_Init(void)

{

  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* USER CODE BEGIN MX_GPIO_Init_1 */



  /* USER CODE END MX_GPIO_Init_1 */



  /* GPIO Ports Clock Enable */

  __HAL_RCC_GPIOC_CLK_ENABLE();

  __HAL_RCC_GPIOH_CLK_ENABLE();

  __HAL_RCC_GPIOA_CLK_ENABLE();

  __HAL_RCC_GPIOB_CLK_ENABLE();



  /*Configure GPIO pin Output Level */

  HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_RESET);



  /*Configure GPIO pin Output Level */

  HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET);



  /*Configure GPIO pin Output Level */

  HAL_GPIO_WritePin(GPIOB, LCD_CS_Pin|SD_CS_Pin, GPIO_PIN_RESET);



  /*Configure GPIO pin : B1_Pin */

  GPIO_InitStruct.Pin = B1_Pin;

  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;

  GPIO_InitStruct.Pull = GPIO_NOPULL;

  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);



  /*Configure GPIO pin : LCD_RESET_Pin */

  GPIO_InitStruct.Pin = LCD_RESET_Pin;

  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;

  GPIO_InitStruct.Pull = GPIO_NOPULL;

  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;

  HAL_GPIO_Init(LCD_RESET_GPIO_Port, &GPIO_InitStruct);



  /*Configure GPIO pin : LCD_DC_Pin */

  GPIO_InitStruct.Pin = LCD_DC_Pin;

  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;

  GPIO_InitStruct.Pull = GPIO_NOPULL;

  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;

  HAL_GPIO_Init(LCD_DC_GPIO_Port, &GPIO_InitStruct);



  /*Configure GPIO pins : LCD_CS_Pin SD_CS_Pin */

  GPIO_InitStruct.Pin = LCD_CS_Pin|SD_CS_Pin;

  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;

  GPIO_InitStruct.Pull = GPIO_NOPULL;

  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;

  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);



  /* USER CODE BEGIN MX_GPIO_Init_2 */



  /* USER CODE END MX_GPIO_Init_2 */

}



/* USER CODE BEGIN 4 */



/* USER CODE END 4 */



/**

  * @brief  This function is executed in case of error occurrence.

  * @retval None

  */

void Error_Handler(void)

{

  /* USER CODE BEGIN Error_Handler_Debug */

  /* User can add his own implementation to report the HAL error return state */

  __disable_irq();

  while (1)

  {

  }

  /* USER CODE END Error_Handler_Debug */

}

#ifdef USE_FULL_ASSERT

/**

  * @brief  Reports the name of the source file and the source line number

  *         where the assert_param error has occurred.

  * @param  file: pointer to the source file name

  * @param  line: assert_param error line source number

  * @retval None

  */

void assert_failed(uint8_t *file, uint32_t line)

{

  /* USER CODE BEGIN 6 */

  /* User can add his own implementation to report the file name and line number,

     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* USER CODE END 6 */

}

#endif /* USE_FULL_ASSERT */
