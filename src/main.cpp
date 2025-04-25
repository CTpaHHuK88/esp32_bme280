#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ArduinoJson.h>

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme; // I2C

void setup() {
  Serial.begin(115200);
  
  if (!bme.begin(0x76)) { // Адрес 0x76 или 0x77
    Serial.println("Не удалось найти BME280 датчик!");
    while (1);
  }
}

void loop() {
  Serial.print("Температура = ");
  Serial.print(bme.readTemperature());
  Serial.println(" *C");
  
  double mm = (bme.readPressure() / 100.0F)/ 1.33322;

  Serial.print("Давление = ");
  Serial.print(mm);
  Serial.println(" mmHg");
  
  Serial.print("Влажность = ");
  Serial.print(bme.readHumidity());
  Serial.println(" %");
  
  Serial.print("Высота = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");  
  Serial.println();
  delay(5000);

/*Формируем json документ. Для работы используем библиотеку: 
#include <ArduinoJson.h>*/

  JsonDocument doc;
/*Добавляем данные*/
  doc["temperature"] = bme.readTemperature(); /*Температура*/
  doc["pressure"] = mm; /*Давление*/
  doc["humidity"] = bme.readHumidity(); /*Влажность*/
  doc["altitude"] = bme.readAltitude(SEALEVELPRESSURE_HPA); /*Высота над уровнем моря*/
/*Возвращаясь к нашему коду для Arduino, чтобы преобразовать документ в строку JSON, 
нам понадобится буфер символов для его хранения.*/
  char buffer[100];
/*Затем, чтобы получить строку JSON, нам нужно просто вызвать функцию serializeJson. 
В качестве первого параметра нам нужно передать наш объект StaticJsonDocument, 
а в качестве второго параметра — буфер символов, который мы ранее объявили.
Обратите внимание, что этот вызов функции выведет минимизированную строку, то есть 
в ней не будет пробелов или разрывов строк [5]. Естественно, такая строка занимает меньше места, 
но её гораздо сложнее читать человеку. Если вы хотите получить удобочитаемую строку
(подходящую для чтения человеком), вам следует использовать функцию serializeJsonPretty.*/
  serializeJson(doc, buffer);
/*В завершение мы выведем содержимое, полученное в нашем буфере символов. Должна отображаться строка, 
содержащая наш документ JSON.*/
  Serial.println(buffer);

}