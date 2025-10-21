# Lihgt sensor MCU
Контроллер исполняет две команды: начало потоковой передачи и отправка одиночного значения.
```cpp
const char GET_VALUE = 'p';
const char STREAM_MODE = 's';
bool stream_state = false;
```
Также содержит в себе состояние потоковой передачи ```bool stream_state```, по умолчанию ```false```. Если на контроллер приходит какой-то ввод, мы сопоставляем его с заданными командами: ```GET_VALUE``` выключает потоковую передачу и отправляет одиночное значение, а ```STREAM_MODE```, напротив, включает потоковую передачу и объявляет о её начале, и впоследствии при проверке ```bool stream_state``` постоянно выводятся значения освещённости:
```cpp
void UpdateState(int lightValue) {
  if (Serial.available() > 0) {
    char command = Serial.read();
    switch (command) {
      case GET_VALUE:
        stream_state = false;
        Serial.print("SENSOR_VALUE:");
        Serial.println(lightValue);
        break;
      case STREAM_MODE:
        stream_state = true;
        Serial.println("STREAM_STARTED");
        break;
    }
  }
}

void loop() {
  int lightValue = analogRead(lightSensorPin);
  UpdateState(lightValue);
  if (stream_state) {
    Serial.print("SENSOR_VALUE:");
    Serial.println(lightValue);
  }
}
```
По задумке контроллер не должен передавать значения, пока того явно не укажет **publisher**, чтобы они не копились в ```connection.in_waiting```, и чтобы в топик публиковались только самые свежие единичные показатели освещённости, поэтому режим потоковой передачи выключен. 

# Publisher
Каждую итерацию в цикле **publisher** запрашивает значение освещённости отправив команду ```GET_VALUE = b'p'``` и ожидает 1мс, чтобы контроллер успел обратать команду и отправить ответ:
```py
    for i in range(1800):
        print(f'Itteration {i}:')

        connection.write(GET_VALUE)
        print('\tCommand GET_VALUE send to MCU')
        time.sleep(0.1)
```
Хотя на практике 1мс хватало для обработки команды и получения ответа, всё равно проводится проверка на вывод в ожидании, далее значение извлекается и публикуется в соответствующий топик. Значения освещённости выводятся с задержкой в 3 секунды, чтобы не получить бан от брокера и чтобы **subscriber** успевал обрабатывать полученные значения:
```py
    if connection.in_waiting > 0:
        line = connection.readline()
        print(f'\tGot line from MCU: {line}')
        value = line.decode().strip().split('SENSOR_VALUE:',1)[1]
        print(f'\tPublish luminosity - {value} to {luminosity_topic}')
        client.publish(luminosity_topic, value, qos=2)
    time.sleep(3)
```

# LED MCU
Контроллер содержит две команды: включения и выключения, - и состояние по умолчанию - выключено.
```cpp
const char LED_ON = 'u';
const char LED_OFF = 'd';

char LED_state = 'd';
```
При получении команды контроллер сопоставляет её и выполняет соответсвующие действия по смене состояния и отправке сообщения о его изменении:
```cpp
void updateState(int lightValue) {
    if(Serial.available() > 0) {
        LED_state = Serial.read();
        if(LED_state==LED_ON) {
            Serial.println("LED_GOES_ON");
        } else if(LED_state==LED_OFF) {
            Serial.println("LED_GOES_OFF");
        }
    }
}
```
В зависимости от состояния на LED подаётся ток:
```cpp
void handleLED() {
    if(LED_state==LED_ON) {
      digitalWrite(led_pin, HIGH);
    } else if(LED_state==LED_OFF) {
      digitalWrite(led_pin, LOW);
    }
}
```

# Subscriber
Чтобы постоянно не задавать состояние LED под каждое полученное значение, хранится ```led_state```, считающийся при запуске как **LED_ON**, поскольку сразу после соединения явно отправляется команда на включение лампочки:
```py
LED_ON = b'u'
LED_OFF = b'd'

connection = serial.Serial('COM4', 9600, timeout=1)
time.sleep(2)
connection.write(LED_ON)
led_state = LED_ON
```
В дальнейшем это состояние переключается:
- **LED_ON**, если состояние в данный момент **LED_OFF** и уровень освещённости ниже порога
- **LED_OFF**, если состояние в данный момент **LED_ON** и уровень освещённости выше или равен порогу
- В иных случаях никаких действий не предпринимается и не производятся лишние обращения к контроллеру и лишние выводы в консоль и публикации о переключении его состояния.

```py
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
```

Если же команда была отправлена на контроллер, с него можно получить ответ, заранее проверив, есть ли какие-либо сообщения от контроллера в ожидании, и отправить его в соответстующий топик о состоянии LED:
```py
    time.sleep(0.1)
    if connection.in_waiting > 0:
        line = connection.readline().decode().strip()
        print(f'Got line from MCU: {line}')
        print(f'Publish LED response - {line} to {led_state_topic}')
        client.publish(led_state_topic, line, qos=2)
```

# Monitor
Аналогичным другим подключениям способом подписан на все топики и просто выводит сообщения с них в консоль.