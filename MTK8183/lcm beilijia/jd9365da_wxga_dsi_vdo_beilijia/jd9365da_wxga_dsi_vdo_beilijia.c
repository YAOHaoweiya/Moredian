/*
 * Copyright (C) 2015 MediaTek Inc.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 */

//#define LOG_TAG "LCM"

#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/gpio.h>
#include <linux/pinctrl/consumer.h>
#endif

#ifdef BUILD_LK
#include <platform/mt_gpio.h>
#include <platform/mt_pmic.h>
#include <platform/mt_i2c.h>
#include <platform/upmu_common.h>
#include "ddp_hal.h"
#else
#include <mach/mt_gpio.h>
#include <mach/mt_pm_ldo.h>
#include <mach/upmu_hw.h>
#include <mach/upmu_common.h>
#include "ddp_hal.h"
#endif
#include "lcm_drv.h"

#define LCM_DEBUG (1)

#if LCM_DEBUG

#define LCM_PREFIX "jd9365da_beilijia"

#if defined(BUILD_LK)||defined(BUILD_UBOOT)
#define PRINT_FUN printf
#define LCM_LOG_LEVEL
#define LCM_MODULE_PART "lk"
#else
#define PRINT_FUN printk
#define LCM_LOG_LEVEL   KERN_ERR
#define LCM_MODULE_PART "kernel"
#endif

