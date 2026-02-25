/**
 ******************************************************************************
 * @file stm8s103_adc_pwm_main.c
 * @brief UART RX interrupt example.
 * @author yooaroma@gmail.com
 * @version V1.0.0
 * @date 2023.1.6
 ******************************************************************************
 */
/*
  info :
    STM8S103F3 MCU
    System Clock : 16MHz (HSI)
    UART1 : 115200, 8, N, 1
    TIM4 : 1msec interrupt
    TIM2 : PWM output : 1KHz, 50% duty cycle
    TIM2_CH1 : PD4

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
  PD2 : D11 : A1 : AIN3
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
void adc_pwm_info(void);
void tim2_pwm_test(uint16_t vwPeriod, uint16_t vwDuty);
uint16_t getKeyAdc(char vbAdcChannel);
uint8_t Get_Key_Number(uint16_t adc_val);
void joystick_info(void);
char *key_info[6] = {"NO KEY", "RIGHT", "LEFT", "DOWN", "UP", "ENTER"};
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
  uint16_t vwSec = 0;
  uint16_t vwCh = 0;
  uint8_t vbIndex = 0;
  uint32_t vwData = 0;
  uint32_t pre_vwData = 0;
  uint32_t delta_data = 0;

  char line_buffer[31]; // 최대 30자 + NULL
  uint8_t index = 0;
  char c;

  // 파싱 결과 저장용 변수
  char cmd[10];
  int value_a;
  int value_b;
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
    adc_pwm_info();
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
            MibWriteDebugByte('\n'); // 터미널 줄바꿈 에코
            if (index > 0)
            {
              // 3. sscanf 호출하여 해석 (예: "SET 100" 형식)
              found = mib_sscanf(line_buffer, "%s %d %d", cmd, &value_a, &value_b);

              if (found >= 1)
              {
                // 해석 성공 시 처리 (예: 명령어 확인)
                if (strcmp(cmd, "adc") == 0)
                {
                  mib_printf("\r\n %s [%d] channel", cmd, value_a);
                  vwData = getKeyAdc(value_a);
                  mib_printf("\r\n ADC raw value = %d", (uint16_t)vwData);
                  vwData = (vwData * 3300) / 1023; // mV
                  mib_printf("\r\n ADC voltage = %d mV", (uint16_t)vwData);
                  vwData = (vwData * 100) / 3300; // %
                  mib_printf("\r\n ADC voltage = %d %%", (uint16_t)vwData);
                  mib_printf("\r\n");
                }
                else if (strcmp(cmd, "auto") == 0)
                {
                  mib_printf("\r\n %s : key press auto adc to pwm", cmd);
                  pre_vwData = 0;
                  do
                  {
                    value_a = 3; // ADC channel 3 : PD2
                    vwData = getKeyAdc(value_a);
                    if (vwData > pre_vwData)
                    {
                      delta_data = vwData - pre_vwData;
                    }
                    else
                    {
                      delta_data = pre_vwData - vwData;
                    }
                    if (delta_data < 10)
                    {
                      delay_ms(100);
                      vwCh = MibReadDebugByte();
                      if (vwCh != MIB_DEBUG_READ_NODATA)
                      {
                        break;
                      }
                      continue;
                    }
                    pre_vwData = vwData;
                    mib_printf("\r\n ADC raw value = %d", (uint16_t)vwData);
                    vwData = (vwData * 3300) / 1023; // mV
                    mib_printf("\r\n ADC voltage = %d mV", (uint16_t)vwData);
                    vwData = (vwData * 100) / 3300; // %
                    mib_printf("\r\n ADC voltage = %d %%", (uint16_t)vwData);
                    if (vwData < 5)
                    {
                      vwData = 5;
                    }
                    else if (vwData > 95)
                    {
                      vwData = 95;
                    }
                    mib_printf("\r\n");
                  } while (1);
                }
                else if (strcmp(cmd, "key") == 0)
                {
                  mib_printf("\r\n %s : key press key \r\n", cmd);
                  pre_vwData = 0;
                  do
                  {
                    value_a = 3; // ADC channel 3 : PD2
                    vwData = getKeyAdc(value_a);
                    value_a = Get_Key_Number(vwData);
                    if (value_a > 0)
                    {
                      mib_printf("\r\n KEY : [%s]%d[%d]\r\n", key_info[value_a], value_a, (uint16_t)vwData);
                      delay_ms(100);
                    }
                    {
                      vwCh = MibReadDebugByte();
                      if (vwCh != MIB_DEBUG_READ_NODATA)
                      {
                        break;
                      }
                    }
                  } while (1);
                }
                else if ((strcmp(cmd, "help") == 0) || (strcmp(cmd, "?") == 0))
                {
                  adc_pwm_info();
                }
                else
                {
                  adc_pwm_info();
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
  // CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);
  // CLK_Configuration();
  {
    /* Clear High speed internal clock prescaler */
    CLK->CKDIVR &= (uint8_t)(~CLK_CKDIVR_HSIDIV);
    /* Set High speed internal clock prescaler */
    CLK->CKDIVR |= (uint8_t)CLK_PRESCALER_HSIDIV1;
  }
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
    STM8S103F3 MCU
    System Clock : 16MHz (HSI)
    UART1 : 115200, 8, N, 1
    TIM4 : 1msec interrupt
    TIM2 : PWM output : 1KHz, 50% duty cycle
    TIM2_CH1 : PD4

    STM8S Reference Manuals을 참고한다.

    stm8s 가 지니고 있는 PWM 기능에 대해서 공부한다.
    Channel 1 PWM 50% 2 KHz PD4 CN4.9
    Channel 2 PWM 25% 2 KHz PD3 CN4.8
    Channel 3 PWM 75% 2 KHz PA3 CN1.9
