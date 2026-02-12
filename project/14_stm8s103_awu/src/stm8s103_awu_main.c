/**
 ******************************************************************************
 * @file stm8s103_awu_main.c
 * @brief UART RX interrupt example.
 * @author yooaroma@gmail.com
 * @version V1.0.0
 * @date 2023.1.6
 ******************************************************************************
 */
/*
  info :
    auto wake up 기능에 대해서 공부한다.

    AWU : auto wake up
    #define USE_AWU_INTERRUPT_HANDLER_CB  1
    컴파일 시 위 변수가 define이 되어야 인터럽트시 콜백이 된다.

    awu start 1000 : halt & wait 1000 msec
    void AWU_InterruptHandler_cb(void) 함수가 call back 이 된다.
    halt , 1000msec 후에 AWU_InterruptHandler_cb 함수가 수행이 될때 클리어가 되어야 한다.
    클리어가 되지 않으면 halt후에 깨어나지 못한다.
    이점 주의해야 한다.

    auto wakeup 은 다음의 값을 calConfigAWU(delay msec) 설정하고 halt 모드에 들어가면 irq가 발생하며
    AWU_InterruptHandler_cb 를 callback 하여 수행하며 수행시 irq flag를 클리어를 하여야 한다.
    클리어를 하지 않으면 동작하지 않는다.
*/

/* Includes ------------------------------------------------------------------*/
#include "stm8s.h"
#include "stm8s_mib.h"
#include "string.h"

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

#if defined(USE_AWU_INTERRUPT_HANDLER_CB)
void calConfigAWU(uint16_t vwDelay);
void AWU_InterruptHandler_cb(void);
#endif // USE_AWU_INTERRUPT_HANDLER_CB

