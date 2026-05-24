#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/delay.h>

#define DEVICE_NAME "lcd"

static int major;
static struct i2c_client *lcd_client;

void lcd_send_4bit(u8 val) {
    u8 buf;
    buf = val | 0x0C;
    i2c_master_send(lcd_client, &buf, 1);
    usleep_range(1000, 2000);
    buf = val | 0x08;
    i2c_master_send(lcd_client, &buf, 1);
    usleep_range(1000, 2000);
}

void lcd_send_byte(u8 val, u8 mode) {
    lcd_send_4bit((val & 0xf0) | mode);
    lcd_send_4bit(((val << 4) & 0xf0) | mode);
}

static ssize_t dev_write(struct file *file, const char __user *buf,
                          size_t len, loff_t *off) {
    char kbuf[32];
    int i, n = (len > 32) ? 32 : len;
    if (copy_from_user(kbuf, buf, n)) return -EFAULT;
    lcd_send_byte(0x01, 0x00); msleep(5);
    for (i = 0; i < n; i++) {
        if (kbuf[i] == '\n') { lcd_send_byte(0xC0, 0x00); msleep(2); continue; }
        lcd_send_byte(kbuf[i], 0x01);
        usleep_range(500, 1000);
    }
    return len;

static struct file_perations fops = { .write = dev_write };

static int lcd_probe(struct i2c_client *client, const struct i2c_device_id *id) {
    lcd_client = client;
    major = register_chrdev(0, DEVICE_NAME, &fops);
    msleep(50);
    lcd_send_4bit(0x30); msleep(5);
    lcd_send_4bit(0x30); msleep(2);
    lcd_send_4bit(0x30); msleep(2);
    lcd_send_4bit(0x20); msleep(2);
    lcd_send_byte(0x28, 0x00);
    lcd_send_byte(0x0C, 0x00);
    lcd_send_byte(0x06, 0x00);
    lcd_send_byte(0x01, 0x00); msleep(5);
    printk(KERN_INFO "LCD Driver: Probe thanh cong!\n");
    return 0;
}

static int lcd_remove(struct i2c_client *client) {
    unregister_chrdev(major, DEVICE_NAME);
    return 0;
}

static const struct i2c_device_id lcd_ids[] = { {"i2c_lcd", 0}, {} };
static struct i2c_driver lcd_driver = {
    .driver = { .name = "i2c_lcd" },
    .probe = lcd_probe,
    .remove = lcd_remove,
    .id_table = lcd_ids
};
module_i2c_driver(lcd_driver);
MODULE_LICENSE("GPL");
