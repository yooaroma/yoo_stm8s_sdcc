/**
 ******************************************************************************
 * @file stm8s105_debug_main.c
 * @brief UART RX interrupt example.
 * @author yooaroma@gmail.com
 * @version V1.0.0
 * @date 2023.1.6
 ******************************************************************************
 */
/*
**STM8S105K4T6**는 STM8S 시리즈 중에서도 가장 대중적인 **Access line** 모델로, 특히 32핀 패키지 구성을 가지고 있어 소형 제어 보드에 자주 쓰입니다.
모델명 뒤의 **K4T6**라는 접미사는 구체적인 하드웨어 구성을 의미합니다. 상세 제원을 정리해 드립니다.
---

### 1. 모델명(K4T6)의 의미 분석
* **K**: 32핀 (32-pin) 패키지
* **4**: 16KB Flash 메모리 용량
* **T**: LQFP 패키지 타입
* **6**: 동작 온도 범위 (-40°C ~ 85°C)

### 2. 주요 하드웨어 제원 (Core & Memory)
* **Core:** 16MHz STM8 (8-bit)
* **Flash:** **16 KB** (프로그램 저장 공간)
* **RAM:** **2 KB**
* **Data EEPROM:** **1 KB** (내장되어 있어 설정값 저장 시 별도의 외부 칩이 필요 없음)
* **동작 전압:** 2.95V ~ 5.5V (5V 시스템에서 안정적으로 동작 가능)

### 3. 주변 장치 (Peripherals)
* **I/O 핀:** 총 32핀 중 최대 **25개의 고속 I/O** 사용 가능.
* **통신 포트:**
* **UART:** 1개
* **SPI:** 1개 (최대 8Mbit/s)
* **I2C:** 1개 (최대 400kbit/s)

* **타이머 (Timers):**
* **TIM1:** 16비트 고급 제어 타이머 (3상 모터 제어 및 PWM 지원)
* **TIM2:** 16비트 범용 타이머
* **TIM4:** 8비트 기본 타이머

* **ADC:** 10비트 해상도, **7개 채널** 지원.

### 4. 물리적 특성
* **패키지:** LQFP32 (7x7 mm)
* **핀 피치:** 0.8 mm (납땜이 비교적 수월한 크기입니다)

---

### 💡 STM8S105K4T6 사용 시 팁
1. **내장 클럭:** 외부 크리스탈 없이도 내부 16MHz HSI(High Speed Internal) 오실레이터를 사용할 수 있어 회로를 간소화할 수 있습니다.
2. **프로그래밍:** **ST-LINK/V2** 디버거와 **SWIM** 인터페이스(1개 핀 사용)를 통해 아주 간편하게 소스 코드를 업로드하고 디버깅할 수 있습니다.
3. **메모리 주의:** Flash가 16KB로 넉넉하지 않으므로, 복잡한 라이브러리(표준 라이브러리 등)를 과하게 사용하면 용량이 부족할 수 있습니다. 필요한 기능만 선별해서 구현하는 것이 좋습니다.

혹시 이 칩을 이용해 회로를 설계 중이신가요? **전원 핀(VDD/VSS) 연결이나 디버깅 핀(SWIM) 구성**에 대해 궁금한 점이 있다면 더 자세히 설명해 드릴 수 있습니다.

2	  PA1	(OscIn, no HS)	13	
3	  PA2	(OscOut, no HS)	14	
8	  PF4	Ain12 (not supported, no HS)	15	
11	PB5	Ain5, [SDA], no HS	16	Analog A0
12	PB4	Ain4, [SCL], no HS	17	Analog A1
13	PB3	Ain3, no HS	18	Analog A2
14	PB2	Ain2, no HS	19	Analog A3
15	PB1	Ain1, no HS	20	Analog A4
16	PB0	Ain0, no HS	21	Analog A5
17	PE5	SPI_NSS, no HS	22	LED
18	PC1	T1-1	23	PWM
19	PC2	T1-2	24	PWM
20	PC3	T1-3	0	PWM
21	PC4	T1-4	1	PWM
22	PC5	SCK	2	
23	PC6	MOSI	3	
24	PC7	MISO	4	
25	PD0	T3-2	5	PWM
26	PD1	SWIM	6	
27	PD2	T3-1	7	PWM
28	PD3	T2-2	8	PWM
29	PD4	T2-1/Beep	9	PWM
30	PD5	TX	10	
31	PD6	RX	11	
32	PD7	TLI	12	

*/
/*
  info :
    1. 터미널 창에서 key 입력 상태 표시
    2. led 상태 변경
*/

