#!/bin/bash
cd /home/debian/do_an_nhung/

echo '=== Nap DHT11 driver ==='
sudo insmod dht11_driver.ko 2>/dev/null
sudo chmod 666 /dev/dht11

echo '=== Nap Buzzer driver ==='
sudo insmod buzzer_driver.ko 2>/dev/null
MAJOR=$(grep ' buzzer' /proc/devices | awk '{print $1}')
[ ! -e /dev/buzzer ] && sudo mknod /dev/buzzer c $MAJOR 0
sudo chmod 666 /dev/buzzer
echo 1 | sudo tee /dev/buzzer > /dev/null

echo '=== Nap ADS1115 driver ==='
echo '2-0048' | sudo tee /sys/bus/i2c/drivers/ads1015/unbind 2>/dev/null
echo 'ads1115 0x48' | sudo tee /sys/bus/i2c/devices/i2c-2/new_device 2>/dev/null
sudo rmmod ads1115_driver 2>/dev/null
sudo insmod ads1115_driver.ko 2>/dev/null
sleep 1
MAJOR=$(grep 'mq2' /proc/devices | awk '{print $1}')
[ ! -e /dev/mq2 ] && sudo mknod /dev/mq2 c $MAJOR 0
sudo chmod 666 /dev/mq2

echo '=== Reset LCD ==='
echo '0x27' | sudo tee /sys/bus/i2c/devices/i2c-2/delete_device 2>/dev/null
sleep 1
echo '=== Nap LCD driver ==='
echo 'i2c_lcd 0x27' | sudo tee /sys/bus/i2c/devices/i2c-2/new_device 2>/dev/null
sudo rmmod lcd_driver 2>/dev/null
sudo insmod lcd_driver.ko 2>/dev/null
sleep 1
MAJOR=$(grep ' lcd' /proc/devices | awk '{print $1}')
[ ! -e /dev/lcd ] && sudo mknod /dev/lcd c $MAJOR 0
sudo chmod 666 /dev/lcd

echo '=== Chay chuong trinh chinh ==='
./project_v4
