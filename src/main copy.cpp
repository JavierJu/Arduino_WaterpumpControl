// 로컬웹에서 릴레이제어

#include <Arduino.h>
#include <SoftwareSerial.h>

SoftwareSerial espSerial(2, 3); // RX, TX 소프트웨어 시리얼 설정

const int relayPin = 5; // 릴레이가 연결된 핀 (5번 핀)

// 함수 프로토타입 선언
void sendResponse(String message);
void sendCommand(String command, int timeout);

void setup()
{
  Serial.begin(9600);    // 아두이노 시리얼 통신
  espSerial.begin(9600); // ESP-01 시리얼 통신

  pinMode(relayPin, OUTPUT);    // 릴레이 핀을 출력으로 설정
  digitalWrite(relayPin, HIGH); // 초기 상태: OFF (반대로 설정)

  Serial.println("ESP-01 WiFi Module Test");

  // ESP-01을 초기화
  sendCommand("AT", 1000);
  sendCommand("AT+CWMODE=1", 1000);

  // Wi-Fi에 연결 (SSID와 비밀번호는 실제로 사용 중인 값으로 변경)
  String ssid = "TP-Link_C6A8_JU";
  String password = "Javierju12";
  sendCommand("AT+CWJAP=\"" + ssid + "\",\"" + password + "\"", 5000);

  delay(5000);
  sendCommand("AT+CIFSR", 1000); // IP 주소 확인

  // 다중 연결 허용
  sendCommand("AT+CIPMUX=1", 1000);

  // 서버를 시작 (포트 80 사용)
  sendCommand("AT+CIPSERVER=1,80", 1000);
}

void loop()
{
  if (espSerial.available())
  {
    String request = espSerial.readStringUntil('\n');
    Serial.println(request);

    // HTTP 요청에 따른 릴레이 제어
    if (request.indexOf("GET /on") >= 0)
    {                              // 릴레이 ON 요청
      digitalWrite(relayPin, LOW); // 릴레이 켜기 (반대로 설정)
      sendResponse("ON");
    }
    else if (request.indexOf("GET /off") >= 0)
    {                               // 릴레이 OFF 요청
      digitalWrite(relayPin, HIGH); // 릴레이 끄기 (반대로 설정)
      sendResponse("OFF");
    }
    else if (request.indexOf("GET /status") >= 0)
    {
      String status = (digitalRead(relayPin) == LOW) ? "ON" : "OFF";
      sendResponse(status);
    }
    else if (request.indexOf("GET /") >= 0)
    {
      sendResponse("Hello!");
    }
  }
}

void sendResponse(String message)
{
  // HTTP 응답 준비
  String httpResponse = "HTTP/1.1 200 OK\r\n";
  httpResponse += "Content-Type: text/html\r\n";
  httpResponse += "Connection: close\r\n";
  httpResponse += "\r\n";
  httpResponse += "<h6>" + message + "</h6>\r\n";

  // 데이터 길이 계산
  int contentLength = httpResponse.length();

  // AT+CIPSEND 명령으로 데이터 전송 준비
  espSerial.print("AT+CIPSEND=0,");
  espSerial.println(contentLength); // 데이터 길이 전송
  delay(1500);                      // 충분한 대기 시간 추가

  // HTTP 응답 데이터 전송
  espSerial.print(httpResponse);

  delay(3000); // 더 긴 대기 시간 추가 (데이터가 확실히 전송되도록)

  // 연결 종료 명령
  espSerial.print("AT+CIPCLOSE=0\r\n");
  delay(2000); // 연결 종료 대기 시간 추가
}

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