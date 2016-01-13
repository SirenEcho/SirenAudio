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
 #define ARM_MATH_CM4
 #include "arm_math.h"
 #include "stm32f4xx.h"
 #include "headphone.h"
 #include "microphone.h"
 //USB
 #include "stm32_ub_usb_msc_host.h"
 
 //init custom fft
	arm_rfft_instance_q15 RealFFT_Instance;
	arm_cfft_radix4_instance_q15 MyComplexFFT_Instance;
	
 
//Array zum Speichern des Buffers
q15_t audio_array[23000];
q15_t result_array[800];
q15_t result_array_mag[400];
uint8_t ok =0;
RCC_ClocksTypeDef RCC_Clocks;
extern volatile uint8_t LED_Toggle;
volatile int user_mode;
volatile int msTicks;
volatile uint32_t msec_counter;

uint8_t USB_Beispiel_schreiben(){
	FIL myFile;   // Filehandler
	volatile int k;
  //uint8_t write_ok=ok;

   // Quarz Einstellungen aktivieren

  // Init der LEDs
  //UB_Led_Init();

  // Init vom USB-OTG-Port als MSC-HOST
  // (zum lesen/schreiben auf einen USB-Stick)
  


    // pollen vom USB-Status
    if(ok==0) {
      // wenn USB-Stick erkannt wurde
  //    UB_Led_On(LED_GREEN);

      // wenn File noch nicht geschrieben wurde
      if(UB_USB_MSC_HOST_Do()==USB_MSC_DEV_CONNECTED) {
			ok=1;
  //     	UB_Led_On(LED_RED);
       	// Media mounten
       	if(UB_Fatfs_Mount(USB_0)==FATFS_OK) {
          // File zum schreiben im root neu anlegen
          if(UB_Fatfs_OpenFile(&myFile, "USB_File.txt", F_WR_CLEAR)==FATFS_OK) {
						LED_Toggle = (LED_Toggle & ~LED_CTRL_BLUE_TOGGLE) | LED_CTRL_BLUE_ON;
            // ein paar Textzeilen in das File schreiben
            UB_Fatfs_WriteString(&myFile,"Fick");
            UB_Fatfs_WriteString(&myFile,"Penis");
            UB_Fatfs_WriteString(&myFile,"ENDE");
            // File schliessen
            UB_Fatfs_CloseFile(&myFile);
          }
					if(UB_Fatfs_OpenFile(&myFile, "Outpud.txt", F_WR_CLEAR)==FATFS_OK){
						char strbuf[48];
						
						LED_Toggle = (LED_Toggle & ~LED_CTRL_BLUE_TOGGLE) | LED_CTRL_BLUE_ON;
						for(k=0; k<140;k++){
							snprintf(strbuf, sizeof(strbuf), "%d", result_array_mag[k]);
							UB_Fatfs_WriteString(&myFile, strbuf);
							UB_Fatfs_WriteString(&myFile, ",");
						}
						UB_Fatfs_CloseFile(&myFile);
						LED_Toggle = (LED_Toggle & ~LED_CTRL_BLUE_TOGGLE) | LED_CTRL_BLUE_OFF;
					}
					
					if(UB_Fatfs_OpenFile(&myFile, "Output.txt", F_WR_CLEAR)==FATFS_OK){
						char strbuf[48];
						
						LED_Toggle = (LED_Toggle & ~LED_CTRL_BLUE_TOGGLE) | LED_CTRL_BLUE_ON;
						for(k=0; k<300;k++){
							snprintf(strbuf, sizeof(strbuf), "%d", result_array[k]);
							UB_Fatfs_WriteString(&myFile, strbuf);
							UB_Fatfs_WriteString(&myFile, ",");
						}
						UB_Fatfs_CloseFile(&myFile);
						LED_Toggle = (LED_Toggle & ~LED_CTRL_BLUE_TOGGLE) | LED_CTRL_BLUE_OFF;
					}
					
					
          // Media unmounten
          UB_Fatfs_UnMount(USB_0);
        }
   //     UB_Led_Off(LED_RED);
      }
    }
    else {
      // wenn kein USB-Stick vorhanden
   //   UB_Led_Off(LED_GREEN);
			return 0;
    }
  
	return 1;
}

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
 

void StartRec(){
	
	u16 data = 0;
	volatile int i = 0;
	
	while(i < 23000)
	{		
		data = SPI_I2S_ReceiveData(SPI2);
		audio_array[i] = HTONS(data); 
		i++;
	}
	
	 
	//WaveRecorderBeginSampling ();
	//WavePlayerInit(16000);
	
	//WavePlayBack(I2S_AudioFreq_48k);
	//return array;
}

void EXTI0_IRQHandler(void)
{
	uint32_t i = 0;
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
						
						LED_Toggle = (LED_Toggle & ~LED_CTRL_ORANGE_TOGGLE) | LED_CTRL_ORANGE_ON;
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

void custom_fft(void){
	
}


int main(void)
{ 
	volatile int i;

	//USB
	UB_USB_MSC_HOST_Init();
	SystemInit();
	
	
  /* Initialize LEDs */
  STM_EVAL_LEDInit(LED3);
  STM_EVAL_LEDInit(LED4);
  STM_EVAL_LEDInit(LED5);
  STM_EVAL_LEDInit(LED6);
	
	//init micro
	SPI_I2S_ITConfig(SPI2, SPI_I2S_IT_RXNE, ENABLE);
	I2S_Cmd(SPI2, ENABLE); 	
	WaveRecorderBeginSampling();

	
  /* SysTick end of count event each 1ms */
  RCC_GetClocksFreq(&RCC_Clocks);
  SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);
	
  /* Initialize User Button */
  STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_EXTI);
	
	//Warte auf interrupt
	//WavePlayBack(I2S_AudioFreq_48k);
 	 
//	 for(i = 0; i < 1000 ; i++)
//	 {
//	 audio_array[i] = i+1;
//	 }
	
	 
		while(1){
						StartRec();
						
						arm_rfft_init_q15(&RealFFT_Instance, &MyComplexFFT_Instance, 128,1,1);
						arm_rfft_q15(&RealFFT_Instance, audio_array, result_array);
							for(i=0; i<256;i++){
								result_array[i]<<=8;
						}
						arm_cmplx_mag_q15(result_array, result_array_mag, 128);
						USB_Beispiel_schreiben();

	}
}