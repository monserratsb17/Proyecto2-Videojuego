/* USER CODE BEGIN Header */
/**
 ******************************************************************************
  * @file    user_diskio.c
  * @brief   This file includes a diskio driver skeleton to be completed by the user.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
 /* USER CODE END Header */

#ifdef USE_OBSOLETE_USER_CODE_SECTION_0
/*
 * Warning: the user section 0 is no more in use (starting from CubeMx version 4.16.0)
 * To be suppressed in the future.
 * Kept to ensure backward compatibility with previous CubeMx versions when
 * migrating projects.
 * User code previously added there should be copied in the new user sections before
 * the section contents can be deleted.
 */
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */
#endif

/* USER CODE BEGIN DECL */

/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include "ff_gen_drv.h"
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
/* Disk status */
static volatile DSTATUS Stat = STA_NOINIT;

/* ---- Driver SD por SPI (SPI1 compartido con el LCD, CS = SD_CS) ---- */
extern SPI_HandleTypeDef hspi1;

#define SD_CS_L()  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET)
#define SD_CS_H()  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_SET)

static uint8_t sd_hc = 0;   // 1 = SDHC (direccion por bloque)

static uint8_t SD_Xfer(uint8_t b)
{
  uint8_t r = 0xFF;
  HAL_SPI_TransmitReceive(&hspi1, &b, &r, 1, 100);
  return r;
}

static void SD_Speed(uint32_t presc)
{
  __HAL_SPI_DISABLE(&hspi1);
  hspi1.Instance->CR1 = (hspi1.Instance->CR1 & ~SPI_CR1_BR) | presc;
  __HAL_SPI_ENABLE(&hspi1);
}

static int SD_WaitReady(void)
{
  uint32_t t = HAL_GetTick();
  do { if (SD_Xfer(0xFF) == 0xFF) return 1; } while (HAL_GetTick() - t < 500);
  return 0;
}

static void SD_Deselect(void) { SD_CS_H(); SD_Xfer(0xFF); }

static int SD_Select(void)
{
  SD_CS_L();
  SD_Xfer(0xFF);
  if (SD_WaitReady()) return 1;
  SD_Deselect();
  return 0;
}

// Manda un comando y devuelve R1 (0xFF = sin respuesta). Deja CS en bajo.
static uint8_t SD_Cmd(uint8_t cmd, uint32_t arg)
{
  uint8_t r, n;
  if (cmd & 0x80) {            // ACMD = CMD55 + cmd
    cmd &= 0x7F;
    r = SD_Cmd(55, 0);
    if (r > 1) return r;
  }
  SD_Deselect();
  if (!SD_Select()) return 0xFF;
  SD_Xfer(0x40 | cmd);
  SD_Xfer(arg >> 24); SD_Xfer(arg >> 16); SD_Xfer(arg >> 8); SD_Xfer(arg);
  n = 0x01;
  if (cmd == 0) n = 0x95;
  if (cmd == 8) n = 0x87;
  SD_Xfer(n);
  n = 10;
  do { r = SD_Xfer(0xFF); } while ((r & 0x80) && --n);
  return r;
}

static int SD_Init(void)
{
  uint8_t i, r = 0, ocr[4];
  uint32_t t;

  sd_hc = 0;
  SD_Speed(SPI_BAUDRATEPRESCALER_256);       // ~330 kHz para iniciar
  SD_CS_H();
  for (i = 0; i < 10; i++) SD_Xfer(0xFF);    // 80 pulsos con CS en alto

  if (SD_Cmd(0, 0) == 0x01) {
    t = HAL_GetTick();
    if (SD_Cmd(8, 0x1AA) == 0x01) {          // tarjeta SD v2
      for (i = 0; i < 4; i++) ocr[i] = SD_Xfer(0xFF);
      if (ocr[2] == 0x01 && ocr[3] == 0xAA) {
        while (SD_Cmd(0x80 | 41, 0x40000000) != 0 && HAL_GetTick() - t < 1000);
        if (HAL_GetTick() - t < 1000 && SD_Cmd(58, 0) == 0) {
          for (i = 0; i < 4; i++) ocr[i] = SD_Xfer(0xFF);
          sd_hc = (ocr[0] & 0x40) ? 1 : 0;
          r = 1;
        }
      }
    } else {                                 // tarjeta SD v1
      while (SD_Cmd(0x80 | 41, 0) != 0 && HAL_GetTick() - t < 1000);
      if (HAL_GetTick() - t < 1000 && SD_Cmd(16, 512) == 0) r = 1;
    }
  }
  SD_Deselect();
  SD_Speed(SPI_BAUDRATEPRESCALER_8);         // velocidad normal (igual que la pantalla)
  return r;
}

static int SD_ReadBlock(uint8_t *buf, uint32_t sector)
{
  uint8_t tok;
  uint32_t t;
  if (!sd_hc) sector *= 512;
  if (SD_Cmd(17, sector) != 0) { SD_Deselect(); return 0; }
  t = HAL_GetTick();
  do { tok = SD_Xfer(0xFF); } while (tok == 0xFF && HAL_GetTick() - t < 200);
  if (tok != 0xFE) { SD_Deselect(); return 0; }
  memset(buf, 0xFF, 512);
  HAL_SPI_TransmitReceive(&hspi1, buf, buf, 512, 1000);
  SD_Xfer(0xFF); SD_Xfer(0xFF);              // CRC (se ignora)
  SD_Deselect();
  return 1;
}

