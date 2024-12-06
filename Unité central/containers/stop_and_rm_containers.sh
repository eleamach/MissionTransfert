#!/bin/bash

printf "\nStop containers if not already done : \n"
docker stop homeassistant  mosquitto_broker
printf "\nRemove containers : \n"
docker rm   homeassistant  mosquitto_broker