#define LCM_ALOGD(fmt, ...) \
    do { \
        PRINT_FUN(LCM_LOG_LEVEL "<<-" LCM_PREFIX "-dbg-%s>> [%04d] [@%s] " fmt "\n", LCM_MODULE_PART, __LINE__, __FUNCTION__,##__VA_ARGS__); \
    } while (0)

#define LCM_ALOGE(fmt, ...) \
    do { \
        PRINT_FUN(LCM_LOG_LEVEL "<<-" LCM_PREFIX "-err-%s>> [%04d] [@%s] " fmt "\n", LCM_MODULE_PART, __LINE__, __FUNCTION__,##__VA_ARGS__); \
    } while (0)

#define LCM_ALOGF() \
    do { \
        PRINT_FUN(LCM_LOG_LEVEL "<<-" LCM_PREFIX "-fun-%s>> [%04d] [@%s] %s() is call!\n", LCM_MODULE_PART, __LINE__, __FUNCTION__, __FUNCTION__); \
    } while (0)

#else

#define LCM_ALOGF()                      do {} while (0)
#define LCM_ALOGD(fmt, ...)              do {} while (0)
#define LCM_ALOGE(fmt, ...) \
    do { \
        PRINT_FUN(LCM_LOG_LEVEL "<<-" LCM_PREFIX "-err-%s>> [%04d] [@%s] " fmt "\n", LCM_MODULE_PART, __LINE__, __FUNCTION__,##__VA_ARGS__); \
    } while (0)

#endif

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------
#define FRAME_WIDTH  (800)//720
#define FRAME_HEIGHT (1280)



#define REGFLAG_DELAY                                       0xFFFC
#define REGFLAG_END_OF_TABLE                                0xFFFD
// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------
static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)	(lcm_util.set_reset_pin((v)))

#define UDELAY(n)                           (lcm_util.udelay(n))
#define MDELAY(n)                           (lcm_util.mdelay(n))

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------
#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)        lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)           lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)                                          lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)                      lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg                                                lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)                   lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)
#define dsi_set_cmdq_V3(para_tbl,size,force_update)             lcm_util.dsi_set_cmdq_V3(para_tbl,size,force_update)

//extern void DSI_clk_HS_mode(DISP_MODULE_ENUM module, void* cmdq, bool enter);

struct LCM_setting_table {
	unsigned int cmd;
	unsigned char count;
	unsigned char para_list[64];
};


static struct LCM_setting_table lcm_initialization_setting[] = {

			//========= JD9365DA HSD8p initial setting =========//

{0xE0,1,{0x00}},
{0xE1,1,{0x93}},
{0xE2,1,{0x65}},
{0xE3,1,{0xF8}},
{0x80,1,{0x03}},

{0xE0,1,{0x01}},

{0x00,1,{0x00}},
{0x01,1,{0x21}},
{0x03,1,{0x00}},
{0x04,1,{0x25}},

{0x17,1,{0x00}},
{0x18,1,{0xD7}},
{0x19,1,{0x01}},
{0x1A,1,{0x00}},
{0x1B,1,{0xD7}},
{0x1C,1,{0x01}},

{0x25,1,{0x20}},

{0x35,1,{0x23}},

{0x37,1,{0x09}},

{0x38,1,{0x04}},
{0x39,1,{0x08}},
{0x3A,1,{0x12}},
{0x3C,1,{0x78}},
{0x3D,1,{0xFF}},
{0x3E,1,{0xFF}},
{0x3F,1,{0xFF}},

{0x40,1,{0x06}},
{0x41,1,{0xA0}},
{0x43,1,{0x14}},
{0x44,1,{0x0F}},
{0x45,1,{0x30}},
{0x4B,1,{0x04}},

{0x55,1,{0x02}},//page 1的55下02 是2 POWER，0F是3 POWER
{0x57,1,{0x6D}},
{0x59,1,{0x0A}},
{0x5A,1,{0x28}},
{0x5B,1,{0x19}},


{0x5D,1,{0x70}},
{0x5E,1,{0x61}},
{0x5F,1,{0x54}},
{0x60,1,{0x49}},
{0x61,1,{0x46}},
{0x62,1,{0x37}},
{0x63,1,{0x3A}},
{0x64,1,{0x22}},
{0x65,1,{0x38}},
{0x66,1,{0x34}},
{0x67,1,{0x31}},
{0x68,1,{0x4D}},
{0x69,1,{0x39}},
{0x6A,1,{0x3F}},
{0x6B,1,{0x30}},
{0x6C,1,{0x2E}},
{0x6D,1,{0x23}},
{0x6E,1,{0x14}},
{0x6F,1,{0x02}},
{0x70,1,{0x70}},
{0x71,1,{0x61}},
{0x72,1,{0x54}},
{0x73,1,{0x49}},
{0x74,1,{0x46}},
{0x75,1,{0x37}},
{0x76,1,{0x3A}},
{0x77,1,{0x22}},
{0x78,1,{0x38}},
{0x79,1,{0x34}},
{0x7A,1,{0x31}},
{0x7B,1,{0x4D}},
{0x7C,1,{0x39}},
{0x7D,1,{0x3F}},
{0x7E,1,{0x30}},
{0x7F,1,{0x2E}},
{0x80,1,{0x23}},
{0x81,1,{0x14}},
{0x82,1,{0x02}},


{0xE0,1,{0x02}},

{0x00,1,{0x5E}},
{0x01,1,{0x5F}},
{0x02,1,{0x57}},
{0x03,1,{0x77}},
{0x04,1,{0x48}},
{0x05,1,{0x4A}},
{0x06,1,{0x44}},
{0x07,1,{0x46}},
{0x08,1,{0x40}},
{0x09,1,{0x5F}},
{0x0A,1,{0x5F}},
{0x0B,1,{0x5F}},
{0x0C,1,{0x5F}},
{0x0D,1,{0x5F}},
{0x0E,1,{0x5F}},
{0x0F,1,{0x42}},
{0x10,1,{0x5F}},
{0x11,1,{0x5F}},
{0x12,1,{0x5F}},
{0x13,1,{0x5F}},
{0x14,1,{0x5F}},
{0x15,1,{0x5F}},

{0x16,1,{0x5E}},
{0x17,1,{0x5F}},
{0x18,1,{0x57}},
{0x19,1,{0x77}},
{0x1A,1,{0x49}},
{0x1B,1,{0x4B}},
{0x1C,1,{0x45}},
{0x1D,1,{0x47}},
{0x1E,1,{0x41}},
{0x1F,1,{0x5F}},
{0x20,1,{0x5F}},
{0x21,1,{0x5F}},
{0x22,1,{0x5F}},
{0x23,1,{0x5F}},
{0x24,1,{0x5F}},
{0x25,1,{0x43}},
{0x26,1,{0x5F}},
{0x27,1,{0x5F}},
{0x28,1,{0x5F}},
{0x29,1,{0x5F}},
{0x2A,1,{0x5F}},
{0x2B,1,{0x5F}},

{0x2C,1,{0x1F}},
{0x2D,1,{0x1E}},
{0x2E,1,{0x17}},
{0x2F,1,{0x37}},
{0x30,1,{0x07}},
{0x31,1,{0x05}},
{0x32,1,{0x0B}},
{0x33,1,{0x09}},
{0x34,1,{0x03}},
{0x35,1,{0x1F}},
{0x36,1,{0x1F}},
{0x37,1,{0x1F}},
{0x38,1,{0x1F}},
{0x39,1,{0x1F}},
{0x3A,1,{0x1F}},
{0x3B,1,{0x01}},
{0x3C,1,{0x1F}},
{0x3D,1,{0x1F}},
{0x3E,1,{0x1F}},
{0x3F,1,{0x1F}},
{0x40,1,{0x1F}},
{0x41,1,{0x1F}},

{0x42,1,{0x1F}},
{0x43,1,{0x1E}},
{0x44,1,{0x17}},
{0x45,1,{0x37}},
{0x46,1,{0x06}},
{0x47,1,{0x04}},
{0x48,1,{0x0A}},
{0x49,1,{0x08}},
{0x4A,1,{0x02}},
{0x4B,1,{0x1F}},
{0x4C,1,{0x1F}},
{0x4D,1,{0x1F}},
{0x4E,1,{0x1F}},
{0x4F,1,{0x1F}},
{0x50,1,{0x1F}},
{0x51,1,{0x00}},
{0x52,1,{0x1F}},
{0x53,1,{0x1F}},
{0x54,1,{0x1F}},
{0x55,1,{0x1F}},
{0x56,1,{0x1F}},
{0x57,1,{0x1F}},

{0x58,1,{0x40}},
{0x59,1,{0x00}},
{0x5A,1,{0x00}},
{0x5B,1,{0x30}},
{0x5C,1,{0x05}},
{0x5D,1,{0x30}},
{0x5E,1,{0x01}},
{0x5F,1,{0x02}},
{0x60,1,{0x30}},
{0x61,1,{0x03}},
{0x62,1,{0x04}},
{0x63,1,{0x03}},
{0x64,1,{0x6A}},
{0x65,1,{0x75}},
{0x66,1,{0x0D}},
{0x67,1,{0x73}},
{0x68,1,{0x09}},
{0x69,1,{0x06}},
{0x6A,1,{0x6A}},
{0x6B,1,{0x08}},
{0x6C,1,{0x00}},
{0x6D,1,{0x0C}},
{0x6E,1,{0x04}},
{0x6F,1,{0x88}},
{0x70,1,{0x00}},
{0x71,1,{0x00}},
{0x72,1,{0x06}},
{0x73,1,{0x7B}},
{0x74,1,{0x00}},
{0x75,1,{0xBB}}, //80
{0x76,1,{0x00}},
{0x77,1,{0x0D}},
{0x78,1,{0x24}},
{0x79,1,{0x00}},
{0x7A,1,{0x00}},
{0x7B,1,{0x00}},
{0x7C,1,{0x00}},
{0x7D,1,{0x03}},
{0x7E,1,{0x7B}},


{0xE0,1,{0x04}},
{0x00,1,{0x0E}},
{0x02,1,{0xB3}},
{0x09,1,{0x60}},
{0x0E,1,{0x48}},

{0xE0,1,{0x03}},
{0x9B,1,{0x05}},
{0xAC,1,{0x66}},

{0xE0,1,{0x00}},
{0x53,1,{0x2C}},


{0x11,1,{0x00}},
{REGFLAG_DELAY,120,{}},

{0x29,1,{0x00}},
{REGFLAG_DELAY,5,{}},

{0x35,1,{0x00}},




{REGFLAG_END_OF_TABLE,0x00,{}}
};


static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

	for (i = 0; i < count; i++) {
		unsigned cmd;
		cmd = table[i].cmd;

		switch (cmd) {

			case REGFLAG_DELAY:
				if (table[i].count <= 10)
					MDELAY(table[i].count);
				else
					MDELAY(table[i].count);
				break;

			case REGFLAG_END_OF_TABLE:
				break;

			default:
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
		}
	}
}

// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------
static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
	memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}

