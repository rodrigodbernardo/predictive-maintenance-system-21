import paho.mqtt.client as mqtt 
import time

broker_addr = 'broker.mqtt-dashboard.com'
broker_port = 1883

client = mqtt.Client("Temperature_Inside")
client.connect(broker_addr,broker_port)

num = 0

while 1:
    num += 1
    client.publish("pibiti-ifce/topico-teste-saida",'Hello WOrld')
    print('Publicado: ','Hello WOrld')
    time.sleep(2)