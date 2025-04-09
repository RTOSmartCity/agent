#!/bin/bash

IP1="3.226.148.224"
IP2="34.194.7.63"

USERNAME="qnxuser"
REMOTE_DIR="/home/qnxuser"

scp server "$USERNAME@$IP1:$REMOTE_DIR"

scp client "$USERNAME@$IP2:$REMOTE_DIR"


ssh -i ../blls.pem root@"$IP1" "chmod +x $REMOTE_DIR/server"
# ssh -i ../blls.pem root@"$IP1" "/bin/ln -s /lib/libsocket.so.4 /lib/libsocket.so.3"


ssh -i ../blls.pem root@"$IP2" "chmod +x $REMOTE_DIR/client"
# ssh -i ../blls.pem root@"$IP2" "/bin/ln -s /lib/libsocket.so.4 /lib/libsocket.so.3"
