/**
  ******************************************************************************
  * @file stm8s_mib_flash_option.c
  * @brief Functions to write and read data to FLASH inside the chip.
  * @author MYMEDIA Co., Ltd.
  * @version V1.0.0
  * @date 2023.1.6
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm8s.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
void mmFlashOptionLock(void);
uint8_t mmFlashOptionUnlock(void);
uint8_t mmFlashOptionWrite(uint16_t vbAddress,uint8_t vbData);
uint16_t mmFlashOptionRead(uint16_t vbAddress);
uint8_t mmFlashOptionErase(uint16_t vbAddress);
uint8_t FlashOptionWaitForLastOperation(uint8_t FLASH_MemType);
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------  
/*
  How to program STM8S and STM8A Flash program memory and data FLASH_OPTION (PM0051)
  <https://www.st.com/resource/en/programming_manual/pm0051-how-to-program-stm8s-and-stm8a-flash-program-memory-and-data-eeprom-stmicroelectronics.pdf>

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


*/

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------  
/**
  * @brief Data Flash unlocking routine
  * @par Parameters: None
  * @retval None
  */  
uint8_t mmFlashOptionUnlock(void) 
{
  volatile uint16_t vwDelay = 0x7fff;
	volatile uint8_t DataUnlocked = 0;
	
	/* Unlock Data memory Keys registers */		
	while( DataUnlocked == 0 )
	{
		FLASH->DUKR=0xAE;
		FLASH->DUKR=0x56;
		
		DataUnlocked = (uint8_t)FLASH->IAPSR;	
		DataUnlocked &= FLASH_IAPSR_DUL;
    if(vwDelay) vwDelay--;
    else 
    {
      return 0;
    }
	}
  return 1;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------  
/**
  * @brief Data Flash unlocking routine
  * @par Parameters: None
  * @retval None
  */  
void mmFlashOptionLock(void) 
{
  /* Lock memory */  
  FLASH->IAPSR &= (uint8_t)(~FLASH_IAPSR_DUL);  // FLASH_MEMTYPE_DATA      = (uint8_t)0xF7  /*!< Data EEPROM memory */ FLASH_IAPSR_DUL
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------- 
/**
  * @brief  Programs option byte
  * @param  Address : option byte address to program
  * @param  Data : Value to write
  * @retval None
  */
// void FLASH_ProgramOptionByte(uint16_t Address, uint8_t Data)
uint8_t mmFlashOptionWrite(uint16_t vbAddress,uint8_t vbData) 
{
  /* Check parameter */
  if(vbAddress > OPTION_BYTE_END_PHYSICAL_ADDRESS)
  {
    return 0; // false...
  }
  else if(vbAddress < OPTION_BYTE_START_PHYSICAL_ADDRESS)
  {
    return 0; // false...
  }

  if((FLASH->IAPSR & FLASH_IAPSR_DUL)==0)
  {
    return 0; // false...
  }

  /* Enable write access to option bytes */
  FLASH->CR2 |= FLASH_CR2_OPT;
  FLASH->NCR2 &= (uint8_t)(~FLASH_NCR2_NOPT);

  /* check if the option byte to program is ROP*/
  if(vbAddress == 0x4800)
  {
    /* Program option byte*/
    *((NEAR uint8_t*)vbAddress) = vbData;
  }
  else
  {
    /* Program option byte and his complement */
    *((NEAR uint8_t*)vbAddress) = vbData;
    *((NEAR uint8_t*)((uint16_t)(vbAddress + 1))) = (uint8_t)(~vbData);
  }
  // FLASH_WaitForLastOperation(FLASH_MEMTYPE_PROG);
  {
    uint8_t vbRet = 0;
    vbRet = FlashOptionWaitForLastOperation(FLASH_MEMTYPE_PROG);
    if((vbRet==0) || (vbRet==2))
    {
      return 0; // false...
    }
  }

  /* Disable write access to option bytes */
  FLASH->CR2 &= (uint8_t)(~FLASH_CR2_OPT);
  FLASH->NCR2 |= FLASH_NCR2_NOPT;
  return 1; // true...
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------- 

/**
  * @brief  Reads one option byte
  * @param  Address  option byte address to read.
  * @retval Option byte read value + its complement
  */
// uint16_t FLASH_ReadOptionByte(uint16_t Address)
uint16_t mmFlashOptionRead(uint16_t vbAddress)
{
  uint8_t value_optbyte, value_optbyte_complement = 0;
  uint16_t res_value = 0;

  /* Check parameter */
  if(vbAddress > OPTION_BYTE_END_PHYSICAL_ADDRESS)
  {
    return 0; // false...
  }
  else if(vbAddress < OPTION_BYTE_START_PHYSICAL_ADDRESS)
  {
    return 0; // false...
  }

  value_optbyte = *((NEAR uint8_t*)vbAddress); /* Read option byte */
  value_optbyte_complement = *(((NEAR uint8_t*)vbAddress) + 1); /* Read option byte complement */

  /* Read-out protection option byte */
  if(vbAddress == 0x4800)
  {
    res_value =	 value_optbyte;
  }
  else
  {
    if(value_optbyte == (uint8_t)(~value_optbyte_complement))
    {
      res_value = (uint16_t)((uint16_t)value_optbyte << 8);
      res_value = res_value | (uint16_t)value_optbyte_complement;
    }
    else
    {
      res_value = FLASH_OPTIONBYTE_ERROR;
    }
  }
  return(res_value);
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------- 
/**
  * @brief  Erases option byte
  * @param  Address : Option byte address to erase
  * @retval None
  */
// void FLASH_EraseOptionByte(uint16_t Address)
uint8_t mmFlashOptionErase(uint16_t vbAddress)
{
  /* Check parameter */
  if(vbAddress > OPTION_BYTE_END_PHYSICAL_ADDRESS)
  {
    return 0; // false...
  }
  else if(vbAddress < OPTION_BYTE_START_PHYSICAL_ADDRESS)
  {
    return 0; // false...
  }

  /* Enable write access to option bytes */
  FLASH->CR2 |= FLASH_CR2_OPT;
  FLASH->NCR2 &= (uint8_t)(~FLASH_NCR2_NOPT);

  /* check if the option byte to erase is ROP */
  if(vbAddress == 0x4800)
  {
    /* Erase option byte */
    *((NEAR uint8_t*)vbAddress) = 0x00; // FLASH_CLEAR_BYTE;
  }
  else
  {
    /* Erase option byte and his complement */
    *((NEAR uint8_t*)vbAddress) = 0x00; // FLASH_CLEAR_BYTE;
    *((NEAR uint8_t*)((uint16_t)(vbAddress + (uint16_t)1 ))) = 0xff; // FLASH_SET_BYTE;
  }
  // FLASH_WaitForLastOperation(FLASH_MEMTYPE_PROG);
  {
    uint8_t vbRet = 0;
    vbRet = FlashOptionWaitForLastOperation(FLASH_MEMTYPE_PROG);
    if((vbRet==0) || (vbRet==2))
    {
      return 0; // false...
    }
  }

  /* Disable write access to option bytes */
  FLASH->CR2 &= (uint8_t)(~FLASH_CR2_OPT);
  FLASH->NCR2 |= FLASH_NCR2_NOPT;
  return 1; // true...
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------- 
/**
  * @brief  Wait for a Flash operation to complete.
  * @note   The call and execution of this function must be done from RAM in case
  *         of Block operation.
  * @param  FLASH_MemType : Memory type
  *         This parameter can be a value of @ref FLASH_MemType_TypeDef
  * @retval FLASH status
  */
uint8_t FlashOptionWaitForLastOperation(uint8_t FLASH_MemType)
{
  uint8_t flagstatus = 0x00;
  uint16_t timeout = 0xffff; // OPERATION_TIMEOUT;

  /* Wait until operation completion or write protection page occurred */
#if defined (STM8S208) || defined(STM8S207) || defined(STM8S007) || defined(STM8S105) || \
  defined(STM8S005) || defined(STM8AF52Ax) || defined(STM8AF62Ax) || defined(STM8AF626x)
    if(FLASH_MemType == FLASH_MEMTYPE_PROG)
    {
      while((flagstatus == 0x00) && (timeout != 0x00))
      {
        flagstatus = (uint8_t)(FLASH->IAPSR & (uint8_t)(FLASH_IAPSR_EOP |
                                                        FLASH_IAPSR_WR_PG_DIS));
        timeout--;
      }
    }
    else
    {
      while((flagstatus == 0x00) && (timeout != 0x00))
      {
        flagstatus = (uint8_t)(FLASH->IAPSR & (uint8_t)(FLASH_IAPSR_HVOFF |
                                                        FLASH_IAPSR_WR_PG_DIS));
        timeout--;
      }
    }
#else /*STM8S103, STM8S903, STM8AF622x */
  (void)FLASH_MemType; // Eliminate compiler warning about unused variable
  while((flagstatus == 0x00) && (timeout != 0x00))
  {
    flagstatus = (uint8_t)(FLASH->IAPSR & (FLASH_IAPSR_EOP | FLASH_IAPSR_WR_PG_DIS));
    timeout--;
  }
#endif /* STM8S208, STM8S207, STM8S105, STM8AF52Ax, STM8AF62Ax, STM8AF262x */

  if(timeout == 0x00 )
  {
    flagstatus = FLASH_STATUS_TIMEOUT;
  }

  return((uint8_t)flagstatus);
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------- 