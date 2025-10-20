import time
from paho.mqtt.client import Client, MQTTMessage
from paho.mqtt.enums import CallbackAPIVersion
import random
import serial

broker = 'broker.emqx.io'
luminosity_topic = 'matveymftopic/luminosity'
led_state_topic = 'matveymftopic/led_state'

LED_ON = b'u'
LED_OFF = b'd'

connection = serial.Serial('COM4', 9600, timeout=1)
time.sleep(2)
connection.write(LED_ON)
led_state = LED_ON

def on_connect(client: Client, userdata, flags, reason_code, properties):
    if reason_code == 0:
        print('Connected to MQTT Broker!')
    else:
        print('Failed to connect, return code %d\n', reason_code)

def on_message(client: Client, userdata, message: MQTTMessage):
    global connection, led_state
    value = int(message.payload.decode('utf-8'))
    topic = message.topic
    print(f'Received luminosity value {value} from topic {topic}')
    if led_state == LED_OFF and value < 400:
        print(f'Luminosity is too low, turining LED on')
        connection.write(LED_ON)
        led_state = LED_ON
    if led_state == LED_ON and value >= 400:
        print(f'Luminosity is too high, turining LED off')
        connection.write(LED_OFF)
        led_state = LED_OFF
    
    time.sleep(0.1)
    if connection.in_waiting > 0:
        line = connection.readline().decode().strip()
        print(f'Got line from MCU: {line}')
        print(f'Publish LED response - {line} to {led_state_topic}')
        client.publish(led_state_topic, line, qos=2)


if __name__ == '__main__':
    client= Client(
        callback_api_version=CallbackAPIVersion.VERSION2,
        client_id = f'MY_CLIENT_ID_{random.randint(10000, 99999)}'
    )
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(broker)
    client.loop_start()
    print(f'Subcribing to {luminosity_topic}')
    client.subscribe(luminosity_topic, qos=2)
    time.sleep(1800)
    client.loop_stop()
    client.disconnect()
    connection.close()
