sudo mkdir /mnt/pico
sudo mount -t drvfs e: /mnt/pico -o uid=$(id -u $USER),gid=$(id -g $USER),metadata
cp ./build/*.uf2 /mnt/pico/