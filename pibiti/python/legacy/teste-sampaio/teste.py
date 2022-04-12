# python 3.6

import random
import time

from paho.mqtt import client as mqtt_client

broker = '127.0.0.1'
port = 1883
topic = "/feeds/entrada"

username = 'cliente'
password = 'cliente'

# generate client ID with pub prefix randomly
client_id = f'python-mqtt-{random.randint(0, 1000)}'

def connect_mqtt():

    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("\n Conectado ao Broker MQTT!")
        else:
            print("Falha ao conectar. Erro %d\n", rc)

    client = mqtt_client.Client(client_id)
    client.username_pw_set(username, password)

    client.on_connect = on_connect


    client.connect(broker, port)
    return client

def publish(client):
    command = input("Comando: ")
    
    result = client.publish(topic, command)
    # result: [0, 1]
    
    status = result[0]
    if status == 0:
        print(f"Send `{command}` to topic `{topic}`")
    else:
        print(f"Failed to send message to topic {topic}")

############################

def run():
    client = connect_mqtt()
    while True:
        client.loop_start()
        publish(client)

############################

if __name__ == '__main__':
    run()
