#include <linux/i2c.h>
#include <linux/bcd.h>
#include <linux/rtc.h>
#include <linux/module.h>

#define PRINT_ERR         (1U << 0)
#define PRINT_INFO        (1U << 1)
#define PRINT_DBG         (1U << 2)

static int g_debug_mask = PRINT_ERR | PRINT_INFO;
#define md_prt(debug_level, fmt, args...) \
do {\
    if (g_debug_mask & PRINT_##debug_level) \
        printk(KERN_INFO "rtc:sd25xx>>%s>>"fmt"\n", __func__, ##args); \
} while (0)


#define SD25XX_TIME_REG_NUM (7)
#define SD25XX_VBAT_REG_NUM (2)
#define SD25XX_ID_REG_NUM   (8)

// SD-25xx Basic Time and Calendar Register definitions
#define SD25xx_SEC					0x00
#define SD25xx_MIN					0x01
#define SD25xx_HOUR				    0x02
#define SD25xx_WEEK				    0x03
#define SD25xx_DAY					0x04
#define SD25xx_MONTH				0x05
#define SD25xx_YEAR				    0x06

#define SD25xx_ALARM_SEC			0x07
#define SD25xx_ALARM_MIN			0x08
#define SD25xx_ALARM_HOUR			0x09
#define SD25xx_ALARM_WEEK			0x0A
#define SD25xx_ALARM_DAY			0x0B
#define SD25xx_ALARM_MONTH		    0x0C
#define SD25xx_ALARM_YEAR			0x0D

#define SD25xx_ALARM_EN			0x0E
#define SD25XX_ALARM_EAY	(1 << 6)
#define SD25xx_ALARM_EAMO	(1 << 5)
#define SD25xx_ALARM_EAD	(1 << 4)
#define SD25xx_ALARM_EAW	(1 << 3)
#define SD25xx_ALARM_EAH	(1 << 2)
#define SD25xx_ALARM_EAMN	(1 << 1)
#define SD25xx_ALARM_EAS	(1 << 0)

#define SD25xx_CTR1				0x0F	/* Control register 1 */
#define SD25xx_CTR1_WRTC3	(1 << 7)
#define SD25xx_CTR1_INTAF	(1 << 5)
#define SD25xx_CTR1_INTDF	(1 << 4)
#define SD25xx_CTR1_WRTC2	(1 << 2)
#define SD25xx_CTR1_RTCF	(1 << 0)

#define SD25xx_CTR2				0x10
#define SD25xx_CTR2_WRTC1	(1 << 7)
#define SD25xx_CTR2_IM		(1 << 6)
#define SD25xx_CTR2_INTS1	(1 << 5)
#define SD25xx_CTR2_INTS0	(1 << 4)
#define SD25xx_CTR2_FOBAT	(1 << 3)
#define SD25xx_CTR2_INTDE	(1 << 2)
#define SD25xx_CTR2_INTAE	(1 << 1)
#define SD25xx_CTR2_INTFE	(1 << 0)

#define SD25xx_CTR3				0x11
#define SD25xx_CTR3_ARST	(1 << 7)
#define SD25xx_CTR3_TDS1	(1 << 5)
#define SD25xx_CTR3_TDS0	(1 << 4)
#define SD25xx_CTR3_FS3		(1 << 3)
#define SD25xx_CTR3_FS2		(1 << 2)
#define SD25xx_CTR3_FS1		(1 << 1)
#define SD25xx_CTR3_FS0		(1 << 0)

#define SD25xx_CTR5				0x1A
#define SD25xx_CTR5_BAT8_VAL	(1 << 7)

#define SD25xx_BAT_VAL				0x1B
#define SD25xx_CTR5_BAT7_VAL	(1 << 7)
#define SD25xx_CTR5_BAT6_VAL	(1 << 6)
#define SD25xx_CTR5_BAT5_VAL	(1 << 5)
#define SD25xx_CTR5_BAT4_VAL	(1 << 4)
#define SD25xx_CTR5_BAT3_VAL	(1 << 3)
#define SD25xx_CTR5_BAT2_VAL	(1 << 2)
#define SD25xx_CTR5_BAT1_VAL	(1 << 1)
#define SD25xx_CTR5_BAT0_VAL	(1 << 0)

#define	SD25xx_ID_1st				0x72


#define READ_RTC_INFO 0

enum Freq{F_0Hz, F_32KHz, F_4096Hz, F_1024Hz, F_64Hz, F_32Hz, F_16Hz, F_8Hz,
			F_4Hz, F_2Hz, F_1Hz, F1_2Hz, F1_4Hz, F1_8Hz, F1_16Hz, F_1s};

static struct i2c_driver sd2058_driver;
/* static unsigned short ignore[]      = { I2C_CLIENT_END }; */
/* static unsigned short normal_addr[] = { 0x32, I2C_CLIENT_END }; /1* 地址值是7位 *1/ */

//----------------------------------------------------------------------
// SD25xx_read_reg()
// reads a SD25xx register (see Register defines)
// See also SD25xx_read_regs() to read multiple registers.
//
//----------------------------------------------------------------------
static int SD25xx_read_reg(struct i2c_client *client, u8 addr)
{
	int ret = i2c_smbus_read_byte_data(client, addr) ;
		
	//check for error
	if (ret < 0) {
		dev_err(&client->dev, "Unable to read register #%d\n", addr);		
	}

	return ret;
}

//----------------------------------------------------------------------
// SD25xx_get_ID()
// gets the ID from the SD25xx registers
//
//----------------------------------------------------------------------
static int SD25xx_get_ID(struct i2c_client *client)
{
	int ret;
	u8 buf[8];

    ret = i2c_smbus_read_i2c_block_data(client, SD25xx_ID_1st, SD25XX_ID_REG_NUM, buf);
    if (SD25XX_ID_REG_NUM != ret)
    {
        printk("%s: i2c failed, ret=%d.\n", __func__, ret);
        return ret;
    }

	dev_info(&client->dev, "%s: %02x-%02x-%02x-%02x-%02x-%02x-%02x-%02x\n",
            __func__,buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);
	return ret;
}

//----------------------------------------------------------------------
// SD25xx_get_VBAT()
// gets the the battery voltage from the SD25xx registers
//
//----------------------------------------------------------------------
static int SD25xx_get_VBAT(struct i2c_client *client)
{
	int ret;
	u8 Vbat_buf[2];
	int Vbat_val;

    ret = i2c_smbus_read_i2c_block_data(client, SD25xx_CTR5, SD25XX_VBAT_REG_NUM, Vbat_buf);
    if (SD25XX_VBAT_REG_NUM != ret)
    {
        printk("%s: i2c failed, ret=%d.\n", __func__, ret);
        return ret;
    }

	Vbat_val = (Vbat_buf[0] >> 7) * 256 + Vbat_buf[1];
	dev_info(&client->dev, "%s: SD25xx_VBAT = %d.%d%dV\n",
		__func__,Vbat_val / 100, (Vbat_val % 100) / 10, Vbat_val % 10);
	return ret;
}

//----------------------------------------------------------------------
// SD25xx_get_time()
// gets the current time from the SD25xx registers
//
//----------------------------------------------------------------------
static int SD25xx_get_time(struct device *dev, struct rtc_time *dt)
{
	//struct SD25xx_data *SD25xx = dev_get_drvdata(dev);
	u8 date[7];
	int ret;
	struct i2c_client *client = to_i2c_client(dev);

    ret = i2c_smbus_read_i2c_block_data(client, SD25xx_SEC, SD25XX_TIME_REG_NUM, date);
    if (SD25XX_TIME_REG_NUM != ret)
    {
        md_prt(ERR, "i2c failed, ret=%d.", ret);
        return -1;
    }

	md_prt(DBG,  "read 0x%02x 0x%02x "
		   "0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n",
		   date[0], date[1], date[2], date[3], date[4], date[5], date[6]);

	dt->tm_sec  = bcd2bin(date[SD25xx_SEC] & 0x7f);
	dt->tm_min  = bcd2bin(date[SD25xx_MIN] & 0x7f);
	dt->tm_hour = bcd2bin(date[SD25xx_HOUR] & 0x3f);
	dt->tm_wday = bcd2bin(date[SD25xx_WEEK] & 0x7f );
	dt->tm_mday = bcd2bin(date[SD25xx_DAY] & 0x3f);
	dt->tm_mon  = bcd2bin(date[SD25xx_MONTH] & 0x1f) - 1;
	dt->tm_year = bcd2bin(date[SD25xx_YEAR]);
	
	if (dt->tm_year < 70)
		dt->tm_year += 100;

	md_prt(INFO, "date %d:%d:%d  %d-%d-%d  weekday:%d, ret=%d", 
           dt->tm_hour, dt->tm_min, dt->tm_sec, dt->tm_year + 1900, dt->tm_mon,
           dt->tm_mday, dt->tm_wday, rtc_valid_tm(dt));

	return rtc_valid_tm(dt);
}


//----------------------------------------------------------------------
// i2c_write_reg()
// writes a byte
//----------------------------------------------------------------------
static int i2c_write_byte(struct i2c_client *client, u8 addr, u8 value)
{
	int ret = i2c_smbus_write_byte_data(client, addr, value);

	//check for error
	if (ret)
		md_prt(ERR, "Unable to write register #%d", addr);

	return ret;
}

//----------------------------------------------------------------------
// SD25xx_write_enable()
// sets SD25xx write enable
//----------------------------------------------------------------------
static int SD25xx_write_enable(struct i2c_client *client)
{
	int ret;

	ret = SD25xx_read_reg(client, SD25xx_CTR2);
    if (ret < 0)
    {
		md_prt(ERR, "Unable to read register #%d", SD25xx_CTR2);
        return -1;
    }

	ret = ret | SD25xx_CTR2_WRTC1;
	i2c_write_byte(client, SD25xx_CTR2, ret);

	ret = SD25xx_read_reg(client, SD25xx_CTR1);
    if (ret < 0)
    {
		md_prt(ERR, "Unable to read register #%d", SD25xx_CTR1);
        return -1;
    }

	ret = ret | SD25xx_CTR1_WRTC3 | SD25xx_CTR1_WRTC2;
	i2c_write_byte(client, SD25xx_CTR1, ret);

    return 0;
}

//----------------------------------------------------------------------
// SD25xx_write_disable()
// sets SD25xx write disable
//----------------------------------------------------------------------
static int SD25xx_write_disable(struct i2c_client *client)
{
	char ret;

	ret = SD25xx_read_reg(client, SD25xx_CTR1);
    if (ret < 0)
    {
		md_prt(ERR, "Unable to read register #%d", SD25xx_CTR1);
        return -1;
    }

	ret = ret & (~SD25xx_CTR1_WRTC3) & (~SD25xx_CTR1_WRTC2);
	i2c_write_byte(client, SD25xx_CTR1, ret);

	ret = SD25xx_read_reg(client, SD25xx_CTR2);
    if (ret < 0)
    {
		md_prt(ERR, "Unable to read register #%d", SD25xx_CTR2);
        return -1;
    }

	ret = ret & (~SD25xx_CTR2_WRTC1);
	i2c_write_byte(client, SD25xx_CTR2, ret);

    return 0;
}

//----------------------------------------------------------------------
// SD25xx_write_regs()
// writes a specified number of SD25xx registers (see Register defines)
// See also SD25xx_write_reg() to write a single register.
//
//----------------------------------------------------------------------
static int SD25xx_write_regs(struct i2c_client *client, u8 addr, u8 length, u8 *values)
{
	int ret, res;

	ret = SD25xx_write_enable(client);
    if (ret < 0)
    {
		md_prt(ERR, "SD25xx_write_enable failed");
        return -1;
    }

	res = i2c_smbus_write_i2c_block_data(client, addr, length, values);
	//check for error
	if (res)
		md_prt(ERR, "Unable to write registers #%d..#%d", addr, addr + length - 1);

	ret = SD25xx_write_disable(client);
    if (ret < 0)
    {
		md_prt(ERR, "SD25xx_write_disable failed");
        return -1;
    }

	return res;
}

//----------------------------------------------------------------------
// SD25xx_set_time()
// Sets the current time in the SD25xx registers
//
//----------------------------------------------------------------------
static int SD25xx_set_time(struct device *dev, struct rtc_time *dt)
{
	u8 date[7];
	int ret = 0;
	struct i2c_client *client = to_i2c_client(dev);
	
	date[SD25xx_SEC]   = bin2bcd(dt->tm_sec);
	date[SD25xx_MIN]   = bin2bcd(dt->tm_min);
	date[SD25xx_HOUR]  = bin2bcd(dt->tm_hour) | 0x80;		
	date[SD25xx_WEEK]  = bin2bcd(dt->tm_wday);
	date[SD25xx_DAY]   = bin2bcd(dt->tm_mday);
	date[SD25xx_MONTH] = bin2bcd(dt->tm_mon + 1);
	date[SD25xx_YEAR]  = bin2bcd(dt->tm_year % 100);

	ret =  SD25xx_write_regs(client, SD25xx_SEC, 7, date);

	md_prt(INFO, "write 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x, ret=%d\n",
		   date[0], date[1], date[2], date[3], date[4], date[5], date[6], ret);

	return ret;
}

//----------------------------------------------------------------------
// SD25xx_set_Freq(enum Freq F_Out)
// Sets the Frequency interrupt in the SD25xx registers
//
//----------------------------------------------------------------------
#if 0
static int SD25xx_set_Freq(enum Freq F_Out, struct i2c_client *client)
{
	u8 buf[2];
	int ret;
	
	buf[0] = SD25xx_CTR2_WRTC1|SD25xx_CTR2_INTS1|SD25xx_CTR2_INTFE;
	buf[1] = F_Out;
	ret =  SD25xx_write_regs(client, SD25xx_CTR2, 2, buf);

	return ret;
}

//----------------------------------------------------------------------
// SD25xx_set_alarm_time(struct rtc_time *t)
// Sets the AlarmTime in the SD25xx registers
//
//----------------------------------------------------------------------
static int SD25xx_set_alarm_time(struct rtc_time *t)
{
	u8 date[7];
	int ret = 0;
	
	date[0] = bin2bcd(t->tm_sec);
	date[1] = bin2bcd(t->tm_min);
	date[2] = bin2bcd(t->tm_hour);		
	date[3] = bin2bcd(t->tm_wday);
	date[4] = bin2bcd(t->tm_mday);
	date[5] = bin2bcd(t->tm_mon);
	date[6] = bin2bcd(t->tm_year % 100);

	ret =  SD25xx_write_regs(SD25xx_client, SD25xx_ALARM_SEC, 7, date);

	return ret;
}

//----------------------------------------------------------------------
// SD25xx_set_alarm_enable(struct rtc_time *t)
// Sets the AlarmTime in the SD25xx registers
//
//----------------------------------------------------------------------
static int SD25xx_set_alarm_enable(u8 dat)
{
	u8 buf;
	int ret;
	ret =  SD25xx_write_regs(SD25xx_client, SD25xx_ALARM_EN, 1, &dat);
	buf = SD25xx_CTR2_WRTC1 | SD25xx_CTR2_INTS0 | SD25xx_CTR2_INTAE;
	ret =  SD25xx_write_regs(SD25xx_client, SD25xx_CTR2, 1, &buf);

	return ret;
}
#endif

static ssize_t SD25XX_dbg_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    /* char *s = buf; */
    /* s += sprintf(s,"debug level: 0x%02x\n", g_debug_mask); */
    /* return (s - buf); */
    return 0;
}

static ssize_t SD25XX_dbg_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    char *p;
    int len;
    int val, addr, ret;
	struct i2c_client *client = to_i2c_client(dev);
    struct rtc_time dt;

    p = memchr(buf, '\n', count);
    len = p ? p - buf : count;

    if ((len == 5 && !strncmp(buf, "debug", len)) || (len == 3 && !strncmp(buf, "dbg", len)))
    {
        g_debug_mask = PRINT_ERR | PRINT_INFO | PRINT_DBG;
    } 
    else if (len == 4 && !strncmp(buf, "info", len)) 
    {
        g_debug_mask = PRINT_ERR | PRINT_INFO;
    } 
    else if (len == 3 && !strncmp(buf, "err", len)) 
    {
        g_debug_mask = PRINT_ERR;
    }
    else if (len == 2 && !strncmp(buf, "id", len)) 
    {
        SD25xx_get_ID(client);
    } 
    else if (len == 3 && !strncmp(buf, "bat", len)) 
    {
        SD25xx_get_VBAT(client);
    }
    else if (len == 4 && !strncmp(buf, "time", len)) 
    {
        SD25xx_get_time(&client->dev, &dt);
    }
    else if (!strncmp(buf, "read 0x", 7)) 
    {
        sscanf(buf, "read 0x%x", &addr);
        val = SD25xx_read_reg(client, addr);
        printk("%s: read reg[0x%x]=0x%x\n", __func__, addr, val);
    }
    else if (!strncmp(buf, "write 0x", 8)) 
    {
        sscanf(buf, "write 0x%x 0x%x", &addr, &val);
        ret = i2c_write_byte(client, addr, val);
        if (0 == ret)
        {
            printk("%s: write reg[0x%x]=0x%x\n", __func__, addr, val);
        }
    }

    return count;
}
static DEVICE_ATTR(dbg, S_IWUSR | S_IRUGO, SD25XX_dbg_show, SD25XX_dbg_store);

