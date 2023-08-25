#define THINGER_SERIAL_DEBUG
#define _DISABLE_TLS_
// #define id "-1001747424601"

#include <ThingerESP32.h>
#include <DHTesp.h>
#include <LiquidCrystal_I2C.h>
#include <CTBot.h>
#include <WiFi.h>

// setting Telegram Bot
TBMessage tMessage;
String tokenbot = "5115284087:AAFw0dRJRYSetvYttlJDw1nzP2EzzhKqy_I"; // Token Bot
CTBot myBot;
const long id = -1001747424601;

// 6061511087:AAFOqFWhRotndP6pEkbIXXmtpQg_45MDRdE

// TBMessage tMessage;
// String tokenbot = "5969913440:AAELWQbCduXOxqD-o8x4otbFPnF2ML8h2wA";
// CTBot myBot;
// const int id = 708984228;

// setting thinger.io device
#define IOT_SERVER "ESP32"
#define IOT_USER "puti_ittelkomsby"
#define IOT_CRED "mp8Z?F!FHv_CZFIi"

// setting WiFi ESP32
#define ssid "Ruang_PUTI"
#define pass "Put1Put1"

// setting DHT
#define DHT_PIN 15

// setting variabel hitung
long suhu = 0;
float kelembapan = 0;

// declaration hardware
ThingerESP32 thing(IOT_USER, IOT_SERVER, IOT_CRED);

DHTesp dht;

LiquidCrystal_I2C lcd(0x27, 16, 2);
byte SimbolDerajat[] = {
    B00110,
    B01001,
    B01001,
    B00110,
    B00000,
    B00000,
    B00000,
    B00000,
};

void connectWifi();
void loginTelegram();
void sensorDHT();

void setup()
{
  // put your setup code here, to run once:
  lcd.init();
  lcd.createChar(0, SimbolDerajat);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("PuTi x ");
  lcd.setCursor(0, 1);
  lcd.print("AIR Team");
  delay(4000);
  lcd.clear();

  Serial.begin(115200);

  dht.setup(DHT_PIN, DHTesp::DHT22);

  // thing.add_wifi(SSID, password);
  connectWifi();
  loginTelegram();

  thing["sensorSuhuKelembapan"] >> [](pson &out)
  {
    sensorDHT();

    if (isnan(suhu) || isnan(kelembapan))
    {
      Serial.println("Sensor belum kebaca");
      suhu = 0;
      kelembapan = 0;
      return;
    }

    out["kelembapan"] = (long)kelembapan;
    out["suhu"] = (long)suhu;
  };
}

bool msgSentTemp = false;
bool msgSentHum = false;
bool timerTemp = false;
bool timerHum = false;
unsigned long previousTimeTemp = 0;
unsigned long previousTimeHum = 0;
const int resetinterval = 7200000; // ngirim pesan interval 1 jam

