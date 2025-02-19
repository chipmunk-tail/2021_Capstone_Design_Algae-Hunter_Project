#include <OneWire.h>              // TEMP //
#include <Wire.h>                 // LCD //
#include <LiquidCrystal_I2C.h>    // LCD //

#define pHPin A0                  // pH // pH 데이터 핀 A0
#define pHOff 0.00                // pH // pH Offset, 보정값
#define SInterval 20              // pH // Sampling Interval, 표집 간격 20
#define PInterval 800             // pH // Print Interval, 출력 간격 800
#define AryLenth 40               // pH // Array Lenth, 배열 길이 / 측정 시간

int pHAry[AryLenth];              // pH // pH Array, 센서 피드백의 평균값 저장
int pHAryIndex = 0;

int RelayElec = 2;                              //@ 릴레이 데이터선
int RelayPump = 3;                              //@ 릴레이 펌프선
int RelayValve = 4;                             //@ 릴레이 밸브선
int MOBut = 5;                                  //@ BUTTON ON/OFF // 스타트 버튼
int NOBut = 6;                                  //@ BUTTON // NO Button
int SCBut = 7;                                  //@ BUTTON // SC Button
int LED_E = 8;
int LED_P = 9;
int LED_V = 10;
int LED_N = 11;
int LED_S = 12;
int STBut = 13;                                 //노랑

const int TempPin = A1;                          //@ TEMP //

const long interval = 1000;

bool Start = 0;                                 // 부울 0 = false 1 = true
bool SCReady = 0;

float Temp;                                     // TEMP //
float ECValue = 1900;                           // EC //
float pH;
float a = 0;
float b = 0;
float c = 0;

unsigned long previousMillis = 0;


OneWire ds(TempPin);                            // TEMP //
LiquidCrystal_I2C lcd(0x27, 20, 4);             // LCD //


void setup() 
{
  Serial.begin(9600);

  lcd.init();                               // LCD // ldc 초기화
  lcd.backlight();                          // LCD //

  lcd.setCursor(6, 1);
  lcd.print("Welcome!");


  pinMode(RelayElec, OUTPUT);
  pinMode(RelayPump, OUTPUT);
  pinMode(RelayValve, OUTPUT);
  pinMode(MOBut, INPUT);
  pinMode(NOBut, INPUT);
  pinMode(SCBut, INPUT);
  pinMode(LED_E, OUTPUT);
  pinMode(LED_P, OUTPUT);
  pinMode(LED_V, OUTPUT);
  pinMode(LED_N, OUTPUT);
  pinMode(LED_S, OUTPUT);
  pinMode(STBut, INPUT);


  a = random(220);
  ECValue = ECValue + a;
  
  LedAUp();
  
  delay(1000);
  
  LedADown();
}




