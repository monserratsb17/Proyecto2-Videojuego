/*
 * ili9341.c
 *
 *  Created on: Aug 20, 2024
 *      Author: Pablo Mazariegos
 */
#include <stdlib.h> // malloc()
#include <string.h> // memset()
#include "pgmspace.h"
#include "ili9341.h"
#include "main.h"
#include "ff.h"

extern const uint8_t smallFont[1140];
extern const uint16_t bigFont[1520];
extern SPI_HandleTypeDef hspi1;
//***************************************************************************************************************************************
// Función para inicializar LCD
//***************************************************************************************************************************************
void LCD_Init(void) {

	//****************************************
	// Secuencia de Inicialización
	//****************************************
	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
	/*Configure GPIO pin Output Level */
	//HAL_GPIO_WritePin(GPIOA, LCD_RD_Pin | LCD_WR_Pin | LCD_RS_Pin,GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin,GPIO_PIN_SET);

	/*Configure GPIO pin Output Level */
	HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_SET);
	HAL_Delay(5);
	HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_RESET);
	HAL_Delay(20);
	HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_SET);
	HAL_Delay(150);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);

	//****************************************
	LCD_CMD(0xE9);  // SETPANELRELATED
	LCD_DATA(0x20);
	//****************************************
	LCD_CMD(0x11); // Exit Sleep SLEEP OUT (SLPOUT)
	HAL_Delay(100);
	//****************************************
	LCD_CMD(0xD1);    // (SETVCOM)
	LCD_DATA(0x00);
	LCD_DATA(0x71);
	LCD_DATA(0x19);
	//****************************************
	LCD_CMD(0xD0);   // (SETPOWER)
	LCD_DATA(0x07);
	LCD_DATA(0x01);
	LCD_DATA(0x08);
	//****************************************
	LCD_CMD(0x36);  // (MEMORYACCESS)
	LCD_DATA(0x40 | 0x80 | 0x20 | 0x08); // LCD_DATA(0x19);
	//****************************************
	LCD_CMD(0x3A); // Set_pixel_format (PIXELFORMAT)
	LCD_DATA(0x05); // color setings, 05h - 16bit pixel, 11h - 3bit pixel
	//****************************************
	LCD_CMD(0xC1);    // (POWERCONTROL2)
	LCD_DATA(0x10);
	LCD_DATA(0x10);
	LCD_DATA(0x02);
	LCD_DATA(0x02);
	//****************************************
	LCD_CMD(0xC0); // Set Default Gamma (POWERCONTROL1)
	LCD_DATA(0x00);
	LCD_DATA(0x35);
	LCD_DATA(0x00);
	LCD_DATA(0x00);
	LCD_DATA(0x01);
	LCD_DATA(0x02);
	//****************************************
	LCD_CMD(0xC5); // Set Frame Rate (VCOMCONTROL1)
	LCD_DATA(0x04); // 72Hz
	//****************************************
	LCD_CMD(0xD2); // Power Settings  (SETPWRNORMAL)
	LCD_DATA(0x01);
	LCD_DATA(0x44);
	//****************************************
	LCD_CMD(0xC8); //Set Gamma  (GAMMASET)
	LCD_DATA(0x04);
	LCD_DATA(0x67);
	LCD_DATA(0x35);
	LCD_DATA(0x04);
	LCD_DATA(0x08);
	LCD_DATA(0x06);
	LCD_DATA(0x24);
	LCD_DATA(0x01);
	LCD_DATA(0x37);
	LCD_DATA(0x40);
	LCD_DATA(0x03);
	LCD_DATA(0x10);
	LCD_DATA(0x08);
	LCD_DATA(0x80);
	LCD_DATA(0x00);
	//****************************************
	LCD_CMD(0x2A); // Set_column_address 320px (CASET)
	LCD_DATA(0x00);
	LCD_DATA(0x00);
	LCD_DATA(0x01);
	LCD_DATA(0x3F);
	//****************************************
	LCD_CMD(0x2B); // Set_page_address 480px (PASET)
	LCD_DATA(0x00);
	LCD_DATA(0x00);
	LCD_DATA(0x01);
	LCD_DATA(0xE0);
	//  LCD_DATA(0x8F);
	LCD_CMD(0x29); //display on
	LCD_CMD(0x2C); //display on

	LCD_CMD(ILI9341_INVOFF); //Invert Off
	HAL_Delay(120);
	LCD_CMD(ILI9341_SLPOUT);    //Exit Sleep
	HAL_Delay(120);
	LCD_CMD(ILI9341_DISPON);    //Display on
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}
//***************************************************************************************************************************************
// Función para enviar comandos a la LCD - parámetro (comando)
//***************************************************************************************************************************************
void LCD_CMD(uint8_t cmd) {
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET);
	//HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_RESET);
	//HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_RESET);

	HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
	//HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_SET);
	//HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);


}
//***************************************************************************************************************************************
// Función para enviar datos a la LCD - parámetro (dato)
//***************************************************************************************************************************************
void LCD_DATA(uint8_t data) {
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
	//HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_RESET);
	//HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_RESET);

	HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
	//HAL_GPIO_WritePin(LCD_WR_GPIO_Port, LCD_WR_Pin, GPIO_PIN_SET);
	//HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);

}
//***************************************************************************************************************************************
// Función para definir rango de direcciones de memoria con las cuales se trabajara (se define una ventana)
//***************************************************************************************************************************************
void SetWindows(unsigned int x1, unsigned int y1, unsigned int x2,
		unsigned int y2) {
	LCD_CMD(0x2a); // Set_column_address 4 parameters
	LCD_DATA(x1 >> 8);
	LCD_DATA(x1);
	LCD_DATA(x2 >> 8);
	LCD_DATA(x2);
	LCD_CMD(0x2b); // Set_page_address 4 parameters
	LCD_DATA(y1 >> 8);
	LCD_DATA(y1);
	LCD_DATA(y2 >> 8);
	LCD_DATA(y2);
	LCD_CMD(0x2c); // Write_memory_start
}
//***************************************************************************************************************************************
// Función para borrar la pantalla - parámetros (color)
//***************************************************************************************************************************************
void LCD_Clear(unsigned int c) {
	unsigned int x, y;
	LCD_CMD(0x02c); // write_memory_start
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
	SetWindows(0, 0, 319, 239); // 479, 319);
	for (x = 0; x < 320; x++)
		for (y = 0; y < 240; y++) {
			LCD_DATA(c >> 8);
			LCD_DATA(c);
		}
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}
//***************************************************************************************************************************************
// Función para dibujar una línea horizontal - parámetros ( coordenada x, cordenada y, longitud, color)
//***************************************************************************************************************************************
void H_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {
	unsigned int i;
	LCD_CMD(0x02c); //write_memory_start
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
	l = l + x;
	SetWindows(x, y, l, y);
	//j = l; // * 2;
	for (i = 0; i < l; i++) {
		LCD_DATA(c >> 8);
		LCD_DATA(c);
	}
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}
//***************************************************************************************************************************************
// Función para dibujar una línea vertical - parámetros ( coordenada x, cordenada y, longitud, color)
//***************************************************************************************************************************************
void V_line(unsigned int x, unsigned int y, unsigned int l, unsigned int c) {
	unsigned int i;
	LCD_CMD(0x02c); //write_memory_start
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
	l = l + y;
	SetWindows(x, y, x, l);
	//j = l; //* 2;
	for (i = 1; i <= l; i++) {
		LCD_DATA(c >> 8);
		LCD_DATA(c);
	}
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
void Rect(unsigned int x, unsigned int y, unsigned int w, unsigned int h,
		unsigned int c) {
	H_line(x, y, w, c);
	H_line(x, y + h, w, c);
	V_line(x, y, h, c);
	V_line(x + w, y, h, c);
}
//***************************************************************************************************************************************
// Función para dibujar un rectángulo relleno - parámetros ( coordenada x, cordenada y, ancho, alto, color)
//***************************************************************************************************************************************
/*void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h, unsigned int c) {
 unsigned int i;
 for (i = 0; i < h; i++) {
 H_line(x  , y  , w, c);
 H_line(x  , y+i, w, c);
 }
 }
 */

void FillRect(unsigned int x, unsigned int y, unsigned int w, unsigned int h,
		unsigned int c) {
	LCD_CMD(0x02c); // write_memory_start
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);

	unsigned int x2, y2;
	x2 = x + w;
	y2 = y + h;
	SetWindows(x, y, x2 - 1, y2 - 1);
	unsigned int k = w * h * 2 - 1;
	for (int i = 0; i < w; i++) {
		for (int j = 0; j < h; j++) {
			LCD_DATA(c >> 8);
			LCD_DATA(c);

			//LCD_DATA(bitmap[k]);
			k = k - 2;
		}
	}
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}
//***************************************************************************************************************************************
// Función para dibujar texto - parámetros ( texto, coordenada x, cordenada y, color, background)
//***************************************************************************************************************************************
void LCD_Print(char *text, int x, int y, int fontSize, int color,
		int background) {

	int fontXSize;
	int fontYSize;

	if (fontSize == 1) {
		fontXSize = fontXSizeSmal;
		fontYSize = fontYSizeSmal;
	}
	if (fontSize == 2) {
		fontXSize = fontXSizeBig;
		fontYSize = fontYSizeBig;
	}
	if (fontSize == 3) {
			fontXSize = fontXSizeNum;
			fontYSize = fontYSizeNum;
		}

	char charInput;
	int cLength = strlen(text);
	//Serial.println(cLength, DEC);
	int charDec;
	int c;
	//int charHex;
	char char_array[cLength + 1];
	for(int i = 0; text[i] != '\0'; i++){
		char_array[i]=text[i];
	}

	//text.toCharArray(char_array, cLength + 1);

	for (int i = 0; i < cLength; i++) {
		charInput = char_array[i];
		//Serial.println(char_array[i]);
		charDec = (int) charInput;
		HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
		SetWindows(x + (i * fontXSize), y, x + (i * fontXSize) + fontXSize - 1, y + fontYSize);
		long charHex1;
		for (int n = 0; n < fontYSize; n++) {
			if (fontSize == 1) {
				charHex1 = pgm_read_word_near(
						smallFont + ((charDec - 32) * fontYSize) + n);
			}
			if (fontSize == 2) {
				charHex1 = pgm_read_word_near(
						bigFont + ((charDec - 32) * fontYSize) + n);
			}
			for (int t = 1; t < fontXSize + 1; t++) {
				if ((charHex1 & (1 << (fontXSize - t))) > 0) {
					c = color;
				} else {
					c = background;
				}
				LCD_DATA(c >> 8);
				LCD_DATA(c);
			}
		}
		HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
	}
}
//***************************************************************************************************************************************
// Función para dibujar una imagen a partir de un arreglo de colores (Bitmap) Formato (Color 16bit R 5bits G 6bits B 5bits)
//***************************************************************************************************************************************
void LCD_Bitmap(unsigned int x, unsigned int y, unsigned int width,
		unsigned int height, unsigned char bitmap[]) {
	LCD_CMD(0x02c); // write_memory_start
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);

	unsigned int x2, y2;
	x2 = x + width;
	y2 = y + height;
	SetWindows(x, y, x2 - 1, y2 - 1);
	unsigned int k = 0;

	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
			LCD_DATA(bitmap[k]);
			LCD_DATA(bitmap[k + 1]);
			//LCD_DATA(bitmap[k]);
			k = k + 2;
		}
	}
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}
//***************************************************************************************************************************************
// Función para dibujar una imagen sprite - los parámetros columns = número de imagenes en el sprite, index = cual desplegar, flip = darle vuelta
//***************************************************************************************************************************************
void LCD_Sprite(int x, int y, int width, int height, unsigned char bitmap[],
		int columns, int index, char flip, char offset) {
	LCD_CMD(0x02c); // write_memory_start
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);

	unsigned int x2, y2;
	x2 = x + width;
	y2 = y + height;
	SetWindows(x, y, x2 - 1, y2 - 1);
	int k = 0;
	int ancho = ((width * columns));
	if (flip) {
		for (int j = 0; j < height; j++) {
			k = (j * (ancho) + index * width - 1 - offset) * 2;
			k = k + width * 2;
			for (int i = 0; i < width; i++) {
				LCD_DATA(bitmap[k]);
				LCD_DATA(bitmap[k + 1]);
				k = k - 2;
			}
		}
	} else {
		for (int j = 0; j < height; j++) {
			k = (j * (ancho) + index * width + 1 + offset) * 2;
			for (int i = 0; i < width; i++) {
				LCD_DATA(bitmap[k]);
				LCD_DATA(bitmap[k + 1]);
				k = k + 2;
			}
		}

	}
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

