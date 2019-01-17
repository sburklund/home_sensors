#! /bin/bash
if [ $USER != sbmint ]; then
  su sbmint
fi

python3 /home/sbmint/home_automation/mqtt_gateway.py &>/dev/null &