void loop()
{
  if(digitalRead(STBut) == HIGH)
  {
    Start = 1;
  }
  else
  {
    Start = 0;
  }


  while(Start == false)
  {
    lcd.init();

    lcd.setCursor(5, 1);
    lcd.print("Press the");

    lcd.setCursor(4, 2);
    lcd.print("Start Button");  

    LedADown();

    digitalWrite(RelayElec, LOW);
    digitalWrite(RelayPump, LOW);
    digitalWrite(RelayValve, HIGH);
    
    if(digitalRead(STBut) == HIGH)
    {
      Start = 1;
      
      digitalWrite(RelayElec, HIGH);
      digitalWrite(RelayPump, HIGH);
      digitalWrite(RelayValve, LOW);    
    }
    else
    {
      Start = 0;
    }

    if(Start == true)
    {
      break;
    }
  }

  int Mode = 0;                                 //Mode Normal

  LedADown();

  digitalWrite(RelayPump, HIGH);                // 펌프 가동
  digitalWrite(RelayValve, LOW);                // 밸브 개방   
             
  digitalWrite(LED_P, HIGH);               
  digitalWrite(LED_V, HIGH);            
  digitalWrite(LED_N, HIGH);
  
  getpH();
  
  Temp = getTemp();
  
  delay(100);

  unsigned long currentMillis = millis();
  
  if(currentMillis - previousMillis >= interval)
  {
    previousMillis = currentMillis;
   
    b = random(90, 110);
    c = b / 3600;
    ECValue = ECValue - c;
  }


  lcd.init();
        
  lcd.setCursor(0, 0);
  lcd.print("Mode: Normal");

  if(ECValue < 800)
  {
    digitalWrite(RelayElec, LOW);
    Condition3();
  }
  else
  {
    digitalWrite(RelayElec, HIGH);
    Condition1();
  }


  if(digitalRead(MOBut) == HIGH && Mode == 0)
  {
    Mode = 1;
    
    DisplayModeSC();

    digitalWrite(LED_E, LOW);              
    digitalWrite(LED_P, LOW);               
    digitalWrite(LED_V, LOW);            
    digitalWrite(LED_N, LOW);
    digitalWrite(LED_S, HIGH);

    delay(2000);
      
    if(digitalRead(SCBut) == HIGH)
    {         
      for(int i =0; i < 28800; i++)
      {
        digitalWrite(RelayElec, HIGH);              // 전극 가동
        digitalWrite(RelayPump, LOW);               // 펌프 가동 중단
        digitalWrite(RelayValve, HIGH);             // 밸브 닫음
        
        digitalWrite(LED_E, HIGH);              
        digitalWrite(LED_P, LOW);               
        digitalWrite(LED_V, LOW);            
        digitalWrite(LED_N, LOW);
        digitalWrite(LED_S, HIGH);

        lcd.init();
        
        lcd.setCursor(0, 0);
        lcd.print("Mode: SC / Charge");

        Condition2();

        delay(1000);

        
        if(i == 28750)
        {
          SCReady = 1;
        }

        if(digitalRead(STBut) == LOW)
        {
          break;
        }
      
        if(digitalRead(MOBut) == HIGH && Mode == 1)
        {
          DisplayModeNO();

          delay(2000);

          if(digitalRead(NOBut) == HIGH)
          {
            break;
          }
        }
      }
    }
    
  }


  if(SCReady == true)
  {
    for(int i = 0; i < 3600; i++)
    {
      digitalWrite(RelayElec, HIGH);              // 전극 가동
      digitalWrite(RelayPump, HIGH);               // 펌프 가동
      digitalWrite(RelayValve, LOW);             // 밸브 열림
      
      digitalWrite(LED_E, HIGH);              
      digitalWrite(LED_P, HIGH);               
      digitalWrite(LED_V, HIGH);            
      digitalWrite(LED_N, LOW);
      digitalWrite(LED_S, HIGH);

      lcd.init();

      lcd.setCursor(0, 0);
      lcd.print("Mode: SC / Deploy");

      Condition1();
      
      delay(1000);

      if(digitalRead(STBut) == LOW)
      {
        break;
      }

      if(digitalRead(MOBut) == HIGH)
      {
        DisplayModeNO();

        delay(2000);

        if(digitalRead(NOBut) == HIGH)
        {
          break;
        }
      }
      
    }

    SCReady = 0;
  }


  delay(1000);
}






void getpH() 
{
  static unsigned long STime = millis();        // pH // Sampling Time, 
  static unsigned long PTime = millis();        // pH // Print Time,
  static float pHValue;
  static float pHVoltage;

  if(millis() - STime > SInterval)
  {
    pHAry[pHAryIndex++] = analogRead(pHPin);
    
    if(pHAryIndex == AryLenth) pHAryIndex = 0;
      pHVoltage = avgAry(pHAry, AryLenth) * 5.0 / 1024;
      pHValue = 3.5 * pHVoltage + pHOff;
      STime = millis();
  }
  if(millis() - PTime > PInterval)
  {
    PTime = millis();

    pH = pHValue;      
  }
}


double avgAry (int*arr, int number)             // pH //
{
  int i;
  int max, min;
  double avg;
  long amount = 0;
  if(number <= 0);
  {
    return 0;
  }
  if(number < 5)                  // 직접 계산된 5 미만의 통계
  {
    for(i=0; i < number; i++)
    {
      amount += arr[i];
    }
    avg = amount / number;
    return avg;
  }
  else
  {
    if(arr[0] < arr[1])
    {
      min = arr[0]; max = arr[1];
    }
    else
    {
      min = arr[1]; max = arr[0];
    }
    for(i = 2; i < number; i++)
    {
      if(arr[i] < min)
      {
        amount += min;
        max = arr[i];
      }
      else
      {
        if(arr[i] > max)
        {
          amount += max;
          max = arr[i];
        }
        else
        {
          amount += arr[i];
        }
      }
    }
    avg = (double) amount / (number - 2);
  }
  return avg;
}


