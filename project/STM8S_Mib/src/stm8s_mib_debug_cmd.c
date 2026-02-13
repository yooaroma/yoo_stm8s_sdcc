/**
  ******************************************************************************
  * @file stm8s_mib_cmd.c
  * @brief 
  * @author MYMEDIA Co., Ltd.
  * @version V1.0.0
  * @date 2023.1.6
  ******************************************************************************
  */
/*
  디버깅 프로그램 설명
  cmd : 명령어
  run 실행 함수 : 명령어에 따른 콜백 함수
  usage : help 리스트 및 사용법에 대한 예제..

  명령어를 uart로 받아 들여서 해석하여 실행한다.

  int GetCommand(char *command, uint16_t len, uint8_t timeout); // sec...
    문자열을 받아 들이는 함수 (UART로 부터 받아 들인다.)
  int GetArgs(char *s, char **args);
    주어진 문자열을 구분하여 주는 함수
  int DoPrintHelp(int argc, char **argv);
    주어진 명령어에 대한 도움말을 표현하여 주는 함수

  int HexToInt(char *s, void *retval, uint16_t type);
    문자를 16진수로 변환하는 함수
  int DecToLong(char *s, void *retval, uint16_t type);
    문자를 십진수로 변환하는 함수
  void HexDump(uint32_t addr, uint32_t len);
    어드레스에 데이터를 hex로 표현하여 주는 함수
*/
#if defined(__MIB_DEBUG_CMD__) || defined(__MIB_DEBUG_TOP__)
// #if 1
/* Includes ------------------------------------------------------------------*/
#include "stm8s.h"
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "stm8s_mib.h"
// #include "stm8s_mib_debug_cmd.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/*
  명령어의 종류를 저장하는 변수 : 여기서는 6개를 최대로 한다.
*/
CMD_MY cmdTbl[MAX_COMMANDS] = { 
		CMD_MY_END,
		CMD_MY_END,
		CMD_MY_END,
		CMD_MY_END,
		CMD_MY_END,
		CMD_MY_END,
};
/*
  받아들인 문자열을 저장하는 변수이며 여기서는 4개를 최대로 한다.
*/
CMD_ARRAY cmd_list;
/* Private function prototypes -----------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
// extern uint16_t gwMibSec;
/* Private variables ---------------------------------------------------------*/
/* Public functions ----------------------------------------------------------*/

int GetCommand(char *command, uint16_t len, uint8_t timeout); // sec...
int GetArgs(char *s, char **args);
int DoPrintHelp(int argc, char **argv);

int HexToInt(char *s, void *retval, uint16_t type);
int DecToLong(char *s, void *retval, uint16_t type);
void HexDump(uint32_t addr, uint32_t len);
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/**
 * @brief
 * @par Parameters: None
 * @retval None
 */
