
/**
 ******************************************************************************
 * @file stm8s_mib_debug.c
 * @brief
 * @author MYMEDIA Co., Ltd.
 * @version V1.0.0
 * @date 2023.1.6
 ******************************************************************************
 */
/* Includes ------------------------------------------------------------------*/
#include "stm8s.h"
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "stm8s_mib_debug.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
#if defined(UART_IRQ_RX_USE)
// These buffers may be any size from 2 to 256 bytes.
#define RX_BUFFER_SIZE 32
static uint8_t rx_buffer[RX_BUFFER_SIZE];
static uint8_t rx_buffer_head;
static uint8_t rx_buffer_tail;
#endif
#if defined(UART_IRQ_TX_USE)
#define TX_BUFFER_SIZE 32
static uint8_t tx_buffer[TX_BUFFER_SIZE];
static uint8_t tx_buffer_head;
static uint8_t tx_buffer_tail;
#endif
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//  PD7 : 485_DIR
#if defined(UCOM_485_USE)
#define UCOM_485_GPIO GPIOD			// PD7  //rs485 dir (rx, tx)
#define UCOM_485_PIN GPIO_PIN_7 // PD7
#define UCOM_485_MODE GPIO_MODE_OUT_PP_LOW_FAST
#define UCOM_485_DIR_HIGH (UCOM_485_GPIO->ODR |= (UCOM_485_PIN)) // GPIO_WriteHigh(UCOM_485_GPIO, UCOM_485_PIN) // tx
#define UCOM_485_DIR_LOW (UCOM_485_GPIO->ODR &= ~(UCOM_485_PIN)) // GPIO_WriteLow(UCOM_485_GPIO, UCOM_485_PIN)	 // rx
#endif
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
// #define UARTXdebug                    ((UART1_TypeDef *) UART1_BaseAddress)
// #if defined(STM8S003)
// #if defined(STM8S103)
#if defined(UART1)

//  PD5 : TXD
//  PD6 : RXD
#define UCOM_TXD_GPIO GPIOD			// PD5
#define UCOM_TXD_PIN GPIO_PIN_5 // PD5
#define UCOM_RXD_GPIO GPIOD			// PD6
#define UCOM_RXD_PIN GPIO_PIN_6 // PD6

#define UARTXdebug UART1
#define UARTX_Init UART1_Init
#define UARTX_WORDLENGTH_8D (UART1_WORDLENGTH_8D)
#define UARTX_STOPBITS_1 (UART1_STOPBITS_1)
#define UARTX_PARITY_NO (UART1_PARITY_NO)
#define UARTX_SYNCMODE_CLOCK_DISABLE (UART1_SYNCMODE_CLOCK_DISABLE)

#define UARTX_SR_TXE UART1_SR_TXE		// ((uint8_t)0x80) /*!< Transmit Data Register Empty mask */
#define UARTX_SR_TC UART1_SR_TC			// ((uint8_t)0x40) /*!< Transmission Complete mask */
#define UARTX_SR_RXNE UART1_SR_RXNE // ((uint8_t)0x20) /*!< Read Data Register Not Empty mask */
#define UARTX_SR_IDLE UART1_SR_IDLE // ((uint8_t)0x10) /*!< IDLE line detected mask */
#define UARTX_SR_OR UART1_SR_OR			// ((uint8_t)0x08) /*!< OverRun error mask */
#define UARTX_SR_NF UART1_SR_NF			// ((uint8_t)0x04) /*!< Noise Flag mask */
#define UARTX_SR_FE UART1_SR_FE			// ((uint8_t)0x02) /*!< Framing Error mask */
#define UARTX_SR_PE UART1_SR_PE			// ((uint8_t)0x01) /*!< Parity Error mask */

#define UARTX_BRR1_DIVM UART1_BRR1_DIVM // ((uint8_t)0xFF) /*!< LSB mantissa of UART1DIV [7:0] mask */

#define UARTX_BRR2_DIVM UART1_BRR2_DIVM // ((uint8_t)0xF0) /*!< MSB mantissa of UART1DIV [11:8] mask */
#define UARTX_BRR2_DIVF UART1_BRR2_DIVF // ((uint8_t)0x0F) /*!< Fraction bits of UART1DIV [3:0] mask */

