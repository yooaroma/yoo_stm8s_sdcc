/**
 ******************************************************************************
 * @file stm8s103_wwdg_main.c
 * @brief UART RX interrupt example.
 * @author yooaroma@gmail.com
 * @version V1.0.0
 * @date 2023.1.6
 ******************************************************************************
 */
/*
  info :
    STM8S Reference Manuals을 참고한다.
    stm8s 가 지니고 있는 Window watchdog (WWDG) 기능에 대해서 공부한다.

  초기화 방법에서 다음의 순서에 의해서 시작된다
  1. WWDG_CR의 레지스터의 WDGA의 bit의 값을 enable하여야 한다.
  2. T6의 비트가 1에서 0으로 되는 순간 무조건 리셋이 된다.
  3. WWDG_CR의 값이 설정이 되면 먼저 WWDG_WR의 레지스터의 값보다 크면 곧 바로 리셋이 발생한다.

  사용방법은 딜레이 주기를 WWDG_CR에 계속적으로 로드함으로서 유효하다.
  한번 enable이 되면 클리어는 리셋에 의해서만 된다.
  따라서 계속적으로 WWDG_CR에 새로운 딜레이 값을 계속적으로 로드하여야 한다.
  되지 않으면 리셋이 발생한다.

  프로그램 예제에서는 키 입력 또는 100번 후에는 refresh를 멈춘다.
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

void wwdg_info(void);
void wwdg_test_refresh(int vwData);
uint16_t calConfigWWDG(uint16_t vwDelay); // msec...

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
    wwdg_info();
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
          wwdg_test(); 
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
                if (strcmp(cmd, "wwdg") == 0)
                {
                  mib_printf("\r\n %s refresh time [%d] msec\r\n", cmd, value_a);
                  wwdg_test_refresh(value_a);
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
    STM8S Reference Manuals을 참고한다.
    stm8s 가 지니고 있는 Window watchdog (WWDG) 기능에 대해서 공부한다.

  초기화 방법에서 다음의 순서에 의해서 시작된다
  1. WWDG_CR의 레지스터의 WDGA의 bit의 값을 enable하여야 한다.
  2. T6의 비트가 1에서 0으로 되는 순간 무조건 리셋이 된다.
  3. WWDG_CR의 값이 설정이 되면 먼저 WWDG_WR의 레지스터의 값보다 크면 곧 바로 리셋이 발생한다.

  사용방법은 딜레이 주기를 WWDG_CR에 계속적으로 로드함으로서 유효하다.
  한번 enable이 되면 클리어는 리셋에 의해서만 된다.
  따라서 계속적으로 WWDG_CR에 새로운 딜레이 값을 계속적으로 로드하여야 한다.
  되지 않으면 리셋이 발생한다.

  프로그램 예제에서는 키 입력 또는 100번 후에는 refresh를 멈춘다.
  tWWDG = TCPU * 12288 ×(T[5:0]+ 1) = 768us * (T[5:0]+ 1) = 49152us (max)  = 49.152ms  @
  fCPU = 16MHz
*/
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
#define _DEBUG_CMD_A_ 1
extern uint16_t calConfigWWDG(uint16_t vwDelay);
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
void wwdg_info(void)
{
  ccprintf(_DEBUG_CMD_A_, ("fcpu : 16MHz refresh time (1 to 49)msec\r\n"));
  ccprintf(_DEBUG_CMD_A_, ("tWWDG = TCPU * 12288 ×(T[5:0]+ 1)\r\n"));
  ccprintf(_DEBUG_CMD_A_, ("768us * (T[5:0]+ 1) = 49152us (max)\r\n"));
  ccprintf(_DEBUG_CMD_A_, ("WDGA(7bit) : This bit is set by software \r\n"));
  ccprintf(_DEBUG_CMD_A_, ("and only cleared by hardware after a reset.\r\n"));
  MibWriteDebugStringCheck(1, " ##########################\r\n");
  MibWriteDebugStringCheck(1, " # wwdg (refresh msec)    #\r\n");
  MibWriteDebugStringCheck(1, " # wwdg (1 to 50) msec    #\r\n");
  MibWriteDebugStringCheck(1, " ##########################\r\n");
}
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
extern uint16_t gwMibMiliSec;
void wwdg_test_refresh(int vwData)
{
  uint8_t vbDelayCR = 0;
  int vbIndex = 0;
  int vwDelayWD = 0;
  if (vwData < 10)
    vwData = 10;
  else if (vwData > 49)
    vwData = 49;

  ccprintf(_DEBUG_CMD_A_, ("fcpu : 16MHz refresh time (%d)msec\r\n", vwData));
  vbDelayCR = (calConfigWWDG(vwData) & 0x3F) | 0xC0;
  gwMibMiliSec = 0;
  WWDG->WR = 0x7F;
  WWDG->CR = vbDelayCR;
  do
  {
    if (gwMibMiliSec == 0)
    {
      vbIndex++;
      if (vbIndex == 100)
      {
        WWDG->CR = 0xFF; // reset 
        ccprintf(_DEBUG_CMD_A_, ("wwdg reset start....(%d)\r\n", vbIndex));
        MibWriteDebugEmptyCheck();
        break;
      }
      else
      {
        WWDG->CR = vbDelayCR;
      }
      gwMibMiliSec = vwData - 10;
    }
    {
      uint16_t vwCh = MibReadDebugByte();
      if (vwCh != MIB_DEBUG_READ_NODATA)
      {
        if (vwCh == 'x')
        {
          WWDG->CR = 0xFF; // reset
          ccprintf(_DEBUG_CMD_A_, ("key wwdg reset start..(%d)\r\n", vbIndex));
          MibWriteDebugEmptyCheck();
          WWDG->CR = vbDelayCR; // reset
          break;
        }
      }
    }
  } while (1);
}
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
uint16_t calConfigWWDG(uint16_t vwDelay) // msec... 
{
	uint32_t vdwData = vwDelay;
	if(vwDelay>50)
	{
		vdwData = 0x3F;
	}
	else
	{
		vdwData = vdwData * 16000000;
		vdwData = vdwData / 12288;
		vdwData = vdwData / 1000;
	}
	vwDelay = vdwData;
	ccprintf(1, ("calConfigWWDG : [%d][0x%x]\r\n",vwDelay,vwDelay));		 
	return vwDelay;
}
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