int GetCommand(char *cmd, uint16_t len, uint8_t timeout)
{
		uint16_t vwCh;
		uint16_t vwIndex, rdCnt, rdMax = len - 1;
		uint16_t endTime;

		endTime = MibGetSecs() + (uint16_t)timeout;
		cmd_list.cur = cmd_list.next;
		for (rdCnt = 0, vwIndex = 0; rdCnt <= rdMax;)
		{
			// try to get a byte from the serial port.
			while (1)
			{
				vwCh = MibReadDebugByte();
				if (vwCh != 0xffff)	break;
				if (MibGetSecs() > endTime)
				{
					cmd[rdCnt++] = '\0';
					memset(cmd_list.buf[cmd_list.next], 0, CMD_CHAR_MAX);
					memcpy(cmd_list.buf[cmd_list.next], cmd, strlen(cmd));
					cmd_list.next = (cmd_list.next + 1) % CMD_LINE_MAX;
					return rdCnt;
				}
			}

			// mib_printf("0x%x",c);

			if ((vwCh == '@') || (vwCh == '\n') || (vwCh == 0x03))
			{
				cmd[rdCnt++] = '\0';
				memset(cmd_list.buf[cmd_list.next], 0, CMD_CHAR_MAX);
				memcpy(cmd_list.buf[cmd_list.next], cmd, strlen(cmd));
				cmd_list.next = (cmd_list.next + 1) % CMD_LINE_MAX;
				// print newline.
				mib_printf("\r\n");
				return rdCnt;
			}
			else if (vwCh == '\b')
			{
				if (rdCnt > 0)
				{
					rdCnt--;
					// cursor one position back.
					mib_printf("\b \b");
				}
			}
			else if (vwCh == 0x1b)
			{ // ESC (0x1b)
				// mib_printf("@");
				while (1)
				{
					vwCh = MibReadDebugByte();
					if (vwCh != 0xffff)
						break;
				}
				if (vwCh == '[')
				{
					// mib_printf("%c",c);
					while (1)
					{
						vwCh = MibReadDebugByte();
						if (vwCh != 0xffff)
							break;
					}
					// mib_printf("%c",c);
					if (vwCh == 'B')
					{ // down
						cmd_list.cur++;
						if (cmd_list.cur == CMD_LINE_MAX)
							cmd_list.cur = 0;
						rdCnt = strlen(cmd_list.buf[cmd_list.cur]);
						if (rdCnt >= rdMax)
							rdCnt = rdMax;
						memcpy(cmd, cmd_list.buf[cmd_list.cur], rdCnt);
						cmd[rdCnt] = 0;
						//CMD_StringMy();
						MibWriteDebugString(cmd);
					}
					else if (vwCh == 'A')
					{ // up
						if (cmd_list.cur == 0)
							cmd_list.cur = CMD_LINE_MAX - 1;
						else
							cmd_list.cur--;
						rdCnt = strlen(cmd_list.buf[cmd_list.cur]);
						if (rdCnt >= rdMax)
							rdCnt = rdMax;
						memcpy(cmd, cmd_list.buf[cmd_list.cur], rdCnt);
						cmd[rdCnt] = 0;
						//CMD_StringMy();
						MibWriteDebugString(cmd);
					}
				}
			}
			else
			{
				cmd[rdCnt++] = vwCh;
				// print character.
				MibWriteDebugByte(vwCh);
			}
		}
		return (rdCnt);
} // GetCommand.

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
/**
 * @brief
 * @par Parameters: None
 * @retval None
 */
int GetArgs(char *s, char **argv)
{
		int args = 0;

		if (!s || *s == '\0')
			return 0;
		while (args < MAX_ARGS)
		{
			// skip space and tab.
			while ((*s == ' ') || (*s == '\t'))
				s++;

			// check end of line.
			if (*s == '\0')
			{
				argv[args] = 0;
				return args;
			}
			// start get arg.
			argv[args++] = s;

			// remove ' ' and '\t'.
			while (*s && (*s != ' ') && (*s != '\t'))
				s++;
			// end of line.
			if (*s == '\0')
			{
				argv[args] = 0;
				return args;
			}
			*s++ = '\0';
		}
		return args;
} // GetArgs.

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
/**
 * @brief  A function that converts a value according to the given character 
 *         and the format of the variable.
 * @par Parameters: None
 * @retval None
 */
int HexToInt(char *s, void *retval, uint16_t type)
{
		char c;
		char i;
		uint32_t rval;

		if (!s || !retval)
			return FALSE;
		if (!strncmp(s, "0x", 2))
			s += 2;
		// fine int value.
		for (i = 0, rval = 0; i < type / 4; i++)
		{
			if (*s == '\0')
			{
				if (i == 0)
					return FALSE;
				else
					break;
			}
			c = *s++;

			if (c >= '0' && c <= '9')
				c -= '0';
			else if (c >= 'a' && c <= 'f')
				c = c - 'a' + 10;
			else if (c >= 'A' && c <= 'F')
				c = c - 'A' + 10;
			else
				return FALSE;

			rval = rval << 4 | c;
		}
		// make retval.
		switch (type)
		{
		case 8:
			*(uint8_t *)retval = (uint8_t)rval;
			break;
		case 16:
			*(uint16_t *)retval = (uint16_t)rval;
			break;
		case 32:
			*(uint32_t *)retval = (uint32_t)rval;
			break;
		default:
			return FALSE;
		}
		return TRUE;
} // HexToInt.

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
/**
 * @brief
 * @par Parameters: None
 * @retval None
 */