//***************************************************************************************************************************************
// Sprite con transparencia: los pixeles = transparentColor se pintan con bgColor
// bitmap: bytes (alto, bajo) por pixel. columns = cuadros en el sprite, index = cuadro, flip = espejo
//***************************************************************************************************************************************
void LCD_SpriteBG(int x, int y, int width, int height, const unsigned char *bitmap,
                  int columns, int index, char flip,
                  uint16_t transparentColor, uint16_t bgColor)
{
	uint8_t linea[640];   // maximo 64 pixeles de ancho
	if (width > 320) return;

	SetWindows(x, y, x + width - 1, y + height - 1);

	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);   // datos
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);

	for (int j = 0; j < height; j++) {
		int base = (j * columns * width + index * width) * 2;   // inicio de la fila en el cuadro
		for (int i = 0; i < width; i++) {
			int k = flip ? base + 2 * (width - 1 - i) : base + 2 * i;
			uint16_t pixel = (bitmap[k] << 8) | bitmap[k + 1];
			if (pixel == transparentColor) pixel = bgColor;
			linea[2 * i]     = pixel >> 8;
			linea[2 * i + 1] = pixel & 0xFF;
		}
		HAL_SPI_Transmit(&hspi1, linea, 2 * width, HAL_MAX_DELAY);
	}

	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