static void lcm_get_params(LCM_PARAMS *params)
{
	LCM_ALOGF();

	memset(params, 0, sizeof(LCM_PARAMS));

	params->type = LCM_TYPE_DSI;

	params->width = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;

	params->dsi.mode = SYNC_PULSE_VDO_MODE;
	params->dsi.switch_mode = CMD_MODE;
	params->dsi.switch_mode_enable = 0;

	/* DSI */
	/* Command mode setting */
	params->dsi.LANE_NUM = LCM_FOUR_LANE;
	/* The following defined the fomat for data coming from LCD engine. */
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format = LCM_DSI_FORMAT_RGB888;

	/* Highly depends on LCD driver capability. */
	params->dsi.packet_size = 256;
	/* video mode timing */

	params->dsi.PS = LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.vertical_sync_active = 4;
	params->dsi.vertical_backporch = 12;
	params->dsi.vertical_frontporch = 20;
	params->dsi.vertical_frontporch_for_low_power = 620;
	params->dsi.vertical_active_line = FRAME_HEIGHT;

	params->dsi.horizontal_sync_active = 18;
	params->dsi.horizontal_backporch = 18;
	params->dsi.horizontal_frontporch = 18;
	params->dsi.horizontal_active_pixel = FRAME_WIDTH;
	
    params->dsi.PLL_CLOCK = 200;//MHZ(410Mbps)
	
	params->dsi.CLK_HS_POST = 36;
	params->dsi.clk_lp_per_line_enable = 0;
	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable = 0;
	//params->dsi.lcm_esd_check_table[0].cmd          = 0x0A;
	//params->dsi.lcm_esd_check_table[0].count        = 1;
	//params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9C;
}

