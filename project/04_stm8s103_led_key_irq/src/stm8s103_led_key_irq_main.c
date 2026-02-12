/**
  ******************************************************************************
  * @file stm8s103_led_key_irq_main.c
  * @brief Make the LED blink.
  * @author yooaroma@gmail.com
  * @version V1.0.0
  * @date 2023.1.6
  ******************************************************************************
  */
/*
    info :    
    모듈 기반의 네이밍 (Namespace 효과)
    규모가 큰 프로젝트라면 콜백 이름만 보고도 어느 계층이나 모듈에서 온 것인지 알 수 있어야 합니다.
    [Module]_[Event]_[Callback] 구조를 추천합니다.
    예시:
    Wifi_Connected_cb (와이파이 모듈에서 연결 완료 시 호출)
    Ble_DataReceived_cb (BLE 모듈에서 데이터 수신 시 호출)
    Gpio_PinInterruptHandler_cb (GPIO 모듈에서 핀 인터럽트 발생 시 호출)
    Timer1_Overflow_cb (타이머1에서 오버플로우 발생 시 호출)
    I2C_MasterTxComplete_cb (I2C 모듈에서 마스터 전송 완료 시 호출)
    Uart_RxError_cb (UART 모듈에서 수신 오류 발생 시 호출
    Adc_ConversionComplete_cb (ADC 모듈에서 변환 완료 시 호출)
    Pwm_ChannelUpdate_cb (PWM 모듈에서 채널 업데이트 시 호출)
    Dac_OutputReady_cb (DAC 모듈에서 출력 준비 완료 시 호출)

    stm8s103_led_key_irq_main.c 는 STM8S103 마이크로컨트롤러에서 GPIO
    인터럽트를 사용하여 LED를 제어하고 KEY 입력을 감지하는 예제입니다.
    이 프로그램은 LED(GPIO_B5)를 최초 ON 상태로 설정하고,
    KEY(GPIO_A1)의 입력을 Falling edge(스위치를 누룰때)에서 인터럽트로 받아서 LED를 OFF를 하는 기능이다.
    동시에 PORTA1 인터럽트를 비활성화 시킨다. 실 메인 프로그램에서 비활성화 비트를 체크하여 
    5초 딜레이 후에 다시 LED를 ON 시키고 PORTA1 인터럽트를 활성화 시킨다.
    이 프로그램에서는 gpio 인터럽트를 이해하고 인터럽트가 발생시 처리하는 절차를 
    간단히 이해하기 위한 프로그램이다. 모든 개발에는 gpio 인터럽트에 대한 처리를 하기위해서
    필요한 순서에 대한 이해가 필요하며 이것에 대한 간단 명료하게 이해하기 위한 예제이다.
    stm8s_it.c 파일의 EXTI_PORTA_IRQHandler 함수 내부에
    Gpio_PORTA1_InterruptHandler_cb 을 구현하였다.
    USE_PORTA_INTERRUPT_HANDLER_CB define을 추가하여 해당 콜백 함수를 호출하도록 하였다.
*/

/* Includes ------------------------------------------------------------------*/
#include "stm8s.h"

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
#define UCOM_LED1_GPIO    GPIOB        // PB5
#define UCOM_LED1_PIN     GPIO_PIN_5   // PB5
#define UCOM_LED1_MODE    GPIO_MODE_OUT_PP_LOW_FAST

#define UCOM_KEY1_GPIO    GPIOA        // PA1
#define UCOM_KEY1_PIN     GPIO_PIN_1   // PA1
// #define UCOM_KEY1_MODE    GPIO_MODE_IN_PU_NO_IT // GPIO_MODE_IN_FL_NO_IT
#define UCOM_KEY1_MODE    GPIO_MODE_IN_FL_IT     /*!< Input floating, external interrupt */
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void CLK_Configuration(void);
void GPIO_Configuration(void);
void Toggle(void);

/* Private functions ---------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
uint32_t gwDelay = 0;
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

#if defined(USE_PORTA_INTERRUPT_HANDLER_CB)
void Gpio_PORTA_InterruptHandler_cb(void)
{
  gwDelay = 0x40000*5; // 5sec..
  UCOM_LED1_GPIO->ODR |= (uint8_t)UCOM_LED1_PIN; // led off
  UCOM_KEY1_GPIO->CR2 &= (uint8_t)(~(UCOM_KEY1_PIN)); /* External interrupt disable */
}
#endif /* USE_PORTA_INTERRUPT_HANDLER_CB */
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/**
  ******************************************************************************
  * @brief          : Main function.
  * @par Parameters :
  * None
  * @retval         : void None
  * @par Required preconditions:
  * None
  ******************************************************************************
  */
