
/*
 * board/bananapi/m2pro/firmware/scp_task/pwr_ctrl.c
 *
 * Copyright (C) 2018 Amlogic, Inc. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include "g12a_generic_pwm_ctrl.h"
#include "pwm_ctrl.h"

#include <g12a_generic_pwr_ctrl.c>

static void bananapi_set_vddee_voltage(unsigned int target_voltage)
{
	unsigned int to;

	for (to = 0; to < ARRAY_SIZE(pwm_voltage_table_ee); to++) {
		if (pwm_voltage_table_ee[to][1] >= target_voltage) {
			break;
		}
	}

	if (to >= ARRAY_SIZE(pwm_voltage_table_ee)) {
		to = ARRAY_SIZE(pwm_voltage_table_ee) - 1;
	}

	writel(pwm_voltage_table_ee[to][0],AO_PWM_PWM_B);
}

static void bananapi_power_off_at_24M(unsigned int suspend_from)
{
	/* set gpioE_2 output/low to power off sd vddio */
	//writel(readl(AO_GPIO_O) & (~(1 << 18)), AO_GPIO_O);
	//writel(readl(AO_GPIO_O_EN_N) & (~(1 << 18)), AO_GPIO_O_EN_N);
	//writel(readl(AO_RTI_PIN_MUX_REG1) & (~(0xf << 24)), AO_RTI_PIN_MUX_REG1);

	/* set gpioH_8 low to power off sd vdd */
	writel(readl(PREG_PAD_GPIO3_EN_N) & (~(1 << 8)), PREG_PAD_GPIO3_EN_N);
	writel(readl(PERIPHS_PIN_MUX_C) & (~(0xf)), PERIPHS_PIN_MUX_C);

	/* set gpioAO_6 output/low to set sd 3v3_1v8_en to 3v3 default */
	//writel(readl(AO_GPIO_O) & (~(1 << 6)), AO_GPIO_O);
	//writel(readl(AO_GPIO_O_EN_N) & (~(1 << 6)), AO_GPIO_O_EN_N);
	//writel(readl(AO_RTI_PIN_MUX_REG) & (~(0xf << 24)), AO_RTI_PIN_MUX_REG);

	if (!enable_5V_system_power.val) {
		/*set gpioH_6 low to power off vcc 5v*/
		writel(readl(PREG_PAD_GPIO3_O) & (~(1 << 6)), PREG_PAD_GPIO3_O);
		writel(readl(PREG_PAD_GPIO3_EN_N) & (~(1 << 6)), PREG_PAD_GPIO3_EN_N);
		writel(readl(PERIPHS_PIN_MUX_B) & (~(0xf << 24)), PERIPHS_PIN_MUX_B);
	}

	if (!enable_wol.val) {
		/*set test_n low to power off vcck_b & vcc 3.3v*/
		writel(readl(AO_GPIO_O) & (~(1 << 31)), AO_GPIO_O);
		writel(readl(AO_GPIO_O_EN_N) & (~(1 << 31)), AO_GPIO_O_EN_N);
		writel(readl(AO_RTI_PIN_MUX_REG1) & (~(0xf << 28)), AO_RTI_PIN_MUX_REG1);
	}

	/*step down ee voltage*/
	bananapi_set_vddee_voltage(CONFIG_VDDEE_SLEEP_VOLTAGE);
}

static void bananapi_power_on_at_24M(unsigned int suspend_from)
{
	/*step up ee voltage*/
	bananapi_set_vddee_voltage(CONFIG_VDDEE_INIT_VOLTAGE);

	/* set gpioE_2 output/high to power on sd vddio */
	//writel(readl(AO_GPIO_O) | (1 << 18), AO_GPIO_O);
	//writel(readl(AO_GPIO_O_EN_N) & (~(1 << 18)), AO_GPIO_O_EN_N);
	//writel(readl(AO_RTI_PIN_MUX_REG1) & (~(0xf << 24)), AO_RTI_PIN_MUX_REG1);

	/* set gpioH_8 output/external pull high to power on sd vdd */
	writel(readl(PREG_PAD_GPIO3_EN_N) | (1 << 8), PREG_PAD_GPIO3_EN_N);
	writel(readl(PERIPHS_PIN_MUX_C) & (~(0xf)), PERIPHS_PIN_MUX_C);

	/* set gpioAO_6 output/low to set sd 3v3_1v8_en to 3v3 default */
	//writel(readl(AO_GPIO_O) & (~(1 << 6)), AO_GPIO_O);
	//writel(readl(AO_GPIO_O_EN_N) & (~(1 << 6)), AO_GPIO_O_EN_N);
	//writel(readl(AO_RTI_PIN_MUX_REG) & (~(0xf << 24)), AO_RTI_PIN_MUX_REG);

	if (!enable_wol.val) {
		/*set test_n high to power on vcck_b & vcc 3.3v*/
		writel(readl(AO_GPIO_O) | (1 << 31), AO_GPIO_O);
		writel(readl(AO_GPIO_O_EN_N) & (~(1 << 31)), AO_GPIO_O_EN_N);
		writel(readl(AO_RTI_PIN_MUX_REG1) & (~(0xf << 28)), AO_RTI_PIN_MUX_REG1);
		_udelay(100);
	}

	if (!enable_5V_system_power.val) {
		/*set gpioH_6 high to power on vcc 5v*/
		writel(readl(PREG_PAD_GPIO3_O) | (1 << 6), PREG_PAD_GPIO3_O);
		writel(readl(PREG_PAD_GPIO3_EN_N) & (~(1 << 6)), PREG_PAD_GPIO3_EN_N);
		writel(readl(PERIPHS_PIN_MUX_B) & (~(0xf << 24)), PERIPHS_PIN_MUX_B);
	}

	_udelay(10000);
}

