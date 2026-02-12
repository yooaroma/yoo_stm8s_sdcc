/**
 ******************************************************************************
 * @file stm8s103_tim2_pwm_main.c
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

void tim2_pwm_info(void);
void tim2_pwm_test(uint16_t vwPeriod,uint16_t vwDuty);

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
void TIM2_Cmd(FunctionalState NewState);
void TIM2_ARRPreloadConfig(FunctionalState NewState);
void TIM2_OC1PreloadConfig(FunctionalState NewState);
void TIM2_OC2PreloadConfig(FunctionalState NewState);
void TIM2_OC3PreloadConfig(FunctionalState NewState);
void TIM2_OC1Init(TIM2_OCMode_TypeDef TIM2_OCMode,
                  TIM2_OutputState_TypeDef TIM2_OutputState,
                  uint16_t TIM2_Pulse,
                  TIM2_OCPolarity_TypeDef TIM2_OCPolarity);
void TIM2_OC2Init(TIM2_OCMode_TypeDef TIM2_OCMode,
                  TIM2_OutputState_TypeDef TIM2_OutputState,
                  uint16_t TIM2_Pulse,
                  TIM2_OCPolarity_TypeDef TIM2_OCPolarity);
void TIM2_OC3Init(TIM2_OCMode_TypeDef TIM2_OCMode,
                  TIM2_OutputState_TypeDef TIM2_OutputState,
                  uint16_t TIM2_Pulse,
                  TIM2_OCPolarity_TypeDef TIM2_OCPolarity);
void TIM2_TimeBaseInit( TIM2_Prescaler_TypeDef TIM2_Prescaler,
                        uint16_t TIM2_Period);
void TIM2_DeInit(void);
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
    tim2_pwm_info();
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
          tim2_pwm_test(); 
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
              found = mib_sscanf(line_buffer, "%s %d %d", cmd, &value_a, &value_b);

              if (found >= 2)
              {
                // 해석 성공 시 처리 (예: 명령어 확인)
                if (strcmp(cmd, "pwm") == 0)
                {
                  mib_printf("\r\n %s [%d]usec period [%d]%% duty",cmd, value_a, value_b);
                  tim2_pwm_test(value_a, value_b); 
                }
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
void tim2_pwm_info(void)
{
  MibWriteDebugStringCheck(1, " #################################\r\n");
  MibWriteDebugStringCheck(1, " # PWM : Pulse Width Modulation  #\r\n");
  MibWriteDebugStringCheck(1, " # pwm [period] [duty] (decimal) #\r\n");
  MibWriteDebugStringCheck(1, " # PD4, usec, percent            #\r\n");
  MibWriteDebugStringCheck(1, " # pwm 1000 50  (1msec) (50%%)   #\r\n");
  MibWriteDebugStringCheck(1, " #################################\r\n");
}
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
void tim2_pwm_test(uint16_t vwPeriod,uint16_t vwDuty) 
{
  // TIM2_PRESCALER_1
  // uint16_t vwPeriod = 1000;                         // 1 msec
  uint16_t vwCh1Ratio = (((16 * vwPeriod) / 100) * vwDuty);      // duty % of period
  // uint16_t vwCh1Ratio = ((16 * vwPeriod) / 2);      // 50% of period
  uint16_t vwCh2Ratio = ((16 * vwPeriod) / 5);      // 20% of period
  uint16_t vwCh3Ratio = ((16 * vwPeriod) / 10 * 7); // 70% of period

  mib_printf("\r\n  pwm start Period = %d usec", vwPeriod);
  mib_printf("\r\n  Ch1 PD4 = %d%% duty ", vwDuty);
  // mib_printf("\r\n  Ch1 PD4 = 50%% of period ");
  mib_printf("\r\n  Ch2 PD3 = 20%% of period ");
  mib_printf("\r\n  Ch3 PA3 = 70%% of period ");
  /* TIM2 Peripheral Configuration */
  TIM2_DeInit();

  /* Set TIM2 Frequency to 2Mhz */
  /*
    16MHz / 1 = 16MHz
    1/16 * 1000 = 62.5uSec (주기)
    period_usec = (1/16) * value
    value = 16 * period_usec
  */
  TIM2_TimeBaseInit(TIM2_PRESCALER_1, (16 * vwPeriod) / 1 - 1);

  /* Channel 1 PWM configuration */
  TIM2_OC1Init(TIM2_OCMODE_PWM2, TIM2_OUTPUTSTATE_ENABLE, vwCh1Ratio, TIM2_OCPOLARITY_LOW);
  TIM2_OC1PreloadConfig(ENABLE);

  /* Channel 2 PWM configuration */
  TIM2_OC2Init(TIM2_OCMODE_PWM2, TIM2_OUTPUTSTATE_ENABLE, vwCh2Ratio, TIM2_OCPOLARITY_LOW);
  TIM2_OC2PreloadConfig(ENABLE);

  /* Channel 3 PWM configuration */
  TIM2_OC3Init(TIM2_OCMODE_PWM2, TIM2_OUTPUTSTATE_ENABLE, vwCh3Ratio, TIM2_OCPOLARITY_LOW);
  TIM2_OC3PreloadConfig(ENABLE);

  /* Enables TIM2 peripheral Preload register on ARR */
  TIM2_ARRPreloadConfig(ENABLE);

  /* Enable TIM2 */
  TIM2_Cmd(ENABLE);

  MibWriteDebugStringCheck(1, "\r\n pwm end \r\n");

  // while (1);
}
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void TIM2_DeInit(void)
{
  TIM2->CR1 = (uint8_t)TIM2_CR1_RESET_VALUE;
  TIM2->IER = (uint8_t)TIM2_IER_RESET_VALUE;
  TIM2->SR2 = (uint8_t)TIM2_SR2_RESET_VALUE;

  /* Disable channels */
  TIM2->CCER1 = (uint8_t)TIM2_CCER1_RESET_VALUE;
  TIM2->CCER2 = (uint8_t)TIM2_CCER2_RESET_VALUE;


  /* Then reset channel registers: it also works if lock level is equal to 2 or 3 */
  TIM2->CCER1 = (uint8_t)TIM2_CCER1_RESET_VALUE;
  TIM2->CCER2 = (uint8_t)TIM2_CCER2_RESET_VALUE;
  TIM2->CCMR1 = (uint8_t)TIM2_CCMR1_RESET_VALUE;
  TIM2->CCMR2 = (uint8_t)TIM2_CCMR2_RESET_VALUE;
  TIM2->CCMR3 = (uint8_t)TIM2_CCMR3_RESET_VALUE;
  TIM2->CNTRH = (uint8_t)TIM2_CNTRH_RESET_VALUE;
  TIM2->CNTRL = (uint8_t)TIM2_CNTRL_RESET_VALUE;
  TIM2->PSCR = (uint8_t)TIM2_PSCR_RESET_VALUE;
  TIM2->ARRH  = (uint8_t)TIM2_ARRH_RESET_VALUE;
  TIM2->ARRL  = (uint8_t)TIM2_ARRL_RESET_VALUE;
  TIM2->CCR1H = (uint8_t)TIM2_CCR1H_RESET_VALUE;
  TIM2->CCR1L = (uint8_t)TIM2_CCR1L_RESET_VALUE;
  TIM2->CCR2H = (uint8_t)TIM2_CCR2H_RESET_VALUE;
  TIM2->CCR2L = (uint8_t)TIM2_CCR2L_RESET_VALUE;
  TIM2->CCR3H = (uint8_t)TIM2_CCR3H_RESET_VALUE;
  TIM2->CCR3L = (uint8_t)TIM2_CCR3L_RESET_VALUE;
  TIM2->SR1 = (uint8_t)TIM2_SR1_RESET_VALUE;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/**
  * @brief  Initializes the TIM2 Time Base Unit according to the specified parameters.
  * @param    TIM2_Prescaler specifies the Prescaler from TIM2_Prescaler_TypeDef.
  * @param    TIM2_Period specifies the Period value.
  * @retval None
  */
void TIM2_TimeBaseInit( TIM2_Prescaler_TypeDef TIM2_Prescaler,
                        uint16_t TIM2_Period)
{
  /* Set the Prescaler value */
  TIM2->PSCR = (uint8_t)(TIM2_Prescaler);
  /* Set the Autoreload value */
  TIM2->ARRH = (uint8_t)(TIM2_Period >> 8);
  TIM2->ARRL = (uint8_t)(TIM2_Period);
}



//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/**
  * @brief  Initializes the TIM2 Channel1 according to the specified parameters.
  * @param   TIM2_OCMode specifies the Output Compare mode  from @ref TIM2_OCMode_TypeDef.
  * @param   TIM2_OutputState specifies the Output State  from @ref TIM2_OutputState_TypeDef.
  * @param   TIM2_Pulse specifies the Pulse width  value.
  * @param   TIM2_OCPolarity specifies the Output Compare Polarity  from @ref TIM2_OCPolarity_TypeDef.
  * @retval None
  */
void TIM2_OC1Init(TIM2_OCMode_TypeDef TIM2_OCMode,
                  TIM2_OutputState_TypeDef TIM2_OutputState,
                  uint16_t TIM2_Pulse,
                  TIM2_OCPolarity_TypeDef TIM2_OCPolarity)
{
  /* Check the parameters */
  assert_param(IS_TIM2_OC_MODE_OK(TIM2_OCMode));
  assert_param(IS_TIM2_OUTPUT_STATE_OK(TIM2_OutputState));
  assert_param(IS_TIM2_OC_POLARITY_OK(TIM2_OCPolarity));

  /* Disable the Channel 1: Reset the CCE Bit, Set the Output State , the Output Polarity */
  TIM2->CCER1 &= (uint8_t)(~( TIM2_CCER1_CC1E | TIM2_CCER1_CC1P));
  /* Set the Output State &  Set the Output Polarity  */
  TIM2->CCER1 |= (uint8_t)((uint8_t)(TIM2_OutputState & TIM2_CCER1_CC1E ) |
                           (uint8_t)(TIM2_OCPolarity & TIM2_CCER1_CC1P));

  /* Reset the Output Compare Bits  & Set the Ouput Compare Mode */
  TIM2->CCMR1 = (uint8_t)((uint8_t)(TIM2->CCMR1 & (uint8_t)(~TIM2_CCMR_OCM)) |
                          (uint8_t)TIM2_OCMode);

  /* Set the Pulse value */
  TIM2->CCR1H = (uint8_t)(TIM2_Pulse >> 8);
  TIM2->CCR1L = (uint8_t)(TIM2_Pulse);
}


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

/**
  * @brief  Enables or disables the TIM2 peripheral Preload Register on CCR1.
  * @param   NewState new state of the Capture Compare Preload register.
  * This parameter can be ENABLE or DISABLE.
  * @retval None
  */
void TIM2_OC1PreloadConfig(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONALSTATE_OK(NewState));

  /* Set or Reset the OC1PE Bit */
  if (NewState != DISABLE)
  {
    TIM2->CCMR1 |= (uint8_t)TIM2_CCMR_OCxPE;
  }
  else
  {
    TIM2->CCMR1 &= (uint8_t)(~TIM2_CCMR_OCxPE);
  }
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/**
  * @brief  Enables or disables TIM2 peripheral Preload register on ARR.
  * @param   NewState new state of the TIM2 peripheral Preload register.
  * This parameter can be ENABLE or DISABLE.
  * @retval None
  */
void TIM2_ARRPreloadConfig(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONALSTATE_OK(NewState));

  /* Set or Reset the ARPE Bit */
  if (NewState != DISABLE)
  {
    TIM2->CR1 |= (uint8_t)TIM2_CR1_ARPE;
  }
  else
  {
    TIM2->CR1 &= (uint8_t)(~TIM2_CR1_ARPE);
  }
}


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/**
  * @brief  Enables or disables the TIM2 peripheral.
  * @param   NewState new state of the TIM2 peripheral. This parameter can
  * be ENABLE or DISABLE.
  * @retval None
  */
