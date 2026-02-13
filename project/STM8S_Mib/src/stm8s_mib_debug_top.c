/**
  ******************************************************************************
  * @file stm8s_top_main.c
  * @brief 
  * @author MYMEDIA Co., Ltd.
  * @version V1.0.0
  * @date 2023.1.6
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "stm8s.h"
#include "stm8s_mib.h"
#include  <string.h> 
// #include<memory.h> 도 괜찮습니다.
#include  <stdio.h>
// #include "stdlib.h"
#include <stdarg.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void CLK_Configuration(void);
void debug_main(void);
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
/* Private functions ---------------------------------------------------------*/
/* Global variables ----------------------------------------------------------*/
extern CMD_ARRAY cmd_list;
extern CMD_MY cmdTbl[];
/* Public functions ----------------------------------------------------------*/
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
#define _DEBUG_TOP_A_ 1
void main(void)
{
  {
    /* Configures clocks */
    /* Fmaster = 16MHz */
    // CLK_HSIPrescalerConfig(CLK_PRESCALER_HSIDIV1);  
    CLK->CKDIVR &= (uint8_t)(~CLK_CKDIVR_HSIDIV);  
    /* Set High speed internal clock prescaler */
    CLK->CKDIVR |= (uint8_t)CLK_PRESCALER_HSIDIV1;
  }
  {
    MibDebugInit(9600);
    timIrqInit();
    #if defined(__MIB_DEBUG_TEST__)
    {
      extern void cmd_test_init_before_irq_enable(void);
      cmd_test_init_before_irq_enable();
    }
    #endif
  }
  {
    enableInterrupts();
  }
  {
    // https://www.st.com/resource/en/datasheet/stm8s105c6.pdf (9 Unique ID / page 50)
    uint8_t *vpbUniqueID = (uint8_t *)(0x48cd);
    uint8_t vbIndexID; 
    uint8_t vbBufferID[13]; // total 12byte...
    for(vbIndexID=0;vbIndexID<12;vbIndexID++)
    {
      vbBufferID[vbIndexID] = *vpbUniqueID++;
    }
    vbBufferID[12] = 0;
    MibWriteDebugStringCheck(1, "\r\n ###################################\r\n");
    MibWriteDebugStringCheck(1, " file name : " __FILE__ "\r\n");
    MibWriteDebugStringCheck(1, " date :  "__DATE__"  :  "__TIME__"\r\n");
    MibWriteDebugStringCheck(1, " webgpio.com by MYMEDIA Co., Ltd.\r\n");
    MibWriteDebugStringCheck(1, " ###################################\r\n");    
    ccprintf(_DEBUG_TOP_A_, ("X co-ordinate on the wafer [%02X][%02X]\r\n",vbBufferID[0],vbBufferID[1]));
    ccprintf(_DEBUG_TOP_A_, ("Y co-ordinate on the wafer [%02X][%02X]\r\n",vbBufferID[2],vbBufferID[3]));
    ccprintf(_DEBUG_TOP_A_, ("Wafer number               [%02X]\r\n",vbBufferID[4]));
    ccprintf(_DEBUG_TOP_A_, ("Lot number [%s]\r\n",&(vbBufferID[5])));

    debug_main();
  }
  while(1);
}
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
void debug_main(void)
{
  char cmd[CMD_CHAR_MAX];
  char *cmd_str;
  int argc = 0;
  char *argv[MAX_ARGS];
  CMD_MY *cptr;
  {
    /* Copy kernel data to RAM & zero out BSS */
    cmd[0] = 0;
    cmd[1] = 0;
    ccprintf(_DEBUG_TOP_A_, ("\r\n [help or ?] to get a list of commands\r\n\r"));
    cmd_list.cur = 0;
    cmd_list.next = 0;
    memset((void *)(&cmd_list), 0, sizeof(cmd_list));
    {
      #if defined(__MIB_DEBUG_TOP__) || defined(__MIB_DEBUG_MEM__)
      {
        cmd_mem();
      }
      #endif
      #if defined(__MIB_DEBUG_TEST__)
      {
        extern void cmd_test(void);
        cmd_test();
      }
      #endif
    }
    for (;;)
    {
      ccprintf(1,("My>"));
      // wait an hour to get a command.
      GetCommand(cmd, CMD_CHAR_MAX - 1, 60*3);
      if (!cmd || !cmd[0]) continue;
      cmd_str = (char *)cmd;
      argc = GetArgs(cmd_str, argv);
      for (cptr = cmdTbl; cptr->cmd; cptr++)
      {
        if (!strcmp(argv[0], cptr->cmd))
        {
          (cptr->run)(cptr, argc, argv);
          break;
        }
      }
      if (!strcmp(argv[0], "help") || !strcmp(argv[0], "?"))
      {
        DoPrintHelp(argc, argv);
      }
      if (!strcmp(argv[0], "q") || !strcmp(argv[0], "Q"))
      {
        ccprintf(_DEBUG_TOP_A_,("\r\nmonitor program end!!!\r\n"));
        break;
      }
    }
  }
}
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------
