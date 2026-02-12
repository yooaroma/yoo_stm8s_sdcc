# 1. 초기화 (공백으로 시작하거나 첫 번째 디렉토리 지정)
SUBDIRS := 01_stm8s103_void

# 2. 추가 디렉토리들을 += 로 연결
SUBDIRS += 01_stm8s103_void
SUBDIRS += 02_stm8s103_led_blink
SUBDIRS += 03_stm8s103_led_key
SUBDIRS += 04_stm8s103_led_key_irq
SUBDIRS += 05_stm8s103_led_tim4
SUBDIRS += 06_stm8s103_led_tim4_irq
SUBDIRS += 07_stm8s103_uart_polling
SUBDIRS += 08_stm8s103_uart_rx_irq
SUBDIRS += 09_stm8s103_uart_rtx_irq
SUBDIRS += 10_stm8s103_uart_rx_tim4_irq
SUBDIRS += 11_stm8s103_vsprintf
SUBDIRS += 12_stm8s103_remap_flash
SUBDIRS += 13_stm8s103_eeprom
SUBDIRS += 14_stm8s103_awu
SUBDIRS += 15_stm8s103_beep
SUBDIRS += 16_stm8s103_iwdg
SUBDIRS += 17_stm8s103_wwdg
SUBDIRS += 18_stm8s103_tim2_pwm
SUBDIRS += 19_stm8s103_adc_pwm
SUBDIRS += 20_stm8s103_i2c_pcf8591
SUBDIRS += 21_stm8s103_i2c_lcd1602
SUBDIRS += 22_stm8s103_i2c_adxl345
SUBDIRS += 23_stm8s103_i2c_gy30_bh1750
SUBDIRS += 24_stm8s103_ds18b20_dtemp

# (선택 사항) 조건부 추가 예시
# ifeq ($(DEBUG), 1)
#     SUBDIRS += tests
#     SUBDIRS += debug_tools
# endif

# 기본 타겟
all: $(SUBDIRS)

# 각 하위 디렉토리 순회 빌드
$(SUBDIRS):
	@echo "========================================"
	@echo " Entering directory: $@"
	@echo "========================================"
	$(MAKE) -C $@

# clean 타겟
clean:
	@for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

# 디렉토리 이름이 파일명과 겹칠 경우를 대비한 .PHONY 설정
.PHONY: all $(SUBDIRS) clean