/**
 ******************************************************************************
 * @file stm8s103_eeprom_main.c
 * @brief UART RX interrupt example.
 * @author yooaroma@gmail.com
 * @version V1.0.0
 * @date 2023.1.6
 ******************************************************************************
 */
/*

  info : STM8S103F3 사용시 EEPROM 및 옵션 바이트 읽기/쓰기는 아래 주소 범위 내에서만 가능하다.
        0x00 4000
        0x00 427F
        640 bytes data EEPROM
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
void eeprom_info(void);
void eeprom_read_id_info(void);
void eeprom_read_all(void);
void eeprom_read(uint16_t address, uint16_t length);
void eeprom_write(uint16_t address, uint8_t data_to_write);
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

  char line_buffer[31]; // 최대 30자 + NULL
  uint8_t index = 0;
  char c;

  // 파싱 결과 저장용 변수
  char cmd[10];
  char cmd_sub[10];
  uint16_t value_a;
  uint16_t value_b;
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
  eeprom_info();
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
              found = mib_sscanf(line_buffer, "%s %s %x %x", cmd, cmd_sub, &value_a, &value_b);
              // found = mib_sscanf(line_buffer, "%s %s", cmd, cmd_sub);
              if (found >= 2)
              {
                // 해석 성공 시 처리 (예: 명령어 확인)
                if (strcmp(cmd, "eeprom") == 0)
                {                
                  if (strcmp(cmd_sub, "rd") == 0)
                  {
                    mib_printf("\r\n eeprom read address [0x%x] length [%d] \r\n", value_a, value_b);
                    eeprom_read((uint16_t)value_a, (uint16_t)value_b);
                  }
                  else if (strcmp(cmd_sub, "rda") == 0)
                  {
                    eeprom_read_all();
                  } 
                  else if (strcmp(cmd_sub, "id") == 0)
                  {
                    eeprom_read_id_info();
                  }  
                  else if (strcmp(cmd_sub, "help") == 0)
                  {
                    eeprom_info();
                  }  
                  else if (strcmp(cmd_sub, "wr") == 0)
                  {
                    mib_printf("\r\n eeprom write address [0x%x] data [0x%x] \r\n", value_a, value_b);
                    eeprom_write((uint16_t)value_a, (uint8_t)value_b);
                  }
                }
                else
                {
                  mib_printf("\r\n Unknown command: %s\r\n", cmd);
                } 
              }
              else
              {
                mib_printf("\r\n Invalid command format(%d)\r\n",found);  
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
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
#define MCU_INFO_NAME "STM8S103F3"
void eeprom_info(void)
{
  mib_printf("\r\n ################################");
  mib_printf("\r\n # MCU info : %s",MCU_INFO_NAME);
  mib_printf("\r\n # flash addr base  : (0x%04X)  #", (uint16_t)FLASH_PROG_START_PHYSICAL_ADDRESS);
  mib_printf("\r\n # flash addr end   : (0x%04X)  #", (uint16_t)FLASH_PROG_END_PHYSICAL_ADDRESS);
  mib_printf("\r\n # option addr base : (0x%04X)  #", (uint16_t)OPTION_BYTE_START_PHYSICAL_ADDRESS);
  mib_printf("\r\n # option addr end  : (0x%04X)  #", (uint16_t)OPTION_BYTE_END_PHYSICAL_ADDRESS);
  mib_printf("\r\n # eeprom addr base : (0x%04X)  #", (uint16_t)FLASH_DATA_START_PHYSICAL_ADDRESS);
  mib_printf("\r\n # eeprom addr end  : (0x%04X)  #", (uint16_t)FLASH_DATA_END_PHYSICAL_ADDRESS);
  mib_printf("\r\n # eerprom size : 640           #");
  mib_printf("\r\n # eeprom help                  #");
  mib_printf("\r\n # eeprom rd (address) (size)   #");
  mib_printf("\r\n # eeprom rda                   #");
  mib_printf("\r\n # eeprom id                    #");
  mib_printf("\r\n # eeprom wr (address) (data)   #");
  mib_printf("\r\n ################################\r\n"); 
}
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
void eeprom_read_id_info(void)
{
  uint16_t vwAddress = 0;
  uint8_t vbIndex = 0;
  uint8_t vbBufferID[14]; // total 12byte...  
  vwAddress = 0x4865;
  mib_printf("\r\n  Table 15. Unique ID registers (96 bits)");
  for (vbIndex = 0; vbIndex < 12; vbIndex++)
  {
    vbBufferID[vbIndex] = *((uint8_t *)(vwAddress));
    vwAddress++;
  }
  vbBufferID[vbIndex] = 0; // null terminate
  mib_printf("\r\n X co-ordinate on the wafer [%02X][%02X]", (uint16_t)vbBufferID[0], (uint16_t)vbBufferID[1]);
  mib_printf("\r\n Y co-ordinate on the wafer [%02X][%02X]", (uint16_t)vbBufferID[2], (uint16_t)vbBufferID[3]);
  mib_printf("\r\n Wafer number               [%02X]", (uint16_t)vbBufferID[4]);
  mib_printf("\r\n Lot number [%s]\r\n", &(vbBufferID[5]));
}
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
void eeprom_read_all(void)
{
  uint16_t vwAddress = 0;
  uint8_t vbIndex = 0;
  uint8_t vbIndexA = 0;
  uint8_t vbData = 0;
  for (vbIndexA = 0; vbIndexA < (640 / 16); vbIndexA++)
  {
    mib_printf("\r\n  mmEepromRead(0x%04lx) : ", (long)vwAddress);
    for (vbIndex = 0; vbIndex < 16; vbIndex++)
    {
      vwAddress = vbIndexA * 16 + vbIndex;
      vbData = mmEepromRead(vwAddress);
      mib_printf(" %02lx", (long)vbData);
    }
  }
  mib_printf("\r\n");
}
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
void eeprom_read(uint16_t address, uint16_t length)
{
    uint16_t i;
    uint8_t vbData;

    for (i = 0; i < length; i++)
    {
        // 1. 16바이트마다 줄바꿈 및 현재 주소 출력
        if (i % 16 == 0)
        {
            mib_printf("\r\n 0x%04X : ", address + i);
        }

        // 2. 실제 데이터 읽기 (시작 주소 + 오프셋)
        vbData = mmEepromRead(address + i);

        // 3. 데이터 출력
        mib_printf("%02X ", (uint16_t)vbData); 
    }
    mib_printf("\r\n"); // 마지막 줄바꿈
}
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
void eeprom_write(uint16_t address, uint8_t data_to_write)
{
    uint8_t read_back_data = 0;

    // 1. 쓰기 전 현재 상태 확인 (선택 사항)
    read_back_data = mmEepromRead(address);
    mib_printf("\r\n [Before] Address: 0x%04X, Data: 0x%02X", address, (uint16_t)read_back_data);

    // 2. EEPROM 잠금 해제 및 데이터 쓰기
    mmEepromUnlock(); 
    mmEepromWrite(address, data_to_write);
    
    // 중요: 일부 MCU는 쓰기 완료까지 약간의 대기 시간이 필요할 수 있습니다.
    // 예: while(FLASH_GetFlagStatus(FLASH_FLAG_EOP) == RESET); 

    // 3. 쓰기 후 데이터 검증 (Verification)
    read_back_data = mmEepromRead(address);
    mib_printf("\r\n [After ] Address: 0x%04X, Data: 0x%02X", address, (uint16_t)read_back_data);

    // 4. 보안을 위해 다시 잠금 (지원하는 경우)
    mmEepromLock(); 
    mib_printf("\r\n"); // 마지막 줄바꿈
}
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