int DecToLong(char *s, void *retval, uint16_t type)
{
		char c;
		uint32_t rval;
		
		if (!s || !s[0] || !retval)
			return FALSE;

		for (rval= 0; *s; s++)
		{
			if (*s < '0' || *s > '9')
				return FALSE;
			c = *s - '0';
			rval = rval * 10 + c;
		}
		// make retval.
		switch (type)
		{
		case 8:
			*(uint8_t *)retval = (uint8_t)rval;
			break;
		case 16:
			*(uint16_t *)retval = (uint16_t)rval;
			break;
		case 32:
			*(uint32_t *)retval = (uint32_t)rval;
			break;
		default:
			return FALSE;
		}
		return TRUE;
} // DecToLong.

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
/**
 * @brief
 * @par Parameters: None
 * @retval None
 */
void HexDump(uint32_t addr, uint32_t len)
{
		char c;
		uint32_t endPtr = (addr + len);
		int i, remainder = len & 0xf;
		uint8_t p_data[20];
		uint16_t *p_data_ptr;
		uint32_t p_address;

		// mib_printf("\r\n");
		mib_printf("\r\n");
		mib_printf("address     Hex Value                                        Ascii value\r\n");

		// print out 16 byte blocks.
		p_address = (uint32_t)(addr);
		p_data_ptr = (uint16_t *)p_data;
		while ((p_address + 16) <= endPtr)
		{
			mib_printf("0x%08lx : ", p_address);
			for (i = 0; i < 8; i++)
			{
				p_data_ptr[i] = rd_ADDR16(p_address + i * 2);
				mib_printf("%02x ", p_data[i * 2]);
				mib_printf("%02x ", p_data[i * 2 + 1]);
			}
			mib_printf(" ");
			for (i = 0; i < 16; i++)
			{
				c = p_data[i];
				if (c >= 32 && c <= 125)
					mib_printf("%c", c);
				else
					mib_printf(".");
			}
			p_address += 16;
			// mib_printf("\r\n");
			mib_printf("\r\n");
		}

		// Print out remainder.
		if (remainder)
		{
			mib_printf("0x%08lx  ", p_address);
			for (i = 0; i < (remainder >> 1); i++)
			{
				p_data_ptr[i] = rd_ADDR16(p_address + i * 2);
				mib_printf("%02x ", p_data[i * 2]);
				mib_printf("%02x ", p_data[i * 2 + 1]);
			}
			for (i = 0; i < (16 - (remainder >> 1) * 2); i++)
			{
				mib_printf("   ");
			}
			mib_printf(" ");
			for (i = 0; i < remainder; i++)
			{
				c = p_data[i];
				if (c >= 32 && c <= 125)
					mib_printf("%c", c);
				else
					mib_printf(".");
			}
			for (i = 0; i < (16 - remainder); i++)
			{
				mib_printf(" ");
			}
			// mib_printf("\r\n");
			mib_printf("\r\n");
		}
		return;
} // HexDump.
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
/**
 * @brief
 * @par Parameters: None
 * @retval None
 */
int DoPrintHelp(int argc, char **argv)
{
		CMD_MY *cptr;

		if (argc == 1)
		{
			mib_printf("*******\r\n");
			mib_printf("  help  Help for commands.\r\n");

			for (cptr = cmdTbl; cptr->cmd; cptr++)
			{
				if (cptr->usage)
					MibWriteDebugString(cptr->usage);
			}
			mib_printf("*******\r\n");
		}
		else
		{
			mib_printf("\tUnknown command : ");
			mib_printf("%s", argv[0]);
			mib_printf("\r\n");
		}
		return TRUE;
}

//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
/**
 * @brief
 * @par Parameters: None
 * @retval None
 */
int getdec(uint8_t **ptr)
{
		uint8_t *p = *ptr;
		int ret = 0;
		if ((*p < '0') || (*p > '9'))
			return (-1);
		while ((*p >= '0') && (*p <= '9'))
		{
			ret = ret * 10 + (int)(*p - '0');
			p++;
		}
		*ptr = p;
		return (ret);
}
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
#endif // __MIB_DEBUG_CMD__
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