/* Includes ------------------------------------------------------------------*/
#include "stm8s.h"
#include "stm8s_mib.h"
#include "string.h"
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
    mib_printf("\r\n STM8S105 DEBUG Test Program Start...\r\n");
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
      //     // mib_printf(" uart test...\r\n");
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
                      // UCOM_LED1_GPIO->ODR |= (uint8_t)UCOM_LED1_PIN;
                      UCOM_LED1_GPIO->ODR &= ~(uint8_t)UCOM_LED1_PIN;
                      mib_printf(" led off..\r\n");
                    }
                    else
                    {
                      UCOM_LED1_GPIO->ODR |= (uint8_t)UCOM_LED1_PIN;
                      // UCOM_LED1_GPIO->ODR &= ~(uint8_t)UCOM_LED1_PIN;
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
 * @brief Toggle PD7 (Led LD1)
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
  mib_printf(" # PA1 : KEY, PD7 : LED     #\r\n");
  mib_printf(" # <key> <rd>               #\r\n");
  mib_printf(" # <led> <wr> [data]        #\r\n");
  mib_printf(" # PD4 : BEEP : 1,2,4 KHz   #\r\n");
  mib_printf(" # <beep> <start> [freq] HZ #\r\n");
  mib_printf(" # <beep> <end>             #\r\n");
  mib_printf(" ############################\r\n");
}
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
void flash_write_option_opt7(uint8_t mode) // mode 0: reset, 1: set
{
    uint16_t vwAddress = 0;
    uint16_t vwData = 0;
    vwAddress = OPT2_REG;
    mib_printf("\r\n  Table 11. Option byte : OPT2 : AFR7 , BEEP");
    {
      vwData = mmFlashOptionRead(vwAddress);
      mib_printf("\r\n 1. mmFlashOptionRead(0x%lx)=[0x%04lx]", (long)vwAddress, (long)vwData);
    }
    {
      mmFlashOptionUnlock();               // unlock option byte
      if(mode == 0)
      {
        mmFlashOptionWrite(vwAddress, 0x00); // AFR7 : BEEP, reset remap
      }
      else
      {
        mmFlashOptionWrite(vwAddress, AFR7_BIT); // AFR7 : BEEP, set remap
      }
      vwData = mmFlashOptionRead(vwAddress);
      mib_printf("\r\n 2. mmFlashOptionRead(0x%lx)=[0x%04lx]", (long)vwAddress, (long)vwData);
    }
    mib_printf("\r\n"); // 마지막 줄바꿈
}
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
void fnBeepStart(uint16_t vwFreq) // msec...
{
  uint8_t vOptReg = 0;
  if (vwFreq != 0)  
  {
    {
      {
        UCOM_BEEP_GPIO->DDR |= (UCOM_BEEP_PIN);  /* Set Output mode */
        UCOM_BEEP_GPIO->CR1 |= (UCOM_BEEP_PIN);  /* Pull-Up or Push-Pull */
        UCOM_BEEP_GPIO->CR2 |= (UCOM_BEEP_PIN);  /* Output speed up to 10 MHz */
        UCOM_BEEP_GPIO->ODR &= ~(UCOM_BEEP_PIN); // low...
      }
  #if defined(STM8S105)
      {
        flash_write_option_opt7(1); // AFR7 : PD4 alternate function = BEEP, set remap
      }
  #endif
    }
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