#define UARTX_CR1_R8 UART1_CR1_R8				// ((uint8_t)0x80) /*!< Receive Data bit 8 */
#define UARTX_CR1_T8 UART1_CR1_T8				// ((uint8_t)0x40) /*!< Transmit data bit 8 */
#define UARTX_CR1_UARTD UART1_CR1_UARTD // ((uint8_t)0x20) /*!< UART1 Disable (for low power consumption) */
#define UARTX_CR1_M UART1_CR1_M					// ((uint8_t)0x10) /*!< Word length mask */
#define UARTX_CR1_WAKE UART1_CR1_WAKE		// ((uint8_t)0x08) /*!< Wake-up method mask */
#define UARTX_CR1_PCEN UART1_CR1_PCEN		// ((uint8_t)0x04) /*!< Parity Control Enable mask */
#define UARTX_CR1_PS UART1_CR1_PS				// ((uint8_t)0x02) /*!< UART1 Parity Selection */
#define UARTX_CR1_PIEN UART1_CR1_PIEN		// ((uint8_t)0x01) /*!< UART1 Parity Interrupt Enable mask */

#define UARTX_CR2_TIEN UART1_CR2_TIEN		// ((uint8_t)0x80) /*!< Transmitter Interrupt Enable mask */
#define UARTX_CR2_TCIEN UART1_CR2_TCIEN // ((uint8_t)0x40) /*!< Transmission Complete Interrupt Enable mask */
#define UARTX_CR2_RIEN UART1_CR2_RIEN		// ((uint8_t)0x20) /*!< Receiver Interrupt Enable mask */
#define UARTX_CR2_ILIEN UART1_CR2_ILIEN // ((uint8_t)0x10) /*!< IDLE Line Interrupt Enable mask */
#define UARTX_CR2_TEN UART1_CR2_TEN			// ((uint8_t)0x08) /*!< Transmitter Enable mask */
#define UARTX_CR2_REN UART1_CR2_REN			// ((uint8_t)0x04) /*!< Receiver Enable mask */
#define UARTX_CR2_RWU UART1_CR2_RWU			// ((uint8_t)0x02) /*!< Receiver Wake-Up mask */
#define UARTX_CR2_SBK UART1_CR2_SBK			// ((uint8_t)0x01) /*!< Send Break mask */

#define UARTX_CR3_LINEN UART1_CR3_LINEN // ((uint8_t)0x40) /*!< Alternate Function output mask */
#define UARTX_CR3_STOP UART1_CR3_STOP		// ((uint8_t)0x30) /*!< STOP bits [1:0] mask */
#define UARTX_CR3_CKEN UART1_CR3_CKEN		// ((uint8_t)0x08) /*!< Clock Enable mask */
#define UARTX_CR3_CPOL UART1_CR3_CPOL		// ((uint8_t)0x04) /*!< Clock Polarity mask */
#define UARTX_CR3_CPHA UART1_CR3_CPHA		// ((uint8_t)0x02) /*!< Clock Phase mask */
#define UARTX_CR3_LBCL UART1_CR3_LBCL		// ((uint8_t)0x01) /*!< Last Bit Clock pulse mask */

#define UARTX_CR4_LBDIEN UART1_CR4_LBDIEN // ((uint8_t)0x40) /*!< LIN Break Detection Interrupt Enable mask */
#define UARTX_CR4_LBDL UART1_CR4_LBDL			// ((uint8_t)0x20) /*!< LIN Break Detection Length mask */
#define UARTX_CR4_LBDF UART1_CR4_LBDF			// ((uint8_t)0x10) /*!< LIN Break Detection Flag mask */
#define UARTX_CR4_ADD UART1_CR4_ADD				// ((uint8_t)0x0F) /*!< Address of the UART1 node mask */

#define UARTX_CR5_SCEN UART1_CR5_SCEN		// ((uint8_t)0x20) /*!< Smart Card Enable mask */
#define UARTX_CR5_NACK UART1_CR5_NACK		// ((uint8_t)0x10) /*!< Smart Card Nack Enable mask */
#define UARTX_CR5_HDSEL UART1_CR5_HDSEL // ((uint8_t)0x08) /*!< Half-Duplex Selection mask */
#define UARTX_CR5_IRLP UART1_CR5_IRLP		// ((uint8_t)0x04) /*!< Irda Low Power Selection mask */
#define UARTX_CR5_IREN UART1_CR5_IREN		// ((uint8_t)0x02) /*!< Irda Enable mask */
// #endif
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
// #if defined(STM8S105)
#elif defined(UART2)