static void lcm_power_init(void)
{
#ifdef BUILD_LK
	//release pwr key
/*	mt_set_gpio_mode(8, GPIO_MODE_00);
	mt_set_gpio_dir(8, GPIO_DIR_OUT);
	mt_set_gpio_out(8, GPIO_OUT_ONE);
	
	//usb sel
	mt_set_gpio_mode(5, GPIO_MODE_00);
	mt_set_gpio_dir(5, GPIO_DIR_OUT);
	mt_set_gpio_out(5, GPIO_OUT_ZERO);
	
	mt_set_gpio_mode(168, GPIO_MODE_00);
	mt_set_gpio_dir(168, GPIO_DIR_OUT);
	mt_set_gpio_out(168, GPIO_OUT_ZERO);
	*/

	// enable avdd 3.3v
	//mt_set_gpio_mode(100, GPIO_MODE_00);
	//mt_set_gpio_dir(100, GPIO_DIR_OUT);
	//mt_set_gpio_out(100, GPIO_OUT_ONE);
	printf("[LK/LCM] lcm_power_init enable lcm 3.3v\n");
	
	MDELAY(20);
	//lcm_Enable_HW(1800);
#endif
}

static void lcm_suspend_power(void)
{
#ifdef BUILD_LK
	printf("[LK/LCM] lcm_suspend_power() enter\n");
	//mt_set_gpio_mode(100, GPIO_MODE_00);
	//mt_set_gpio_dir(100, GPIO_DIR_OUT);
	//mt_set_gpio_out(100, GPIO_OUT_ZERO);
	MDELAY(20);

	//lcm_Disable_HW();
#endif
}

static void lcm_resume_power(void)
{
#ifdef BUILD_LK
	printf("[LK/LCM] lcm_resume_power() enter\n");
	//mt_set_gpio_mode(100, GPIO_MODE_00);
	//mt_set_gpio_dir(100, GPIO_DIR_OUT);
	//mt_set_gpio_out(100, GPIO_OUT_ONE);
	MDELAY(20);
	//lcm_Enable_HW(1800);
#endif
}

static void lcm_init(void)
{
#ifdef BUILD_LK
	printf("[LK/LCM] lcm_init\n");
#endif
	SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(20);
	SET_RESET_PIN(1);
	MDELAY(120);
	//lcm_init_register();
	push_table(lcm_initialization_setting,sizeof(lcm_initialization_setting)/sizeof(lcm_initialization_setting[0]),1);
	MDELAY(10);
}

static void lcm_suspend(void)
{
	unsigned int data_array[16];

	data_array[0] = 0x00280500;
	dsi_set_cmdq(data_array, 1, 1);

	SET_RESET_PIN(0);
	MDELAY(10);
}

static void lcm_resume(void)
{
	lcm_init();
}

