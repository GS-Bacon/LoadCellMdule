#include <M5Stack.h>
#include <WiFi.h>
#include "HX711.h"
#include <ESPmDNS.h>
#include <stdio.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ETH.h>
#include <Update.h>

void triggerIFTTT(String cells,float value);

String maker_Event = "LoadCell";             // Maker Webhooks
String maker_Key = "bU4abkKjYZhGsNad_Lx65w"; // Maker Webhooks


const String ssid = "M5Stack_LoadCell_AP";
const String password = "123456789";

int counter =1;

const int DT_PIN = 22;
const int SCK_PIN = 21;
float OneWeight = 100000;
int piece = 1;
int http_code;
HTTPClient http;

String wifiSsid = "";
String wifiPassword = "";

int wifiStatus = WiFi.status();
WiFiServer server(80);
HX711 scale;

String getSsid(String path)
{
  int start = path.indexOf("?ssid=");
  int end = path.indexOf("&");
  return path.substring(start + 6, end);
}

String getPassword(String path)
{
  int start = path.indexOf("&password=");
  int end = path.length();
  return path.substring(start + 10, end);
}

void setup()
{
  M5.begin();
  // M5.Power.begin();
  M5.Lcd.begin();
  M5.Lcd.setBrightness(200);
  Serial.begin(115200);

  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);

  pinMode(DT_PIN, INPUT);
  pinMode(SCK_PIN, OUTPUT);
  scale.begin(DT_PIN, SCK_PIN);
  scale.set_scale(20618.7739);
  scale.tare();

  WiFi.softAP(ssid.c_str(), password.c_str());
  if (!MDNS.begin("esp32"))
  {
    Serial.println("Error setting up MDNS responder!");

    while (1)
    {
      delay(1000);
    }
  }
  Serial.println("mDNS responder started");
  server.begin();
  Serial.println("Web server started");
  MDNS.addService("http", "tcp", 80);
  M5.Lcd.print("SSID: ");
  M5.Lcd.print(ssid);
  M5.Lcd.print("\n\n");
  M5.Lcd.print("PASSWORD: ");
  M5.Lcd.print(password);
  M5.Lcd.print("\n\n");
  M5.Lcd.print("http://192.168.4.1/");

}
void loop()
{
  M5.update();
  float average = 0;
  M5.Lcd.setCursor(0, 0);
  M5.Lcd.setTextColor(WHITE);
  M5.Lcd.setTextSize(2);

  // Serial.println(scale.get_units(10), 1);
  if (M5.BtnA.wasPressed())
  {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.print("OffSet");
    delay(1000);
    long offset = scale.read_average(50);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.println(offset);
    scale.set_offset(offset);
    scale.tare();
    delay(500);
  }
  /*	else if (M5.BtnB.isPressed())
    {
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.print("Value");
      long value=scale.get_value(255);
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.print(value);
      delay(2000);
    }
  */
  else if (M5.BtnB.wasPressed())
  {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.print("Set weight per piece ");
    M5.Lcd.print("\n");
    delay(1000);
    OneWeight = scale.get_units(50) * 1000;
    M5.Lcd.print(OneWeight);
    M5.Lcd.print(" [g]");
    delay(1000);
  }
  else if (M5.BtnC.wasPressed())
  {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.print("Set iPhone13 Mini (140g)\n");
    delay(2000);
    long value=scale.get_value(255);
    scale.set_scale(value/0.14);
  }
  else
  {
    if (wifiSsid.length() > 0 && wifiPassword.length() > 0)
    {
      if (wifiStatus != WL_CONNECTED)
      {
        M5.Lcd.setCursor(0, 0, 1);
        M5.Lcd.fillScreen(BLACK);

        Serial.println("SSID: " + wifiSsid);
        Serial.println("Pass: " + wifiPassword);

        WiFi.begin(wifiSsid.c_str(), wifiPassword.c_str());
        while (wifiStatus != WL_CONNECTED)
        {
          delay(500);
          wifiStatus = WiFi.status();
          Serial.print(".");
          M5.Lcd.setCursor(0, 0, 1);
          M5.Lcd.println("WiFi connecting");
        }

        M5.Lcd.setCursor(0, 0, 1);
        M5.Lcd.fillScreen(BLACK);
        M5.Lcd.println("Connected!");
        delay(500);

        Serial.print("WiFi connected\r\nIP address: ");
        Serial.println(WiFi.localIP());
      }
    }
    else
    {
      WiFiClient client = server.available();
      if (!client)
      {
        return;
      }
      Serial.println("");
      Serial.println("New client");
      if (client)
      {
        Serial.println("new client");

        while (client.connected())
        {
          if (client.available())
          {
            String req = client.readStringUntil('\r');
            Serial.print("Request: ");
            Serial.println(req);

            int addr_start = req.indexOf(' ');
            int addr_end = req.indexOf(' ', addr_start + 1);
            if (addr_start == -1 || addr_end == -1)
            {
              Serial.print("Invalid request: ");
              Serial.println(req);
              return;
            }
            String path = req.substring(addr_start + 1, addr_end);
            Serial.print("Path: ");
            Serial.println(path);

            String s = "";
            if (path == "/")
            {
              IPAddress ip = client.remoteIP(); // クライアント側のIPアドレス
              String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
              s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
              s += "<!DOCTYPE html>";
              s += "<html lang=\"ja\">";
              s += "  <head>";
              s += "    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"/>";
              s += "    <style>";
              s += "      body {";
              s += "        font-size: 48px;";
              s += "        margin: 0;";
              s += "        background-color: #f2f2f2;";
              s += "        font-family: 'Arial',YuGothic,'Yu Gothic','Hiragino Kaku Gothic ProN','ヒラギノ角ゴ ProN W3','メイリオ', Meiryo,'ＭＳ ゴシック',sans-serif;";
              s += "      }";
              s += "      header {";
              s += "        width: 100%;";
              s += "        height: 120px;";
              s += "        line-height: 120px;";
              s += "        background-color: #F1B514;";
              s += "        text-align: center;";
              s += "        color: #f2f2f2;";
              s += "        font-weight: bold;";
              s += "      }";
              s += "      main {";
              s += "        padding-left: 24px;";
              s += "        padding-right: 24px;";
              s += "      }";
              s += "      input {";
              s += "        width: 100%;";
              s += "        height: 80px;";
              s += "        border: none;";
              s += "        font-size: 48px;";
              s += "        padding: 16px;";
              s += "        margin-top: 16px;";
              s += "      }";
              s += "      button {";
              s += "        width: 100%;";
              s += "        height: 120px;";
              s += "        background-color: #254DEA;";
              s += "        font-size: 48px;";
              s += "        color: #f2f2f2;";
              s += "        border: none;";
              s += "        border-radius: 60px;";
              s += "        margin-top: 40px;";
              s += "        font-weight: bold;";
              s += "      }";
              s += "    </style>";
              s += "  </head>";
              s += "  <body>";
              s += "    <header>重量測定モジュール Wi-fi接続</header>";
              s += "    <main>";
              s += "      <form method=\"GET\" action=\"/connect\">";
              s += "        <input type=\"text\" name=\"ssid\" placeholder=\"SSID\" />";
              s += "        <input type=\"password\" name=\"password\" placeholder=\"Password\" />";
              s += "        <button type=\"submit\">接続する</button>";
              s += "      </form>";
              s += "    </main>";
              s += "  </body>";
              s += "</html>";
              Serial.println("Response 200");
            }
            else if (path.startsWith("/connect"))
            {
              Serial.println("send to /connect");
              s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n";
              s += "<!DOCTYPE html>";
              s += "<html lang=\"ja\">";
              s += "  <head>";
              s += "    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\"/>";
              s += "    <style>";
              s += "      body {";
              s += "        font-size: 48px;";
              s += "        margin: 0;";
              s += "        background-color: #f2f2f2;";
              s += "        font-family: 'Arial',YuGothic,'Yu Gothic','Hiragino Kaku Gothic ProN','ヒラギノ角ゴ ProN W3','メイリオ', Meiryo,'ＭＳ ゴシック',sans-serif;";
              s += "      }";
              s += "      header {";
              s += "        width: 100%;";
              s += "        height: 120px;";
              s += "        line-height: 120px;";
              s += "        background-color: #F1B514;";
              s += "        text-align: center;";
              s += "        color: #f2f2f2;";
              s += "        font-weight: bold;";
              s += "      }";
              s += "      main {";
              s += "        padding-left: 24px;";
              s += "        padding-right: 24px;";
              s += "        text-align: center;";
              s += "      }";
              s += "      input {";
              s += "        width: 100%;";
              s += "        height: 80px;";
              s += "        border: none;";
              s += "        font-size: 48px;";
              s += "        padding: 16px;";
              s += "        margin-top: 16px;";
              s += "      }";
              s += "      button {";
              s += "        width: 100%;";
              s += "        height: 120px;";
              s += "        background-color: #254DEA;";
              s += "        font-size: 48px;";
              s += "        color: #f2f2f2;";
              s += "        border: none;";
              s += "        border-radius: 60px;";
              s += "        margin-top: 40px;";
              s += "        font-weight: bold;";
              s += "      }";
              s += "    </style>";
              s += "  </head>";
              s += "  <body>";
              s += "    <header>ロードセルモジュール WiFi接続</header>";
              s += "    <main>";
              s += "      <h4>WiFiに接続しています...</h4>";
              s += "      <h6>モニターにConnectedと表示されるまでお待ちください</h6>";
              s += "    </main>";
              s += "  </body>";
              s += "</html>";

              wifiSsid = getSsid(path);
              wifiPassword = getPassword(path);

              Serial.println("SSID: " + wifiSsid);
              Serial.println("Pass: " + wifiPassword);
              Serial.println("Response 200");
            }
            else
            {
              s = "HTTP/1.1 404 Not Found\r\n\r\n";
              Serial.println("Sending 404");
            }
            client.print(s);
            client.flush();
            client.stop();
          }
        }
      }
      Serial.println("Done with client");
    }
    average = scale.get_units(20) * 1000;
    M5.Lcd.fillScreen(BLACK);
    float sum = 0;
    sum += average / OneWeight;
    sum += average / (OneWeight * 1.1);
    sum += average / (OneWeight * 0.9);
    piece = round(sum / 3);

    M5.Lcd.printf("%d piece\n\n", piece);
    M5.Lcd.printf("Weight : \n%f [g]\n\n", average);
    M5.Lcd.printf("Weight per piece : \n%d [g]\n\n", OneWeight);
    M5.Lcd.print("SSID: ");
    M5.Lcd.print(ssid);
    M5.Lcd.print("\n\n");
    M5.Lcd.print("PASSWORD: ");
    M5.Lcd.print(password);
    M5.Lcd.print("\n\n");
    String cellR="a";
    String celladdress;
    celladdress=cellR+String(counter);
    triggerIFTTT(celladdress,average);
    counter++;

    delay(500);
    scale.power_up();
  }
}

void triggerIFTTT(String cells,float value)
{
  String url = "https://maker.ifttt.com/trigger/" + maker_Event + "/with/key/" + maker_Key + "?value1=" + cells + "&value2=" + value;
  http.begin(url);
  Serial.println("pressed!");
  Serial.println(url);
  http_code = http.GET(); // IFTTTにアクセスしてトリガ
  if (!(http_code > 0))
  {
    Serial.println("request failed!! try again");
    return; // 失敗だけど対策が難しいので戻る
  }
  Serial.println("IFTTT triggered"); // 正常終了
}