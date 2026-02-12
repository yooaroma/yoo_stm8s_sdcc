/**
 ******************************************************************************
 * @file stm8s103_vsprintf_main.c
 * @brief UART RX interrupt example.
 * @author yooaroma@gmail.com
 * @version V1.0.0
 * @date 2023.1.6
 ******************************************************************************
 */
/*
  info :
  STM8S103 MCU를 사용하여 일반적인 vsprintf 함수를 구현한 예제이며 메모리가 8K flash와 1K sram을 가지고서
  구현한 심플한 vsprintf이다. 여기에서 UART는 RX 인터럽트와 TIM4의 타이머 인터럽트를 기본으로 구현한다.

  stm8s_conf.h 파일에서 아래와 같이 정의해야 한다.
  #if defined(_stm8s103_vsprintf_)
  #define UART_IRQ_RX_USE 1
  #define USE_TIM4_INTERRUPT_HANDLER_CB 1
  #endif

*/

/* Includes ------------------------------------------------------------------*/
#include "stm8s.h"
#include "stm8s_mib.h"

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

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void CLK_Configuration(void);
void GPIO_Configuration(void);
void Toggle(void);
void key_released(void);
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
  {
    /* Configures clocks */
    CLK_Configuration();

    /* Configures GPIOs */
    GPIO_Configuration();
  }
  {
    /* Configures TIM4 */
    tim4IrqInit();
  }
  {
    MibDebugInit(9600);
  }
  {
    enableInterrupts();
  }
  {
    MibWriteDebugStringCheck(1, "\r\n ###################################\r\n");
    // MibWriteDebugStringCheck(1," file name : "__FILE__"\r\n");
    mib_printf(" file name : ( %s )\r\n", __FILE__);
    mib_printf(" file line : ( %d )\r\n", __LINE__);
    MibWriteDebugStringCheck(1, " date :  " __DATE__ "  :  " __TIME__ "\r\n");
    MibWriteDebugStringCheck(1, " yooaroma.com by MYMEDIA Co., Ltd.\r\n");
    MibWriteDebugStringCheck(1, " ###################################\r\n");
  }
  {
    uint16_t vwSec = 0;
    uint16_t vwCh = 0;
    uint32_t vdwCount = 0;
    uint32_t vdwTestValue = 0x80000000; // 0x87620394;
    int vwTestValue = 0x7fff;
    int vwTestValueA = 0xff89;
    int vbTestValue = 5;
    uint32_t vdwDelay = 0;


    Toggle();
    do
    {
      if ((UCOM_KEY1_GPIO->IDR & UCOM_KEY1_PIN) == 0)
      {
        Toggle();
        key_released();
        vwSec = MibGetSecs();
      }
      else
      {
        if (vwSec != MibGetSecs())
        {
          vwSec = MibGetSecs();
          Toggle();
          // MibWriteDebugByte('$');
        }
      }
      // vwCh = MibReadDebugByte();
      // if (vwCh != MIB_DEBUG_READ_NODATA)
      // {
      //   Toggle();
      //   vwSec = MibGetSecs();
      //   MibWriteDebugByte(vwCh);
      // }

      vwCh = MibReadDebugByte();
      if (vwCh != MIB_DEBUG_READ_NODATA)
      {
        MibWriteDebugByte(vwCh);
        if (vwCh == 'a')
        {
          mib_printf("test vdwCount[%ld/0x%08lx]\r\n", vdwCount, vdwCount);
          mib_printf("test vdwCount[%d/0x%x]\r\n", vdwCount, vdwCount);
          MibWriteDebugStringCheck(1, " webgpio.com by MYMEDIA Co., Ltd.\r\n");
        }
        else if (vwCh == 'd')
        {
          mib_printf("test vdwTestValue[0x%lx/%ld]\r\n", vdwTestValue, vdwTestValue);
          mib_printf("test vdwTestValue[0x%x/%d]\r\n", vdwTestValue, vdwTestValue);
          mib_printf("test 0x68[0x%x/%d]\r\n", vbTestValue, vbTestValue);
          mib_printf("test 0x68[0x%lx/%ld]\r\n", (long)vbTestValue, (long)vbTestValue);
        }
        else if (vwCh == 'w')
        {
          mib_printf("test vwTestValue [0x%04x/%d]\r\n", (long)vwTestValue, (long)vwTestValue);
          mib_printf("test vwTestValueA[0x%04x/%u]\r\n", (long)vwTestValueA, (long)vwTestValueA);
          mib_printf("test vwTestValue [0x%x/%d]\r\n", vwTestValue, vwTestValue);
          mib_printf("test vwTestValue [0x%lx/%ld]\r\n", (long)vwTestValue, (long)vwTestValue);
        }
        else if (vwCh == 'b')
        {
          mib_printf("test vbTestValue[0x%02x/%03d]\r\n", vbTestValue, vbTestValue);
          mib_printf("test vbTestValue[0x%x/%d]\r\n", vbTestValue, vbTestValue);
          mib_printf("test vbTestValue[0x%02x/%02d]\r\n", vbTestValue, vbTestValue);
          mib_printf("test vbTestValue[0x%lx/%ld]\r\n", (long)vbTestValue, (long)vbTestValue);
        }
      }
      vdwCount++;
    } while (1);
  }
}