void TIM2_Cmd(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONALSTATE_OK(NewState));

  /* set or Reset the CEN Bit */
  if (NewState != DISABLE)
  {
    TIM2->CR1 |= (uint8_t)TIM2_CR1_CEN;
  }
  else
  {
    TIM2->CR1 &= (uint8_t)(~TIM2_CR1_CEN);
  }
}


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/**
  * @brief  Initializes the TIM2 Channel2 according to the specified parameters.
  * @param   TIM2_OCMode specifies the Output Compare mode  from @ref TIM2_OCMode_TypeDef.
  * @param   TIM2_OutputState specifies the Output State  from @ref TIM2_OutputState_TypeDef.
  * @param   TIM2_Pulse specifies the Pulse width  value.
  * @param   TIM2_OCPolarity specifies the Output Compare Polarity  from @ref TIM2_OCPolarity_TypeDef.
  * @retval None
  */
void TIM2_OC2Init(TIM2_OCMode_TypeDef TIM2_OCMode,
                  TIM2_OutputState_TypeDef TIM2_OutputState,
                  uint16_t TIM2_Pulse,
                  TIM2_OCPolarity_TypeDef TIM2_OCPolarity)
{
  /* Check the parameters */
  assert_param(IS_TIM2_OC_MODE_OK(TIM2_OCMode));
  assert_param(IS_TIM2_OUTPUT_STATE_OK(TIM2_OutputState));
  assert_param(IS_TIM2_OC_POLARITY_OK(TIM2_OCPolarity));


  /* Disable the Channel 1: Reset the CCE Bit, Set the Output State, the Output Polarity */
  TIM2->CCER1 &= (uint8_t)(~( TIM2_CCER1_CC2E |  TIM2_CCER1_CC2P ));
  /* Set the Output State & Set the Output Polarity */
  TIM2->CCER1 |= (uint8_t)((uint8_t)(TIM2_OutputState  & TIM2_CCER1_CC2E ) |
                           (uint8_t)(TIM2_OCPolarity & TIM2_CCER1_CC2P));


  /* Reset the Output Compare Bits & Set the Output Compare Mode */
  TIM2->CCMR2 = (uint8_t)((uint8_t)(TIM2->CCMR2 & (uint8_t)(~TIM2_CCMR_OCM)) |
                          (uint8_t)TIM2_OCMode);


  /* Set the Pulse value */
  TIM2->CCR2H = (uint8_t)(TIM2_Pulse >> 8);
  TIM2->CCR2L = (uint8_t)(TIM2_Pulse);
}