*/
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
#define _DEBUG_CMD_A_ 1
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
void adc_pwm_info(void)
{
  MibWriteDebugStringCheck(1, " #################################\r\n");
  MibWriteDebugStringCheck(1, " # <help> or <?>                 #\r\n");
  MibWriteDebugStringCheck(1, " # PD2, AIN3                     #\r\n");
  MibWriteDebugStringCheck(1, " # adc 3                         #\r\n");
  MibWriteDebugStringCheck(1, " # <adc> [channel number]        #\r\n");
  MibWriteDebugStringCheck(1, " # <auto>                        #\r\n");
  MibWriteDebugStringCheck(1, " # <key>                         #\r\n");
  MibWriteDebugStringCheck(1, " #################################\r\n");
  joystick_info();
}
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
/**
 * @brief ADC 채널 값을 읽어오는 함수
 * @param vbAdcChannel 읽고자 하는 ADC 채널 번호
 * @return 10비트 ADC 결과값 (0~1023)
 */
uint16_t getKeyAdc(uint8_t vbAdcChannel)
{
  uint16_t vsResult = 0;
  volatile uint8_t vbDelay;

  // 1. ADC 설정 및 채널 선택
  // CR2: 데이터 우측 정렬 (ALIGN = 1)
  ADC1->CR2 = 0x08;
  // CSR: 채널 선택 (CH[3:0])
  ADC1->CSR = (vbAdcChannel & 0x0F);

  // 2. ADC 전원 켜기 (ADON = 1)
  // STM8S는 ADON 비트를 처음 세팅하면 전원만 켜집니다.
  ADC1->CR1 |= 0x01;

  // 3. ADC 안정화 대기 (최소 7us)
  // 최적화에 의해 루프가 사라지지 않도록 vbDelay는 volatile 권장
  for (vbDelay = 50; vbDelay > 0; vbDelay--)
    ;

  // 4. 변환 시작 (ADON 비트를 한 번 더 1로 세팅)
  ADC1->CR1 |= 0x01;

  // 5. 변환 완료 대기 (EOC 비트 확인)
  // CSR의 7번 비트(0x80)가 1이 될 때까지 대기
  while (!(ADC1->CSR & 0x80))
    ;

  // 6. 결과 값 읽기 (우측 정렬 방식)
  // DRL을 먼저 읽고 DRH를 나중에 읽어 조합합니다.
  vsResult = ADC1->DRL;
  vsResult |= (uint16_t)(ADC1->DRH << 8);

  // 7. EOC 플래그 클리어 (다음 변환을 위해 필수)
  ADC1->CSR &= ~0x80;

  return vsResult;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
// 앞서 계산한 10-bit ADC 값 (0~1023) 기준 임계값 설정
// 실제 측정값의 중간 지점을 경계로 삼습니다.
#define KEY_R1 10000.
#define KEY_R2 4500.
#define KEY_R3 3900.
#define KEY_R4 6800.
#define KEY_R5 15000.

#define V_REF 3300.                                                                                      // mV
#define ADC_MAX 1023.                                                                                    // 10-bit ADC 최대값
#define KEY0_V V_REF                                                                                     // 키가 눌리지 않은 상태 (3.3V 풀업 상태)
#define KEY1_V V_REF *(KEY_R2 + KEY_R3 + KEY_R4 + KEY_R5) / (KEY_R1 + KEY_R2 + KEY_R3 + KEY_R4 + KEY_R5) // 약 1100 mV
#define KEY2_V V_REF *(KEY_R2 + KEY_R3 + KEY_R4) / (KEY_R1 + KEY_R2 + KEY_R3 + KEY_R4)                   // 약 700 mV
#define KEY3_V V_REF *(KEY_R2 + KEY_R3) / (KEY_R1 + KEY_R2 + KEY_R3)                                     // 약 500 mV
#define KEY4_V V_REF *(KEY_R2) / (KEY_R1 + KEY_R2)                                                       // 약 200 mV
#define KEY5_V 0

#define KEY0_MIN (KEY0_V * ADC_MAX / V_REF - 50) // 973 mV 기준 // NO KEY (3.3V 풀업 상태)
#define KEY1_MIN (KEY1_V * ADC_MAX / V_REF - 50) // 718 mV 기준 // RIGHT
#define KEY2_MIN (KEY2_V * ADC_MAX / V_REF - 50) // 567 mV 기준 // LEFT
#define KEY3_MIN (KEY3_V * ADC_MAX / V_REF - 50) // 417 mV 기준 // DOWN
#define KEY4_MIN (KEY4_V * ADC_MAX / V_REF - 50) // 267 mV 기준 // UP
#define KEY5_MIN 0 // ENETER 
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
uint8_t Get_Key_Number(uint16_t adc_val)
{
  if (adc_val >= KEY0_MIN)
    return 0;  // 눌린 키 없음 (3.3V 풀업 상태)
  else if (adc_val >= KEY1_MIN)
    return 1;
  else if (adc_val >= KEY2_MIN)
    return 2;
  else if (adc_val >= KEY3_MIN)
    return 3;
  else if (adc_val >= KEY4_MIN)
    return 4;

  return 5;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void joystick_info(void)
{
  mib_printf("\r\n KEY0_MIN = %d [NO KEY]", (uint16_t)KEY0_MIN);
  mib_printf("\r\n KEY1_MIN = %d [RIGHT]", (uint16_t)KEY1_MIN);
  mib_printf("\r\n KEY2_MIN = %d [LEFT]", (uint16_t)KEY2_MIN);
  mib_printf("\r\n KEY3_MIN = %d [DOWN]", (uint16_t)KEY3_MIN);
  mib_printf("\r\n KEY4_MIN = %d [UP]", (uint16_t)KEY4_MIN);
  mib_printf("\r\n KEY5_MIN = %d [ENTER]\r\n", (uint16_t)KEY5_MIN);
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