/**
 ******************************************************************************
 * @brief Configures clocks
 * @par Parameters:
 * None
 * @retval void None
 * @par Required preconditions:
 * None
 ******************************************************************************
 */
void CLK_Configuration(void)
{

  /* Fmaster = 16MHz */
  CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);
}

/**
 ******************************************************************************
 * @brief Configures GPIOs
 * @par Parameters:
 * None
 * @retval void None
 * @par Required preconditions:
 * None
 ******************************************************************************
 */
void GPIO_Configuration(void)
{
  // /* GPIOD reset */
  // GPIO_DeInit(UCOM_LED1_GPIO);

  // /* Configure PD0 (LED1) as output push-pull low (led switched on) */
  // GPIO_Init(UCOM_LED1_GPIO, UCOM_LED1_PIN, UCOM_LED1_MODE);

  // GPIO_DeInit(UCOM_KEY1_GPIO);
  // /* Configure PA3 : KEY IN as input push-pull low (led switched on) */
  // GPIO_Init(UCOM_KEY1_GPIO, UCOM_KEY1_PIN, UCOM_KEY1_MODE);

  /* Configure PD0 (LED1) as output push-pull low (led switched on) */
  // GPIO_Init(UCOM_LED1_GPIO, UCOM_LED1_PIN, UCOM_LED1_MODE);
  {
    UCOM_LED1_GPIO->DDR |= (UCOM_LED1_PIN);  /* Set Output mode */
    UCOM_LED1_GPIO->CR1 |= (UCOM_LED1_PIN);  /* Pull-Up or Push-Pull */
    UCOM_LED1_GPIO->CR2 |= (UCOM_LED1_PIN);  /* Output speed up to 10 MHz */
    UCOM_LED1_GPIO->ODR &= ~(UCOM_LED1_PIN); // low...
  }
  /* Configure PA3 : KEY IN as input push-pull low (led switched on) */
  // GPIO_Init(UCOM_KEY1_GPIO, UCOM_KEY1_PIN, UCOM_KEY1_MODE);
  {
    UCOM_KEY1_GPIO->DDR &= ~(UCOM_KEY1_PIN); /* Set input mode */
    UCOM_KEY1_GPIO->CR1 |= (UCOM_KEY1_PIN);  /* Pull-Up or Push-Pull */
    UCOM_KEY1_GPIO->CR2 &= ~(UCOM_KEY1_PIN); /*  External interrupt disabled */
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
  do
  {
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
