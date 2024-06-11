sudo mount -t drvfs d: /mnt/d -o uid=$(id -u $USER),gid=$(id -g $USER),metadata
cp $1/build/*.uf2 /mnt/d/