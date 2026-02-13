/**
 ******************************************************************************
 * @file stm8s103_debug_dtemp_main.c
 * @brief UART RX interrupt example.
 * @author yooaroma@gmail.com
 * @version V1.0.0
 * @date 2023.1.6
 ******************************************************************************
 */
/*
  info :
    1. 터미널 창에서 key 입력 상태 표시
    2. led 상태 변경
    3. beep 음 시작
    4. beep 음 끝
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
  PD4 : D13 : BEEP
  PD5 : D14 : A3 : TXD
  PD6 : D15 : A4 : RXD
  RESET
  PA1 : D0  : KEY1
  PA2 : D1  : LED1
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

void debug_info(void);
void fnBeepStart(uint16_t vwFreq);
void calConfigBEEP(uint16_t vwFreq); // msec...

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
  uint32_t vwData = 0;
  uint32_t pre_vwData = 0;
  uint32_t delta_data = 0;

  char line_buffer[31]; // 최대 30자 + NULL
  uint8_t index = 0;

  // 파싱 결과 저장용 변수
  char cmd[10];
  char cmd_a[10];
  int value_a;
  int found;
  char cmd_clear[17] = "                "; // 16칸 공백 문자열

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
    mib_printf("\r\n ###################################\r\n");
    // MibWriteDebugStringCheck(1," file name : "__FILE__"\r\n");
    // mib_printf(" file name : ( %s )\r\n", __FILE__);
    // __FILE__ 매크로는 전체 경로를 포함하기 때문에 get_filename_manual() 함수를 사용하여 파일 이름만 추출
    mib_printf(" file name : ( %s )\r\n", get_filename_manual(__FILE__));
    mib_printf(" file line : ( %d )\r\n", __LINE__);
    mib_printf(" date :  " __DATE__ "  :  " __TIME__ "\r\n");
    mib_printf(" yooaroma.com by MYMEDIA Co., Ltd.\r\n");
    mib_printf(" ###################################\r\n");
  }
  {
    mib_printf("\r\n STM8S103 DEBUG Test Program Start...\r\n");
    // I2C_Config();
  }
  {
    debug_info();
  }
  {
    Toggle();
    MibWriteDebugByte('>'); // 문자 하나하나 에코 보냄
    do
    {
      // if ((UCOM_KEY1_GPIO->IDR & UCOM_KEY1_PIN) == 0)
      // {
      //   Toggle();
      //   key_released();
      //   vwSec = MibGetSecs();
      // }
      // else
      // {
      //   if (vwSec != MibGetSecs())
      //   {
      //     vwSec = MibGetSecs();
      //     Toggle();
      //   }
      // }
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
            MibWriteDebugByte('\n'); // 터미널 줄바꿈 에코
            if (index > 0)
            {
              // 3. sscanf 호출하여 해석 (예: "SET 100" 형식)
              found = mib_sscanf(line_buffer, "%s %s %d", cmd, cmd_a, &value_a);

              if (found >= 1)
              {
                // 해석 성공 시 처리 (예: 명령어 확인)
                if (strcmp(cmd, "key") == 0)
                {
                  if (strcmp(cmd_a, "rd") == 0)
                  {
                    if ((UCOM_KEY1_GPIO->IDR & UCOM_KEY1_PIN) == 0)
                    {
                      mib_printf(" Key down...\r\n");
                      key_released();
                      mib_printf(" Key up...\r\n");
                    }
                    else
                    {
                      mib_printf(" Key up...\r\n");
                    }
                  }
                  else
                  {
                    mib_printf(" Unknown debug command\r\n");
                  }
                  // mib_printf("\r\n %s %s %s",cmd, cmd_a, cmd_b);
                }
                else if (strcmp(cmd, "led") == 0)
                {
                  if (strcmp(cmd_a, "wr") == 0)
                  {
                    if (value_a == 0)
                    {
                      UCOM_LED1_GPIO->ODR |= (uint8_t)UCOM_LED1_PIN;
                      mib_printf(" led off..\r\n");
                    }
                    else
                    {
                      UCOM_LED1_GPIO->ODR &= ~(uint8_t)UCOM_LED1_PIN;
                      mib_printf(" led on..\r\n");
                    }
                  }
                  else
                  {
                    mib_printf(" Unknown debug command\r\n");
                  }
                  // mib_printf("\r\n %s %s %s",cmd, cmd_a, cmd_b);
                }
                else if (strcmp(cmd, "beep") == 0)
                {
                  if (strcmp(cmd_a, "start") == 0)
                  {
                    if(value_a  < 1000)
                    {
                      mib_printf(" beep stop\r\n");
                      value_a = 0;
                      fnBeepStart((uint16_t)value_a);
                    }
                    else 
                    {
                      mib_printf("\r\n beep freq [%d]Hz (1Kz to 4Kz)\r\n", value_a);
                      fnBeepStart((uint16_t)value_a);
                    }
                  }
                  else if (strcmp(cmd_a, "end") == 0)
                  {
                    mib_printf(" beep stop\r\n");
                    value_a = 0;
                    fnBeepStart((uint16_t)value_a);
                  }
                  else
                  {
                    mib_printf(" Unknown debug command\r\n");
                  }
                  
                }
                else if ((strcmp(cmd, "help") == 0) || (strcmp(cmd, "?") == 0))
                {
                  debug_info();
                }
                else
                {
                  debug_info();
                }
              }
            }
            index = 0;              // 버퍼 인덱스 초기화
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
              index = 0;              // 버퍼 인덱스 초기화
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
  /* Configure PB5 (LED1) as output push-pull low (led switched on) */
  // GPIO_Init(UCOM_LED1_GPIO, UCOM_LED1_PIN, UCOM_LED1_MODE);
  {
    UCOM_LED1_GPIO->DDR |= (UCOM_LED1_PIN);  /* Set Output mode */
    UCOM_LED1_GPIO->CR1 |= (UCOM_LED1_PIN);  /* Pull-Up or Push-Pull */
    UCOM_LED1_GPIO->CR2 |= (UCOM_LED1_PIN);  /* Output speed up to 10 MHz */
    UCOM_LED1_GPIO->ODR &= ~(UCOM_LED1_PIN); // low...
  }
  /* Configure PA1 : KEY IN as input push-pull low (led switched on) */
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
    mib_printf(" %s, %d\r\n",file,line);
  }
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
void debug_info(void)
{
  mib_printf(" ############################\r\n");
  mib_printf(" # <help> or <?>            #\r\n");
  mib_printf(" # PD4 : BEEP : 1,2,4 KHz   #\r\n");
  mib_printf(" # PA1 : KEY, PB5 : LED     #\r\n");
  mib_printf(" # <beep> <start> [freq] HZ #\r\n");
  mib_printf(" # <beep> <end>             #\r\n");
  mib_printf(" # <key> <rd>               #\r\n");
  mib_printf(" # <led> <wr> [data]        #\r\n");
  mib_printf(" ############################\r\n");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

#define rd_ADDR8(A) (*((volatile uint8_t *)(A)))
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
/**
 * @brief
 * @par Parameters:
 * vwFreq: bbuzzer frequency
 * @retval Return value:
 * None
 */
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
#define UCOM_BEEP_GPIO GPIOD     // PD4
#define UCOM_BEEP_PIN GPIO_PIN_4 // PD4
#define UCOM_BEEP_MODE GPIO_MODE_OUT_PP_LOW_FAST
#define OPT2_REG 0x4803
#define AFR7_BIT 0x80 // 1: Port D4 alternate function = BEEP // AFR7 Alternate function remapping option 7
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
void fnBeepStart(uint16_t vwFreq) // msec...
{
  // static uint8_t gbBeepStartBit = 0;
  // GPIO init
  /* Configure PD4 (BEEPER) as output push-pull low */
  // GPIO_Init(GPIOD, GPIO_PIN_4 , GPIO_MODE_OUT_PP_LOW_FAST);
  // PD4 : BEEP
  /* Configure PD4 (BEEP) as output push-pull low (led switched on) */
  // GPIO_Init(UCOM_BEEP_GPIO, UCOM_BEEP_PIN, UCOM_BEEP_MODE);
  //  if(gbBeepStartBit == 0)
  {
    // gbBeepStartBit = 1;
    {
      UCOM_BEEP_GPIO->DDR |= (UCOM_BEEP_PIN);  /* Set Output mode */
      UCOM_BEEP_GPIO->CR1 |= (UCOM_BEEP_PIN);  /* Pull-Up or Push-Pull */
      UCOM_BEEP_GPIO->CR2 |= (UCOM_BEEP_PIN);  /* Output speed up to 10 MHz */
      UCOM_BEEP_GPIO->ODR &= ~(UCOM_BEEP_PIN); // low...
    }
#if defined(STM8S105)
    {
      // GPIO remap
      /* set option bytes */
      if (FLASH_ReadByte(OPT2_REG) != AFR7_BIT)
      {
        FLASH_Unlock(FLASH_MEMTYPE_DATA);
        /* Enable by HW WWDG */
        FLASH_ProgramOptionByte(OPT2_REG, AFR7_BIT);
      }
      // vbData=FLASH_ReadOptionByte(OPT2_REG);
    }
#endif
  }
  if (vwFreq != 0)  
  {
    {
      // set freq
      // mib_printf("beep start : [%d] Hz\r\n",(vwFreq));
      calConfigBEEP(vwFreq);
    }
    {    
      // enable beep
      /* Enable the BEEP peripheral */
      BEEP->CSR |= BEEP_CSR_BEEPEN;
      // mib_printf("beep CSR ....(0x%x)\r\n",BEEP->CSR);
      // mib_printf("beep AFR7....(0x%x)\r\n",rd_ADDR8(0x4803));
    }
  }
  else 
  {
    /* Disable the BEEP peripheral */
    BEEP->CSR &= (uint8_t)(~BEEP_CSR_BEEPEN);
  }
}
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
/**
 * @brief
 * @par Parameters:
 * vwFreq: bbuzzer frequency
 * @retval Return value:
 * None
 */
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
void calConfigBEEP(uint16_t vwFreq) // msec...
{
  uint8_t vbBEEPSEL = 0;
  uint8_t vbBEEPDIV = 0;
  uint32_t vdwData = vwFreq;
  uint32_t vdwDataLast = 0;
  uint8_t vdwDiv = 0;
  if (vwFreq < 500)
  {
    // error...
    vbBEEPSEL = 0xFF;
  }
  else if (vwFreq < 1000)
  {
    vbBEEPSEL = 0x00; // f / (8 * div) khz
    vdwDiv = (128000 / 8) / vdwData;
    vbBEEPDIV = vdwDiv - 2;
    vdwDataLast = (128000 / 8) / vdwDiv;
  }
  else if (vwFreq < 2000)
  {
    vbBEEPSEL = 0x01; // f / (4 * div) khz
    vdwDiv = (128000 / 4) / vdwData;
    vbBEEPDIV = vdwDiv - 2;
    vdwDataLast = (128000 / 8) / vdwDiv;
  }
  else if (vwFreq < 32000)
  {
    vbBEEPSEL = 0x02; // f / (2 * div) khz
    vdwDiv = (128000 / 2) / vdwData;
    vbBEEPDIV = vdwDiv - 2;
    vdwDataLast = (128000 / 8) / vdwDiv;
  }
  else
  {
    // error...
    vbBEEPSEL = 0xFF;
  }
  if (vbBEEPSEL != 0xFF)
  {
    mib_printf("beep init [%d : SEL(%d) : DIV(%d)] Hz\r\n", vwFreq, vbBEEPSEL, vbBEEPDIV);
    /* Set a default calibration value if no calibration is done */
    // if ((BEEP->CSR & BEEP_CSR_BEEPDIV) == BEEP_CSR_BEEPDIV)
    BEEP->CSR = BEEP_CSR_BEEPDIV;
    {
      BEEP->CSR &= (uint8_t)(~BEEP_CSR_BEEPDIV); /* Clear bits */
      BEEP->CSR |= (vbBEEPDIV & BEEP_CSR_BEEPDIV);
    }
    {
      /* Select the output frequency */
      BEEP->CSR &= (uint8_t)(~BEEP_CSR_BEEPSEL);
      BEEP->CSR |= (uint8_t)((vbBEEPSEL & 0x03) << 6);
    }
  }
  else
  {
    BEEP->CSR = BEEP_CSR_BEEPDIV;
    mib_printf("beep init error... [%d] Hz\r\n", vwFreq);
  }
}
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
