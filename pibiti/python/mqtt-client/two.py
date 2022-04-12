# TWO
# Publica em /feeds/entrada
# Recebe de  /feeds/saida

import paho.mqtt.client as mqtt
import time


broker_addr = '10.106.1.32'
broker_port = 1883

username = 'cliente'
password = 'cliente'

def mqttConnect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    client.subscribe("/feeds/saida")

def mqttInput(client, userdata, msg):
    print(msg.payload)

client = mqtt.Client()
client.on_message = mqttInput
client.on_connect = mqttConnect
client.username_pw_set(username, password)

client.connect(broker_addr, broker_port)
client.publish("/feeds/entrada", 'abcs')

client.loop_forever()