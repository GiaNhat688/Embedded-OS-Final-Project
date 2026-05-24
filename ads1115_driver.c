#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/delay.h>

static struct i2c_client *ads_client;
static int major;

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    uint8_t config[3] = {0x01, 0xC3, 0x83};
    uint8_t reg = 0x00;
    uint8_t data[2];
    char msg[16];
    i2c_master_send(ads_client, config, 3);
    mdelay(10);
    i2c_master_send(ads_client, &reg, 1);
    i2c_master_recv(ads_client, data, 2);
    int16_t value = (data[0] << 8) | data[1];
    int n = sprintf(msg, "%d\n", value);
    return copy_to_user(buffer, msg, n) ? -EFAULT : n;
}

static struct file_operations fops = { .read = dev_read };

static int ads_probe(struct i2c_client *client, const struct i2c_device_id *id) {
    ads_client = client;
    major = register_chrdev(0, "mq2", &fops);
    printk(KERN_INFO "MQ2 Driver: Da nhan ADS1115 tai 0x%x. Major: %d\n",
           client->addr, major);
    return 0;
}

static int ads_remove(struct i2c_client *client) {
    unregister_chrdev(major, "mq2");
    return 0;
}

static const struct i2c_device_id ads_ids[] = { {"ads1115", 0}, {} };
static struct i2c_driver ads_driver = {
    .driver = { .name = "ads1115" },
    .probe = ads_probe,
    .remove = ads_remove,
    .id_table = ads_ids
};
module_i2c_driver(ads_driver);
MODULE_LICENSE("GPL");
