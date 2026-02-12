/**
  ******************************************************************************
  * @file stm8s_mib_cmd.c
  * @brief 
  * @author MYMEDIA Co., Ltd.
  * @version V1.0.0
  * @date 2023.1.6
  ******************************************************************************
  */

#if defined(__MIB_DEBUG_MEM__) || defined(__MIB_DEBUG_TOP__)
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
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
static int DoMem(CMD_MY * cptr, int argc, char **argv);
/* Private macro -------------------------------------------------------------*/
#define CMD_MY_MEM_ALL	\
{   "mem", DoMem,		\
	"  mem {cpy}   [dest] [src]   [len] Copy to SDRAM from Flash or SDRAM.\r\n"\
	"  mem {cmp}   [add1] [add2]  [len] Compare data in addr1 and addr2.\r\n"\
	"  mem {set}   [addr] [value] [len] Fill Memory with value.\r\n"\
	"  mem {hdump} [addr] [len]         Dump Memory.\r\n"	\
	"  mem {wrn} {c/s/l} [addr] [value] [loop]  Wrn in addr. c:8 s:16 l:32 bits.\r\n"\
	"  mem {rdn}  {c/s/l} [addr] [loop]   Rdn in addr. c:8 s:16 l:32 bits.\r\n"\
	"  mem {write} {c/s/l} [addr] [value]  Write in addr. c:8 s:16 l:32 bits.\r\n"\
	"  mem {read}  {c/s/l} [addr]          Read in addr. c:8 s:16 l:32 bits.\r\n",\
},
/* Private variables ---------------------------------------------------------*/
static CMD_MY cmdTbl_only[] = {
		CMD_MY_MEM_ALL
		CMD_MY_END
};
/* Public functions ----------------------------------------------------------*/
extern CMD_ARRAY cmd_list;
extern CMD_MY cmdTbl[];
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
/**
 * @brief
 * @par Parameters: None
 * @retval None
 */ 
void cmd_mem(void)
{
	CMD_MY *cptr;
	uint32_t index;

	index = 0;
	for (cptr = cmdTbl;; cptr++) 
	{
		if(cptr->cmd==0)
		{
			ccprintf(1,("INFO:+cmd_mem...\r\n"));
			memcpy((void *)&(cptr->cmd),(void *)&(cmdTbl_only[0].cmd),sizeof(CMD_MY));
			break;
		}
		index++;
		if(index>MAX_COMMANDS)
		{ 
			ccprintf(1,("INFO:-cmd_mem...\r\n"));
			return;
		}
	}
	return;
}
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
/**
 * @brief
 * @par Parameters: None
 * @retval None
 */ 