float getTemp()
  { 
  byte data[12];
  byte addr[8];

  if(!ds.search(addr))
  {
    ds.reset_search();
    return -1000;
  }
    
  if(OneWire::crc8(addr, 7) != addr[7])
  {
    Serial.println("CRC is not valid!");
    return -1000;
  }
  
  if(addr[0] != 0x10 && addr[0] != 0x28)
  {
    Serial.println("Device is not recognized");
    return -1000;
  }
  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);

  byte present = ds.reset();
  ds.select(addr);
  ds.write(0xBE);

  for(int i = 0; i < 9; i++)
  {
    data[i] = ds.read();
  }

  ds.reset_search();
  byte MSB = data[1];
  byte LSB = data[0];
  float tempRead = ((MSB << 8) | LSB);
  float TemperatureSum = tempRead / 16;
  return TemperatureSum;
}


void Condition1()
{  
  lcd.setCursor(0, 1);
  lcd.print("pH:");
  lcd.print(pH, 1);
  lcd.setCursor(7, 1);
  lcd.print("TEMP:");
  lcd.print(Temp, 0);
  lcd.print("C");

  lcd.setCursor(0, 2);
  lcd.print("EC:");
  lcd.print(ECValue, 0);

  lcd.setCursor(8, 2);
  lcd.print("Valve:OPEN");  

  lcd.setCursor(0, 3);
  lcd.print("Electrode:ON");

  digitalWrite(LED_E, HIGH);
  digitalWrite(LED_V, HIGH);
}


void Condition2()
{
  lcd.setCursor(0, 1);
  lcd.print("pH:");
  lcd.print(pH, 1);
  lcd.setCursor(7, 1);
  lcd.print("TEMP:");
  lcd.print(Temp, 0);
  lcd.print("C");

  lcd.setCursor(0, 2);
  lcd.print("EC:");
  lcd.print(ECValue, 0);

  lcd.setCursor(8, 2);
  lcd.print("Valve:CLOSE");  

  lcd.setCursor(0, 3);
  lcd.print("Electrode:ON");

  digitalWrite(LED_E, HIGH);
  digitalWrite(LED_V, LOW);
}


void Condition3()
{
  lcd.setCursor(0, 1);
  lcd.print("pH:");
  lcd.print(pH, 1);
  lcd.setCursor(7, 1);
  lcd.print("TEMP:");
  lcd.print(Temp, 0);
  lcd.print("C");

  lcd.setCursor(0, 2);
  lcd.print("EC:");
  lcd.print(ECValue, 0);

  lcd.setCursor(8, 2);
  lcd.print("Valve:OPEN");  

  lcd.setCursor(0, 3);
  lcd.print("Electrode:OFF");

  digitalWrite(LED_E, LOW);
  digitalWrite(LED_V, HIGH);
}


void Condition4()
{
  lcd.setCursor(0, 1);
  lcd.print("pH:");
  lcd.print(pH, 1);
  lcd.setCursor(7, 1);
  lcd.print("TEMP:");
  lcd.print(Temp, 0);
  lcd.print("C");

  lcd.setCursor(0, 2);
  lcd.print("EC:");
  lcd.print(ECValue, 0);

  lcd.setCursor(8, 2);
  lcd.print("Valve:CLOSE");  

  lcd.setCursor(0, 3);
  lcd.print("Electrode:OFF");

  digitalWrite(LED_E, LOW);
  digitalWrite(LED_V, LOW);
}


void DisplayModeNO()
{
  lcd.init();

  lcd.setCursor(0, 0);
  lcd.print("Change the Operating");
    
  lcd.setCursor(7, 1);
  lcd.print("Mode?");  
      
  lcd.setCursor(4, 2);
  lcd.print("Normal Drive");

  lcd.setCursor(2, 3);
  lcd.print("Press the Button");
}


void DisplayModeSC()
{
  lcd.init();

  lcd.setCursor(0, 0);
  lcd.print("Change the Operating");
    
  lcd.setCursor(7, 1);
  lcd.print("Mode?");  
      
  lcd.setCursor(0, 2);
  lcd.print("Shock Chlorination");

  lcd.setCursor(2, 3);
  lcd.print("Press the Button");
}


void LedADown()
{
  digitalWrite(LED_E, LOW);              
  digitalWrite(LED_P, LOW);               
  digitalWrite(LED_V, LOW);            
  digitalWrite(LED_N, LOW);
  digitalWrite(LED_S, LOW);
}


void LedAUp()
{
  digitalWrite(LED_E, HIGH);              
  digitalWrite(LED_P, HIGH);               
  digitalWrite(LED_V, HIGH);            
  digitalWrite(LED_N, HIGH);
  digitalWrite(LED_S, HIGH);
}
