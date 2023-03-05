#!/bin/bash

sudo mkdir -p /opt/UCTRONICS/SKU_RM0004
sudo cp ./display /opt/UCTRONICS/SKU_RM0004
sudo cp ./SKU_RM0004.service /etc/systemd/system
sudo systemctl daemon-reload
sudo systemctl enable --now RM0004
