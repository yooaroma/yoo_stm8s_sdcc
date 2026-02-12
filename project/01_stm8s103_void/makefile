# makefile 
# yooaroma : 20230104 
# https://www.st.com/en/evaluation-tools/stm8s-discovery.html
# https://www.st.com/en/microcontrollers-microprocessors/stm8s105c6.html
# https://www.st.com/resource/en/datasheet/stm8s105c6.pdf
# https://www.st.com/en/microcontrollers-microprocessors/stm8s103f3.html
# https://www.st.com/resource/en/datasheet/stm8s103f3.pdf
# https://www.st.com/resource/en/reference_manual/rm0016-stm8s-series-and-stm8af-series-8bit-microcontrollers-stmicroelectronics.pdf
# https://www.st.com/resource/en/programming_manual/pm0044-stm8-cpu-programming-manual-stmicroelectronics.pdf
# https://github.com/yooaroma/yoo_stm8s
# https://yooaroma.com/mzip/stm8s_chip.pdf
# https://yooaroma.com/mzip/stm8s_pgm.pdf
# https://yooaroma.com/mzip/stm8s_ref.pdf
# https://yooaroma.com/mzip/stm8s_yoo.pdf

PRJ_NUM = 01
PRJ = stm8s103_void
PRJ_DIR = $(PRJ_NUM)_$(PRJ)
CC = sdcc
FLASH_SIZE = 8192

#do not edit
TOP_DIR = ./../..
OBJ_DIR = ./obj

SRC = $(PRJ)_main.c
# SRC += stm8s_adc1.c
# SRC += stm8s_adc2.c
# SRC += stm8s_awu.c
# SRC += stm8s_beep.c
# SRC += stm8s_can.c
# SRC += stm8s_clk.c
# SRC += stm8s_exti.c
# SRC += stm8s_flash.c
# SRC += stm8s_gpio.c
# SRC += stm8s_i2c.c
# SRC += stm8s_itc.c
# SRC += stm8s_iwdg.c
# SRC += stm8s_rst.c
# SRC += stm8s_spi.c
# SRC += stm8s_tim1.c
# SRC += stm8s_tim2.c
# SRC += stm8s_tim3.c
# SRC += stm8s_tim4.c
# SRC += stm8s_tim5.c
# SRC += stm8s_tim6.c
# SRC += stm8s_uart1.c
# SRC += stm8s_uart2.c
# SRC += stm8s_uart3.c
# SRC += stm8s_uart4.c
# SRC += stm8s_wwdg.c
# SRC += stm8s_mib_adxl345.c
SRC += stm8s_mib_debug.c
# SRC += stm8s_mib_debug_cmd.c
# SRC += stm8s_mib_debug_mem.c
# SRC += stm8s_mib_debug_tim4.c
# SRC += stm8s_mib_debug_top.c
# SRC += stm8s_mib_ds18b20_dtemp.c
# SRC += stm8s_mib_eeprom.c
# SRC += stm8s_mib_flash.c
# SRC += stm8s_mib_flash_option.c
# SRC += stm8s_mib_gy30_bh1750.c
# SRC += stm8s_mib_gy521_mpu6050.c
# SRC += stm8s_mib_i2c.c
SRC += stm8s_mib_it.c
# SRC += stm8s_mib_lcd1602.c
# SRC += stm8s_mib_pcf8591.c
# SRC += stm8s_mib_vsprintf.c

VPATH = $(TOP_DIR)/project/$(PRJ_DIR)/src
VPATH += $(TOP_DIR)/STM8S_Mib/src
VPATH += $(TOP_DIR)/STM8S_StdPeriph_Driver/src

OBJS = $(patsubst %.c,$(OBJ_DIR)/%.rel,$(SRC))
C_DEFINE = -mstm8
C_DEFINE += -DSTM8S103 
C_DEFINE += -D_$(PRJ)_
C_DEFINE += -I$(TOP_DIR)/STM8S_Mib/inc
C_DEFINE += -I$(TOP_DIR)/STM8S_StdPeriph_Driver/inc
C_DEFINE += -I$(TOP_DIR)/project/$(PRJ_DIR)/src
C_DEFINE += --debug
C_DEFINE += --out-fmt-ihx
C_DEFINE += --all-callee-saves
C_DEFINE += --stack-auto
C_DEFINE += --opt-code-size



all: $(OBJS)
	$(CC) -mstm8 $(OBJS) -o $(OBJ_DIR)/$(PRJ).ihx
	packihx $(OBJ_DIR)/$(PRJ).ihx > $(OBJ_DIR)/$(PRJ).hex
	cp $(OBJ_DIR)/$(PRJ).hex $(TOP_DIR)/project/bin/$(PRJ_DIR).hex
	./../check_size.sh
#	makebin -s $(FLASH_SIZE) $(PRJ).ihx $(PRJ).bin
# 디렉토리가 없을 경우 생성

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.rel: %.c
	$(CC) -c $(C_DEFINE) $< -o $@

clean:
	rm -f $(OBJ_DIR)/*.*

flash: all
	stm8flash -c stlinkv2 -p stm8s103f3 -w $(TOP_DIR)/project/bin/$(PRJ_DIR).hex