static int SD_WriteBlock(const uint8_t *buf, uint32_t sector)
{
  if (!sd_hc) sector *= 512;
  if (SD_Cmd(24, sector) != 0) { SD_Deselect(); return 0; }
  SD_Xfer(0xFF);
  SD_Xfer(0xFE);                             // token de inicio
  HAL_SPI_Transmit(&hspi1, (uint8_t *)buf, 512, 1000);
  SD_Xfer(0xFF); SD_Xfer(0xFF);              // CRC
  if ((SD_Xfer(0xFF) & 0x1F) != 0x05) { SD_Deselect(); return 0; }
  if (!SD_WaitReady()) { SD_Deselect(); return 0; }
  SD_Deselect();
  return 1;
}

/* USER CODE END DECL */

/* Private function prototypes -----------------------------------------------*/
DSTATUS USER_initialize (BYTE pdrv);
DSTATUS USER_status (BYTE pdrv);
DRESULT USER_read (BYTE pdrv, BYTE *buff, DWORD sector, UINT count);
#if _USE_WRITE == 1
  DRESULT USER_write (BYTE pdrv, const BYTE *buff, DWORD sector, UINT count);
#endif /* _USE_WRITE == 1 */
#if _USE_IOCTL == 1
  DRESULT USER_ioctl (BYTE pdrv, BYTE cmd, void *buff);
#endif /* _USE_IOCTL == 1 */

Diskio_drvTypeDef  USER_Driver =
{
  USER_initialize,
  USER_status,
  USER_read,
#if  _USE_WRITE
  USER_write,
#endif  /* _USE_WRITE == 1 */
#if  _USE_IOCTL == 1
  USER_ioctl,
#endif /* _USE_IOCTL == 1 */
};

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Initializes a Drive
  * @param  pdrv: Physical drive number (0..)
  * @retval DSTATUS: Operation status
  */
DSTATUS USER_initialize (
	BYTE pdrv           /* Physical drive nmuber to identify the drive */
)
{
  /* USER CODE BEGIN INIT */
    if (SD_Init()) Stat &= ~STA_NOINIT; else Stat = STA_NOINIT;
    return Stat;
  /* USER CODE END INIT */
}

/**
  * @brief  Gets Disk Status
  * @param  pdrv: Physical drive number (0..)
  * @retval DSTATUS: Operation status
  */
DSTATUS USER_status (
	BYTE pdrv       /* Physical drive number to identify the drive */
)
{
  /* USER CODE BEGIN STATUS */
    return Stat;
  /* USER CODE END STATUS */
}

/**
  * @brief  Reads Sector(s)
  * @param  pdrv: Physical drive number (0..)
  * @param  *buff: Data buffer to store read data
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to read (1..128)
  * @retval DRESULT: Operation result
  */
DRESULT USER_read (
	BYTE pdrv,      /* Physical drive nmuber to identify the drive */
	BYTE *buff,     /* Data buffer to store read data */
	DWORD sector,   /* Sector address in LBA */
	UINT count      /* Number of sectors to read */
)
{
  /* USER CODE BEGIN READ */
    if (Stat & STA_NOINIT) return RES_NOTRDY;
    for (UINT i = 0; i < count; i++)
      if (!SD_ReadBlock(buff + 512 * i, sector + i)) return RES_ERROR;
    return RES_OK;
  /* USER CODE END READ */
}

/**
  * @brief  Writes Sector(s)
  * @param  pdrv: Physical drive number (0..)
  * @param  *buff: Data to be written
  * @param  sector: Sector address (LBA)
  * @param  count: Number of sectors to write (1..128)
  * @retval DRESULT: Operation result
  */
#if _USE_WRITE == 1
DRESULT USER_write (
	BYTE pdrv,          /* Physical drive nmuber to identify the drive */
	const BYTE *buff,   /* Data to be written */
	DWORD sector,       /* Sector address in LBA */
	UINT count          /* Number of sectors to write */
)
{
  /* USER CODE BEGIN WRITE */
    if (Stat & STA_NOINIT) return RES_NOTRDY;
    for (UINT i = 0; i < count; i++)
      if (!SD_WriteBlock(buff + 512 * i, sector + i)) return RES_ERROR;
    return RES_OK;
  /* USER CODE END WRITE */
}
#endif /* _USE_WRITE == 1 */

/**
  * @brief  I/O control operation
  * @param  pdrv: Physical drive number (0..)
  * @param  cmd: Control code
  * @param  *buff: Buffer to send/receive control data
  * @retval DRESULT: Operation result
  */
#if _USE_IOCTL == 1
DRESULT USER_ioctl (
	BYTE pdrv,      /* Physical drive nmuber (0..) */
	BYTE cmd,       /* Control code */
	void *buff      /* Buffer to send/receive control data */
)
{
  /* USER CODE BEGIN IOCTL */
    switch (cmd) {
      case CTRL_SYNC:        SD_Select(); SD_Deselect(); return RES_OK;
      case GET_SECTOR_SIZE:  *(WORD *)buff = 512;  return RES_OK;
      case GET_BLOCK_SIZE:   *(DWORD *)buff = 1;   return RES_OK;
      default:               return RES_PARERR;
    }
  /* USER CODE END IOCTL */
}
#endif /* _USE_IOCTL == 1 */