void awu_test(int vwDelay);
void awu_info(void);
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
  uint16_t vwSec = 0;
  uint16_t vwCh = 0;
  uint8_t vbIndex = 0;
  uint8_t vbIndexA = 0;
  uint16_t vwAddress = 0x0;
  uint16_t vwData = 0;
  uint8_t vbData = 0;
  uint8_t vbBufferID[13]; // total 12byte...

  char line_buffer[31]; // 최대 30자 + NULL
  uint8_t index = 0;
  char c;

  // 파싱 결과 저장용 변수
  char cmd[10];
  int value_a;
  int found;
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
    awu_info();
  }
  {
    Toggle();
    MibWriteDebugByte('>'); // 문자 하나하나 에코 보냄
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
        }
      }
      {
        // 1. getchar()로 문자 하나 읽기
        vwCh = MibReadDebugByte();
        if (vwCh != MIB_DEBUG_READ_NODATA)
        {
          // 2. \n(또는 \r)을 만났을 때 처리
          if (vwCh == '\r' || vwCh == '\n')
          {
            line_buffer[index] = '\0'; // 문자열 종료
            MibWriteDebugByte('\r');
            MibWriteDebugByte('\n');// 터미널 줄바꿈 에코
            if (index > 0)
            {
              // 3. sscanf 호출하여 해석 (예: "SET 100" 형식)
              found = mib_sscanf(line_buffer, "%s %d", cmd, &value_a);
              if (found >= 2)
              {
                // 해석 성공 시 처리 (예: 명령어 확인)
                if (strcmp(cmd, "awu") == 0)
                {
                  mib_printf("\r\n %s wake up time [%d] msec\r\n", cmd, value_a);
                  awu_test(value_a);
                }
                else
                {
                  mib_printf("\r\n Unknown command: %s\r\n", cmd);
                } 
              }
              else
              {
                mib_printf("\r\n Invalid command format\r\n");  
              }
            }
            index = 0; // 버퍼 인덱스 초기화
            MibWriteDebugByte('>'); // 문자 하나하나 에코 보냄
          }
          // 3. 일반 문자 처리 (최대 30자 제한)
          else
          {
            if (index < 30)
            {
              line_buffer[index++] = (uint8_t)vwCh;
              MibWriteDebugByte(vwCh); // 문자 하나하나 에코 보냄
            }
            else
            {
              // 버퍼가 꽉 찼을 때의 처리 (선택 사항: 경고음 등)
              index = 0; // 버퍼 인덱스 초기화
              MibWriteDebugByte('>'); // 문자 하나하나 에코 보냄
            }
          }
        }
      }
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
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
#if defined(USE_AWU_INTERRUPT_HANDLER_CB)
void calConfigAWU(uint16_t vwDelay) // msec...
{
  uint32_t vdwData = vwDelay;
  uint32_t vdwDataLast = 0;
  int vbIndex = 15;
  if (vwDelay > 5120)
  {
    vbIndex = 12;
  }
  else if (vwDelay > 2048)
  {
    vbIndex = 11;
  }
  else
  {
    do
    {
      if (vwDelay > (1 << vbIndex))
      {
        break;
      }
      if (vbIndex)
        vbIndex--;
      else
        break;
    } while (1);
  }
  if (vwDelay == 1)
    vbIndex = 2;
  else
    vbIndex = vbIndex + 3;
  ccprintf(1, ("awu vwDelay [%08lx,%d] bit\r\n", vdwData, vbIndex));
  {
    uint8_t vbAWUTB = 0;
    uint8_t vbAPR = 0;
    vbAWUTB = vbIndex;
    vdwData = vdwData * 128;
    ccprintf(1, ("awu vdwData [%08lx,%d,%d] \r\n", vdwData, vbAPR, vbAWUTB));
    if (vbAWUTB == 14)
    {
      vdwData = vdwData / (uint32_t)(5 * (1 << 11));
      vdwDataLast = (5 * ((1 << 11) / 128)) * vdwData;
    }
    else if (vbAWUTB == 15)
    {
      vdwData = vdwData / 30;
      vdwData = vdwData / (uint32_t)(1 * (1 << 11));
      vdwDataLast = (30 * ((1 << 11) / 128)) * vdwData;
    }
    else
    {
      vdwData = vdwData / (1 << (vbAWUTB - 1));
      vdwDataLast = (1 << (vbAWUTB - 1)) * vdwData / 128;
    }
    vbAPR = vdwData;
    ccprintf(1, ("awu vdwData [%08lx,%d,%d] \r\n", vdwDataLast, (int)vbAPR, (int)vbAWUTB));
    {
      AWU->APR = (uint8_t)vbAPR;
      AWU->TBR = (uint8_t)vbAWUTB;
      // AWU->CSR = (uint8_t)0;
    }
  }
}
#endif // USE_AWU_INTERRUPT_HANDLER_CB
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
#if defined(USE_AWU_INTERRUPT_HANDLER_CB)
void AWU_InterruptHandler_cb(void)
{
  // clear AWUF
  uint8_t vbData;
  vbData = (((uint8_t)(AWU->CSR & AWU_CSR_AWUF) == (uint8_t)0x00) ? RESET : SET);
  if (vbData == SET)
  {
    UCOM_LED1_GPIO->ODR ^= (uint8_t)UCOM_LED1_PIN;
  }
  else
  {
    UCOM_LED1_GPIO->ODR ^= (uint8_t)UCOM_LED1_PIN;
  }
}
#endif // USE_AWU_INTERRUPT_HANDLER_CB
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
void awu_info(void)
{
  MibWriteDebugStringCheck(1, " ##########################\r\n");
  MibWriteDebugStringCheck(1, " # awu(auto wake up)      #\r\n");
  MibWriteDebugStringCheck(1, " # min 0.01 msec delay    #\r\n");
  MibWriteDebugStringCheck(1, " # max 30.720 sec         #\r\n");
  MibWriteDebugStringCheck(1, " # awu (wake up time)msec #\r\n");
  MibWriteDebugStringCheck(1, " # awu 1000               #\r\n");
  MibWriteDebugStringCheck(1, " ##########################\r\n");
}
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
void awu_test(int vwDelay)
{
  int vbIndex = 5;
  ccprintf(1, ("awu delay [%d] msec wake up\r\n", (vwDelay)));
  calConfigAWU(vwDelay);
  AWU->CSR |= AWU_CSR_AWUEN;
  do
  {
    ccprintf(1, ("awu start....(%d)\r\n", vbIndex));
    MibWriteDebugEmptyCheck();
    {
      // halt & Wait...
      halt();
      // wfi();
    }
    ccprintf(1, ("awu end....(%d)\r\n", vbIndex));
    vbIndex--;
    if (vbIndex == 0)
    {
      ccprintf(1, ("awu test end....\r\n"));
      break;
    }
  } while (1);
  AWU->CSR &= (uint8_t)(~AWU_CSR_AWUEN);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