//***************************************************************************************************************************************
// Redibuja un pedazo de una imagen de pantalla completa (sirve para "borrar" un sprite)
//***************************************************************************************************************************************
void LCD_RestoreFondo(int x, int y, int w, int h, const unsigned char *fondo, int fondoW)
{
	SetWindows(x, y, x + w - 1, y + h - 1);

	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);

	for (int j = 0; j < h; j++) {
		const unsigned char *fila = &fondo[((y + j) * fondoW + x) * 2];
		HAL_SPI_Transmit(&hspi1, (uint8_t *)fila, 2 * w, HAL_MAX_DELAY);
	}

	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

//***************************************************************************************************************************************
// Sprite de 1 cuadro: los pixeles = transparente se rellenan con el pixel de "fondo" que esta debajo
//***************************************************************************************************************************************
void LCD_SpriteSobreFondo(int x, int y, int w, int h, const unsigned char *sprite,
                          uint16_t transparente, const unsigned char *fondo, int fondoW)
{
	uint8_t linea[640];
	if (w > 320) return;

	SetWindows(x, y, x + w - 1, y + h - 1);

	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);

	for (int j = 0; j < h; j++) {
		for (int i = 0; i < w; i++) {
			int k = (j * w + i) * 2;
			uint16_t pixel = (sprite[k] << 8) | sprite[k + 1];
			if (pixel == transparente) {
				int kf = ((y + j) * fondoW + (x + i)) * 2;   // pixel del fondo
				linea[2 * i]     = fondo[kf];
				linea[2 * i + 1] = fondo[kf + 1];
			} else {
				linea[2 * i]     = sprite[k];
				linea[2 * i + 1] = sprite[k + 1];
			}
		}
		HAL_SPI_Transmit(&hspi1, linea, 2 * w, HAL_MAX_DELAY);
	}

	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}

