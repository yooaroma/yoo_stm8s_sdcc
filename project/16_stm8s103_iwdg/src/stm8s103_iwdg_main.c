/**
 ******************************************************************************
 * @file stm8s103_iwdg_main.c
 * @brief UART RX interrupt example.
 * @author yooaroma@gmail.com
 * @version V1.0.0
 * @date 2023.1.6
 ******************************************************************************
 */
/*
  info :
    STM8S Reference Manuals을 참고한다.
    stm8s 가 지니고 있는 IWDG : Independent watchdog 기능에 대해서 공부한다.

  초기화 방법에서 다음의 순서에 의해서 시작되어야 하고
    IWDG->KR = 0xCC; // KEY_START , enable...
    IWDG->KR = 0x55; // KEY_ACCESS
    IWDG->PR = (uint8_t)vbPRBIT;
    IWDG->RLR = vbRLRBIT;
    IWDG->KR = 0xAA; // KEY_REFRESH

    주기적으로 KEY_REFRESH 가 이루어져야 한다.
    최대 값은 calConfigIWDG(Delay msec) 함수에서 설정한 것에 의해서 한다.

    refress 를 100msec 씩 증가 시켜서 watch dog이 되도록 한다.
    예를 들어 1000 msec delay이면 11번 후에 watch dog이 발생하여 리셋이 된다.
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

void iwdg_info(void);
void calConfigIWDG(uint16_t vwDelay); // msec...
void iwdg_test_refresh(int vwDelay);
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
    iwdg_info();
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
      #if 0
      vwCh = MibReadDebugByte();
      if (vwCh != MIB_DEBUG_READ_NODATA)
      {
        uint16_t vwFreq = 1000;
        uint16_t vwDelay = 1000;
        if (vwCh == '1')
        {          
          /* test start program */
          iwdg_test(); 
        }
        else if (vwCh == 'f')
        {
        }
        else
        {
          MibWriteDebugByte(vwCh);
        }
      }
      #else
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
                if (strcmp(cmd, "iwdg") == 0)
                {
                  mib_printf("\r\n %s %d",cmd, value_a);
                  // wwdg y (refresh msec)
                  mib_printf("\r\n iwdg refresh time [%d] msec\r\n", value_a);
                  iwdg_test_refresh(value_a);
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
      #endif
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
/*
  info :
    STM8S Reference Manuals을 참고한다.
    stm8s 가 지니고 있는 IWDG : Independent watchdog 기능에 대해서 공부한다.

  초기화 방법에서 다음의 순서에 의해서 시작되어야 하고
    IWDG->KR = 0xCC; // KEY_START , enable...
    IWDG->KR = 0x55; // KEY_ACCESS
    IWDG->PR = (uint8_t)vbPRBIT;
    IWDG->RLR = vbRLRBIT;
    IWDG->KR = 0xAA; // KEY_REFRESH

    주기적으로 KEY_REFRESH 가 이루어져야 한다.
    최대 값은 calConfigIWDG(Delay msec) 함수에서 설정한 것에 의해서 한다.

    refress 를 100msec 씩 증가 시켜서 watch dog이 되도록 한다.
    예를 들어 1000 msec delay이면 11번 후에 watch dog이 발생하여 리셋이 된다.
*/
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
extern void calConfigIWDG(uint16_t vwDelay);
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
void iwdg_info(void)
{  
  ccprintf(1, ("100msec inc time watch dog generate..\r\n"));
  MibWriteDebugStringCheck(1, " ################################\r\n");
  MibWriteDebugStringCheck(1, " # IWDG : Independent watchdog  #\r\n");
  MibWriteDebugStringCheck(1, " # min refresh time : 62.5 usec #\r\n");
  MibWriteDebugStringCheck(1, " # max refresh time : 1.02 sec  #\r\n");
  MibWriteDebugStringCheck(1, " # iwdg (refresh msec)          #\r\n");
  MibWriteDebugStringCheck(1, " # iwdg 900       900 msec      #\r\n");
  MibWriteDebugStringCheck(1, " ################################\r\n");
}
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
extern uint16_t gwMibMiliSec;
void iwdg_test_refresh(int vwDelay)
{
  int vbIndex = 0;
  int vbIndexKR = 0;
  int vwDelayWD = 0;

  calConfigIWDG((uint16_t)vwDelay);
  gwMibMiliSec = 0;
  do
  {
    if (gwMibMiliSec == 0)
    {
      gwMibMiliSec = 100;
      vbIndex++;
      if(vbIndex<5)
      {
        IWDG->KR = 0xAA; // KEY_REFRESH
        vbIndexKR = 0;
      }
      else
      {
        vbIndexKR++;
        vwDelayWD += 100;
      }
      ccprintf(1, ("iwdg test ing....(%d)KR[%d]WD[%d]\r\n", vbIndex, vbIndexKR, vwDelayWD));
    }
  } while (1);
}
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
/**
  * @brief 
  * @par Parameters:
  * vwDelay: independent watchdog delay msec 
  * @retval Return value:
  * None
  */
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
uint16_t gwTableDataIWDG[] = {15,31,63,127,255,510,1000,0};
void calConfigIWDG(uint16_t vwDelay) // msec... 
{
	uint32_t vdwData = vwDelay;
	uint8_t vbIndex = 0;
	uint8_t vbPRBIT = 0;
	uint8_t vbRLRBIT = 0;
	for(vbIndex=0;;vbIndex++)
	{
		if(gwTableDataIWDG[vbIndex]==0)
		{
			vwDelay = gwTableDataIWDG[vbIndex-1];
			vbPRBIT = vbIndex - 1;
			break;
		}
		else if(vwDelay <= gwTableDataIWDG[vbIndex])
		{
			vbPRBIT = vbIndex;
			break;
		}
	}
	{
		vdwData = (vdwData * 128) / 2;
		vdwData = vdwData / (1<<(vbPRBIT+2));
		if(vdwData > 255)
		{
			vbRLRBIT = 0xff;
		}
		else
		{
			vbRLRBIT = (uint8_t)(vdwData&0xff);
		}
		ccprintf(1, ("calConfigIWDG...vbPRBIT[%u]/vbRLRBIT[%u]\r\n",vbPRBIT,vbRLRBIT));		
		MibWriteDebugEmptyCheck();
		IWDG->KR = 0xCC; // KEY_START , enable...
		IWDG->KR = 0x55; // KEY_ACCESS
		IWDG->PR = (uint8_t)vbPRBIT; 
		IWDG->RLR = vbRLRBIT;
		IWDG->KR = 0xAA; // KEY_REFRESH
	}
}
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