//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/**
  * @brief  Initializes the TIM2 Channel3 according to the specified parameters.
  * @param   TIM2_OCMode specifies the Output Compare mode from @ref TIM2_OCMode_TypeDef.
  * @param   TIM2_OutputState specifies the Output State from @ref TIM2_OutputState_TypeDef.
  * @param   TIM2_Pulse specifies the Pulse width value.
  * @param   TIM2_OCPolarity specifies the Output Compare Polarity  from @ref TIM2_OCPolarity_TypeDef.
  * @retval None
  */
void TIM2_OC3Init(TIM2_OCMode_TypeDef TIM2_OCMode,
                  TIM2_OutputState_TypeDef TIM2_OutputState,
                  uint16_t TIM2_Pulse,
                  TIM2_OCPolarity_TypeDef TIM2_OCPolarity)
{
  /* Check the parameters */
  assert_param(IS_TIM2_OC_MODE_OK(TIM2_OCMode));
  assert_param(IS_TIM2_OUTPUT_STATE_OK(TIM2_OutputState));
  assert_param(IS_TIM2_OC_POLARITY_OK(TIM2_OCPolarity));
  /* Disable the Channel 1: Reset the CCE Bit, Set the Output State, the Output Polarity */
  TIM2->CCER2 &= (uint8_t)(~( TIM2_CCER2_CC3E  | TIM2_CCER2_CC3P));
  /* Set the Output State & Set the Output Polarity */
  TIM2->CCER2 |= (uint8_t)((uint8_t)(TIM2_OutputState & TIM2_CCER2_CC3E) |
                           (uint8_t)(TIM2_OCPolarity & TIM2_CCER2_CC3P));

  /* Reset the Output Compare Bits & Set the Output Compare Mode */
  TIM2->CCMR3 = (uint8_t)((uint8_t)(TIM2->CCMR3 & (uint8_t)(~TIM2_CCMR_OCM)) |
                          (uint8_t)TIM2_OCMode);

  /* Set the Pulse value */
  TIM2->CCR3H = (uint8_t)(TIM2_Pulse >> 8);
  TIM2->CCR3L = (uint8_t)(TIM2_Pulse);
}





