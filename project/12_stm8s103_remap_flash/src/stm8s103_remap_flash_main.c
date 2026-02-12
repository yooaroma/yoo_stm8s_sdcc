/**
 ******************************************************************************
 * @file stm8s103_remap_flash_main.c
 * @brief UART RX interrupt example.
 * @author yooaroma@gmail.com
 * @version V1.0.0
 * @date 2023.1.6
 ******************************************************************************
 */
#if 0
  stm8s_conf.h 파일에서 아래와 같이 정의해야 한다.
  #if defined(_stm8s103_remap_flash_)
  #define UART_IRQ_RX_USE 1
  #define USE_TIM4_INTERRUPT_HANDLER_CB 1
  #endif

  info :
    How to program STM8S and STM8A Flash program memory and data EEPROM (PM0051)
    STM8S Reference Manuals : <https://www.st.com/resource/en/reference_manual/rm0016-stm8s-series-and-stm8af-series-8bit-microcontrollers-stmicroelectronics.pdf>
    page 34, 참고한다.
    stm8s 가 지니고 있는 flash에 데이터를 써넣기 방법에 대한 예제이다.

    STM8S microcontrollers offer low density (8 Kbytes), medium density (from 16 to 32 Kbytes)
    and high density (from 32 to 128 Kbytes) Flash program memory, plus data EEPROM.
    STM8A microcontrollers feature medium density (from 8 to 32 Kbytes) and high density
    (from 32 to 128 Kbytes) Flash memory, plus data EEPROM.

                    BLOCK      -> PAGE
      8K  (low)     64 bytes      0 (1 page = 1 block) to n
      32K (medium)  128 bytes     0 (1 page = 4 block) to n
      128K(high)    128 bytes     0 (1 page = 4 block) to n

    Data EEPROM and option bytes
        Unlock : Write 0xAE then 56h in FLASH_DUKR (0x00 5064)(1)(2)
        Lock   : Reset bit 3 (DUL) in FLASH_IAPSR (0x00 505F)
    Program memory
        Unlock : Write 0x56 then 0xAE in FLASH_PUKR (0x00 5062)(3)
        Lock   : Reset bit 1 (PUL) in FLASH_IAPSR (0x00 505F)

      1. The OPT and NOPT bits of FLASH_CR2 and FLASH_NCR2 registers must be set/cleared to enable access to the option bytes.
      2. If wrong keys have been entered, another key programming sequence can be issued without resetting the device.
      3. If wrong keys have been entered, the device must be reset, and a key program sequence issued.


#define FLASH_PROG_START_PHYSICAL_ADDRESS ((uint32_t)0x008000) /*!< Program memory: start address */

#if defined (STM8S208) || defined(STM8S207) || defined(STM8S007) || defined (STM8AF52Ax) || defined (STM8AF62Ax)
 #define FLASH_PROG_END_PHYSICAL_ADDRESS   ((uint32_t)0x027FFF) /*!< Program memory: end address */
 #define FLASH_PROG_BLOCKS_NUMBER          ((uint16_t)1024)     /*!< Program memory: total number of blocks */
 #define FLASH_DATA_START_PHYSICAL_ADDRESS ((uint32_t)0x004000) /*!< Data EEPROM memory: start address */
 #define FLASH_DATA_END_PHYSICAL_ADDRESS   ((uint32_t)0x0047FF) /*!< Data EEPROM memory: end address */
 #define FLASH_DATA_BLOCKS_NUMBER          ((uint16_t)16)       /*!< Data EEPROM memory: total number of blocks */
 #define FLASH_BLOCK_SIZE                  ((uint8_t)128)       /*!< Number of bytes in a block (common for Program and Data memories) */
#endif /* STM8S208, STM8S207, STM8S007, STM8AF52Ax, STM8AF62Ax */

#if defined(STM8S105) || defined(STM8S005) || defined(STM8AF626x)
 #define FLASH_PROG_END_PHYSICAL_ADDRESS   ((uint32_t)0xFFFF)   /*!< Program memory: end address */
 #define FLASH_PROG_BLOCKS_NUMBER          ((uint16_t)256)      /*!< Program memory: total number of blocks */
 #define FLASH_DATA_START_PHYSICAL_ADDRESS ((uint32_t)0x004000) /*!< Data EEPROM memory: start address */
 #define FLASH_DATA_END_PHYSICAL_ADDRESS   ((uint32_t)0x0043FF) /*!< Data EEPROM memory: end address */
 #define FLASH_DATA_BLOCKS_NUMBER          ((uint16_t)8)        /*!< Data EEPROM memory: total number of blocks */
 #define FLASH_BLOCK_SIZE                  ((uint8_t)128)       /*!< Number of bytes in a block (common for Program and Data memories) */
