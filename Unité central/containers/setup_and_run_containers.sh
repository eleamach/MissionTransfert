#!/bin/bash

# Define the base directory
BASE_DIR="${HOME}/docker_volumes"

# Create directories for homeAssistant
mkdir -p "${BASE_DIR}/homeAssistant/data"

# Create directories for mosquitto
mkdir -p "${BASE_DIR}/mosquitto/config"
mkdir -p "${BASE_DIR}/mosquitto/data"
mkdir -p "${BASE_DIR}/mosquitto/log"

# Check if mosquitto.conf does not exist in the config directory, then download it
#if [ ! -f "${BASE_DIR}/mosquitto/config/mosquitto.conf" ]; then
#    echo "mosquitto.conf not found, downloading..."
#    wget -O "${BASE_DIR}/mosquitto/config/mosquitto.conf" https://raw.githubusercontent.com/eclipse/mosquitto/master/mosquitto.conf   
#fi

cp mosquitto.conf "${BASE_DIR}/mosquitto/config/mosquitto.conf"

# cd current directory
cd "$(dirname "$0")"

# Run the Docker Compose file
docker compose up

# Stop and remove the containers
./stop_and_rm_containers.sh
