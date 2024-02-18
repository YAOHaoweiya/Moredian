/*
 * An I2C driver for the JXR260 RTC
 * Copyright 2014 Rose Technology
 *
 * Author: Søren Andersen <san@rosetechnology.dk>
 * Maintainers: http://www.nslu2-linux.org/
 *
 * based on the other drivers in this same directory.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/i2c.h>
#include <linux/bcd.h>
#include <linux/rtc.h>
#include <linux/module.h>

#define JXR260_REG_CTRL1		0x00 /* status */
#define JXR260_REG_TYPE		    0x11

#define JXR260_REG_SC			0x02 /* datetime */
#define JXR260_REG_MN			0x03
#define JXR260_REG_HR			0x04
#define JXR260_REG_DM			0x05
#define JXR260_REG_DW			0x06
#define JXR260_REG_MO			0x07
#define JXR260_REG_YR			0x08

static struct i2c_driver jxr260_driver;

static int jxr260_rtc_read_time(struct device *dev, struct rtc_time *tm)
{
	struct i2c_client *client = to_i2c_client(dev);
	int rc;
	u8 regs[7];

	/*
	 * while reading, the time/date registers are blocked and not updated
	 * anymore until the access is finished. To not lose a second
	 * event, the access must be finished within one second. So, read all
	 * time/date registers in one turn.
	 */
	printk(KERN_WARNING "RTC start read time\n");
	rc = i2c_smbus_read_i2c_block_data(client, JXR260_REG_SC,
					   sizeof(regs), regs);
	if (rc != sizeof(regs)) {
		printk(KERN_WARNING "RTC date/time register read error\n");
		dev_err(&client->dev, "date/time register read error\n");
		return -EIO;
	}

	tm->tm_sec = bcd2bin(regs[0] & 0x7F);
	tm->tm_min = bcd2bin(regs[1] & 0x7F);
	tm->tm_hour = bcd2bin(regs[2] & 0x3F); /* rtc hr 0-23 */
	tm->tm_mday = bcd2bin(regs[3] & 0x3F);
	tm->tm_wday = regs[4] & 0x07;
	tm->tm_mon = bcd2bin(regs[5] & 0x1F) - 1; /* rtc mn 1-12 */
	tm->tm_year = bcd2bin(regs[6]);
	tm->tm_year += 100;
    printk(KERN_WARNING "RTC read result year: %d, mon: %d, wday: %d, mday: %d, hour: %d, min: %d, sec: %d\n",\
	      tm->tm_year,tm->tm_mon,tm->tm_wday,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);
	return 0;
}

static int jxr260_rtc_set_time(struct device *dev, struct rtc_time *tm)
{
	struct i2c_client *client = to_i2c_client(dev);
	int rc;
	u8 regs[7];

	printk(KERN_WARNING "RTC start to set time year: %d, mon: %d, wday: %d, mday: %d, hour: %d, min: %d, sec: %d\n",\
	      tm->tm_year,tm->tm_mon,tm->tm_wday,tm->tm_mday,tm->tm_hour,tm->tm_min,tm->tm_sec);
	if ((tm->tm_year < 100) || (tm->tm_year > 199))
	{
		printk(KERN_WARNING "RTC, set time year is error, year: %d\n",tm->tm_year);
		return -EINVAL;
	}

	/* hours, minutes and seconds */
	regs[0] = bin2bcd(tm->tm_sec) & 0x7F; /* clear OS flag */

	regs[1] = bin2bcd(tm->tm_min);
	regs[2] = bin2bcd(tm->tm_hour);

	/* Day of month, 1 - 31 */
	regs[3] = bin2bcd(tm->tm_mday);

	/* Day, 0 - 6 */
	regs[4] = tm->tm_wday & 0x07;

	/* month, 1 - 12 */
	regs[5] = bin2bcd(tm->tm_mon + 1);

	/* year and century */
	regs[6] = bin2bcd(tm->tm_year - 100);

	/* write all registers at once */
	rc = i2c_smbus_write_i2c_block_data(client, JXR260_REG_SC,
					    sizeof(regs), regs);
	if (rc < 0) {
		printk(KERN_WARNING "RTC date/time register write error, rc: %d\n",rc);
		dev_err(&client->dev, "date/time register write error\n");
		return rc;
	}

	printk(KERN_WARNING "RTC set time OK");
	return 0;
}

static const struct rtc_class_ops jxr260_rtc_ops = {
	.read_time	= jxr260_rtc_read_time,
	.set_time	= jxr260_rtc_set_time
};

static int jxr260_probe(struct i2c_client *client,
				const struct i2c_device_id *id)
{
	struct rtc_device *rtc;
	int err;
	u8 regs[1];

	dev_dbg(&client->dev, "%s\n", __func__);

	if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C))
		return -ENODEV;

	err = i2c_smbus_read_byte_data(client, JXR260_REG_CTRL1);
	if (err < 0) {
		dev_err(&client->dev, "RTC chip is not present\n");
		return err;
	}

	err = i2c_smbus_read_i2c_block_data(client, JXR260_REG_TYPE,
					   sizeof(regs), regs);
	dev_err(&client->dev, "%s: read JXR260_REG_TYPE vlaue = 0x%02x\n", __func__, regs[0]);

	if (regs[0] != 0x03) {
		dev_err(&client->dev, "RTC chip is not jxr260 !\n");
		return -ENODEV;
	}

	rtc = devm_rtc_device_register(&client->dev,
				       jxr260_driver.driver.name,
				       &jxr260_rtc_ops, THIS_MODULE);

	return PTR_ERR_OR_ZERO(rtc);
}

static const struct i2c_device_id jxr260_id[] = {
	{ "jxr260", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, jxr260_id);

#ifdef CONFIG_OF
static const struct of_device_id jxr260_of_match[] = {
	{ .compatible = "jxzn,jxr260" },
	{}
};
MODULE_DEVICE_TABLE(of, jxr260_of_match);
#endif

static struct i2c_driver jxr260_driver = {
	.driver		= {
		.name	= "rtc-jxr260",
		.of_match_table = of_match_ptr(jxr260_of_match),
	},
	.probe		= jxr260_probe,
	.id_table	= jxr260_id,
};

module_i2c_driver(jxr260_driver);

MODULE_AUTHOR("Søren Andersen <san@rosetechnology.dk>");
MODULE_DESCRIPTION("JXR260 RTC driver");
MODULE_LICENSE("GPL");