//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

/**
  * @brief  Enables or disables the TIM2 peripheral Preload Register on CCR2.
  * @param   NewState new state of the Capture Compare Preload register.
  * This parameter can be ENABLE or DISABLE.
  * @retval None
  */
void TIM2_OC2PreloadConfig(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONALSTATE_OK(NewState));

  /* Set or Reset the OC2PE Bit */
  if (NewState != DISABLE)
  {
    TIM2->CCMR2 |= (uint8_t)TIM2_CCMR_OCxPE;
  }
  else
  {
    TIM2->CCMR2 &= (uint8_t)(~TIM2_CCMR_OCxPE);
  }
}



//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------


/**
  * @brief  Enables or disables the TIM2 peripheral Preload Register on CCR3.
  * @param   NewState new state of the Capture Compare Preload register.
  * This parameter can be ENABLE or DISABLE.
  * @retval None
  */
void TIM2_OC3PreloadConfig(FunctionalState NewState)
{
  /* Check the parameters */
  assert_param(IS_FUNCTIONALSTATE_OK(NewState));

  /* Set or Reset the OC3PE Bit */
  if (NewState != DISABLE)
  {
    TIM2->CCMR3 |= (uint8_t)TIM2_CCMR_OCxPE;
  }
  else
  {
    TIM2->CCMR3 &= (uint8_t)(~TIM2_CCMR_OCxPE);
  }
}




//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
