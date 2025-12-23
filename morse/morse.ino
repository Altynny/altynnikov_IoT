// __Пины передачи информации__
const uint8_t tx = 3;
const uint8_t rx = 2;

// __Базовая единица времени, погрешность и длительность бита__
const uint8_t base = 250;
const uint8_t e = 50;
volatile unsigned long tm = 0;

// __Состояния контроллера__
const uint8_t IDLE = 0;
const uint8_t READ = 1;
const uint8_t WRITE = 2;
volatile uint8_t state = IDLE;

// __Переменные для режима WRITE__
volatile char outgoingMessage[64];
volatile uint8_t outgoingLen = 0;
volatile uint8_t outgoingIdx = 0;
volatile uint8_t morseIdx = 0;
volatile const char* currentMorseCode = nullptr;
volatile uint8_t txState = 0;
volatile unsigned long txTargetDuration = 0;

volatile char buffer[6];
volatile uint8_t bufferIdx = 0;
volatile uint8_t currBit = 1;

void addSignal(char signal) {
  if (bufferIdx < 5) {
    buffer[bufferIdx] = signal;
    bufferIdx++;
    buffer[bufferIdx] = '\0';
  }
}

void resetBuffer() {
  bufferIdx = 0;
  buffer[0] = '\0';
}

// __Перевод__
const char letterAlphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
const char* morseAlphabet[] = {
  ".-", "-...", "-.-.", "-..", ".", "..-.", "--.", "....", "..", ".---",
  "-.-", ".-..", "--", "-.", "---", ".--.", "--.-", ".-.", "...", "-",
  "..-", "...-", ".--", "-..-", "-.--", "--..",
  "-----", ".----", "..---", "...--", "....-", ".....", "-....", "--...", "---..", "----."
};

const char* charToMorse(char c) {
  for (int i = 0; i < sizeof(letterAlphabet) - 1; i++) {
    if (letterAlphabet[i] == c) {
      return morseAlphabet[i];
    }
  }
  return nullptr;
}

void decodeBuffer() {
  for (int i = 0; i < sizeof(morseAlphabet) / sizeof(morseAlphabet[0]); i++) {
    if (strcmp((const char*)buffer, morseAlphabet[i]) == 0) {
      Serial.print(letterAlphabet[i]);
      return;
    }
  }
  Serial.print("?");
}

void setup() {
  cli();
  
  pinMode(tx, OUTPUT);
  pinMode(rx, INPUT_PULLUP);
  digitalWrite(tx, HIGH);
  
  TCCR2A = 0;
  TCCR2B = 0;
  TCNT2 = 0;
  OCR2A = 125;
  TCCR2A |= 1 << WGM21;
  TCCR2B |= (1 << CS22) | (1 << CS20); // 128
  TIMSK2 |= 1 << OCIE2A;
  
  Serial.begin(9600);
  sei();
}

ISR(TIMER2_COMPA_vect) {
  switch (state) {
    case IDLE:
      handleIdle();
      break;
    case READ:
      handleRead();
      break;
    case WRITE:
      handleWrite();
      break;
  }
}

void handleIdle() {
  if (digitalRead(rx) == 0) {
    state = READ;
    resetBuffer();
    tm = 0;
    return;
  }
  
  if (outgoingIdx < outgoingLen) {
    state = WRITE;
    txState = 0;
    tm = 0;
    currentMorseCode = nullptr;
    morseIdx = 0;
  }
}

