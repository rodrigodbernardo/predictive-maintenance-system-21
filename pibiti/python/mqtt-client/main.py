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

#dados_dict = eval(dados)

capt_counter = 0
endFlag = 0

command = {}

def mqttConnect(client, userdata, flags, rc):
    print("Broker conectado com sucesso!")

    client.subscribe("/feeds/saida")

def mqttInput(client, userdata, msg):
    msg.payload = msg.payload.decode("utf-8")  ### <--- ATENCAO PARA DECODIFICAR EM UTF-8

    
    if(msg.payload == 'fim'):
        print('CAPTURA FINALIZADA')
        endFlag = 1
    else:
        #capt_counter += 1
        #print('Captura ' + str(capt_counter))
        matrix.append(msg.payload)


    
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

client.loop_start()



while(1):
    try:
        print('''

ANALISE DE VIBRAÇÃO

Menu principal
1 - CAPTURA EM TEMPO REAL
2 - CAPTURA RÁPIDA
3 - REINICIAR SENSOR
4 - FECHAR PROGRAMA

''')
        command.update({'cmd':int(input('>>> '))})

        if not command['cmd'] in range(1,5):
            raise ValueError

    except:
        break
    
    if(command['cmd'] == 2):
        command.update({'npk':int(input('Quantidade de pacotes: '))})
        command.update({'ncp':int(input('Amostras por pacote: '))})
    else:
        n_packets  = 1
        n_captures = 1
    
    command.update({'spe':int(input('Período amostral (em us): '))})

    mqtt_message = '{}'.format(command)

    print('Publicando comando: ' + mqtt_message)
    client.publish("/feeds/entrada",mqtt_message)

    for pacote in range(command['npk']):
        print('Pacote ' + str(pacote))
        
        command['cmd'] = 0
        mqtt_message = '{}'.format(command)
        client.publish("/feeds/entrada",mqtt_message)
        endFlag = 0

        while(endFlag != 1):
            time.sleep(0.5)

        if(command['cmd'] == 2):
            print('Salvando captura em arquivo...')
            text_file = open("C:/Users/rodri/data.txt", "a")

            for i in range (command['ncp']):
                text_file.write(matrix[i] + "\n")

            print('Pacote salvo. Preparando para iniciar o próximo')
            text_file.close()



        



#client.loop_forever()