#include <linux/module.h>
#include <linux/init.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>

#define GPIO_BUZZER 23
#define DEVICE_NAME "buzzer"

static int major;
static struct class *buzzer_class;
static struct device *buzzer_device;

static ssize_t dev_write(struct file *file, const char __user *buf,
                         size_t len, loff_t *off) {
    char kbuf[2] = {0};
    if (copy_from_user(kbuf, buf, 1)) return -EFAULT;
    if (kbuf[0] == '0') {
        gpio_set_value(GPIO_BUZZER, 0);
        printk(KERN_INFO "Buzzer Driver: COI DANG KEU\n");
    } else {
        gpio_set_value(GPIO_BUZZER, 1);
        printk(KERN_INFO "Buzzer Driver: COI DA TAT\n");
    }
    return len;
}

static struct file_operations fops = { .write = dev_write };

static int __init buzzer_init(void) {
    if (gpio_request(GPIO_BUZZER, "BUZZER_GPIO")) return -EBUSY;
    gpio_direction_output(GPIO_BUZZER, 1);
    major = register_chrdev(0, DEVICE_NAME, &fops);
    buzzer_class = class_create(THIS_MODULE, "buzzer_class");
    buzzer_device = device_create(buzzer_class, NULL, MKDEV(major,0), NULL, DEVICE_NAME);
    printk(KERN_INFO "Buzzer Driver: Nap thanh cong. Major = %d\n", major);
    return 0;
}

static void __exit buzzer_exit(void) {
    gpio_set_value(GPIO_BUZZER, 1);
    gpio_free(GPIO_BUZZER);
    device_destroy(buzzer_class, MKDEV(major, 0));
    class_destroy(buzzer_class);
    unregister_chrdev(major, DEVICE_NAME);
}

module_init(buzzer_init);
module_exit(buzzer_exit);
MODULE_LICENSE("GPL");
