#include <avr/sleep.h>        // Подключаем встроенную библиотеку для работы сна
#include <avr/power.h>        // Подключаем встроенную библиотеку для включения\выключения модулей в МК
#include <GyverBME280.h>      // Подключяем библиотеку датчика
#include <OLED_I2C.h>         // Подключяем библиотеку экрана

GyverBME280 bme;              // Создаем обьекта bme
OLED  myOLED(SDA, SCL, 8);    // Настраиваем экран

int timer = 0;                  // Задаем переменную для подсчета времени работы в активном режиме
byte iniy = 0;                  // Задаем байт для хранения инцилизации
byte InterruptPin = 3;          // Задаем байт указывающий к какому пину подключена кнопка для выхода из сна

extern uint8_t SmallFont[];     // Стандартный шрифт
extern uint8_t RusFont[];       // Русский шрифт
extern uint8_t BigNumbers[];    // Большие цифры
extern uint8_t MediumNumbers[]; // Средние цифры

void setup() {
  pinMode(2, OUTPUT);                     // Объявляем что 2 пин это выход
  pinMode(4, OUTPUT);                     // Объявляем что 4 пин это выход
  digitalWrite(4, HIGH);                  // Подаем на 4 пин 5В
  pinMode(InterruptPin, INPUT_PULLUP);    // Включаем подтяжку к питанию для кнопки выхода из сна
}

void loop() {
  if (iniy == 0) {                                         // Если бит "iniy" равен нулю, идем инцилизироваться
    power_twi_enable();                                    // Включаем i2c
    digitalWrite(2, HIGH);                                 // Включем питание i2c устройств
    delay(40);                                             // Ждем когда очнутся все что связанно i2c
    myOLED.begin();                                        // Инициализируем дисплей
    myOLED.setBrightness(127);                             // Выставляем яркость
                                           
    myOLED.clrScr();                                       // Выводим строки текста загрузочного экрана с задержкой
    myOLED.setFont(SmallFont);
    myOLED.print("Copyright (C) 2021", 0, 0);
    myOLED.update();
    delay(500);
    myOLED.print("ATmega328P at 16 MHz", CENTER, 15);
    myOLED.update();
    delay(500);
    myOLED.print("Memory : 32768K...OK", CENTER, 25);
    myOLED.update();
    delay(500);
    myOLED.print("128X64 OLED LCD...OK", CENTER, 35);
    myOLED.update();
    delay(500);
    myOLED.print("BME280............OK", CENTER, 45);
    myOLED.update();
    delay(500);
    myOLED.print("Loading Station OS...", 0, 55);
    myOLED.update();
    delay(3000);
    
    iniy = 1;                                              // Присваиваем биту "iniy" единицу, чтоб повторно не инцилизироватся
  }
  
  if (iniy == 1) {                                         // Если бит "iniy" равен единице, идем инцилизироваться
    power_twi_enable();                                    // Включаем i2c
    detachInterrupt(digitalPinToInterrupt(InterruptPin));  // Выключаем прерывание
    digitalWrite(2, HIGH);                                 // Включем питание i2c устройств
    delay(40);                                             // Ждем когда очнутся все что связанно i2c
    bme.begin();                                           // Инициализируем датчик
    myOLED.begin();                                        // Инициализируем дисплей
    myOLED.setBrightness(127);                             // Выставляем яркость
    iniy = 2;                                              // Присваиваем биту "iniy" двойку, чтоб повторно не инцилизироватся
  }
    
  else if (timer <= 5) {                                   // Если переменная меньше 5
    //digitalWrite(LED_BUILTIN, HIGH);                     // Включаем отладочный светодиод
    PORTB |= (1<<5);                                       // То же самое, но напрямую через порт B5

    float p = bme.readPressure();                          // Присваиваем переменную давлению

    myOLED.clrScr();                                       // Очищаем экран

    myOLED.drawRoundRect(0, 13, 127, 41);                  // Рисуем прямоугольник со скругленными краями
    myOLED.drawLine(80, 13, 80, 41);                       // Рисуем линию

    myOLED.setFont(BigNumbers);                                  // Подключаем большие цифры
    myOLED.print(String(bme.readTemperature() - 1, 1), 2, 15);   // Выводим показания и координаты температуры (Х,У) ( добавлена -1 С погрешность)
    myOLED.print(String(bme.readHumidity() - 7, 0), 87, 15);    // Выводим показания и координаты влажности (Х,У)( добавлена -7% погрешность)

    myOLED.setFont(SmallFont);                                   // Подключаем мальенький шрифт
    myOLED.print(String(pressureToMmHg(p), 0), 53, 45);          // Выводим показания и координаты давления (Х,У)
    myOLED.print(String(pressureToAltitude(p) - 26, 1), 82, 55); // Выводим показания и координаты высоты (Х,У)( добавлена -26 м высота над уровнем моря)
    myOLED.print("~C", 58, 15);                                  // Градус цельсия
    myOLED.print("%", 117, 15);                                  // Процент

    myOLED.setFont(RusFont);                                     // Подключаем русский шрифт
    myOLED.print("Vtntjcnfywbz vbrhj", CENTER, 0);               // Метеостанция микро
    myOLED.print("Lfdktybt", 0, 45);                             // Давление
    myOLED.print("vv hncn", 76, 45);                             // мм рт ст
    myOLED.print("Rfr yf dscjnt", 0, 55);                        // Как на высоте
    myOLED.print("v", 115, 55);                                  // м

    myOLED.update();                                             // Обновление экрана

    delay(2000);                                                 // Пауза 2 с
    //_delay_us(2000);                                           // То же самое, но напрямую (не работает)
    timer = timer + 1;                                           // Добавляем к переменной 1
  }
  else {
    myOLED.clrScr();                                                  // Очищяем дисплей
    myOLED.setFont(RusFont);                                          // Подключаем большие цифры
    myOLED.print("Cgjrjqyjq yjxb!", CENTER, 31);                      // Пишем на дисплей то, что мы уходим спать
    myOLED.update();
    delay(1000);                                                      // Ждем cекунду
    digitalWrite(2, LOW);                                             // Выключаем устройства на шине i2c
    power_twi_disable();                                              // Выключаем сам i2c в МК
    digitalWrite(A4, LOW);                                            // Приводим состояние SDA к логическому нулю
    digitalWrite(A5, LOW);                                            // Приводим состояние SCL к логическому нулю
    //digitalWrite(LED_BUILTIN, LOW);                                 // Выключаем отладочный светодиод
    PORTB &= ~(1<<5);                                                 // То же самое, но напрямую через порт B5
    attachInterrupt(digitalPinToInterrupt(InterruptPin), myISR, HIGH);// Разрешаем прерывание по высокому уровню
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);                              // Если спать, то на всю
    sleep_mode();                                                     // Спать!
    // По выходу из спящего режима, и после выполнения условия по прерыванию, МК продолжит свою работу отсюда
    delay(100);                                                       // Пауза 100 мкс
    //_delay_us(100);                                                 // То же самое, но напрямую (не работает)
    // Переходим к началу цикла
  }
}

void myISR() {           // Если случилось прерывание, то заходим в эту функцию, и следственно мы просыпаемся
  iniy = 1;              // Сбрасываем бит "iniy", для переинцилизации устройств на i2c
  timer = 0;             // Сбрасываем переменную "timer"
}                        // Уходим в основной цикл в место после сна