//  PD5 : TXD
//  PD6 : RXD
#define UCOM_TXD_GPIO GPIOD			// PD5
#define UCOM_TXD_PIN GPIO_PIN_5 // PD5
#define UCOM_RXD_GPIO GPIOD			// PD6
#define UCOM_RXD_PIN GPIO_PIN_6 // PD6

#define UARTXdebug UART2
#define UARTX_Init UART2_Init
#define UARTX_WORDLENGTH_8D (UART2_WORDLENGTH_8D)
#define UARTX_STOPBITS_1 (UART2_STOPBITS_1)
#define UARTX_PARITY_NO (UART2_PARITY_NO)
#define UARTX_SYNCMODE_CLOCK_DISABLE (UART2_SYNCMODE_CLOCK_DISABLE)

#define UARTX_SR_TXE UART2_SR_TXE		// ((uint8_t)0x80) /*!< Transmit Data Register Empty mask */
#define UARTX_SR_TC UART2_SR_TC			// ((uint8_t)0x40) /*!< Transmission Complete mask */
#define UARTX_SR_RXNE UART2_SR_RXNE // ((uint8_t)0x20) /*!< Read Data Register Not Empty mask */
#define UARTX_SR_IDLE UART2_SR_IDLE // ((uint8_t)0x10) /*!< IDLE line detected mask */
#define UARTX_SR_OR UART2_SR_OR			// ((uint8_t)0x08) /*!< OverRun error mask */
#define UARTX_SR_NF UART2_SR_NF			// ((uint8_t)0x04) /*!< Noise Flag mask */
#define UARTX_SR_FE UART2_SR_FE			// ((uint8_t)0x02) /*!< Framing Error mask */
#define UARTX_SR_PE UART2_SR_PE			// ((uint8_t)0x01) /*!< Parity Error mask */

#define UARTX_BRR1_DIVM UART2_BRR1_DIVM // ((uint8_t)0xFF) /*!< LSB mantissa of UART2DIV [7:0] mask */

#define UARTX_BRR2_DIVM UART2_BRR2_DIVM // ((uint8_t)0xF0) /*!< MSB mantissa of UART2DIV [11:8] mask */
#define UARTX_BRR2_DIVF UART2_BRR2_DIVF // ((uint8_t)0x0F) /*!< Fraction bits of UART2DIV [3:0] mask */

#define UARTX_CR1_R8 UART2_CR1_R8				// ((uint8_t)0x80) /*!< Receive Data bit 8 */
#define UARTX_CR1_T8 UART2_CR1_T8				// ((uint8_t)0x40) /*!< Transmit data bit 8 */
#define UARTX_CR1_UARTD UART2_CR1_UARTD // ((uint8_t)0x20) /*!< UART2 Disable (for low power consumption) */
#define UARTX_CR1_M UART2_CR1_M					// ((uint8_t)0x10) /*!< Word length mask */
#define UARTX_CR1_WAKE UART2_CR1_WAKE		// ((uint8_t)0x08) /*!< Wake-up method mask */
#define UARTX_CR1_PCEN UART2_CR1_PCEN		// ((uint8_t)0x04) /*!< Parity Control Enable mask */
#define UARTX_CR1_PS UART2_CR1_PS				// ((uint8_t)0x02) /*!< UART2 Parity Selection */
#define UARTX_CR1_PIEN UART2_CR1_PIEN		// ((uint8_t)0x01) /*!< UART2 Parity Interrupt Enable mask */

#define UARTX_CR2_TIEN UART2_CR2_TIEN		// ((uint8_t)0x80) /*!< Transmitter Interrupt Enable mask */
#define UARTX_CR2_TCIEN UART2_CR2_TCIEN // ((uint8_t)0x40) /*!< Transmission Complete Interrupt Enable mask */
#define UARTX_CR2_RIEN UART2_CR2_RIEN		// ((uint8_t)0x20) /*!< Receiver Interrupt Enable mask */
#define UARTX_CR2_ILIEN UART2_CR2_ILIEN // ((uint8_t)0x10) /*!< IDLE Line Interrupt Enable mask */
#define UARTX_CR2_TEN UART2_CR2_TEN			// ((uint8_t)0x08) /*!< Transmitter Enable mask */
#define UARTX_CR2_REN UART2_CR2_REN			// ((uint8_t)0x04) /*!< Receiver Enable mask */
#define UARTX_CR2_RWU UART2_CR2_RWU			// ((uint8_t)0x02) /*!< Receiver Wake-Up mask */
#define UARTX_CR2_SBK UART2_CR2_SBK			// ((uint8_t)0x01) /*!< Send Break mask */

