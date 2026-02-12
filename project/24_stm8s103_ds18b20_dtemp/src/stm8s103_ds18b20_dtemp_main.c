/**
 ******************************************************************************
 * @file stm8s103_ds18b20_dtemp_main.c
 * @brief UART RX interrupt example.
 * @author yooaroma@gmail.com
 * @version V1.0.0
 * @date 2023.1.6
 ******************************************************************************
 */
/*
  info :
    STM8S103에서 DS18B20을 제어하기 위해서는 1-Wire 프로토콜을 구현해야 합니다. 
    이 프로토콜은 매우 정밀한 마이크로초($\mu s$) 단위의 타이밍이 핵심입니다.

    1. 하드웨어 연결 및 주의사항
    VCC: 3.3V ~ 5V
    GND: Ground
    DQ (Data): PD3 (예시)
    필수 사항: DQ 라인과 VCC 사이에 4.7k$\Omega$ 풀업 저항을 반드시 연결해야 합니다. 
    이 저항이 없으면 통신이 절대 되지 않습니다.
    
    2. DS18B20 제어 코드 (C언어)
    이 코드는 STM8S103이 16MHz 클럭으로 동작한다고 가정합니다.
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

int16_t DS18B20_ReadTemperature(void);
void ds18b20_info(void);
void ds18b20_read(void);
void ds18b20_read_auto(uint16_t interval_ms);


// extern delay_ms(uint16_t vwDelay);
// extern tim4IrqInit(void);
// // void tim4IrqMain(void);
// extern MibGetSecs(void);
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
    // mib_printf(" file name : ( %s )\r\n", __FILE__); 
    // __FILE__ 매크로는 전체 경로를 포함하기 때문에 get_filename_manual() 함수를 사용하여 파일 이름만 추출
    mib_printf(" file name : ( %s )\r\n", get_filename_manual(__FILE__)); 
    mib_printf(" file line : ( %d )\r\n", __LINE__);
    MibWriteDebugStringCheck(1, " date :  " __DATE__ "  :  " __TIME__ "\r\n");
    MibWriteDebugStringCheck(1, " yooaroma.com by MYMEDIA Co., Ltd.\r\n");
    MibWriteDebugStringCheck(1, " ###################################\r\n");
  }
  {
    mib_printf("\r\n STM8S103 I2C ADXL345 Test Program Start...\r\n");
    // I2C_Config();
  }
  {
    ds18b20_info();
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
                if (strcmp(cmd, "ds18b20") == 0)
                {
                  if (strcmp(cmd_a, "rd") == 0)
                  {
                    ds18b20_read();
                  }
                  else if (strcmp(cmd_a, "auto") == 0)
                  {
                    mib_printf(" DS18B20 Test : msec = [%d]\r\n", (uint16_t)value_a);
                    ds18b20_read_auto(value_a);
                  }
                  else
                  {
                    MibWriteDebugStringCheck(1, " Unknown ds18b20 command\r\n");
                  }
                  // mib_printf("\r\n %s %s %s",cmd, cmd_a, cmd_b);
                }
                else if ((strcmp(cmd, "help") == 0) || (strcmp(cmd, "?") == 0))
                {
                  ds18b20_info();
                }
                else
                {
                  ds18b20_info();
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
void ds18b20_info(void)
{
  MibWriteDebugStringCheck(1, " ###########################\r\n");
  MibWriteDebugStringCheck(1, " # <help> or <?>           #\r\n");
  MibWriteDebugStringCheck(1, " # <ds18b20> <rd>          #\r\n");
  MibWriteDebugStringCheck(1, " # <ds18b20> <auto> [msec] #\r\n");
  MibWriteDebugStringCheck(1, " ###########################\r\n");
}
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
void ds18b20_read(void)
{
  uint32_t dtemp_32;
  int16_t dtemp;
  dtemp = DS18B20_ReadTemperature();
  // mib_printf(" DS18B20 Test : Temperature=%d\r\n", dtemp);
  if(dtemp >= 0)
  {
    dtemp_32 = (uint32_t)dtemp;
  }
  else
  {
    dtemp_32 = (uint32_t)(-dtemp);
  }
  dtemp_32 = dtemp_32 * 625; // 0.0625도 단위를 마이크로도로 변환
  if(dtemp < 0)
  {
    mib_printf(" DS18B20  : Temperature[%d]=-%ld.%04ld\r\n",dtemp, dtemp_32 / 10000, dtemp_32 % 10000);
  } 
  else
  {
    mib_printf(" DS18B20  : Temperature[%d]=%ld.%04ld\r\n",dtemp, dtemp_32 / 10000, dtemp_32 % 10000);
    // mib_printf(" DS18B20  : Temperature=%ld\r\n", dtemp_32);
    // mib_printf(" DS18B20  : Temperature=%ld\r\n", dtemp_32/10000);
  } 
}
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
void ds18b20_read_auto(uint16_t interval_ms)
{
  int16_t dtemp;
  uint16_t vwCh = 0;
  do
  {
    ds18b20_read();
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
// float DS18B20_ReadTemperature(void) // 함수는 DS18B20 센서에서 온도를 읽어오는 함수입니다.
int16_t DS18B20_ReadTemperature(void) 
{
  volatile uint32_t i;
  uint8_t low, high;
  int16_t raw;

  if (!DS18B20_Reset()) return -999.0; // 센서가 없으면 에러값 반환

  DS18B20_WriteByte(0xCC); // Skip ROM (센서 1개일 때 사용)
  DS18B20_WriteByte(0x44); // Convert T (온도 변환 명령)

  // 변환 완료까지 대기 (최대 750ms 소요)
  for(i = 0; i < 150000; i++); 

  DS18B20_Reset();
  DS18B20_WriteByte(0xCC); // Skip ROM
  DS18B20_WriteByte(0xBE); // Read Scratchpad (메모리 읽기)

  low = DS18B20_ReadByte();  // LSB (하위 8비트)
  high = DS18B20_ReadByte(); // MSB (상위 8비트)

  raw = (high << 8) | low;
  
  // DS18B20은 1단위가 0.0625도입니다.
  // return (float)raw * 0.0625f;
  return raw;
}
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
