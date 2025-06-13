#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <esp_sleep.h>

#define SEALEVELPRESSURE_HPA (1013.25)
#define uS_TO_S_FACTOR 1000000  // Коэффициент перевода микросекунд в секунды
#define TIME_TO_SLEEP  300        // Время сна в секундах

bool statusSendTbotMsg = false; //Инициализируем переменную для проверки статуса отправки сообщения Telegram
const int adcPin = 34; // GPIO34 (аналоговый пин) для считывания напряжения.
const char* CHAT_ID = "";
const char* SSID = "";
const char* PASSWORD_WIFI = "";
const char* ESP_HOSTNAME = ""; //Устанавливаем Имя хоста
const char* mqtt_server = "";
const int mqtt_port = ;
const char* mqtt_topic = "";
RTC_DATA_ATTR int SCHET_MESSAGE = 0;

WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_BME280 bme; // I2C
// Настройка WiFi
void connectWiFi(){
  WiFi.begin(SSID, PASSWORD_WIFI);
  WiFi.setHostname(ESP_HOSTNAME); //Имя хоста
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  //Serial.println("Connected to WiFi");
  //Serial.println("IP-адрес:"); // Вывод IP-адреса ESP32
  //Serial.println(WiFi.localIP()); // Вывод IP-адреса ESP32
  //Serial.println("MAC-адрес:"); // Вывод IP-адреса ESP32
  //Serial.println(WiFi.macAddress()); // Вывод IP-адреса ESP32

}
int schetchik() {
  SCHET_MESSAGE ++;
  return SCHET_MESSAGE;
}

float readPowerOnPin(){
  int rawValue = analogRead(adcPin);
  float voltage = rawValue * (3.3 / 4095.0); // Преобразование в напряжение
  /*
  Serial.print("Raw: ");
  Serial.print(rawValue);
  Serial.print(" | Voltage: ");
  Serial.print(voltage, 3); // 3 знака после запятой
  Serial.println("V");  
  delay(1000);
  */
  return voltage;
}


void reconnectMQTT(){
  while (!client.connected()){
    Serial.print("Попытка подключиться к MQTT...\n");
    if (client.connect("ESP32Client")) {
      Serial.println("Подключено!");

    }else{
      Serial.print("Ошибка, rc=");
      Serial.print(client.state());
      Serial.println(" пробуем снова через 5 сек.");
      delay(5000);
    }
  }
}

void sendMQTTMessage(const char* message){
  if (client.connected()){
    client.publish(mqtt_topic, message);
    Serial.print("Отправлено:");
    Serial.println(message);
  }
}


void setup() {
  Serial.begin(115200);  
  connectWiFi();  
  client.setServer(mqtt_server, mqtt_port);
  reconnectMQTT();
  if (!bme.begin(0x76)) { // Адрес 0x76 или 0x77
    Serial.println("Не удалось найти BME280 датчик!");
    while (1);
  }


    //Serial.print("Температура = ");
    //Serial.print(bme.readTemperature());
    //Serial.println(" *C");
    
    double mm = (bme.readPressure() / 100.0F)/ 1.33322;

    //Serial.print("Давление = ");
    //Serial.print(mm);
    //Serial.println(" mmHg");
    
    //Serial.print("Влажность = ");
    //Serial.print(bme.readHumidity());
    //Serial.println(" %");
    
    //Serial.print("Высота = ");
    //Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
    //Serial.println(" m");  
    //Serial.println();
    delay(1000);

  /*Формируем json документ. Для работы используем библиотеку: 
  #include <ArduinoJson.h>*/

    JsonDocument doc;
  /*Добавляем данные*/
    doc["hostname"] = ESP_HOSTNAME;
    doc["macaddress"] = WiFi.macAddress();
    doc["ipclient"] = WiFi.localIP();
    doc["temperature"] = bme.readTemperature(); /*Температура*/
    doc["pressure"] = mm; /*Давление*/
    doc["humidity"] = bme.readHumidity(); /*Влажность*/
    doc["altitude"] = bme.readAltitude(SEALEVELPRESSURE_HPA); /*Высота над уровнем моря*/
    doc["voltage"] = readPowerOnPin();
    doc["schetchik"] = "msg_"+String(schetchik());


  /*Возвращаясь к нашему коду для Arduino, чтобы преобразовать документ в строку JSON, 
  нам понадобится буфер символов для его хранения.*/
    char buffer[1200];
  /*Затем, чтобы получить строку JSON, нам нужно просто вызвать функцию serializeJson. 
  В качестве первого параметра нам нужно передать наш объект StaticJsonDocument, 
  а в качестве второго параметра — буфер символов, который мы ранее объявили.
  Обратите внимание, что этот вызов функции выведет минимизированную строку, то есть 
  в ней не будет пробелов или разрывов строк [5]. Естественно, такая строка занимает меньше места, 
  но её гораздо сложнее читать человеку. Если вы хотите получить удобочитаемую строку
  (подходящую для чтения человеком), вам следует использовать функцию serializeJsonPretty.*/
    serializeJson(doc, buffer);

    //Serial.print("Напряжение:");
    //Serial.println(readPowerOnPin());
      /*В завершение мы выведем содержимое, полученное в нашем буфере символов. Должна отображаться строка, 
  содержащая наш документ JSON.*/
    //Serial.println(buffer);
    client.loop();

    sendMQTTMessage(buffer);
   
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  delay(1000);
  esp_deep_sleep_start();  // Переход в Deep Sleep
  
}

void loop() {

}