#define UARTX_CR3_LINEN UART2_CR3_LINEN // ((uint8_t)0x40) /*!< Alternate Function output mask */
#define UARTX_CR3_STOP UART2_CR3_STOP		// ((uint8_t)0x30) /*!< STOP bits [1:0] mask */
#define UARTX_CR3_CKEN UART2_CR3_CKEN		// ((uint8_t)0x08) /*!< Clock Enable mask */
#define UARTX_CR3_CPOL UART2_CR3_CPOL		// ((uint8_t)0x04) /*!< Clock Polarity mask */
#define UARTX_CR3_CPHA UART2_CR3_CPHA		// ((uint8_t)0x02) /*!< Clock Phase mask */
#define UARTX_CR3_LBCL UART2_CR3_LBCL		// ((uint8_t)0x01) /*!< Last Bit Clock pulse mask */

#define UARTX_CR4_LBDIEN UART2_CR4_LBDIEN // ((uint8_t)0x40) /*!< LIN Break Detection Interrupt Enable mask */
#define UARTX_CR4_LBDL UART2_CR4_LBDL			// ((uint8_t)0x20) /*!< LIN Break Detection Length mask */
#define UARTX_CR4_LBDF UART2_CR4_LBDF			// ((uint8_t)0x10) /*!< LIN Break Detection Flag mask */
#define UARTX_CR4_ADD UART2_CR4_ADD				// ((uint8_t)0x0F) /*!< Address of the UART2 node mask */

#define UARTX_CR5_SCEN UART2_CR5_SCEN // ((uint8_t)0x20) /*!< Smart Card Enable mask */
#define UARTX_CR5_NACK UART2_CR5_NACK // ((uint8_t)0x10) /*!< Smart Card Nack Enable mask */
#define UARTX_CR5_IRLP UART2_CR5_IRLP // ((uint8_t)0x04) /*!< Irda Low Power Selection mask */
#define UARTX_CR5_IREN UART2_CR5_IREN // ((uint8_t)0x02) /*!< Irda Enable mask */

#define UARTX_CR6_LDUM UART2_CR6_LDUM			// ((uint8_t)0x80) /*!< LIN Divider Update Method */
#define UARTX_CR6_LSLV UART2_CR6_LSLV			// ((uint8_t)0x20) /*!< LIN Slave Enable */
#define UARTX_CR6_LASE UART2_CR6_LASE			// ((uint8_t)0x10) /*!< LIN Auto synchronization Enable */
#define UARTX_CR6_LHDIEN UART2_CR6_LHDIEN // ((uint8_t)0x04) /*!< LIN Header Detection Interrupt Enable */
#define UARTX_CR6_LHDF UART2_CR6_LHDF			// ((uint8_t)0x02) /*!< LIN Header Detection Flag */
#define UARTX_CR6_LSF UART2_CR6_LSF				// ((uint8_t)0x01) /*!< LIN Synch Field */

#else
#error not defined UARTX
#endif

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/**
 * @brief  Sends one character through a function.
 * @param  character
 * @retval None
 */
