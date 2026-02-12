/**
  ******************************************************************************
  * @file stm8s103_led_key_main.c
  * @brief Make the LED blink.
  * @author yooaroma@gmail.com
  * @version V1.0.0
  * @date 2023.1.6
  ******************************************************************************
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
#define UCOM_LED1_GPIO    GPIOB        // PB5
#define UCOM_LED1_PIN     GPIO_PIN_5   // PB5
#define UCOM_LED1_MODE    GPIO_MODE_OUT_PP_LOW_FAST
#define UCOM_KEY1_GPIO    GPIOA        // PA1
#define UCOM_KEY1_PIN     GPIO_PIN_1   // PA1
#define UCOM_KEY1_MODE    GPIO_MODE_IN_PU_NO_IT // GPIO_MODE_IN_FL_NO_IT
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
/**
  ******************************************************************************
  * @brief Main function.
  * @par Parameters:
  * None
  * @retval void None
  * @par Required preconditions:
  * None
  ******************************************************************************
  */
void main(void)
{
  /* Configures clocks */
  // CLK_Configuration();
  {
    /* Clear High speed internal clock prescaler */
    CLK->CKDIVR &= (uint8_t)(~CLK_CKDIVR_HSIDIV);    
    /* Set High speed internal clock prescaler */
    CLK->CKDIVR |= (uint8_t)CLK_PRESCALER_HSIDIV1;
  }

  /* Configures GPIOs */
  // GPIO_Configuration();
  {
    /* Configure PB5 (LED1) as output push-pull low (led switched on) */
    // GPIO_Init(UCOM_LED1_GPIO, UCOM_LED1_PIN, UCOM_LED1_MODE);
    {
      UCOM_LED1_GPIO->ODR &= (uint8_t)(~(UCOM_LED1_PIN)); /* Output mode  Low level */
      UCOM_LED1_GPIO->DDR |= (uint8_t)UCOM_LED1_PIN; /* Set Output mode */
      UCOM_LED1_GPIO->CR1 |= (uint8_t)UCOM_LED1_PIN; /* Push-Pull */
      // UCOM_LED1_GPIO->CR1 &= (uint8_t)(~(UCOM_LED1_PIN)); /* Open drain */
      UCOM_LED1_GPIO->CR2 |= (uint8_t)UCOM_LED1_PIN; /* 10MHz */
      // UCOM_LED1_GPIO->CR2 &= (uint8_t)(~(UCOM_LED1_PIN)); /* 2MHz */
    }
    /* Configure PA1 : KEY IN as input push-pull low (led switched on) */
    // GPIO_Init(UCOM_KEY1_GPIO, UCOM_KEY1_PIN, UCOM_KEY1_MODE);
    {
      UCOM_KEY1_GPIO->DDR &= (uint8_t)(~(UCOM_KEY1_PIN)); /* Set Input mode */
      UCOM_KEY1_GPIO->CR1 |= (uint8_t)UCOM_KEY1_PIN; /* Pull-up */
      //UCOM_KEY1_GPIO->CR1 &= (uint8_t)(~(UCOM_KEY1_PIN)); /* Float */
      // UCOM_KEY1_GPIO->CR2 |= (uint8_t)UCOM_KEY1_PIN; /* External interrupt enable */
      UCOM_KEY1_GPIO->CR2 &= (uint8_t)(~(UCOM_KEY1_PIN)); /* External interrupt disable */
    }
  }
  {    
    do {
      if((UCOM_KEY1_GPIO->IDR & UCOM_KEY1_PIN)==0)
      {
        UCOM_LED1_GPIO->ODR |= (uint8_t)UCOM_LED1_PIN;
      }
      else
      {
        UCOM_LED1_GPIO->ODR &= ~(uint8_t)UCOM_LED1_PIN;
      }
    } while(1);
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

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
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