#endif /* STM8S105 or STM8AF626x */

#if defined(STM8S103) || defined(STM8S003) || defined(STM8S001) || defined(STM8S903) || defined(STM8AF622x)
 #define FLASH_PROG_END_PHYSICAL_ADDRESS   ((uint32_t)0x9FFF)   /*!< Program memory: end address */
 #define FLASH_PROG_BLOCKS_NUMBER          ((uint16_t)128)      /*!< Program memory: total number of blocks */
 #define FLASH_DATA_START_PHYSICAL_ADDRESS ((uint32_t)0x004000) /*!< Data EEPROM memory: start address */
 #define FLASH_DATA_END_PHYSICAL_ADDRESS   ((uint32_t)0x00427F) /*!< Data EEPROM memory: end address */
 #define FLASH_DATA_BLOCKS_NUMBER          ((uint16_t)10)       /*!< Data EEPROM memory: total number of blocks */
 #define FLASH_BLOCK_SIZE                  ((uint8_t)64)        /*!< Number of bytes in a block (common for Program and Data memories) */
#endif /* STM8S103 or STM8S003 or STM8S001 or STM8S903 or STM8AF622x*/

#define FLASH_RASS_KEY1 ((uint8_t)0x56) /*!< First RASS key */
#define FLASH_RASS_KEY2 ((uint8_t)0xAE) /*!< Second RASS key */

#define OPTION_BYTE_START_PHYSICAL_ADDRESS  ((uint16_t)0x4800)
#define OPTION_BYTE_END_PHYSICAL_ADDRESS    ((uint16_t)0x487F)
#define FLASH_OPTIONBYTE_ERROR              ((uint16_t)0x5555) /*!< Error code option byte  (if value read is not equal to complement value read) */

  STM8S103F3 칩이 제공한 플래시 영역 : 008000 에서 009fff까지에 데이터를 쓰는 것에 대한 방법이다.
  블락으로 써 넣기 위해서는 램에서 동작하여 저장하여야 하나
  1byte를 써 넣는 것은 가능한 것으로 보인다.

  flash help
  flash wr 9ff0 aa
  flash rd 9ff0 10

#endif 

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