void main(void)
{
  /* Configures clocks */
  // CLK_Configuration();
  {
    /* Clear High speed internal clock prescaler */
    CLK->CKDIVR &= (uint8_t)(~CLK_CKDIVR_HSIDIV);    
    /* Set High speed internal clock prescaler */
    CLK->CKDIVR |= (uint8_t)CLK_PRESCALER_HSIDIV1;
  }
  // void EXTI_SetExtIntSensitivity(EXTI_Port_TypeDef Port, EXTI_Sensitivity_TypeDef SensitivityValue)
  // EXTI_SetExtIntSensitivity(GPIOA,EXTI_SENSITIVITY_RISE_ONLY); // PA3
  {    // PA3
    #define EXTI_SHIFT_PORTA  0
    #define EXTI_SHIFT_PORTB  2
    #define EXTI_SHIFT_PORTC  4
    #define EXTI_SHIFT_PORTD  6
    #define EXTI_SHIFT_PORTE  0
    #define EXTI_SHIFT_PORT   EXTI_SHIFT_PORTA

    uint8_t vbData = 0;
    // vbData = (uint8_t)(EXTI_SENSITIVITY_FALL_LOW);  /*!< Interrupt on Falling edge and Low level */
    // vbData = (uint8_t)(EXTI_SENSITIVITY_RISE_ONLY); /*!< Interrupt on Rising edge only */ 
    vbData = (uint8_t)(EXTI_SENSITIVITY_FALL_ONLY); /*!< Interrupt on Falling edge only */ 
    // vbData = (uint8_t)(EXTI_SENSITIVITY_RISE_FALL); /*!< Interrupt on Rising and Falling edges */ 
    EXTI->CR1 &= (uint8_t)~(0x03<<EXTI_SHIFT_PORT); /* PA1 : UCOM_KEY1_GPIO */
    EXTI->CR1 |= (vbData<<EXTI_SHIFT_PORT); /* PORT A */
    // EXTI->CR2 &= (uint8_t)~(0x03<<EXTI_SHIFT_PORT); /* PORT E */
    // EXTI->CR2 |= (vbData<<EXTI_SHIFT_PORT); /* PORT E */
  }
  /* Configures GPIOs */
  // GPIO_Configuration();
  {
    /* Configure PD0 (LED1) as output push-pull low (led switched on) */
    // GPIO_Init(UCOM_LED1_GPIO, UCOM_LED1_PIN, UCOM_LED1_MODE);
    {
      UCOM_LED1_GPIO->ODR &= (uint8_t)(~(UCOM_LED1_PIN)); /* Output mode  Low level */
      UCOM_LED1_GPIO->DDR |= (uint8_t)UCOM_LED1_PIN; /* Set Output mode */
      UCOM_LED1_GPIO->CR1 |= (uint8_t)UCOM_LED1_PIN; /* Push-Pull */
      // UCOM_LED1_GPIO->CR1 &= (uint8_t)(~(UCOM_LED1_PIN)); /* Open drain */
      UCOM_LED1_GPIO->CR2 |= (uint8_t)UCOM_LED1_PIN; /* 10MHz */
      // UCOM_LED1_GPIO->CR2 &= (uint8_t)(~(UCOM_LED1_PIN)); /* 2MHz */
    }
    /* Configure PA3 : KEY IN as input push-pull low (led switched on) */
    // GPIO_Init(UCOM_KEY1_GPIO, UCOM_KEY1_PIN, UCOM_KEY1_MODE);
    {
      UCOM_KEY1_GPIO->DDR &= (uint8_t)(~(UCOM_KEY1_PIN)); /* Set Input mode */
      UCOM_KEY1_GPIO->CR1 |= (uint8_t)UCOM_KEY1_PIN; /* Pull-up */
      //UCOM_KEY1_GPIO->CR1 &= (uint8_t)(~(UCOM_KEY1_PIN)); /* Float */
      UCOM_KEY1_GPIO->CR2 |= (uint8_t)UCOM_KEY1_PIN; /* External interrupt enable */
      // UCOM_KEY1_GPIO->CR2 &= (uint8_t)(~(UCOM_KEY1_PIN)); /* External interrupt disable */
    }
    {      
      UCOM_LED1_GPIO->ODR &= ~(uint8_t)UCOM_LED1_PIN; // led on...
    }
  }
  {
    // enable...
    enableInterrupts();
  }
  {
    do {      
      if(((UCOM_KEY1_GPIO->CR2)&(uint8_t)UCOM_KEY1_PIN) == 0) // interrupt disabled check 
      {
        if(gwDelay==0) 
        {      
          // UCOM_LED1_GPIO->ODR = UCOM_LED1_GPIO->ODR & ~UCOM_LED1_PIN; // led on...
          UCOM_LED1_GPIO->ODR &= ~(uint8_t)UCOM_LED1_PIN;
          UCOM_KEY1_GPIO->CR2 |= (uint8_t)UCOM_KEY1_PIN; /* External interrupt enable */
        }
        else 
        {
          gwDelay--;
        }
      }
    } while(1);
  }
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
void assert_failed(u8* file, u32 line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
