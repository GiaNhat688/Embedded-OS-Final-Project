#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/gpio/consumer.h>
#include <linux/delay.h>
#include <linux/irqflags.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>

#define DHT11_PIN 45
#define DEVICE_NAME "dht11"

static int major_number;
static struct class* dht11Class = NULL;
static struct device* dht11Device = NULL;
struct gpio_desc *dht11_desc;
static uint8_t dht11_data[5] = {0};

static int read_dht11_raw_data(void) {
    uint8_t i, j;
    int counter = 0;
    unsigned long flags;
    for(i=0; i<5; i++) dht11_data[i] = 0;

    gpiod_direction_output(dht11_desc, 0);
    mdelay(18);
    gpiod_set_value(dht11_desc, 1);
    udelay(30);
    gpiod_direction_input(dht11_desc);

    local_irq_save(flags);
    counter = 0;
    while (gpiod_get_value(dht11_desc)==1){counter++;udelay(1);if(counter>100)break;}
    counter = 0;
    while (gpiod_get_value(dht11_desc)==0){counter++;udelay(1);if(counter>100)break;}
    counter = 0;
    while (gpiod_get_value(dht11_desc)==1){counter++;udelay(1);if(counter>100)break;}

    for (i = 0; i < 5; i++) {
        for (j = 0; j < 8; j++) {
            counter = 0;
            while(gpiod_get_value(dht11_desc)==0){counter++;udelay(1);if(counter>100)break;}
            counter = 0;
            while(gpiod_get_value(dht11_desc)==1){counter++;udelay(1);if(counter>100)break;}
            dht11_data[i] <<= 1;
            if (counter > 40) dht11_data[i] |= 1;
        }
    }
    local_irq_restore(flags);

    if((uint8_t)(dht11_data[0]+dht11_data[1]+dht11_data[2]+dht11_data[3])==dht11_data[4])
        return 0;
    return -1;
}

static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    int ret, i;
    for(i = 0; i < 10; i++) {
        if (read_dht11_raw_data() == 0) {
            if(dht11_data[0] > 0 && dht11_data[2] > 0) {
                ret = copy_to_user(buffer, dht11_data, 5);
                return (ret == 0) ? 5 : -EFAULT;
            }
        }
        mdelay(100);
    }
    return -EAGAIN;
}

static struct file_operations fops = { .read = dev_read };

static int __init dht11_init(void) {
    if (gpio_request(DHT11_PIN, "DHT11_DATA") < 0) return -EBUSY;
    dht11_desc = gpio_to_desc(DHT11_PIN);
    gpiod_direction_input(dht11_desc);
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    dht11Class = class_create(THIS_MODULE, "dht11_class");
    dht11Device = device_create(dht11Class, NULL, MKDEV(major_number,0), NULL, DEVICE_NAME);
    printk(KERN_INFO "DHT11 Driver Loaded! Device: /dev/%s\n", DEVICE_NAME);
    return 0;
}

static void __exit dht11_exit(void) {
    device_destroy(dht11Class, MKDEV(major_number, 0));
    class_destroy(dht11Class);
    unregister_chrdev(major_number, DEVICE_NAME);
    gpio_free(DHT11_PIN);
    printk(KERN_INFO "DHT11 Driver Removed!\n");
}

module_init(dht11_init);
module_exit(dht11_exit);
MODULE_LICENSE("GPL");
