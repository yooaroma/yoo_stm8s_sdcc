/**
 ******************************************************************************
 * @file stm8s103_i2c_adxl345_main.c
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
    STM8S103 MCU에서 I2C를 이용해 PCF8591(ADC/DAC 컨버터)을 제어하는 프로그램이다.
    PCF8591은 8비트 해상도를 가지며, 4개의 아날로그 입력(ADC)과 1개의 아날로그 출력(DAC)을 제공합니다.
    이 칩의 기본 I2C 주소는 보통 0x90(Write) / 0x91(Read)입니다.

    1. 하드웨어 연결 (STM8S103 기준)
      VCC: 3.3V 또는 5V
      GND: Ground
      SCL: PB4 (STM8S103 I2C SCL)
      SDA: PB5 (STM8S103 I2C SDA)
      주의: SCL과 SDA 라인에는 반드시 4.7kΩ 정도의 풀업 저항이 연결되어 있어야 합니다.
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

void i2c_adxl345_info(void);
void adxl345_read(void);
void adxl345_read_auto(uint16_t interval_ms);

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
  uint8_t reg_value;
  uint32_t vwData = 0;
  uint32_t pre_vwData = 0;
  uint32_t delta_data = 0;

  char line_buffer[31]; // 최대 30자 + NULL
  uint8_t index = 0;
  char c;

  // 파싱 결과 저장용 변수
  char cmd[10];
  char cmd_a[10];
  char cmd_b[17];
  int value_a;
  int value_b;
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
    MibWriteDebugStringCheck(1, "\r\n ###################################\r\n");
    // MibWriteDebugStringCheck(1," file name : "__FILE__"\r\n");
    mib_printf(" file name : ( %s )\r\n", __FILE__);
    mib_printf(" file line : ( %d )\r\n", __LINE__);
    MibWriteDebugStringCheck(1, " date :  " __DATE__ "  :  " __TIME__ "\r\n");
    MibWriteDebugStringCheck(1, " yooaroma.com by MYMEDIA Co., Ltd.\r\n");
    MibWriteDebugStringCheck(1, " ###################################\r\n");
  }
  {
    mib_printf("\r\n STM8S103 I2C ADXL345 Test Program Start...\r\n");
    I2C_Config();
    ADXL345_Init();
  }
  {
    i2c_adxl345_info();
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
              found = mib_sscanf(line_buffer, "%s %s %d", cmd, cmd_a, &value_a);

              if (found >= 1)
              {
                // 해석 성공 시 처리 (예: 명령어 확인)
                if (strcmp(cmd, "adxl") == 0)
                {
                  if (strcmp(cmd_a, "rd") == 0)
                  {
                    adxl345_read();
                  }
                  else if (strcmp(cmd_a, "reg") == 0)
                  {
                    reg_value = ADXL345_ReadReg((uint8_t)value_a);
                    mib_printf(" ADXL345 Reg[0x%02X] = 0x%02X\r\n", (uint16_t)value_a, (uint16_t)reg_value);
                  }
                  else if (strcmp(cmd_a, "auto") == 0)
                  {
                    adxl345_read_auto(value_a);
                  }
                  else
                  {
                    MibWriteDebugStringCheck(1, " Unknown adxl command\r\n");
                  }
                  // mib_printf("\r\n %s %s %s",cmd, cmd_a, cmd_b);
                }
                else if ((strcmp(cmd, "help") == 0) || (strcmp(cmd, "?") == 0))
                {
                  i2c_adxl345_info();
                }
                else
                {
                  i2c_adxl345_info();
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
void i2c_adxl345_info(void)
{
  MibWriteDebugStringCheck(1, " ##########################\r\n");
  MibWriteDebugStringCheck(1, " # <help> or <?>          #\r\n");
  MibWriteDebugStringCheck(1, " # <adxl> <rd>            #\r\n");
  MibWriteDebugStringCheck(1, " # <adxl> <reg> [reg]     #\r\n");
  MibWriteDebugStringCheck(1, " # <adxl> <auto> [msec]   #\r\n");
  MibWriteDebugStringCheck(1, " ##########################\r\n");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
void adxl345_read(void)
{
  volatile uint32_t i;
  int16_t ax, ay, az;
  ADXL345_GetXYZ(&ax, &ay, &az);
  // 여기에 ax, ay, az 값을 LCD에 표시하거나
  // 특정 임계값 이상일 때 LED를 켜는 로직을 추가하세요.
  mib_printf(" ADXL345 Test : X=%d, Y=%d, Z=%d\r\n", ax, ay, az);
  // for ( = 0; i < 50000; i++)
  //   ; // 샘플링 간격 조절
}
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
void adxl345_read_auto(uint16_t interval_ms)
{
  int16_t ax, ay, az;
  uint16_t vwCh = 0;
  do
  {
    ADXL345_GetXYZ(&ax, &ay, &az);
    // 여기에 ax, ay, az 값을 LCD에 표시하거나
    // 특정 임계값 이상일 때 LED를 켜는 로직을 추가하세요.
    mib_printf(" ADXL345 Test : X=%d, Y=%d, Z=%d\r\n", ax, ay, az);
    delay_ms(interval_ms); // 샘플링 간격 조절
    vwCh = MibReadDebugByte();
    if (vwCh != MIB_DEBUG_READ_NODATA)
    {
      break;
    }
  } while (1);
}
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
