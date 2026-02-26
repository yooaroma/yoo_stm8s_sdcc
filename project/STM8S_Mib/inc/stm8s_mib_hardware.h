/**
  ******************************************************************************
  * @file stm8s_mib_hardware.h
  * @brief 
  * @author MYMEDIA Co., Ltd.
  * @version V1.0.0
  * @date 2023.1.6
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM8S_MIB_HARDWARE_H
#define __STM8S_MIB_HARDWARE_H

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
#ifdef __cplusplus
 extern "C" {
#endif
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Evalboard I/Os configuration */
/* Hardware define */
#if defined(STM8S103)
/*
  // LEFT
  PD4 : D13 : BEEP
  PD5 : D14 : A3 : TXD 
  PD6 : D15 : A4 : RXD
  RESET
  PA1 : D0  : KEY1 
  PA2 : D1  : LED
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
#if defined(USE_LED1_PA2)
 #define UCOM_LED1_GPIO    GPIOA        // PA2
 #define UCOM_LED1_PIN     GPIO_PIN_2   // PA2
 #define UCOM_LED1_MODE    GPIO_MODE_OUT_PP_LOW_FAST
#else 
#define UCOM_LED1_GPIO    GPIOB        // PB5
#define UCOM_LED1_PIN     GPIO_PIN_5   // PB5
#define UCOM_LED1_MODE    GPIO_MODE_OUT_PP_LOW_FAST
#endif

#define UCOM_KEY1_GPIO    GPIOA        // PA1
#define UCOM_KEY1_PIN     GPIO_PIN_1   // PA1
#define UCOM_KEY1_MODE    GPIO_MODE_IN_PU_NO_IT // GPIO_MODE_IN_FL_NO_IT
// #define UCOM_KEY1_MODE    GPIO_MODE_IN_FL_IT     /*!< Input floating, external interrupt */

/*
	KEY SCH...
	VCC - SW - VOUT :--- 10K - GND 
	                :--- LED - 100R - GND 
*/
#elif defined(STM8S105)
/*
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
#define UCOM_LED1_GPIO    GPIOD        // PD7
#define UCOM_LED1_PIN     GPIO_PIN_7   // PD7
#define UCOM_LED1_MODE    GPIO_MODE_OUT_PP_LOW_FAST

#define UCOM_KEY1_GPIO    GPIOA        // PA1
#define UCOM_KEY1_PIN     GPIO_PIN_1   // PA1
#define UCOM_KEY1_MODE    GPIO_MODE_IN_PU_NO_IT // GPIO_MODE_IN_FL_NO_IT
// #define UCOM_KEY1_MODE    GPIO_MODE_IN_FL_IT     /*!< Input floating, external interrupt */
#endif 

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
  
#endif /* __STM8S_MIB_HARDWARE_H */


/************************ (C) COPYRIGHT MYMEDIA *****END OF FILE****/