void flash_info(void);
void flash_read_id_info(void);
void flash_read(uint16_t address, uint16_t length);
void flash_write(uint16_t address, uint8_t data_to_write);
void flash_read_option_all(void);
void flash_write_option_opt2(uint8_t mode); // mode 0: reset, 1: set
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
  flash_info();
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
                if (strcmp(cmd, "flash") == 0)
                {                
                  if (strcmp(cmd_sub, "rd") == 0)
                  {
                    mib_printf("\r\n flash read address [0x%x] length [%d] \r\n", value_a, value_b);
                    flash_read((uint16_t)value_a, (uint16_t)value_b);
                  }
                  else if (strcmp(cmd_sub, "id") == 0)
                  {
                    flash_read_id_info();
                  }  
                  else if (strcmp(cmd_sub, "help") == 0)
                  {
                    flash_info();
                  }  
                  else if (strcmp(cmd_sub, "wr") == 0)
                  {
                    mib_printf("\r\n flash write address [0x%x] data [0x%x] \r\n", value_a, value_b);
                    flash_write((uint16_t)value_a, (uint8_t)value_b);
                  }         
                  else if (strcmp(cmd_sub, "oprd") == 0)
                  {
                    mib_printf("\r\n flash option read all \r\n");
                    flash_read_option_all();  
                  }
                  else if (strcmp(cmd_sub, "opwr") == 0)
                  {
                    mib_printf("\r\n flash option write address 0x4803 set mode [%d] \r\n", value_a);
                    flash_write_option_opt2(value_a);
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
void flash_info(void)
{
  mib_printf("\r\n ################################");
  mib_printf("\r\n # MCU info : %s",MCU_INFO_NAME);
  mib_printf("\r\n # flash addr base  : (0x%04X)  #", (uint16_t)FLASH_PROG_START_PHYSICAL_ADDRESS);
  mib_printf("\r\n # flash addr end   : (0x%04X)  #", (uint16_t)FLASH_PROG_END_PHYSICAL_ADDRESS);
  mib_printf("\r\n # eeprom addr base : (0x%04X)  #", (uint16_t)FLASH_DATA_START_PHYSICAL_ADDRESS);
  mib_printf("\r\n # eeprom addr end  : (0x%04X)  #", (uint16_t)FLASH_DATA_END_PHYSICAL_ADDRESS);
  mib_printf("\r\n # option addr base : (0x%04X)  #", (uint16_t)OPTION_BYTE_START_PHYSICAL_ADDRESS);
  mib_printf("\r\n # option addr end  : (0x%04X)  #", (uint16_t)OPTION_BYTE_END_PHYSICAL_ADDRESS);
  mib_printf("\r\n # flash help                   #");
  mib_printf("\r\n # flash rd (address) (size)    #");
  mib_printf("\r\n # flash id                     #");
  mib_printf("\r\n # flash wr (address) (data)    #");
  mib_printf("\r\n # flash oprd                   #");
  mib_printf("\r\n # flash opwr  [mode]           #");
  mib_printf("\r\n # OPT2 : AFR1 , TIM2_CH3       #");
  mib_printf("\r\n ################################\r\n"); 
}
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
void flash_read_id_info(void)
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
void flash_read(uint16_t address, uint16_t length)
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
        vbData = mmFlashRead(address + i);

        // 3. 데이터 출력
        mib_printf("%02X ", (uint16_t)vbData); 
    }
    mib_printf("\r\n"); // 마지막 줄바꿈
}
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
void flash_write(uint16_t address, uint8_t data_to_write)
{
    uint8_t read_back_data = 0;

    // 1. 쓰기 전 현재 상태 확인 (선택 사항)
    read_back_data = mmFlashRead(address);
    mib_printf("\r\n [Before] Address: 0x%04X, Data: 0x%02X", address, (uint16_t)read_back_data);

    // 2. Flash 잠금 해제 및 데이터 쓰기
    mmFlashUnlock(); 
    mmFlashWrite(address, data_to_write);
    
    // 중요: 일부 MCU는 쓰기 완료까지 약간의 대기 시간이 필요할 수 있습니다.
    // 예: while(FLASH_GetFlagStatus(FLASH_FLAG_EOP) == RESET); 

    // 3. 쓰기 후 데이터 검증 (Verification)
    read_back_data = mmFlashRead(address);
    mib_printf("\r\n [After ] Address: 0x%04X, Data: 0x%02X", address, (uint16_t)read_back_data);

    // 4. 보안을 위해 다시 잠금 (지원하는 경우)
    mmFlashLock(); 
    mib_printf("\r\n"); // 마지막 줄바꿈
}
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
void flash_read_option_all(void)
{
    uint16_t vwAddress = 0;
    uint16_t vwData = 0;
    uint8_t vbIndex = 0;
    {
      vwAddress = 0x4800;
      mib_printf("\r\n  Table 11. Option byte");
      vwData = mmFlashOptionRead(vwAddress);
      mib_printf("\r\n  mmFlashOptionRead(0x%lx)=[0x%04lx]", (long)vwAddress, (long)vwData);
      vwAddress++;
      for (vbIndex = 0; vbIndex < 5; vbIndex++)
      {
        vwData = mmFlashOptionRead(vwAddress);
        mib_printf("\r\n  mmFlashOptionRead(0x%lx)=[0x%04lx]", (long)vwAddress, (long)vwData);
        vwAddress += 2;
      }
    }
    mib_printf("\r\n"); // 마지막 줄바꿈
}
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
void flash_write_option_opt2(uint8_t mode) // mode 0: reset, 1: set
{
    uint16_t vwAddress = 0;
    uint16_t vwData = 0;
    vwAddress = 0x4803;
    mib_printf("\r\n  Table 11. Option byte : OPT2 : AFR1 , TIM2_CH3");
    {
      vwData = mmFlashOptionRead(vwAddress);
      mib_printf("\r\n 1. mmFlashOptionRead(0x%lx)=[0x%04lx]", (long)vwAddress, (long)vwData);
    }
    {
      mmFlashOptionUnlock();               // unlock option byte
      if(mode == 0)
      {
        mmFlashOptionWrite(vwAddress, 0x00); // AFR1 : TIM2_CH3, reset remap
      }
      else
      {
        mmFlashOptionWrite(vwAddress, 0x02); // AFR1 : TIM2_CH3, set remap
      }
      vwData = mmFlashOptionRead(vwAddress);
      mib_printf("\r\n 2. mmFlashOptionRead(0x%lx)=[0x%04lx]", (long)vwAddress, (long)vwData);
    }
    mib_printf("\r\n"); // 마지막 줄바꿈
}
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
