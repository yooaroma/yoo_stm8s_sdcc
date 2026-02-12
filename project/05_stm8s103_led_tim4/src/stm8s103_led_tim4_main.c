/**
 ******************************************************************************
 * @file stm8s103_led_tim4_main.c
 * @brief Make the LED blink.
 * @author yooaroma@gmail.com
 * @version V1.0.0
 * @date 2023.1.6
 ******************************************************************************
 */
/*
    info :
    타이머 4 모쥴을 통해서 1 sec 마다 깜빡이도록 하는 기능을 구현한 예제이다.
    즉 타이머 4에 stm8s의 마스터 클럭(16MHz)을 입력으로 받아서
    STM8S Reference Manuals의 page 253, 참고하여
    update flag를 사용하여 엘이디(GPIO_B5)를 토글하는 기능을 구현한 예제이다.
    여기에서는 인터럽트를 사용하지 않은 타이머 사용 예제이다.
    key 입력시에는 delay를 초기화 한다.
    TIM4 를 사용하여 1 msec 마다 update flag가 설정되도록 하였다.
    main 루프에서 이 플래그를 검사하여 1 msec 마다 delay 카운터를 감소시키고,
    delay 카운터가 0이 되면 엘이디를 토글하도록 하였다.
    여기에서는 TIM4의 update flag를 체크하면서 폴링으로 구현하였다.
    또한 키 입력을 받아서 현재의 LED 상태를 반전시키도록 하였다. 그리고 디바운싱을 체크하여 
    키가 릴리즈 될 때까지 기다리도록 하였다. 그 이후에 LED는 1초마다 반전한다. 
    (인터럽트를 사용하지 않음)
    (참고로, 타이머 4는 8비트 자동 재장치 업 카운터이다.)
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
#define UCOM_LED1_GPIO GPIOB     // PB5
#define UCOM_LED1_PIN GPIO_PIN_5 // PB5
#define UCOM_LED1_MODE GPIO_MODE_OUT_PP_LOW_FAST

#define UCOM_KEY1_GPIO GPIOA                 // PA1
#define UCOM_KEY1_PIN GPIO_PIN_1             // PA1
#define UCOM_KEY1_MODE GPIO_MODE_IN_PU_NO_IT // GPIO_MODE_IN_FL_NO_IT
// #define UCOM_KEY1_MODE    GPIO_MODE_IN_FL_IT     /*!< Input floating, external interrupt */
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
void Toggle(void);
void key_released(void);
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
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
    /* Configure PD0 (LED1) as output push-pull low (led switched on) */
    // GPIO_Init(UCOM_LED1_GPIO, UCOM_LED1_PIN, UCOM_LED1_MODE);
    {
      UCOM_LED1_GPIO->ODR &= (uint8_t)(~(UCOM_LED1_PIN)); /* Output mode  Low level */
      UCOM_LED1_GPIO->DDR |= (uint8_t)UCOM_LED1_PIN;      /* Set Output mode */
      UCOM_LED1_GPIO->CR1 |= (uint8_t)UCOM_LED1_PIN;      /* Push-Pull */
      // UCOM_LED1_GPIO->CR1 &= (uint8_t)(~(UCOM_LED1_PIN)); /* Open drain */
      UCOM_LED1_GPIO->CR2 |= (uint8_t)UCOM_LED1_PIN; /* 10MHz */
      // UCOM_LED1_GPIO->CR2 &= (uint8_t)(~(UCOM_LED1_PIN)); /* 2MHz */
    }
    /* Configure PA3 : KEY IN as input push-pull low (led switched on) */
    // GPIO_Init(UCOM_KEY1_GPIO, UCOM_KEY1_PIN, UCOM_KEY1_MODE);
    {
      UCOM_KEY1_GPIO->DDR &= (uint8_t)(~(UCOM_KEY1_PIN)); /* Set Input mode */
      UCOM_KEY1_GPIO->CR1 |= (uint8_t)UCOM_KEY1_PIN;      /* Pull-up */
      // UCOM_KEY1_GPIO->CR1 &= (uint8_t)(~(UCOM_KEY1_PIN)); /* Float */
      //  UCOM_KEY1_GPIO->CR2 |= (uint8_t)UCOM_KEY1_PIN; /* External interrupt enable */
      UCOM_KEY1_GPIO->CR2 &= (uint8_t)(~(UCOM_KEY1_PIN)); /* External interrupt disable */
    }
  }
  {
/* Configures TIM4 */
/*
  19.2 TIM4 main features
      The main features include:
      • 8-bit auto-reload up counter
      • 3-bit programmable prescaler which allows dividing (also “on the fly”) the counter clock
        frequency by 1, 2, 4, 8, 16, 32, 64 and 128.
      • Interrupt generation
        – On counter update: Counter overflow
*/
// TIM4_TimeBaseInit(TIM4_PRESCALER_128, 125 - 1);
// TIM4_Cmd(ENABLE);
#define _MM_UCOM_MSEC_ (16000 / 128)           // 16MHz / 2**7  = 125
    TIM4->PSCR = TIM4_PRESCALER_128;           // 7 : 8 usec / clock...
    TIM4->ARR = (uint8_t)(_MM_UCOM_MSEC_ - 1); // 8 * 125 = 1000 usec = 1 msec....
    // TIM4->ARR = 125; // (uint8_t)~(_MM_UCOM_MSEC_-1);
    TIM4->CNTR = 0; // TIM4->ARR;
    TIM4->SR1 = (uint8_t)(~TIM4_FLAG_UPDATE);
    // TIM4->EGR = 1; // TIM4_EGR_UG
    // TIM4->IER = TIM4_IER_UIE;
    // TIM4->CR1 = TIM4_CR1_CEN | TIM4_CR1_ARPE;
    TIM4->CR1 = TIM4_CR1_CEN;
  }
  {
#define _MM_DELAY_SEC_ 1000             // 1 sec
    uint32_t vdwDelay = _MM_DELAY_SEC_; // 1 sec (테스트용)
    do
    {
      if ((UCOM_KEY1_GPIO->IDR & UCOM_KEY1_PIN) == 0) // Key pressed
      {
        Toggle();
        key_released();
        vdwDelay = _MM_DELAY_SEC_; 
        TIM4->CNTR = 0; // TIM4->ARR;
        TIM4->SR1 = (uint8_t)(~TIM4_FLAG_UPDATE);
      }
      else
      {
        // UCOM_LED1_GPIO->ODR = UCOM_LED1_GPIO->ODR | UCOM_LED1_PIN;
        // if(TIM4_GetFlagStatus(TIM4_FLAG_UPDATE) == SET)
        if (TIM4->SR1 == TIM4_FLAG_UPDATE)
        {
          // TIM4_SetCounter(0);
          // TIM4->CNTR = 0; // TIM4->ARR;
          // TIM4_ClearFlag(TIM4_FLAG_UPDATE);
          TIM4->SR1 &= (~TIM4_FLAG_UPDATE);
          // UCOM_LED1_GPIO->ODR ^= (uint8_t)UCOM_LED1_PIN; // 1msec...
          if (vdwDelay)
            vdwDelay--;
        }
        if (vdwDelay == 0)
        {
          vdwDelay = _MM_DELAY_SEC_; 
          Toggle();
        }
      }
    } while (1);
  }
}
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
/**
 ******************************************************************************
 * @brief Toggle PD0 (Led LD1)
 * @par Parameters:
 * None
 * @retval void None
 * @par Required preconditions:
 * None
 ******************************************************************************
 */
void Toggle(void)
{
  // GPIO_WriteReverse(UCOM_LED1_GPIO, UCOM_LED1_PIN);
  UCOM_LED1_GPIO->ODR ^= (uint8_t)UCOM_LED1_PIN;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
void key_released(void)
{
  uint16_t cnt;
  cnt = 0;  
  do {
    if ((UCOM_KEY1_GPIO->IDR & UCOM_KEY1_PIN) != 0)
    {
      cnt++;
      if (cnt >= 2000)
        break;
    }
    else
    {
      cnt = 0;
    } 
  } while (1);
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
void assert_failed(u8 *file, u32 line)
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
