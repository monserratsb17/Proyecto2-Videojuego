#ifndef AUDIO_JUEGO_H
#define AUDIO_JUEGO_H
/* Buzzer pasivo PB10, AF1 TIM2_CH3. TIM2 se reserva para audio. */
typedef struct { uint16_t hz,ms; } AudioNota;
/* Melodia ORIGINAL de ejemplo; sustituible por una secuencia aportada por el usuario. */
static const AudioNota audio_notas[]={
 {523,160},{659,160},{784,240},{0,80},{698,160},{587,160},{659,320},{0,80},
 {440,160},{523,160},{698,240},{659,160},{587,160},{523,400},{0,160}
};
static uint8_t audio_pos=0,audio_activo=0;
static uint32_t audio_proximo=0;
static void Audio_Tono(uint16_t hz){
 if(!hz){TIM2->CCR3=0;return;}
 uint32_t periodo=1000000u/hz;
 TIM2->CCR3=0;TIM2->ARR=periodo-1;TIM2->CNT=0;
 TIM2->EGR=TIM_EGR_UG;TIM2->CCR3=periodo/2;
}
static void Audio_Init(void){
 __HAL_RCC_GPIOB_CLK_ENABLE();__HAL_RCC_TIM2_CLK_ENABLE();
 GPIO_InitTypeDef g={0};g.Pin=GPIO_PIN_10;g.Mode=GPIO_MODE_AF_PP;
 g.Pull=GPIO_NOPULL;g.Speed=GPIO_SPEED_FREQ_LOW;g.Alternate=GPIO_AF1_TIM2;
 HAL_GPIO_Init(GPIOB,&g);
 uint32_t reloj=HAL_RCC_GetPCLK1Freq();
 if((RCC->CFGR & RCC_CFGR_PPRE1)!=0)reloj*=2;
 TIM2->CR1=0;TIM2->CCER=0;TIM2->DIER=0;
 TIM2->PSC=reloj/1000000u-1;TIM2->ARR=999;TIM2->CCR3=0;
 TIM2->CCMR2=(6u<<4)|TIM_CCMR2_OC3PE;
 TIM2->EGR=TIM_EGR_UG;TIM2->CCER=TIM_CCER_CC3E;
 TIM2->CR1=TIM_CR1_ARPE|TIM_CR1_CEN;
}
static void Audio_Silenciar(void){audio_activo=0;Audio_Tono(0);}
static void Audio_Empezar(void){audio_pos=0;audio_proximo=HAL_GetTick();audio_activo=1;}
static void Audio_Actualizar(uint8_t jugando){
 if(!jugando){if(audio_activo)Audio_Silenciar();return;}
 if(!audio_activo)return;
 uint32_t now=HAL_GetTick();if((int32_t)(now-audio_proximo)<0)return;
 Audio_Tono(audio_notas[audio_pos].hz);audio_proximo=now+audio_notas[audio_pos].ms;
 audio_pos=(audio_pos+1)%(sizeof(audio_notas)/sizeof(audio_notas[0]));
}
#endif
