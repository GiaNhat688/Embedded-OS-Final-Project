obj-m += dht11_driver.o buzzer_driver.o ads1115_driver.o lcd_driver.o

all:
    make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
    make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