extern u8 lcm_name2[2];
static unsigned int lcm_compare_id(void)
{
	unsigned char val_1 = 0;
	unsigned char val_2 = 0;
	unsigned int id = 0, version_id = 0, check_id = 0;
	unsigned char buffer[2];
	unsigned int array[16];
	
	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(1);
	SET_RESET_PIN(1);
	MDELAY(20);

	array[0] = 0x00023700;	/* read id return two byte,version and id */
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0xda, buffer, 1);
	id = buffer[0]; 	/* we only need ID */

	read_reg_v2(0xdb, buffer, 1);
	version_id = buffer[0];

	read_reg_v2(0xdc, buffer, 1);
	check_id = buffer[0];

	printf("%s, lizhi----beilijia_lcm----jd9365da_id=0x%x, version_id=0x%x, check_id=0x%x\n", __func__, id, version_id, check_id);
	printf("[lizhi:lcm_proinfo----lcm_name2]%x %x\n", lcm_name2[0], lcm_name2[1]);
	
	MDELAY(20);
	mt_set_gpio_mode(158, GPIO_MODE_00);
	//mt_set_gpio_dir(158, GPIO_DIR_OUT);
	//mt_set_gpio_out(158, GPIO_OUT_ONE);
	mt_set_gpio_dir(158, GPIO_DIR_IN); // GPIO_DIR_OUT
	mt_set_gpio_pull_enable(158, GPIO_PULL_ENABLE); // GPIO_PULL_DISABLE
	mt_set_gpio_pull_select(158, GPIO_PULL_UP); // GPIO_PULL_DOWN
	MDELAY(5);	
	val_1 = mt_get_gpio_in(158);
	printf("%s, lizhi----beilijia_lcm_jd9365da----val_1 = %d\n", __func__, val_1);
	MDELAY(5);	
	mt_set_gpio_pull_select(158, GPIO_PULL_DOWN);
	MDELAY(5);	
	val_2 = mt_get_gpio_in(158);
	printf("%s, lizhi----beilijia_lcm_jd9365da----val_2 = %d\n", __func__, val_2);
	
	if ((val_1 == 1) && (val_2 == 1)) {
		if ((id == 0x93) && (version_id == 0x65) && (check_id == 0x04)) {
			return 1;
		} else {
			return 0;
		}
	} else {
		return 0;
	}
}

static unsigned int lcm_esd_check(void)
{
#ifndef BUILD_LK
	unsigned char buffer[1];
	unsigned int data_array[16];
#ifdef BUILD_LK
	printf("[cabc] otm1287a: lcm_esd_check enter\n");
#else
	printk("[cabc] otm1287a: lcm_esd_check enter\n");
#endif

	data_array[0] = 0x00013700;// read id return two byte,version and id
	dsi_set_cmdq(data_array, 1, 1);
//	id = read_reg(0xF4);
	read_reg_v2(0x0A, buffer, 1);
#ifdef BUILD_LK
	printf("lcm_esd_check  0x0A = %x\n",buffer[0]);
#else
	printk("lcm_esd_check  0x0A = %x\n",buffer[0]);
#endif
	if (buffer[0] != 0x9C) {
		return 1;
	}

	data_array[0] = 0x00013700;// read id return two byte,version and id
	dsi_set_cmdq(data_array, 1, 1);
//	id = read_reg(0xF4);
	read_reg_v2(0x0D, buffer, 1);
#ifdef BUILD_LK
	printf("lcm_esd_check 0x0D =%x\n",buffer[0]);
#else
	printk("lcm_esd_check 0x0D =%x\n",buffer[0]);
#endif
	if (buffer[0] != 0x00) {
		return 1;
	}
	data_array[0] = 0x00013700;// read id return two byte,version and id
	dsi_set_cmdq(data_array, 1, 1);
//	id = read_reg(0xF4);
	read_reg_v2(0x0E, buffer, 1);
#ifdef BUILD_LK
	printf("lcm_esd_check  0x0E = %x\n",buffer[0]);
#else
	printk("lcm_esd_check  0x0E = %x\n",buffer[0]);
#endif
	if (buffer[0] != 0x80) {
		return 1;
	}

#ifdef BUILD_LK
	printf("[cabc] otm1287a: lcm_esd_check exit\n");
#else
	printk("[cabc] otm1287a: lcm_esd_check exit\n");
#endif

	return 0;
#else
	return 0;
#endif
}

static unsigned int lcm_esd_recover(void)
{
	unsigned int data_array[16];

#ifdef BUILD_LK
	printf("lcm_esd_recover enter");
#else
	printk("lcm_esd_recover enter");
#endif

	lcm_init();
	data_array[0]=0x00110500;
	dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(50);

	data_array[0]=0x00290500;
	dsi_set_cmdq(&data_array, 1, 1);

	data_array[0]= 0x00023902;
	data_array[1]= 0xFF51;
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(10);

	return TRUE;
}

LCM_DRIVER jd9365da_wxga_dsi_vdo_beilijia_lcm_drv = {
	.name           = "jd9365da_wxga_dsi_vdo_beilijia",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init_power     = lcm_power_init,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,
	.resume_power = lcm_resume_power,
	.suspend_power = lcm_suspend_power,
	.esd_check  = lcm_esd_check,
	.esd_recover    = lcm_esd_recover,
};