static const struct rtc_class_ops sd2058_rtc_ops = {
	.read_time	= SD25xx_get_time,
	.set_time	= SD25xx_set_time
};

static int sd2058_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
    int ret;
	struct rtc_device *rtc;

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
    {
		md_prt(ERR, "i2c_check_functionality failed");
		return -ENODEV;
    }

	ret = SD25xx_read_reg(client, SD25xx_CTR2);
    if (ret < 0)
    {
		md_prt(ERR, "Unable to read register #%d", SD25xx_CTR2);
        return -1;
    }

#if 0
    /* 不支持读ID */
	ret = SD25xx_get_ID(client);
	if (ret < 0) {
		dev_err(&client->dev, "rtc sd2058: read ID failed\n");
		return ret;
	}
#endif

	rtc = devm_rtc_device_register(&client->dev, sd2058_driver.driver.name,
				                   &sd2058_rtc_ops, THIS_MODULE);

    device_create_file(&client->dev, &dev_attr_dbg);

	md_prt(INFO , "rtc init success.");
	return PTR_ERR_OR_ZERO(rtc);
}

static const struct i2c_device_id sd2058[] = {
	{ "sd2058", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, sd2058);

#ifdef CONFIG_OF
static const struct of_device_id sd2058_of_match[] = {
	{ .compatible = "sd2058" },
	{}
};
MODULE_DEVICE_TABLE(of, sd2058_of_match);
#endif

static struct i2c_driver sd2058_driver = {
	.driver		= {
		.name	= "rtc-sd2058",
		.of_match_table = of_match_ptr(sd2058_of_match),
	},
	.probe		= sd2058_probe,
	.id_table	= sd2058,
};

module_i2c_driver(sd2058_driver);

MODULE_AUTHOR("moredian");
MODULE_DESCRIPTION("sd2058 RTC driver");
MODULE_LICENSE("GPL");
