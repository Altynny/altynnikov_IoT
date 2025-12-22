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
  TCCR2B |= (1 << CS22) | (1 << CS20);
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

void handleWrite() {
  tm++;
  
  if (txState == 0) {
    digitalWrite(tx, LOW);
    txTargetDuration = base;
    if (tm >= txTargetDuration) {
      tm = 0;
      txState = 1;
    }
    return;
  }
  
  if (txState == 1) {
    if (currentMorseCode == nullptr || currentMorseCode[morseIdx] == '\0') {
      if (outgoingIdx < outgoingLen) {
        char c = outgoingMessage[outgoingIdx];
        outgoingIdx++;
        
        if (c == ' ') {
          digitalWrite(tx, LOW);
          txState = 4;
          txTargetDuration = 4 * base;
          tm = 0;
          return;
        } 
        else {
          currentMorseCode = charToMorse(c);
          morseIdx = 0;
          if (currentMorseCode == nullptr) {
            return;
          }
        }
      } 
      else {
        digitalWrite(tx, LOW);
        txTargetDuration = 8 * base;
        tm = 0;
        txState = 5;
        return;
      }
    }
    
    if (currentMorseCode != nullptr && currentMorseCode[morseIdx] != '\0') {
      char symbol = currentMorseCode[morseIdx];
      digitalWrite(tx, HIGH);
      txTargetDuration = (symbol == '.') ? base : (3 * base);
      tm = 0;
      txState = 2;
    }
    return;
  }
  
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
  
  if (txState == 4) {
    if (tm >= txTargetDuration) {
      tm = 0;
      txState = 1;
    }
    return;
  }
  
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