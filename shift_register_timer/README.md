# Ссылки
[Схема на Tinkercad](https://www.tinkercad.com/things/4hFNRvGSK1L-cascadingregisters?sharecode=EWQbBMdIAwOZ3Z2dbsneuAUzSf4wWBMpy3UwwGjCgww)

Видеодемонстрации:
- [Продолжение счёта после ввода пользователя](https://drive.google.com/file/d/1bQ9RzJQw9Of3wWkeNFYhrTgs5eUp3Brc/view?usp=sharing)
- [Синхронизация с таймером на ноутбуке](https://drive.google.com/file/d/1NxrA6M3vpmpRkB0HOVBbrUQARHq9MAsA/view?usp=sharing)

# Инициализация
По умолчанию дана матрица сигналов для семисегментных дисплеев с общим катодом, если же у контроллера задан анод - матрица инвертируется:
```cpp
if (common_anode == true)
    for (int digit = 0; digit < 10; digit++)
      for (int i = 0; i < 8; i++)
    	digits[digit][i] = !digits[digit][i];
```

Для отправки сигналов data, clock, pin используется PORTD (цифровые пины 0-7). Timer1 (16 бит), prescaler = 1 024, величина для сравнения = 15 625, прерывания происходят каждые 15 625 * 1 024 / 16 000 000 сек = 1 сек, каждую секундну растёт счётчик секунд.
```cpp
  DDRD |= (1 << dataPin) | (1 << latchPin) | (1 << clockPin);
  TCCR1A = 0;
  TCCR1B = 0;
  TCNT1 = 0;
  OCR1A = 15625; // значение для сравнения
  TCCR1B |= (1 << WGM12); // очистка таймера при сравнении 
  TCCR1B |= (1 << CS12) | (1 << CS10); // 1024 prescaler
  TIMSK1 |= (1 << OCIE1A); // прерывания по сравнению
```
# Принцип работы
Чтобы таймер следовал поведению 59->00, от секунд берётся остаток от деления на 60:
```cpp
ISR(TIMER1_COMPA_vect) {
  sec %= 60;
  show_number();
  ++sec;
}
```

Для переведения пинов в состояния **LOW** нужный бит инвертируется и объединяется побитным **И** с портом **D**. Пример подачи **LOW** на 3 пин, предположим, что у нас на пинах 2-4 **HIGH**:
```
PORTD = 0000 1110
~(1 << 3) = ~(0000 0100) = 1111 1011
PORTD &= ~(1 << 3) = 0000 1110 & 1111 1011 = 0000 1010 - пин 3 обнулился, значения остальных остались прежними
```
Для отображения секунд сначала подаётся **LOW** на пин **Latch**, потом в цикле побитно отправляются маски десятков и единиц таймера. Для каждого бита сначала обнуляется пин **Data**, потом на него выводится значение. Сигнал **HIGH** на пине **Clock** даёт регистру команду совершения сдвига. По завершению отправки обеих масок подаётся сигнал **HIGH** на пин **Latch**, все биты выводятся на выходы регистра.
```cpp
void show_number() {
  PORTD &= ~(1 << latchPin);
  for (int digit : (int[]){sec / 10, sec % 10})
  for (int i = 7; i >= 0; i--)
    shift_and_set(digits[digit][i]);
  PORTD |= 1 << latchPin;
}

void shift_and_set(bool val) {
  PORTD &= ~(1 << dataPin);
  PORTD |= (int)val << dataPin;
  PORTD |= 1 << clockPin;
  PORTD &= ~(1 << clockPin);
}
```

В главном цикле реализована только логика изменения значения таймера по пользовательскому вводу. Некорректные значения игнорируются. Поскольку переменная секунд модифицируется в таймере, прерывания перед записью временно отключаются:
```cpp
void update_starting_time() {
	if (Serial.available()) {
    	uint8_t new_time = Serial.parseInt();
      	if (new_time >= 0 && new_time < 60) {
      		cli();
      		sec = new_time;
      		sei();
        }
    }
}

void loop() 
{
  update_starting_time();
}
```
# Проверка
Для проверки используется простой [код](py.py) приложения на python с использованием tkinter. Приложение состоит из окна с таймером и кнопки, в процессе инициализации происходит соединение с микроконтроллером:
```py
import tkinter as tk
import serial
import time

class Timer:
    def __init__(self, root: tk.Tk) -> None:
      # ...
      self.connection = serial.Serial('COM5', 9600, timeout=1)
      time.sleep(2.5)
```

По нажатии на кнопку отправляется команда на контроллер со стартовым значением 0 для синхронизации двух таймеров и начинается вечный цикл обновления таймера приложения:
```py
def start_timer(self) -> None:
    self.seconds = 0
    self.label.config(text='0', fg='green')
            
    self.connection.write(b'0')
    time.sleep(2.5)
            
    self.update_timer()
    
def update_timer(self) -> None:
    self.seconds += 1
    self.label.config(text=self.seconds)
    
    self.root.after(1000, self.update_timer)
```