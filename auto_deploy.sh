#!/bin/bash

IP1="3.226.148.224"
IP2="34.194.7.63"
IP3="13.219.193.24"

USERNAME="qnxuser"
REMOTE_DIR="/home/qnxuser"
CFG_DIR="configs"

scp server "$USERNAME@$IP1:$REMOTE_DIR"

scp vehicle $CFG_DIR/vehicle-config.txt "$USERNAME@$IP2:$REMOTE_DIR"

scp trafficlight $CFG_DIR/traffic-config.txt "$USERNAME@$IP3:$REMOTE_DIR"

ssh -i ../blls.pem root@"$IP1" "chmod +x $REMOTE_DIR/server"
# ssh -i ../blls.pem root@"$IP1" "/bin/ln -s /lib/libsocket.so.4 /lib/libsocket.so.3"

ssh -i ../blls.pem root@"$IP2" "chmod +x $REMOTE_DIR/vehicle" 
# ssh -i ../blls.pem root@"$IP2" "/bin/ln -s /lib/libsocket.so.4 /lib/libsocket.so.3"

ssh -i ../blls.pem root@"$IP3" "chmod +x $REMOTE_DIR/trafficlight"
# ssh -i ../blls.pem root@"$IP3" "/bin/ln -s /lib/libsocket.so.4 /lib/libsocket.so.3"