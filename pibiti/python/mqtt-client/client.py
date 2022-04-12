from email import message
import paho.mqtt.client as mqtt

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT Broker!")
    else:
        print("Failed to connect, return code %d\n", rc)

def on_message(client, userdata, msg):
    message_str = msg
    res = eval(message_str)

    buffer = []

    for key in res:
        buffer.append(res)

    print(buffer)
    


broker_url = "127.0.0.1"
broker_port = 1883

client = mqtt.Client()

username = 'cliente'
password = 'cliente'

topic_input = "/feeds/entrada"
topic_output = "/feeds/saida"

client.username_pw_set(username, password)
client.on_connect = on_connect
client.connect(broker_url, broker_port) 
client.on_message = on_message
client.subscribe(topic_output)

client.loop_start()

msg = "{\"command\":\"live_capture\",\"sample_period\":1000,\"n_packets\":0}\""
print(msg)
client.publish(topic=topic_input, payload=msg, qos=0, retain=False)