void handleRead() {
  uint8_t inBit = digitalRead(rx);
  
  if (inBit == currBit) {
    tm++;
  } 
  else {
    // HIGH -> LOW
    if (currBit == 1) {
      // 1 TU
      if (tm >= base - e && tm <= base + e) {
        addSignal('.');
        tm = 0;
      } 
      // 3 TU
      else if (tm >= 3 * base - e && tm <= 3 * base + e) {
        addSignal('-');
        tm = 0;
      }
    } 
    // LOW -> HIGH
    else {
      // 1 TU - следующий символ Морзе
      if (tm >= base - e && tm <= base + e) {
        tm = 0;
      }
      // 3 TU - следующий символ алфавита
      else if (tm >= 3 * base - e && tm <= 3 * base + e) {
        decodeBuffer();
        resetBuffer();
        tm = 0;
      } 
      // 7 TU - пробел
      else if (tm >= 7 * base - e && tm <= 7 * base + e) {
        decodeBuffer();
        resetBuffer();
        Serial.print(" ");
        tm = 0;
      }
      // >7TU - конец записи
      else if (tm >= 7 * base + e) {
        decodeBuffer();
        resetBuffer();
        Serial.println();
        tm = 0;
        state = IDLE;
      }
    }
    currBit = inBit;
  }
}

void handleWrite() {
  tm++;
  
  // Начало записи HIGH->LOW на 1 TU
  if (txState == 0) {
    digitalWrite(tx, LOW);
    txTargetDuration = base;
    if (tm >= txTargetDuration) {
      tm = 0;
      txState = 1;
    }
    return;
  }
  
  // Определение последующей записи в зависимости от символа строки
  if (txState == 1) {
    if (currentMorseCode == nullptr || currentMorseCode[morseIdx] == '\0') {
      if (outgoingIdx < outgoingLen) {
        char c = outgoingMessage[outgoingIdx];
        outgoingIdx++;
        
        // Пробел LOW на 4 TU
        if (c == ' ') {
          digitalWrite(tx, LOW);
          txState = 4;
          txTargetDuration = 4 * base;
          tm = 0;
          return;
        }
        // Получение кода символа для дальнейшней записи 
        else {
          currentMorseCode = charToMorse(c);
          morseIdx = 0;
          if (currentMorseCode == nullptr) {
            return;
          }
        }
      }

      // Конец записи LOW на >7 TU
      else {
        digitalWrite(tx, LOW);
        txTargetDuration = 8 * base;
        tm = 0;
        txState = 5;
        return;
      }
    }
    
    // Запись символа, хранящегося в currentMorseCode
    if (currentMorseCode != nullptr && currentMorseCode[morseIdx] != '\0') {
      char symbol = currentMorseCode[morseIdx];
      digitalWrite(tx, HIGH);
      txTargetDuration = (symbol == '.') ? base : (3 * base);
      tm = 0;
      txState = 2;
    }
    return;
  }
  
  // Прекращение записи по достижению целевого TU
  if (txState == 2) {
    if (tm >= txTargetDuration) {
      digitalWrite(tx, LOW);
      txTargetDuration = base;
      tm = 0;
      morseIdx++;
      
      if (currentMorseCode[morseIdx] == '\0') {
        txState = 3;
        txTargetDuration = 3 * base;
      } 
      else {
        txState = 3;
        txTargetDuration = base;
      }
    }
    return;
  }
  
  // Обнуление ссылки на код морзе по достижению его конца
  if (txState == 3) {
    if (tm >= txTargetDuration) {
      tm = 0;
      
      if (currentMorseCode[morseIdx] == '\0') {
        currentMorseCode = nullptr;
        morseIdx = 0;
      }
      
      txState = 1;
    }
    return;
  }
  
  // Прекращение записи пробела, начало записи следующего символа
  if (txState == 4) {
    if (tm >= txTargetDuration) {
      tm = 0;
      txState = 1;
    }
    return;
  }
  
  // Конец передачи постоянный HIGH после LOW >7TU
  if (txState == 5) {
    if (tm >= txTargetDuration) {
      digitalWrite(tx, HIGH);
      outgoingLen = 0;
      outgoingIdx = 0;
      currentMorseCode = nullptr;
      morseIdx = 0;
      tm = 0;
      txState = 0;
      state = IDLE;
    }
    return;
  }
}

void loop() {
  if (state == IDLE && Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    input.toUpperCase();
    int len = input.length();
    if (len > 0 && len < 64) {
      for (int i = 0; i < len; i++) {
        outgoingMessage[i] = input[i];
      }
      outgoingLen = len;
      outgoingIdx = 0;
      Serial.print("Sending: ");
      Serial.println(input);
    }
  }
}