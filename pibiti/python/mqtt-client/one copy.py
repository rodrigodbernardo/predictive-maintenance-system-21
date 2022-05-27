# ONE
# Publica em /feeds/saida
# Recebe de  /feeds/entrada

from email import message
import paho.mqtt.client as mqtt
import time


broker_addr = '127.0.0.1'
broker_port = 1883

username = 'cliente'
password = 'cliente'
matrix = []

dados = "{\"AcX\":1000,\"AcY\":1000,\"AcZ\":1000,\"GyX\":1000,\"GyY\":1000,\"GyZ\":1000,\"Tmp\":1000}"

dados_dict = eval(dados)



def mqttConnect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    client.subscribe("spBv1.0/GrupoB/NDATA/No_B")


def mqttInput(client, userdata, msg):
    ##msg.payload = msg.payload.decode("utf-8")  ### <--- ATENCAO PARA DECODIFICAR EM UTF-8

    print(msg.payload)

client = mqtt.Client()
client.on_message = mqttInput
client.on_connect = mqttConnect
client.username_pw_set(username, password)



client.connect(broker_addr, broker_port)



client.loop_forever()

while(1):
    pump    = input('Bomba: ')
    classe  = input('Classe: ')
    
