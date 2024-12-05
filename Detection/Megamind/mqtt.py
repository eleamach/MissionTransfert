#!/usr/bin/env python

from paho.mqtt import client as mqtt_client



def send_mqtt(correcte, malPlace):
    print(f"Correcte : {correcte} Mal placé : {malPlace}")

    broker = 'broker.megamind'
    port = 1883
    topic = "python/mqtt/megamind"
    client_id = f'python-mqtt-1'