static unsigned int bananapi_detect_key(unsigned int suspend_from)
{
	int exit_reason = 0;
	unsigned *irq = (unsigned *)WAKEUP_SRC_IRQ_ADDR_BASE;
#ifdef CONFIG_ADC_KEY
	unsigned char adc_key_cnt = 0;
#endif
#ifdef CONFIG_GPIO_WAKEUP
	unsigned int is_gpiokey = 0;
#endif

	backup_remote_register();
	init_remote();
#ifdef CONFIG_CEC_WAKEUP
		if (hdmi_cec_func_config & 0x1) {
			remote_cec_hw_reset();
			cec_node_init();
		}
#endif

#ifdef CONFIG_GPIO_WAKEUP
	is_gpiokey = init_gpio_key();
#endif

	do {
#ifdef CONFIG_CEC_WAKEUP
		if (!cec_msg.log_addr)
			cec_node_init();
		else {
			if (readl(AO_CECB_INTR_STAT) & CECB_IRQ_RX_EOM) {
				if (cec_power_on_check())
					exit_reason = CEC_WAKEUP;
			}
		}
#endif
		if (irq[IRQ_AO_IR_DEC] == IRQ_AO_IR_DEC_NUM) {
			irq[IRQ_AO_IR_DEC] = 0xFFFFFFFF;
			if (remote_detect_key())
				exit_reason = REMOTE_WAKEUP;
		}

		if (irq[IRQ_VRTC] == IRQ_VRTC_NUM) {
			irq[IRQ_VRTC] = 0xFFFFFFFF;
			exit_reason = RTC_WAKEUP;
		}

#if defined(CONFIG_WOL) || defined(CONFIG_BT_WAKEUP)
		if (enable_wol.val && (irq[IRQ_GPIO1] == CONFIG_WOL_IRQ)) {
			irq[IRQ_GPIO1] = 0xFFFFFFFF;
#ifdef CONFIG_WOL
			if (!(readl(PREG_PAD_GPIO4_I) & (0x01 << CONFIG_WOL))
					&& (readl(PREG_PAD_GPIO4_EN_N) & (0x01 << CONFIG_WOL)))
				exit_reason = ETH_PHY_GPIO;
#endif
#ifdef CONFIG_BT_WAKEUP
			if (!(readl(PREG_PAD_GPIO2_I) & (0x01 << CONFIG_BT_WAKEUP))
				&& (readl(PREG_PAD_GPIO2_O) & (0x01 << 17))
				&& !(readl(PREG_PAD_GPIO2_EN_N) & (0x01 << 17)))
				exit_reason = BT_WAKEUP;
#endif
		}
#endif

#ifdef CONFIG_ADC_KEY
		if (irq[IRQ_AO_TIMERA] == IRQ_AO_TIMERA_NUM) {
			irq[IRQ_AO_TIMERA] = 0xFFFFFFFF;
			saradc_enable();
			if (check_adc_key_resume()) {
				adc_key_cnt++;
				/*using variable 'adc_key_cnt' to eliminate the dithering of the key*/
				if (2 == adc_key_cnt)
					exit_reason = POWER_KEY_WAKEUP;
			} else {
				adc_key_cnt = 0;
			}
			saradc_disable();
		}
#endif

#ifdef CONFIG_POWER_BUTTON
		if (irq[IRQ_AO_GPIO0] == CONFIG_POWER_BUTTON_IRQ) {
			irq[IRQ_AO_GPIO0] = 0xFFFFFFFF;
			if ((readl(AO_GPIO_I) & (1<<CONFIG_POWER_BUTTON)) == 0)
				exit_reason = POWER_KEY_WAKEUP;
		}
#endif

		if (irq[IRQ_ETH_PTM] == IRQ_ETH_PMT_NUM) {
			irq[IRQ_ETH_PTM]= 0xFFFFFFFF;
			exit_reason = ETH_PMT_WAKEUP;
		}

#ifdef CONFIG_GPIO_WAKEUP
		if (is_gpiokey) {
			if (gpio_detect_key())
				exit_reason = GPIO_WAKEUP;
		}
#endif
		if (exit_reason)
			break;
		else
			__switch_idle_task();
	} while (1);

	restore_remote_register();

	return exit_reason;
}

static void pwr_op_init(struct pwr_op *pwr_op)
{
	g12a_generic_pwr_op_init(pwr_op);
	pwr_op->power_off_at_24M = bananapi_power_off_at_24M;
	pwr_op->power_on_at_24M = bananapi_power_on_at_24M;
	pwr_op->detect_key = bananapi_detect_key;
}