void loop()
{
  delay(20);
  thing.handle();
  TempAndHumidity data = dht.getTempAndHumidity();
  // Serial.print("Suhu : ");
  // Serial.print(data.temperature);
  // Serial.print(" Kelembaban : ");
  // Serial.println(data.humidity);

  lcd.setCursor(0, 0);
  lcd.print("Suhu   :" + String(data.temperature));
  lcd.write(0);
  lcd.print("C");

  lcd.setCursor(0, 1);
  lcd.print("Klmbapn:" + String(data.humidity) + "%");

  if (data.temperature <= 18 && !msgSentTemp)
  {
    String reply = "";
    reply += "Temperature is to LOW!!!\n";
    reply += "Temperature reading now : ";
    reply += String(data.temperature) + " °C \n";
    reply += "Please, higher the room temperature\n";
    myBot.sendMessage(tMessage.group.id, reply);
    msgSentTemp = true;
  }
  else if (data.temperature >= 25 && !msgSentTemp)
  {
    String reply = "";
    reply += "Temperature is to HIGH!!!\n";
    reply += "Temperature reading now : ";
    reply += String(data.temperature) + " °C \n";
    reply += "Please, Lower the room temperature\n";
    myBot.sendMessage(tMessage.group.id, reply);
    msgSentTemp = true;
  }

  if (data.humidity <= 40 && !msgSentHum)
  {
    String reply = "";
    reply += "Humidity is to LOW!!!\n";
    reply += "Humidity reading now : ";
    reply += String(data.humidity) + " % \n";
    reply += "Please, higher the room humidity\n";
    myBot.sendMessage(tMessage.group.id, reply);
    msgSentHum = true;
  }
  else if (data.humidity >= 60 && !msgSentHum)
  {
    String reply = "";
    reply += "Humidity is to HIGH!!!\n";
    reply += "Humidity reading now : ";
    reply += String(data.humidity) + " % \n";
    reply += "Please, lower the room humidity\n";
    myBot.sendMessage(tMessage.group.id, reply);
    msgSentHum = true;
  }

  unsigned long currentTime = millis();
  Serial.println("Current Time");
  Serial.println(currentTime);

  if ((data.temperature <= 18 || data.temperature >= 25) && msgSentTemp && !timerTemp)
  {
    previousTimeTemp = currentTime;
    timerTemp = true;
  }
  else if (data.temperature >= 18 && data.temperature <= 25 && msgSentTemp && timerTemp)
  {
    timerTemp = false;
    msgSentTemp = false;
  }

  if ((data.humidity < 40 || data.humidity > 60) && msgSentHum && !timerHum)
  {
    previousTimeHum = currentTime;
    timerHum = true;
  }
  else if (data.humidity >= 40 && data.humidity <= 60 && msgSentHum && timerHum)
  {
    timerHum = false;
    msgSentHum = false;
  }

  unsigned long resetTemp = currentTime - previousTimeTemp;
  unsigned long resetHum = currentTime - previousTimeHum;

  if (resetTemp >= resetinterval && timerTemp)
  {
    msgSentTemp = false;
    timerTemp = false;
  }

  if (resetHum >= resetinterval && timerHum)
  {
    msgSentHum = false;
    timerHum = false;
  }

  if (timerTemp)
  {
    Serial.println("Current Time Temp :");
    Serial.println(resetTemp);
    Serial.println("Previous Time Temp :");
    Serial.println(previousTimeTemp);
  }

  if (timerHum)
  {
    Serial.println("Current Time Hum  :");
    Serial.println(resetHum);
    Serial.println("Previous Time Hum  :");
    Serial.println(previousTimeHum);
  }

  Serial.println("MSGTemp is sent (1) or not (0):");
  Serial.println(msgSentTemp);
  Serial.println("MSGHum is sent (1) or not (0):");
  Serial.println(msgSentHum);
  Serial.println("TimerTemp is active (1) or not (0):");
  Serial.println(timerTemp);
  Serial.println("TimerHum is active (1) or not (0):");
  Serial.println(timerHum);

  if (myBot.getNewMessage(tMessage))
  {
    // Serial.println(tMessage.sender.firstName);
    // Serial.println(tMessage.sender.id);
    // Serial.println(tMessage.text);
    if (tMessage.messageType == CTBotMessageText)
    {
      if (tMessage.text.equalsIgnoreCase("/start"))
      {
        String reply = "";
        reply += "Hi, Welcome to Unit Pusat Teknologi Informasi ITTS \n";
        reply += "Command List as below : \n";
        reply += "Get Temperature : /get_temp \n";
        reply += "Get Humidity : /get_hum \n";
        myBot.sendMessage(tMessage.group.id, reply);
      }
      else if (tMessage.text.equalsIgnoreCase("/get_temp"))
      {
        String reply = "";
        reply += "Temperature Reading : \n";
        reply += String(data.temperature) + " °C";
        myBot.sendMessage(tMessage.group.id, reply);
      }
      else if (tMessage.text.equalsIgnoreCase("/get_hum"))
      {
        String reply = "";
        reply += "Humidity Reading : \n";
        reply += String(data.humidity) + " %";
        myBot.sendMessage(tMessage.group.id, reply);
      }
      else
      {
        String reply = "";
        reply += "Hi, Welcome to Unit Pusat Teknologi Informasi ITTS \n";
        reply += "Command is not valid, please use /start to get Command List \n";

        myBot.sendMessage(tMessage.group.id, reply);
      }
    }
  }
}

void sensorDHT()
{
  TempAndHumidity data = dht.getTempAndHumidity();

  kelembapan = data.humidity;
  suhu = data.temperature;
  // Serial.println(kelembapan);
  // Serial.println(suhu);
}

void connectWifi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(100);
    // Serial.println("Connecting...");
    lcd.setCursor(0, 0);
    lcd.print("Connecting WiFi...");
  }

  lcd.setCursor(0, 0);
  lcd.print("Wifi Connected!");
  delay(2000);
  lcd.clear();
  // Serial.println("Wifi Connected!");
}

void loginTelegram()
{
  // Serial.println("Logging in...");
  lcd.setCursor(0, 0);
  lcd.print("Log in Telegram...");
  while (!myBot.testConnection())
  {
    myBot.setTelegramToken(tokenbot);
    delay(1000);
  }
  lcd.setCursor(0, 0);
  lcd.print("Telegram Connected!");
  lcd.clear();
  // Serial.println("Telegram Connection OK!");
}