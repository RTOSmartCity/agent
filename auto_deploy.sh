#!/bin/bash

IP1="3.226.148.224"
IP2="34.194.7.63"

USERNAME="qnxuser"
REMOTE_DIR="/home/qnxuser"

scp server "$USERNAME@$IP1:$REMOTE_DIR"

scp client "$USERNAME@$IP2:$REMOTE_DIR"