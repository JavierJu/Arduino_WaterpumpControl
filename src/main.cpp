#include <Arduino.h>
#include <SoftwareSerial.h>
#include <WiFi.h> // ESP-01 용 라이브러리

SoftwareSerial espSerial(2, 3); // RX(Pin 3), TX(Pin 2) 소프트웨어 시리얼 설정

const int relayPin = 5;                                       // 릴레이가 연결된 핀 (5번 핀)
String server = "13.208.254.200";                             // AWS 웹서버 IP
String apiUrl = "/var/www/html/Controlpump/pump_control.txt"; // AWS 웹서버에서 명령을 읽어올 파일 경로

// 함수 프로토타입 선언
void sendCommand(String command, int timeout);
String getRelayCommand();

void setup()
{
  Serial.begin(9600);    // 아두이노 시리얼 통신
  espSerial.begin(9600); // ESP-01 시리얼 통신

  pinMode(relayPin, OUTPUT);    // 릴레이 핀을 출력으로 설정
  digitalWrite(relayPin, HIGH); // 초기 상태: OFF (반대로 설정)

  Serial.println("ESP-01 WiFi Module Test");

  // ESP-01 초기화 및 Wi-Fi 연결
  sendCommand("AT", 1000);
  sendCommand("AT+CWMODE=1", 1000);
  sendCommand("AT+CWJAP=\"SSID\",\"PASSWORD\"", 5000); // 실제 Wi-Fi SSID와 비밀번호로 변경

  delay(5000);
}

void loop()
{
  String command = getRelayCommand();

  if (command == "ON")
  {
    digitalWrite(relayPin, LOW); // 릴레이 켜기 (반대로 설정)
    Serial.println("Relay ON");
  }
  else if (command == "OFF")
  {
    digitalWrite(relayPin, HIGH); // 릴레이 끄기 (반대로 설정)
    Serial.println("Relay OFF");
  }

  delay(10000); // 10초마다 명령 확인
}

// AWS 웹서버에서 릴레이 명령을 읽어오는 함수
String getRelayCommand()
{
  sendCommand("AT+CIPSTART=\"TCP\",\"" + server + "\",80", 1000);
  delay(2000);

  String httpRequest = "GET " + apiUrl + " HTTP/1.1\r\nHost: " + server + "\r\nConnection: close\r\n\r\n";
  sendCommand("AT+CIPSEND=" + String(httpRequest.length()), 1000);
  espSerial.print(httpRequest);

  String response = "";
  while (espSerial.available())
  {
    response += espSerial.readString();
  }

  if (response.indexOf("ON") > 0)
  {
    return "ON";
  }
  else if (response.indexOf("OFF") > 0)
  {
    return "OFF";
  }
  return "";
}

// AT 명령을 ESP-01에 전송하는 함수
void sendCommand(String command, int timeout)
{
  espSerial.println(command);
  long int time = millis();
  while ((time + timeout) > millis())
  {
    while (espSerial.available())
    {
      char c = espSerial.read();
      Serial.print(c);
    }
  }
}