void MibWriteDebugByte(uint8_t ch)
{
#if defined(UART_IRQ_TX_USE)
	{
		uint8_t vbIndex;
		vbIndex = tx_buffer_head + 1;
		if (vbIndex >= TX_BUFFER_SIZE)
			vbIndex = 0;
		do
		{
			if (vbIndex != tx_buffer_tail)
				break;
		} while (1);
		tx_buffer[vbIndex] = ch;
		disableInterrupts()
				tx_buffer_head = vbIndex;
		if (UARTXdebug->SR & UARTX_SR_TXE)
		{ // empty..
			UARTXdebug->CR2 |= UARTX_CR2_TIEN;
		}
		enableInterrupts();
	}
#else
	{
		while (!(UARTXdebug->SR & UARTX_SR_TXE))
			;
		UARTXdebug->DR = ch;
	}
#endif
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/**
 * @brief  Sends one character through a function.
 * @param  character
 * @retval None
 */

#if defined(UCOM_485_USE)
void MibWriteDebugByte485(uint8_t ch)
{
#if defined(UCOM_485_USE)
	{
		UCOM_485_DIR_HIGH;
	}
#endif
	{
		MibWriteDebugByte(ch);
	}
#if defined(UCOM_485_USE)
	{
		while (!(UARTXdebug->SR & UARTX_SR_TC))
			;
		UARTXdebug->SR &= ~UARTX_SR_TC;
		UCOM_485_DIR_LOW;
	}
#endif
}
#endif
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/**
 * @brief  Sends one character through a function.
 * @param  character
 * @retval None
 */
void MibWriteDebugEmptyCheck(void)
{
	{
		while (!(UARTXdebug->SR & UARTX_SR_TC))
			;
		UARTXdebug->SR &= ~UARTX_SR_TC;
	}
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/**
 * @brief  Send a string through a function.
 * @param  string
 * @retval None
 */
#if 1
void MibWriteDebugString(uint8_t *v_pStr)
{
#if defined(UCOM_485_USE)
	{
		UCOM_485_DIR_HIGH;
	}
#endif
	while (*v_pStr)
	{
		MibWriteDebugByte(*v_pStr++);
	}
#if defined(UCOM_485_USE)
	{
		while (!(UARTXdebug->SR & UARTX_SR_TC))
			;
		UARTXdebug->SR &= ~UARTX_SR_TC;
		UCOM_485_DIR_LOW;
	}
#endif
}
#endif 
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/**
 * @brief  Send several characters through a function.
 * @param  string
 * @retval None
 */
#if 0
void MibWriteDebugStringN(uint8_t *v_pStr, uint8_t nSize)
{
#if defined(UCOM_485_USE)
	{
		UCOM_485_DIR_HIGH;
	}
#endif
	while (1)
	{
		if (nSize == 0)
			break;
		MibWriteDebugByte(*v_pStr++);
		nSize--;
	}
#if defined(UCOM_485_USE)
	{
		while (!(UARTXdebug->SR & UARTX_SR_TC))
			;
		UARTXdebug->SR &= ~UARTX_SR_TC;
		UCOM_485_DIR_LOW;
	}
#endif
}
#endif
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/**
 * @brief  Check incoming text messages.
 * @param  None
 * @retval character or none (0xffff)
 */
uint16_t MibReadDebugByte(void)
{
#if defined(UART_IRQ_RX_USE)
	{
		uint8_t vbCh, vbIndex;

		// while (rx_buffer_head == rx_buffer_tail) ; // wait for character
		if (rx_buffer_head == rx_buffer_tail) // wait for character
		{
			return MIB_DEBUG_READ_NODATA; // no data
		}
		vbIndex = rx_buffer_tail + 1;
		if (vbIndex >= RX_BUFFER_SIZE)
			vbIndex = 0;
		vbCh = rx_buffer[vbIndex];
		rx_buffer_tail = vbIndex;
		return (uint16_t)vbCh;
	}
#else
	{
		if (UARTXdebug->SR & UARTX_SR_RXNE) // data register empty ?
		{
			// UARTXdebug->SR &= ~UARTX_SR_RXNE;
			return (uint16_t)(UARTXdebug->DR & 0xff);
		}
		else
		{
			return MIB_DEBUG_READ_NODATA;
		}
	}
#endif
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/**
 * @brief  Configure UART for the communication with HyperTerminal
 * @param  None
 * @retval None
 */

/*
Control register 2 (UART_CR2)
Address offset: 0x05
Reset value: 0x00

	7 		6 		5 		4 		3 		2 		1 		0
	TIEN 	CIEN 	RIEN 	ILIEN 	TEN 	REN 	RWU 	SBK
	rw 		rw 		rw 		rw 		rw 		rw 		rw 		rw

	Bit 7 TIEN: Transmitter interrupt enable
		This bit is set and cleared by software.
		0: Interrupt is inhibited
		1: An UART interrupt is generated whenever TXE=1 in the UART_SR register

	Bit 6 TCIEN: Transmission complete interrupt enable
		This bit is set and cleared by software.
		0: Interrupt is inhibited
		1: An UART interrupt is generated whenever TC=1 in the UART_SR register

	Bit 5 RIEN: Receiver interrupt enable
		This bit is set and cleared by software.
		0: Interrupt is inhibited
		1: An UART interrupt is generated whenever OR=1 or RXNE=1 in the UART_SR register

	Bit 4 ILIEN: IDLE Line interrupt enable
		This bit is set and cleared by software.
		0: Interrupt is inhibited
		1: An UART interrupt is generated whenever IDLE=1 in the UART_SR register

	Bit 3 TEN: Transmitter enable (1) (2)
		This bit enables the transmitter. It is set and cleared by software.
		0: Transmitter is disabled
		1: Transmitter is enabled

	Bit 2 REN: Receiver enable
		This bit enables the receiver. It is set and cleared by software.
		0: Receiver is disabled
		1: Receiver is enabled and begins searching for a start bit

	Bit 1 RWU: Receiver wakeup
		? UART mode
		This bit determines if the UART is in mute mode or not. It is set and cleared by software and can be
		cleared by hardware when a wakeup sequence is recognized.(3) (4)
		? LIN slave mode (UART2, UART3 and UART4 only, if bits LINE and LSLV are set)
		While LIN is used in slave mode, setting the RWU bit allows the detection of Headers only and
		prevents the reception of any other characters. Refer to Mute mode and errors on page 355. In LIN
		slave mode, when RXNE is set, the software can not set or clear the RWU bit.
		0: Receiver in active mode
		1: Receiver in mute mode

	Bit 0 SBK: Send break
		This bit set is used to send break characters. It can be set and cleared by software.It should be set
		by software, and will be reset by hardware during the stop bit of break.

	0: No break character is transmitted
	1: Break character will be transmitted
	1. During transmission, a “0” pulse on the TEN bit (“0” followed by “1”) sends a preamble (idle line) after the current word.
	2. When TEN is set there is a 1 bit-time delay before the transmission starts.
	3. Before selecting Mute mode (by setting the RWU bit) the UART must first receive a data byte, otherwise it cannot function
	in Mute mode with wakeup by Idle line detection.
	4. In Address Mark Detection wakeup configuration (WAKE bit=1) the RWU bit cannot be modified by software while the
	RXNE bit is set.

*/

void MibDebugInit(uint32_t baudrate)
{
#if defined(UART_IRQ_RX_USE)
	{
		rx_buffer_head = 0;
		rx_buffer_tail = 0;
	}
#endif
#if defined(UART_IRQ_TX_USE)
	{
		tx_buffer_head = 0;
		tx_buffer_tail = 0;
	}
#endif
#if defined(UCOM_485_USE)
	// GPIO_Init(UCOM_485_GPIO, UCOM_485_PIN, UCOM_485_MODE);
	{																				 // TXD2
		UCOM_485_GPIO->DDR |= (UCOM_485_PIN);	 /* Set Output mode */
		UCOM_485_GPIO->CR1 |= (UCOM_485_PIN);	 /* Pull-Up or Push-Pull */
		UCOM_485_GPIO->CR2 |= (UCOM_485_PIN);	 /* Output speed up to 10 MHz */
		UCOM_485_GPIO->ODR &= ~(UCOM_485_PIN); // low...
	}
// {
// 	GPIO_Init(UCOM_485_GPIO, UCOM_485_PIN, UCOM_485_MODE); // output
// 	GPIO_WriteLow(UCOM_485_GPIO, UCOM_485_PIN);
// }
#endif

	/* EVAL COM (UART) configuration -----------------------------------------*/
	/* USART configured as follow:
		- BaudRate = 115200 baud
		- Word Length = 8 Bits
		- One Stop Bit
		- Odd parity
		- Receive and transmit enabled
		- UART Clock disabled
	*/
	{
		uint32_t BaudRate = baudrate;
		uint32_t v_ClkValue = 16000000; // CLK_GetClockFreq();

		uint8_t BRR2_1, BRR2_2 = 0;
		uint32_t BaudRate_Mantissa, BaudRate_Mantissa100 = 0;

		UARTXdebug->CR1 &= (uint8_t)(~UARTX_CR1_M);			 /**< Clear the word length bit */
		UARTXdebug->CR1 |= (uint8_t)UARTX_WORDLENGTH_8D; /**< Set the word length bit according to UART1_WordLength value */

		UARTXdebug->CR3 &= (uint8_t)(~UARTX_CR3_STOP); /**< Clear the STOP bits */
		UARTXdebug->CR3 |= (uint8_t)UARTX_STOPBITS_1;	 /**< Set the STOP bits number according to UART1_StopBits value  */

		UARTXdebug->CR1 &= (uint8_t)(~(UARTX_CR1_PCEN | UARTX_CR1_PS)); /**< Clear the Parity Control bit */
		UARTXdebug->CR1 |= (uint8_t)UARTX_PARITY_NO;										/**< Set the Parity Control bit to UART1_Parity value */

		UARTXdebug->BRR1 &= (uint8_t)(~UARTX_BRR1_DIVM); /**< Clear the LSB mantissa of UARTDIV  */
		UARTXdebug->BRR2 &= (uint8_t)(~UARTX_BRR2_DIVM); /**< Clear the MSB mantissa of UARTDIV  */
		UARTXdebug->BRR2 &= (uint8_t)(~UARTX_BRR2_DIVF); /**< Clear the Fraction bits of UARTDIV */

		/**< Set the UART2 BaudRates in BRR1 and BRR2 registers according to UART1_BaudRate value */
		BaudRate_Mantissa = ((uint32_t)v_ClkValue / (BaudRate << 4));
		BaudRate_Mantissa100 = (((uint32_t)v_ClkValue * 100) / (BaudRate << 4));
		/**< The fraction and MSB mantissa should be loaded in one step in the BRR2 register*/
		BRR2_1 = (uint8_t)((uint8_t)(((BaudRate_Mantissa100 - (BaudRate_Mantissa * 100)) << 4) / 100) & (u8)0x0F); /**< Set the fraction of UARTDIV  */
		BRR2_2 = (uint8_t)((BaudRate_Mantissa >> 4) & (u8)0xF0);

		UARTXdebug->BRR2 = (uint8_t)(BRR2_1 | BRR2_2);
		UARTXdebug->BRR1 = (uint8_t)BaudRate_Mantissa; /**< Set the LSB mantissa of UARTDIV  */

		UARTXdebug->CR2 &= (uint8_t)~(UARTX_CR2_TEN | UARTX_CR2_REN);																																			 /**< Disable the Transmitter and Receiver before seting the LBCL, CPOL and CPHA bits */
		UARTXdebug->CR3 &= (uint8_t)~(UARTX_CR3_CPOL | UARTX_CR3_CPHA | UARTX_CR3_LBCL);																									 /**< Clear the Clock Polarity, lock Phase, Last Bit Clock pulse */
		UARTXdebug->CR3 |= (uint8_t)((uint8_t)UARTX_SYNCMODE_CLOCK_DISABLE & (uint8_t)(UARTX_CR3_CPOL | UARTX_CR3_CPHA | UARTX_CR3_LBCL)); /**< Set the Clock Polarity, lock Phase, Last Bit Clock pulse */

		UARTXdebug->CR2 |= (uint8_t)UARTX_CR2_TEN; /**< Set the Transmitter Enable bit */

#if defined(UART_IRQ_RX_USE)
		{
			UARTXdebug->CR2 |= (uint8_t)UARTX_CR2_REN | UARTX_CR2_RIEN; /**< Set the Receiver Enable bit */
		}
#else
		{
			UARTXdebug->CR2 |= (uint8_t)UARTX_CR2_REN; /**< Set the Receiver Enable bit */
		}
#endif
		/**< Set the Clock Enable bit, lock Polarity, lock Phase and Last Bit Clock pulse bits according to UART1_Mode value */
		UARTXdebug->CR3 &= (uint8_t)(~UARTX_CR3_CKEN); /**< Clear the Clock Enable bit */
		/**< configure in Push Pull or Open Drain mode the Tx I/O line by setting the correct I/O Port register according the product package and line configuration*/

		{ // gpio init : PD5 : TX, PD6 : RX
			// GPIO_Init(GPIOD, GPIO_PIN_5, GPIO_MODE_OUT_PP_HIGH_FAST);
			{																				// TXD2
				UCOM_TXD_GPIO->DDR |= (UCOM_TXD_PIN); /* Set Output mode */
				UCOM_TXD_GPIO->CR1 |= (UCOM_TXD_PIN); /* Pull-Up or Push-Pull */
				UCOM_TXD_GPIO->CR2 |= (UCOM_TXD_PIN); /* Output speed up to 10 MHz */
				UCOM_TXD_GPIO->ODR |= (UCOM_TXD_PIN); // high...
			}
			// GPIO_Init(GPIOD, GPIO_PIN_6, GPIO_MODE_IN_PU_NO_IT);
			{																				 // RXD2
				UCOM_RXD_GPIO->DDR &= ~(UCOM_RXD_PIN); // Set input mode
				UCOM_RXD_GPIO->CR1 |= (UCOM_RXD_PIN);	 /* Pull-Up or Push-Pull */
				UCOM_RXD_GPIO->CR2 &= ~(UCOM_RXD_PIN); /*  External interrupt disabled */
																							 // GPIOD->ODR   = (1<<6);
			}
		}
		{
			// ITC_SetSoftwarePriority(ITC_IRQ_UART1_RX, ITC_PRIORITYLEVEL_2);
		}
	}
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
#if 0
static const char *g_TableHex = "0123456789ABCDEF";
void MibWriteDebugTransmitHex(uint8_t data)
{
#if defined(UCOM_485_USE)
	{
		UCOM_485_DIR_HIGH;
	}
#endif
	MibWriteDebugByte('.');
	if (data > 0xd)
		MibWriteDebugByte(data);
	MibWriteDebugByte('[');
	MibWriteDebugByte(g_TableHex[data >> 4]);
	MibWriteDebugByte(g_TableHex[data & 0xf]);
	MibWriteDebugByte(']');
	while (!(UARTXdebug->SR & UARTX_SR_TC))
		;
	UARTXdebug->SR &= ~UARTX_SR_TC;
#if defined(UCOM_485_USE)
	{
		UCOM_485_DIR_LOW;
	}
#endif
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
void MibWriteDebugTransmitHex2(uint8_t data)
{
#if defined(UCOM_485_USE)
	{
		UCOM_485_DIR_HIGH;
	}
#endif
	MibWriteDebugByte('.');
	MibWriteDebugByte(g_TableHex[data >> 4]);
	MibWriteDebugByte(g_TableHex[data & 0xf]);
	while (!(UARTXdebug->SR & UARTX_SR_TC))
		;
	UARTXdebug->SR &= ~UARTX_SR_TC;
#if defined(UCOM_485_USE)
	{
		UCOM_485_DIR_LOW;
	}
#endif
}
#endif
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
#if defined(UART_IRQ_RX_USE)
void UART_RX_IRQHandler(void) /* UART RX */
{
	// RXNEIE : RXNE interrupt enable : bit5 (CR1)
	// ORE = 1, RXNE = 1;
	{
		uint8_t vbCh, vbIndex;
		if (UARTXdebug->SR & UARTX_SR_RXNE) // RXNE :  ISR.bit5 // UART1_FLAG_RXNE
		{
			vbCh = (uint8_t)(UARTXdebug->DR);
			vbIndex = rx_buffer_head + 1;
			if (vbIndex >= RX_BUFFER_SIZE)
				vbIndex = 0;
			if (vbIndex != rx_buffer_tail)
			{
				rx_buffer[vbIndex] = vbCh;
				rx_buffer_head = vbIndex;
			}
		}
		else if (UARTXdebug->SR & UARTX_SR_OR) // OE : ISR.bit4 // /*!< OverRun error flag */
		{
			vbCh = (uint8_t)(UARTXdebug->DR);
		}
	}
}
#endif
#if defined(UART_IRQ_TX_USE)
void UART_TX_IRQHandler(void) /* UART TX */
{
	// RXNEIE : RXNE interrupt enable : bit5 (CR1)
	// ORE = 1, RXNE = 1;
	{
		uint8_t vbIndex;
		if (UARTXdebug->SR & UARTX_SR_TXE)
		{
			if (tx_buffer_tail == tx_buffer_head)
			{
				UARTXdebug->CR2 &= ~(UARTX_CR2_TIEN); // disable
			}
			else
			{
				vbIndex = tx_buffer_tail + 1;
				if (vbIndex >= TX_BUFFER_SIZE)
					vbIndex = 0;
				UARTXdebug->DR = tx_buffer[vbIndex];
				tx_buffer_tail = vbIndex;
			}
		}
		// if (UARTXdebug->SR & UARTX_SR_TC)
		// {
		// 		UARTXdebug->CR2 &= ~(UARTX_CR2_TCIEN); // disable
		// }
	}
}
#endif
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