// Dibuja una pantalla completa 320x240 desde un archivo .bin de la SD. Devuelve 1 si salio bien.
uint8_t LCD_Bitmap_SD(const char *nombre)
{
	FIL f;
	UINT br;
	uint8_t buf[512];

	if (f_open(&f, nombre, FA_READ) != FR_OK) return 0;

	SetWindows(0, 0, 319, 239);
	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);   // datos

	for (;;) {
		// el LCD esta deseleccionado mientras se lee la SD
		if (f_read(&f, buf, sizeof(buf), &br) != FR_OK || br == 0) break;
		HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);
		HAL_SPI_Transmit(&hspi1, buf, br, HAL_MAX_DELAY);
		HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
	}
	f_close(&f);
	return 1;
}

//***************************************************************************************************************************************
// Sprite de varios cuadros (sheet). Los pixeles = transp se rellenan con el pedacito de fondo "patch" (w*h*2 bytes, alto-bajo)
//***************************************************************************************************************************************
void LCD_SpritePatch(int x, int y, int w, int h, const unsigned char *sheet,
                     int columns, int index, uint16_t transp, const unsigned char *patch)
{
	uint8_t linea[640];   // maximo 320 pixeles de ancho
	if (w > 320) return;

	SetWindows(x, y, x + w - 1, y + h - 1);

	HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET);   // datos
	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET);

	for (int j = 0; j < h; j++) {
		int base = (j * columns * w + index * w) * 2;   // inicio de la fila j dentro del cuadro "index"
		for (int i = 0; i < w; i++) {
			int k = base + 2 * i;
			uint16_t pixel = (sheet[k] << 8) | sheet[k + 1];
			if (pixel == transp) {
				linea[2 * i]     = patch[(j * w + i) * 2];       // fondo
				linea[2 * i + 1] = patch[(j * w + i) * 2 + 1];
			} else {
				linea[2 * i]     = sheet[k];                     // pixel del sprite
				linea[2 * i + 1] = sheet[k + 1];
			}
		}
		HAL_SPI_Transmit(&hspi1, linea, 2 * w, HAL_MAX_DELAY);
	}

	HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET);
}
