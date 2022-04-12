# python 3.6

import random
import time

from paho.mqtt import client as mqtt_client

broker = '127.0.0.1'
port = 1883

topic_input = "/feeds/entrada"
topic_output = "/feeds/saida"

username = 'cliente'
password = 'cliente'
buffer = []
# generate client ID with pub prefix randomly
client_id = f'python-mqtt-{random.randint(0, 1000)}'

def on_message(client, userdata, msg):
    message_str = msg
    res = eval(message_str)

    

    for key in res:
        buffer.append(res)
    
    print(buffer)

    

def connect_mqtt():

    def on_connect(client, userdata, flags, rc):
        if rc == 0:
            print("\n Conectado ao Broker MQTT!")
        else:
            print("Falha ao conectar. Erro %d\n", rc)

    client = mqtt_client.Client(client_id)
    client.username_pw_set(username, password)

    client.on_connect = on_connect
    client.on_message = on_message
    client.subscribe(topic_output)


    client.connect(broker, port)
    return client

def publish(client):
    command = input("Comando: ")
    
    result = client.publish(topic_input, command)
    # result: [0, 1]
    
    status = result[0]
    if status == 0:
        print(f"Send `{command}` to topic `{topic_input}`")
    else:
        print(f"Failed to send message to topic {topic_input}")


############################

def run():
    client = connect_mqtt()
    publish(client)
    while True:
        client.loop_forever()
        

############################

if __name__ == '__main__':
    run()
