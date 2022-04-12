# ONE
# Publica em /feeds/saida
# Recebe de  /feeds/entrada

import paho.mqtt.client as mqtt
import time


broker_addr = '10.106.1.32'
broker_port = 1883

username = 'cliente'
password = 'cliente'
matrix = []

dados = "{\"AcX\":1000,\"AcY\":1000,\"AcZ\":1000,\"GyX\":1000,\"GyY\":1000,\"GyZ\":1000,\"Tmp\":1000}"

dados_dict = eval(dados)



def mqttConnect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))

    client.subscribe("/feeds/entrada")

def mqttInput(client, userdata, msg):
    text_file = open("C:/Users/lari/data.txt", "a")
    msg.payload = msg.payload.decode("utf-8")  ### <--- ATENCAO PARA DECODIFICAR EM UTF-8
    text_file.write(msg.payload + "\n")
    print('valor adicionado')
    text_file.close()
'''
    if(msg.payload == "end of transmission"):
        print('fim da transmissao')
        print(matrix)
        #salva no arquivo
    else:
        matrix.append(msg.payload)
        matrix[-1] = matrix[-1][2:-1]
        print('valor adicionado')
        print(matrix)
'''
client = mqtt.Client()
client.on_message = mqttInput
client.on_connect = mqttConnect
client.username_pw_set(username, password)

client.connect(broker_addr, broker_port)

client.loop_forever()