#define _DEBUG_CMD_A_  1
static int DoMem(CMD_MY *cptr, int argc, char **argv)
{
	uint32_t addr = 0;
	uint8_t c;
	uint16_t s;
	uint32_t l;
	//    int ch;
	//    uint16_t value;
	//    uint32_t i;
	//    uint32_t tmpIndex;
	uint32_t dest, src, len;
	//    int indexArgc;
	//    char tmpStr[500];

	if (argc < 3)
	{
		MibWriteDebugString(cptr->usage);
		return FALSE;
	}	
	if (!strcmp(argv[1], "write")) // _CMD_MY_WRITE_
	{
		if (argc < 5)
		{
			MibWriteDebugString(cptr->usage);
			return FALSE;
		}
		if (!HexToInt(argv[3], &addr, 32))
		{
			ccprintf(_DEBUG_CMD_A_, ("Illugal character is used.\r\n"));
			return FALSE;
		}

		switch (argv[2][0])
		{
		case 'c':
			if (!HexToInt(argv[4], &c, 8))
			{
				ccprintf(_DEBUG_CMD_A_, ("Illugal character is used.\r\n"));
				return FALSE;
			}
			wr_ADDR8(addr, c);
			break;
		case 's':
			if (!HexToInt(argv[4], &s, 16))
			{
				ccprintf(_DEBUG_CMD_A_, ("Illugal character is used.\r\n"));
				return FALSE;
			}
			wr_ADDR16(addr, s);
			break;
		case 'l':
			if (!HexToInt(argv[4], &l, 32))
			{
				ccprintf(_DEBUG_CMD_A_, ("Illugal character is useqd.\r\n"));
				return FALSE;
			}
			wr_ADDR32(addr, l);
			break;
		default:
			MibWriteDebugString(cptr->usage);
			return FALSE;
		}		
	}
	else if (!strcmp(argv[1], "read")) // _CMD_MY_READ_
	{
		if (argc < 4)
		{
			MibWriteDebugString(cptr->usage);
			return FALSE;
		}
		if (!HexToInt(argv[3], &addr, 32))
		{
			ccprintf(_DEBUG_CMD_A_, ("Illugal character is used.\r\n"));
			return FALSE;
		}
		ccprintf(_DEBUG_CMD_A_,("\taddress : 0x%08lx  ", addr));
		ccprintf(_DEBUG_CMD_A_, ("\tvalue   : "));
		switch (argv[2][0])
		{
		case 'c':
			c = rd_ADDR8(addr);
			ccprintf(_DEBUG_CMD_A_,("0x%02x ", c));
			break;
		case 's':
			s = rd_ADDR16(addr);
			ccprintf(_DEBUG_CMD_A_,("0x%04x", s));
			break;
		case 'l':
			l = rd_ADDR32(addr);
			ccprintf(_DEBUG_CMD_A_,("0x%08lx", l));
			break;
		default:
			ccprintf(_DEBUG_CMD_A_, ("Error.\r\n"));
			return FALSE;
		}
		ccprintf(_DEBUG_CMD_A_, (".\r\n"));		
	}	
	else if (!strcmp(argv[1], "wrn")) // _CMD_MY_WRITE_
	{
		uint32_t v_Loop;
		if (argc < 6)
		{
			MibWriteDebugString(cptr->usage);
			return FALSE;
		}
		if (!HexToInt(argv[3], &addr, 32))
		{
			ccprintf(_DEBUG_CMD_A_, ("Illugal character is used.\r\n"));
			return FALSE;
		}
		if (!HexToInt(argv[5], &v_Loop, 32))
		{
			ccprintf(_DEBUG_CMD_A_, ("Illugal character is used.\r\n"));
			return FALSE;
		}
		switch (argv[2][0])
		{
		case 'c':
			if (!HexToInt(argv[4], &c, 8))
			{
					ccprintf(_DEBUG_CMD_A_, ("Illugal character is used.\r\n"));
					return FALSE;
			}
			for (; v_Loop; v_Loop--) wr_ADDR8(addr, c);
			break;
		case 's':
			if (!HexToInt(argv[4], &s, 16))
			{
					ccprintf(_DEBUG_CMD_A_, ("Illugal character is used.\r\n"));
					return FALSE;
			}
			for (; v_Loop; v_Loop--) wr_ADDR16(addr, s);
			break;
		case 'l':
			if (!HexToInt(argv[4], &l, 32))
			{
					ccprintf(_DEBUG_CMD_A_, ("Illugal character is useqd.\r\n"));
					return FALSE;
			}
			for (; v_Loop; v_Loop--) wr_ADDR32(addr, l);
			break;
		default:
			MibWriteDebugString(cptr->usage);
			return FALSE;
		}	
	}
	else if (!strcmp(argv[1], "rdn")) // _CMD_MY_READ_
	{
		uint32_t v_Loop;
		if (argc < 5)
		{
			MibWriteDebugString(cptr->usage);
			return FALSE;
		}
		if (!HexToInt(argv[3], &addr, 32))
		{
			ccprintf(_DEBUG_CMD_A_, ("Illugal character is used.\r\n"));
			return FALSE;
		}
		if (!HexToInt(argv[4], &v_Loop, 32))
		{
			ccprintf(_DEBUG_CMD_A_, ("Illugal character is used.\r\n"));
			return FALSE;
		}
		ccprintf(_DEBUG_CMD_A_,("\taddress : 0x%08lx  ", addr));
		ccprintf(_DEBUG_CMD_A_,("loop : 0x%08lx  \r\n", v_Loop));
		ccprintf(_DEBUG_CMD_A_, ("value   : "));
		switch (argv[2][0])
		{
		case 'c':
			for (; v_Loop; v_Loop--)
			{
					c = rd_ADDR8(addr);
					if ((v_Loop & 0xfffff) == 0)
					{
						ccprintf(_DEBUG_CMD_A_,("0x%02x ", c));
					}
			}
			break;
		case 's':
			for (; v_Loop; v_Loop--)
			{
					s = rd_ADDR16(addr);
					if ((v_Loop & 0xfffff) == 0)
					{
						ccprintf(_DEBUG_CMD_A_,("0x%04x ", s));
					}
			}
			break;
		case 'l':
			for (; v_Loop; v_Loop--)
			{
					l = rd_ADDR32(addr);
					if ((v_Loop & 0xfffff) == 0)
					{
						ccprintf(_DEBUG_CMD_A_,("0x%08lx ", l));
					}
			}
			break;
		default:
			ccprintf(_DEBUG_CMD_A_, ("Error.\r\n"));
			return FALSE;
		}
		ccprintf(_DEBUG_CMD_A_, (".\r\n"));
	}
	else if (!strcmp(argv[1], "hdump")) 	// _CMD_MY_HEXDUMP_
	{
		if (argc < 4)
		{
			MibWriteDebugString(cptr->usage);
			return FALSE;
		}
		if (!HexToInt(argv[2], &addr, 32) || !HexToInt(argv[3], &len, 32))
		{
			ccprintf(_DEBUG_CMD_A_, ("Illugal character is used.\r\n"));
			return FALSE;
		}
		HexDump(addr, len);		
	}
	else if (!strcmp(argv[1], "cpy")) // _CMD_MY_MEMCPY_
	{
		if (argc < 5)
		{
			MibWriteDebugString(cptr->usage);
			return FALSE;
		}
		if (!HexToInt(argv[2], &dest, 32) || !HexToInt(argv[3], &src, 32) || !HexToInt(argv[4], &len, 32))
		{
			ccprintf(_DEBUG_CMD_A_, ("Illegal character is used.\r\n"));
			return FALSE;
		}
		memcpy((char *)dest, (char *)src, len);		
	}
	else if (!strcmp(argv[1], "cmp")) // _CMD_MY_MEMCMP_
	{
		if (argc < 5)
		{
			MibWriteDebugString(cptr->usage);
			return FALSE;
		}
		if (!HexToInt(argv[2], &dest, 32) || !HexToInt(argv[3], &src, 32) || !HexToInt(argv[4], &len, 32))
		{
			ccprintf(_DEBUG_CMD_A_, ("Illugal character is used.\r\n"));
			return FALSE;
		}

		if (memcmp((char *)dest, (char *)src, (len)) == 0) ccprintf(_DEBUG_CMD_A_, ("equil.\r\n"));
		else ccprintf(_DEBUG_CMD_A_, ("not equil.\r\n"));		
	}
	else if (!strcmp(argv[1], "set")) // _CMD_MY_MEMSET_
	{
		if (argc < 5)
		{
			MibWriteDebugString(cptr->usage);
			return FALSE;
		}
		if (!HexToInt(argv[2], &addr, 32) || !HexToInt(argv[3], &c, 8) || !HexToInt(argv[4], &len, 32))
		{
			ccprintf(_DEBUG_CMD_A_, ("Illugal character is used.\r\n"));
			return FALSE;
		}
		memset((void *)addr, c, len);
	}
	else
	{
		MibWriteDebugString(cptr->usage);
		return FALSE;
	}
	return TRUE;
}
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
#endif // __MIB_DEBUG_MEM__
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
