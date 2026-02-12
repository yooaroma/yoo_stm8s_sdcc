/**
  ******************************************************************************
  * @file stm8s103_void_main.c
  * @brief no action, just to build the project.
  * @author yooaroma@gmail.com
  * @version V1.0.0
  * @date 2023.1.6
  ******************************************************************************
  */
/*
    Function to blink LED (GPIO_B5)
*/

/* Includes ------------------------------------------------------------------*/
#include "stm8s.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Hardware define */
/*
  // LEFT
  PD4 : D13
  PD5 : D14 : A3 : TXD 
  PD6 : D15 : A4 : RXD
  RESET
  PA1 : D0  : KEY1 
  PA2 : D1
  GND
  5V
  3.3V
  PA3 : D2  : SS 
  // RIGHT
  PD3 : D23 : A2
  PD2 : D11 : A1
  PD1 : D10 : SWIM
  PC7 : D9  : MISO
  PC6 : D8  : MOSI
  PC5 : D7  : SCK
  PC4 : D6  : A0
  PC3 : D5
  PB4 : D4  : SCA
  PB5 : LED : SDA
*/


/* Includes ------------------------------------------------------------------*/
#include "stm8s.h"

/* Private defines -----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

void main(void)
{
	int vIndex = 0;
  /* Infinite loop */
  while (1)
  {
		vIndex ++;
		vIndex ++;
		vIndex = vIndex + 10;
  }
  
}

#ifdef USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param file: pointer to the source file name
  * @param line: assert_param error line source number
  * @retval : None
  */
void assert_failed(u8* file, u32 line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
