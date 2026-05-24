1.1. Kết nối SSH vào BeagleBone Black
# Ket noi SSH qua USB (ip mac dinh)
ssh debian@192.168.7.2

# Mat khau mac dinh
temppwd
1.2. Copy file từ Ubuntu lên BBB
# Copy toan bo thu muc project
scp -r /home/manhht10ht/do_an_nhung/ debian@192.168.7.2:/home/debian/

# Copy file don le
scp project_v4.c debian@192.168.7.2:/home/debian/do_an_nhung/

# Copy file tu BBB ve Ubuntu
scp debian@192.168.7.2:/home/debian/do_an_nhung/project_v4 ./
1.3. Chia sẻ internet từ Ubuntu sang BBB qua USB
# Tren Ubuntu - bat ip forward va NAT
sudo sysctl net.ipv4.ip_forward=1
sudo iptables -t nat -A POSTROUTING -o enp0s3 -j MASQUERADE

# Tren BBB - them default route va DNS
sudo ip route add default via 192.168.7.1
echo 'nameserver 8.8.8.8' | sudo tee /etc/resolv.conf

# Kiem tra ket noi
ping 8.8.8.8 -c 3
1.4. Cài đặt các gói cần thiết trên BBB
# Cap nhat danh sach goi
sudo apt update

# Cai kernel headers de build driver
sudo apt install linux-headers-$(uname -r) build-essential -y

# Cai cac cong cu khac
sudo apt install gdbserver i2c-tools curl -y
1.5. Tắt BBB an toàn
sudo shutdown -h now

