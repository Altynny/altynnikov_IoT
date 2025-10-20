import time
from paho.mqtt.client import Client
from paho.mqtt.enums import CallbackAPIVersion
import random
import serial
    
broker='broker.emqx.io'
luminosity_topic = 'matveymftopic/luminosity'

GET_VALUE = b'p'
STREAM = b's'

connection = serial.Serial('COM5', 9600, timeout=1)
time.sleep(2)

if __name__ == '__main__':
    client= Client(
        callback_api_version = CallbackAPIVersion.VERSION2,
        client_id = f'MY_CLIENT_ID_{random.randint(10000, 99999)}'
    )
    client.max_inflight_messages_set(100)
    client.connect(broker)
    client.loop_start()
    for i in range(1800):
        print(f'Itteration {i}:')

        connection.write(GET_VALUE)
        print('\tCommand GET_VALUE send to MCU')
        time.sleep(0.1)
        
        if connection.in_waiting > 0:
            line = connection.readline()
            print(f'\tGot line from MCU: {line}')
            value = line.decode().strip().split('SENSOR_VALUE:',1)[1]
            print(f'\tPublish luminosity - {value} to {luminosity_topic}')
            client.publish(luminosity_topic, value, qos=2)
        time.sleep(3)

    client.loop_stop() 
    client.disconnect()
    connection.close()