#include "sound8bit.h"

/*
 * Audio de fondo estilo videojuego retro para STM32 NUCLEO-F446RE.
 *
 * Configuracion requerida en STM32CubeMX:
 *   - PB10 = TIM2_CH3
 *   - TIM2 Channel 3 = PWM Generation CH3
 *   - Reloj del contador TIM2 = 1 MHz
 *
 * Con un reloj TIM2 de 84 MHz, use Prescaler = 83.
 * El periodo inicial de ARR puede ser 999.
 *
 * Esta libreria es no bloqueante: llame Sound8Bit_Update() en cada
 * iteracion del while(1) para avanzar la melodia sin detener el juego.
 */




#define SOUND_TIMER_TICK_HZ 1000000UL

typedef struct {
    uint16_t frequency;
    uint16_t duration_ms;
} SoundNote;

/* Melodia original estilo arcade. Frecuencia en Hz; duracion en ms.
 * Una frecuencia de 0 representa silencio.
 */
static const SoundNote melody[] = {
    {659, 120}, {784, 120}, {988, 180}, {784, 120},
    {1047, 180}, {988, 120}, {784, 180}, {659, 240},
    {523, 120}, {659, 120}, {784, 180}, {659, 120},
    {587, 180}, {659, 120}, {784, 240}, {0, 100},

    {988, 120}, {1047, 120}, {1175, 180}, {1047, 120},
    {988, 180}, {784, 120}, {659, 240}, {0, 120},

    {523, 120}, {587, 120}, {659, 180}, {784, 180},
    {659, 120}, {587, 120}, {523, 240}, {0, 100},

    {784, 120}, {988, 120}, {1047, 180}, {988, 120},
    {784, 180}, {659, 120}, {523, 300}, {0, 150}
};

#define MELODY_LENGTH (sizeof(melody) / sizeof(melody[0]))

static uint32_t noteIndex = 0U;
static uint32_t noteDeadline = 0U;
static uint8_t initialized = 0U;
static uint8_t playing = 0U;

static void Sound8Bit_SetNote(uint16_t frequency)
{
 if(frequency==0U){TIM2->CCR3=0;return;}
 uint32_t period=SOUND_TIMER_TICK_HZ/frequency;
 if(period<2U)period=2U;
 TIM2->CCR3=0;TIM2->ARR=period-1U;TIM2->CNT=0;
 TIM2->EGR=TIM_EGR_UG;TIM2->CCR3=period/2U;
}
void Sound8Bit_Init(void)
{
 TIM2->CCR3=0;initialized=1U;
}

void Sound8Bit_StartBackground(void)
{
    if (initialized == 0U) {
        return;
    }

    noteIndex = 0U;
    noteDeadline = HAL_GetTick();
    playing = 1U;
}

void Sound8Bit_Update(void)
{
    if ((initialized == 0U) || (playing == 0U)) {
        return;
    }

    uint32_t now = HAL_GetTick();

    /* Comparacion segura incluso cuando HAL_GetTick() desborda. */
    if ((int32_t)(now - noteDeadline) < 0) {
        return;
    }

    const SoundNote *note = &melody[noteIndex];

    Sound8Bit_SetNote(note->frequency);
    noteDeadline = now + note->duration_ms;

    noteIndex++;

    /* Repite la musica como sonido de fondo. */
    if (noteIndex >= MELODY_LENGTH) {
        noteIndex = 0U;
    }
}

void Sound8Bit_Stop(void)
{
    playing = 0U;

    if (initialized != 0U) {
        TIM2->CCR3=0U;
    }
}
