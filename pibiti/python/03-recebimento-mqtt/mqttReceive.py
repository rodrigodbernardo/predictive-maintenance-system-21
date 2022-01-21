import paho.mqtt.client as mqtt 
import time

broker_addr = 'broker.mqtt-dashboard.com'
broker_port = 1883

def mqttConnect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    client.subscribe("pibiti-ifce/topico-teste-saida")

def mqttInput(client, userdata, msg):
    print(msg.payload)

client = mqtt.Client()
client.on_message = mqttInput
client.on_connect = mqttConnect

client.connect(broker_addr, broker_port)

client.loop_forever()