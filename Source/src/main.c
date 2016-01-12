/**
 * @file         main.c
 * @version      1.0
 * @date         2015
 * @author       Christoph Lauer
 * @compiler     armcc
 * @copyright    Christoph Lauer engineering
 */
 
 #include "main.h"
 #include "stm32f4xx_exti.h"
 #include "stm32f4xx_syscfg.h"
 #include "stm32f4xx_rcc.h"
 #include "stm32f4xx_gpio.h"
 #include "misc.h"
 #include "stm32f4xx.h"
 #include "headphone.h"
RCC_ClocksTypeDef RCC_Clocks;
extern volatile uint8_t LED_Toggle;
volatile int user_mode;
volatile int msTicks;
volatile uint32_t msec_counter;


static void LED_periodic_controller (void)
{
    switch (LED_Toggle & LED_CTRL_RED_TOGGLE) {
        case LED_CTRL_RED_ON:
            STM_EVAL_LEDOn(LED5);
            break;
        case LED_CTRL_RED_OFF:
            STM_EVAL_LEDOff(LED5);
            break;
        case LED_CTRL_RED_TOGGLE:
            STM_EVAL_LEDToggle(LED5);
            break;
    }

    switch (LED_Toggle & LED_CTRL_ORANGE_TOGGLE) {
        case LED_CTRL_ORANGE_ON:
            STM_EVAL_LEDOn(LED3);
            break;
        case LED_CTRL_ORANGE_OFF:
            STM_EVAL_LEDOff(LED3);
            break;
        case LED_CTRL_ORANGE_TOGGLE:
            STM_EVAL_LEDToggle(LED3);
            break;
    }

    switch (LED_Toggle & LED_CTRL_GREEN_TOGGLE) {
        case LED_CTRL_GREEN_ON:
            STM_EVAL_LEDOn(LED4);
            break;
        case LED_CTRL_GREEN_OFF:
            STM_EVAL_LEDOff(LED4);
            break;
        case LED_CTRL_GREEN_TOGGLE:
            STM_EVAL_LEDToggle(LED4);
            break;
    }

    switch (LED_Toggle & LED_CTRL_BLUE_TOGGLE) {
        case LED_CTRL_BLUE_ON:
            STM_EVAL_LEDOn(LED6);
            break;
        case LED_CTRL_BLUE_OFF:
            STM_EVAL_LEDOff(LED6);
            break;
        case LED_CTRL_BLUE_TOGGLE:
            STM_EVAL_LEDToggle(LED6);
            break;
    }
}


void SysTick_Handler(void)
{
    msec_counter++;

    // we use bit 1 of the user mode to control the LED toggling frequency

    if ((user_mode & 2) && !(msec_counter & 0x3f) ||
        !(user_mode & 2) && !(msec_counter & 0x7f))
            LED_periodic_controller ();
}

void Aufnahmen_gleich(void){ //ausgabe eines Signals (blinkende LED, theoretisch SMS denkbar), wenn beide Arrays gleich sind
	
	int i;
	STM_EVAL_LEDOn(LED3);
	
	for ( i = 0; i<25000000; i++);
	STM_EVAL_LEDOff(LED3);
}
//Array zum Speichern des Buffers
uint16_t audio_array[540000]; 

/*void StartRec(){
	 
	//WaveRecorderBeginSampling ();
	//WavePlayerInit(16000);
	
	WavePlayBack(I2S_AudioFreq_48k);
	//return array;
}
*/
void EXTI0_IRQHandler(void)
{
  /* Checks whether the User Button EXTI line is asserted*/
	// Logik des Programms
  if (EXTI_GetITStatus(EXTI_Line0) != RESET) 
  { 
      static uint32_t last_button_time;     // used to debounce user button

      if (last_button_time + 200 < msec_counter) {
          if (++user_mode & 1)  {    // bit 0 of the user mode lights the blue LED
						//User nimmt ein Geräusch auf, solange der USER-BUtton nicht nochmal gedrückt wurde 
						// array muss zurückgegeben und gespeichert werden.
              LED_Toggle = (LED_Toggle & ~LED_CTRL_GREEN_TOGGLE) | LED_CTRL_GREEN_ON;
							LED_Toggle = (LED_Toggle & ~LED_CTRL_BLUE_TOGGLE) | LED_CTRL_BLUE_OFF;
							//
						//StartRec();
							//Waverecorder 1. Aufnahme
						// return muss ein array
						LED_Toggle = (LED_Toggle & ~LED_CTRL_RED_TOGGLE) | LED_CTRL_RED_ON;
						//Fourier-Transformation
						//Übergabe des Arrays an ein Array in Main.c
					}
          else{
						//while(prüfung = false && USER BUTTON != RESET){ Wird der User-Button gedrückt springt er auch raus
						//User wartet auf die Erkennung des Geräusches
						//wird das Geräusch erkannt, so wird ein optisches Signal innerhalb einer anderen Funktion
						//gerufen
						  LED_Toggle = (LED_Toggle & ~LED_CTRL_RED_TOGGLE) | LED_CTRL_RED_OFF;
              LED_Toggle = (LED_Toggle & ~LED_CTRL_BLUE_TOGGLE) | LED_CTRL_BLUE_ON;
							LED_Toggle = (LED_Toggle & ~LED_CTRL_GREEN_TOGGLE) | LED_CTRL_GREEN_OFF;
							
						//}
						//if(prüfung = true)
							//call Funktion Aufnahmen_gleich();
					}
          last_button_time = msec_counter;
      }
  } 
  /* Clears the EXTI's line pending bit.*/ 
  EXTI_ClearITPendingBit(EXTI_Line0);
}


int main(void)
{ 
	
  /* Initialize LEDs */
  STM_EVAL_LEDInit(LED3);
  STM_EVAL_LEDInit(LED4);
  STM_EVAL_LEDInit(LED5);
  STM_EVAL_LEDInit(LED6);
	
  /* SysTick end of count event each 1ms */
  RCC_GetClocksFreq(&RCC_Clocks);
  SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);
	
  /* Initialize User Button */
  STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_EXTI);
	
	//Warte auf interrupt
	WavePlayBack(I2S_AudioFreq_48k);
	while(1){} 
		
		

}