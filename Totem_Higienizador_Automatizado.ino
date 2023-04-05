//RFID
#include <MFRC522.h>
#define RSTPIN 9 
#define SSPIN 10
MFRC522 mfrc522(SSPIN, RSTPIN);   // Create MFRC522 instance

//EEPROM
#include <EEPROM.h>
int saveContadorAcionamentoMotor;

//MicroSD
#include <SPI.h>
#include <SD.h>
File myFile;

//RTC(Real Time Clock)
#include "RTClib.h" //INCLUSÃO DA BIBLIOTECA
RTC_DS3231 rtc; //OBJETO DO TIPO RTC_DS3231
char daysOfTheWeek[7][12] = {"Domingo", "Segunda", "Terça", "Quarta", "Quinta", "Sexta", "Sábado"};

//MLX90614(sensor de temperatura)
#include <Adafruit_MLX90614.h>
Adafruit_MLX90614 mlx = Adafruit_MLX90614();
float primeiraLeituraTemp;
float segundaLeituraTemp;
float terceiraLeituraTemp;
float quartaLeituraTemp;
float quintaLeituraTemp;
float mediaTemp;
float correcaoTemp;
int contadorTemp;


//LCD_i2c
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2);

//variáveisDoProjeto
int contadorAcionamentoMotor = 0;
boolean flagAcionamentoMotor = 0;
boolean ativaFuncionamentoMotor = 1;
int contadorResetBuzzer = 0;
int contadorAux = 0;


//servoMotor
#include <Servo.h>
#define SERVO 5
Servo motor;
int pos;
int limiteStopMotor = 5;

//ultrassonico
#define echoPin 7 // Echo Pin
#define trigPin 8 // Trigger Pin
int maximumRange = 200; // Maximum range needed
int minimumRange = 0; // Minimum range needed
long duration, distance; // Duration used to calculate distance

//buzzer
int buzzer = 2;

//-----------------------------------------------------------------------------------------------------------------------------------------------------

void setup() {
  //servoMotor
  motor.attach(SERVO);
  Serial.begin(9600);
  motor.write(0);
  
  //ultrassonico
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  //buzzer
  pinMode(buzzer,OUTPUT);
  digitalWrite(buzzer, LOW);

  //LCD_i2c
  lcd.init();
  lcd.backlight(); // Enable or Turn On the backlight
  lcd.clear();
  lcd.setCursor(0,0); // Position of Cursor on LCD Screen

  //MLX90614(sensor de temperatura)
  mlx.begin();

  //RTC(Real Time Clock)
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
   
  //MicroSD
  SD.begin(4);

  //RFID
  SPI.begin();                                                  // Init SPI bus
  mfrc522.PCD_Init();                                              // Init MFRC522 card
 
}
//-----------------------------------------------------------------------------------------------------------------------------------------------------
void loop() {
  // put your main code here, to run repeatedly:
  //RFID
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) 
  {
    looping();
    return;
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) 
  {
    return;
  }

  String conteudo= "";
  byte letra;
  for (byte i = 0; i < mfrc522.uid.size; i++) 
  {
     Serial.print(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " ");
     Serial.print(mfrc522.uid.uidByte[i], HEX);
     conteudo.concat(String(mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
     conteudo.concat(String(mfrc522.uid.uidByte[i], HEX));
  }
  conteudo.toUpperCase();
 //$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$CADASTRO DE USUÁRIO$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
  if (conteudo.substring(1) == "F0 5E CB 57") //UID 1 - Chaveiro
  {
    myFile = SD.open("LOG.txt",FILE_WRITE);
    myFile.println(""); 
    funcaoRTC();
    myFile.println("MATHEUS");
    myFile.close();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Ola MATHEUS");
    delay(2000);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("   MEDIR");
    lcd.setCursor(0,1);
    lcd.print("  TEMPERATURA");
    while(contadorTemp != 5)
    {
      funcaoMediaTemp();
      if(mediaTemp<34 || mediaTemp>42)
      {
        contadorAux = contadorAux+1;
        delay(1000);
        if(contadorAux==5)
        {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("     TEMPO");
          lcd.setCursor(0,1);
          lcd.print("    EXCEDIDO");
          acionaBuzzer();
          delay(100);
          desacionaBuzzer();
          myFile = SD.open("LOG.txt",FILE_WRITE);
          myFile.println("NÃO AFERIDA");
          myFile.close();
          contadorAux = 0;
          contadorTemp=5;
          delay(3000);
          lcd.clear();
        }
      }
      if(mediaTemp>34 && mediaTemp <42)
      {
        contadorTemp = contadorTemp + 1;
        delay(100);
        if(contadorTemp == 5)
        {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("TEMPERATURA");
          lcd.setCursor(0,1);
          lcd.print("AFERIDA: ");          
          lcd.print(mediaTemp,1);lcd.print("*C");
          acionaBuzzer();
          delay(100);
          desacionaBuzzer();
          myFile = SD.open("LOG.txt",FILE_WRITE);
          myFile.println(mediaTemp,1);
          myFile.close();
          contadorAux = 0;
          delay(2000);
          if(mediaTemp*10 > 375)
          {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print(" ESTADO FEBRIL");
            lcd.setCursor(0,1);
            lcd.print(" FIQUE EM CASA");
            acionaBuzzer();
            delay(3000);
            desacionaBuzzer();
          }
          lcd.clear();
        }
      }
    }
    contadorTemp = 0;
    
    while(distance>11 && contadorAux != 5)
    {
      lcd.setCursor(0,0);
      lcd.print("   POR FAVOR");
      lcd.setCursor(0,1);
      lcd.print("  HIGIENIZE-SE");
      funcaoUltrassonico();
      if(distance>11)
      {
        contadorAux = contadorAux+1;
        delay(1000);
        if(contadorAux == 5)
        {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("     TEMPO");
          lcd.setCursor(0,1);
          lcd.print("    EXCEDIDO");
          acionaBuzzer();
          delay(100);
          desacionaBuzzer();
          myFile = SD.open("LOG.txt",FILE_WRITE);
          myFile.println("HIGIENIZAÇÃO NÃO REALIZADA");
          myFile.close();
          delay(3000);
          lcd.clear();
        }
      }
    }
    lcd.clear();
    if(contadorAux != 5)
    {
      myFile = SD.open("LOG.txt",FILE_WRITE);
      myFile.println("HIGIENIZAÇÃO OK");
      myFile.close();
    }
    contadorAux = 0;
    
  }
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$CADASTRO DE USUÁRIO$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$
if (conteudo.substring(1) == "91 6C A6 A9") //UID 1 - Chaveiro
  {
    myFile = SD.open("LOG.txt",FILE_WRITE);
    myFile.println(""); 
    funcaoRTC();
    myFile.println("HEIDSON");
    myFile.close();
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Ola HEIDSON");
    delay(2000);
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("   MEDIR");
    lcd.setCursor(0,1);
    lcd.print("  TEMPERATURA");
    while(contadorTemp != 5)
    {
      funcaoMediaTemp();
      if(mediaTemp<34 || mediaTemp>42)
      {
        contadorAux = contadorAux+1;
        delay(1000);
        if(contadorAux==5)
        {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("     TEMPO");
          lcd.setCursor(0,1);
          lcd.print("    EXCEDIDO");
          acionaBuzzer();
          delay(100);
          desacionaBuzzer();
          myFile = SD.open("LOG.txt",FILE_WRITE);
          myFile.println("NÃO AFERIDA");
          myFile.close();
          contadorAux = 0;
          contadorTemp=5;
          delay(3000);
          lcd.clear();
        }
      }
      if(mediaTemp>34 && mediaTemp <42)
      {
        contadorTemp = contadorTemp + 1;
        delay(100);
        if(contadorTemp == 5)
        {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("TEMPERATURA");
          lcd.setCursor(0,1);
          lcd.print("AFERIDA: ");
          lcd.print(mediaTemp,1);lcd.print("*C");
          acionaBuzzer();
          delay(100);
          desacionaBuzzer();
          myFile = SD.open("LOG.txt",FILE_WRITE);
          myFile.println(mediaTemp,1);
          myFile.close();
          contadorAux = 0;
          delay(2000);
          if(mediaTemp*10 > 375)
          {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print(" ESTADO FEBRIL");
            lcd.setCursor(0,1);
            lcd.print(" FIQUE EM CASA");
            acionaBuzzer();
            delay(3000);
            desacionaBuzzer();
          }
          lcd.clear();
        }
      }
    }
    contadorTemp = 0;
    
    while(distance>11 && contadorAux != 5)
    {
      lcd.setCursor(0,0);
      lcd.print("   POR FAVOR");
      lcd.setCursor(0,1);
      lcd.print("  HIGIENIZE-SE");
      funcaoUltrassonico();
      if(distance>11)
      {
        contadorAux = contadorAux+1;
        delay(1000);
        if(contadorAux == 5)
        {
          lcd.clear();
          lcd.setCursor(0,0);
          lcd.print("     TEMPO");
          lcd.setCursor(0,1);
          lcd.print("    EXCEDIDO");
          acionaBuzzer();
          delay(100);
          desacionaBuzzer();
          myFile = SD.open("LOG.txt",FILE_WRITE);
          myFile.println("HIGIENIZAÇÃO NÃO REALIZADA");
          myFile.close();
          delay(3000);
          lcd.clear();
        }
      }
    }
    lcd.clear();
    if(contadorAux != 5)
    {
      myFile = SD.open("LOG.txt",FILE_WRITE);
      myFile.println("HIGIENIZAÇÃO OK");
      myFile.close();
    }
    contadorAux = 0;
    
  }
  
//$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$CADASTRO DE USUÁRIO$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$$  
} 

//-----------------------------------------------------------------------------------------------------------------------------------------------------
//**************************************************************HC-SR04(ultrassônico*****************************************************************************
void funcaoUltrassonico()
{
 //ultrassonico
 digitalWrite(trigPin, LOW); 
 delayMicroseconds(2); 

 digitalWrite(trigPin, HIGH);
 delayMicroseconds(10); 
 digitalWrite(trigPin, LOW);
 duration = pulseIn(echoPin, HIGH);
 
 //Calculate the distance (in cm) based on the speed of sound.
 distance = duration/58.2;
}

//********************************************************************FUNCAO LOOPING*****************************************************************************
void looping()
{
  funcaoUltrassonico();
 
 //condição para movimentar o motor
 if(distance <11 && ativaFuncionamentoMotor){
  movimentaMotor();
 }
 
 if(ativaFuncionamentoMotor == 1)
 {
  funcaoMediaTemp();
 }
 
 lerMemoriaEEPROM();
 funcaoAtivaFuncionamentoMotor();
 funcaoLCD();
 funcaoPrint();
 

}

//********************************************************************EEPROM*****************************************************************************
void escreverMemoriaEEPROM()
{
  EEPROM.write(0,contadorAcionamentoMotor);
}
void lerMemoriaEEPROM()
{
  saveContadorAcionamentoMotor = EEPROM.read(0);
  contadorAcionamentoMotor = saveContadorAcionamentoMotor;
}

//********************************************************************LCD*****************************************************************************
void funcaoLCD()
{
  if(ativaFuncionamentoMotor == 1)
  {
    
    lcd.setCursor(0,0); // Position of Cursor on LCD Screen
    lcd.print("Seja Bem Vindo!");
    lcd.setCursor(0,1);
    lcd.print("<<<AFERIR TEMP");
    if(mediaTemp > 34 and mediaTemp <42)
    {
      contadorTemp = contadorTemp+1;
      if(contadorTemp == 5)
      {
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("TEMPERATURA");
        lcd.setCursor(0,1);
        lcd.print("AFERIDA: ");
        lcd.print(mediaTemp,1);lcd.print("*C");
        acionaBuzzer();
        delay(100);
        desacionaBuzzer();
        delay(3000);
        contadorTemp = 0;
        lcd.clear();
        if(mediaTemp*10 > 375)
          {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print(" ESTADO FEBRIL");
            lcd.setCursor(0,1);
            lcd.print(" FIQUE EM CASA");
            acionaBuzzer();
            delay(3000);
            desacionaBuzzer();
            lcd.clear();
          }      
     }
   }
 }
  
  if(ativaFuncionamentoMotor == 0)
  {
    lcd.setCursor(0,0); // Position of Cursor on LCD Screen
    lcd.print(" SEM ALCOOL GEL");
    lcd.setCursor(0,1);
    lcd.print("  TROCAR REFIL");
  }
  
}

//********************************************************************BUZZER*****************************************************************************
void acionaBuzzer()
{
  digitalWrite(buzzer, HIGH);
}
void desacionaBuzzer()
{
  digitalWrite(buzzer, LOW);
}


//********************************************************************MOTOR*****************************************************************************
void movimentaMotor()
{
  for(pos = 20; pos<100; pos++)
  {
    motor.write(pos);
    incrementaAcionamentoMotor();
  }
  //ajusta valor do contador de acionamento do motor
  contadorAcionamentoMotor += 1;
  contadorAcionamentoMotor = contadorAcionamentoMotor - 80;
  escreverMemoriaEEPROM();

  delay(1000);
  for(pos = 100; pos>=20; pos--)
  {
    motor.write(pos);
    delay(2);
  }
  delay(1000);
}

//ativaFuncionamentoMotor
void funcaoAtivaFuncionamentoMotor()
{
  if(contadorAcionamentoMotor >= limiteStopMotor)
  {
    ativaFuncionamentoMotor = 0;
    acionaBuzzer();
    delay(500);
    desacionaBuzzer();
    delay(500);
  }
  if(contadorAcionamentoMotor>= limiteStopMotor && ativaFuncionamentoMotor == 0)
  {
    if(distance <= 11)
    {
      contadorResetBuzzer += 1;
    }
    else
    {
      contadorResetBuzzer = 0;
    }
    
    if(contadorResetBuzzer >= 5)
    {
      desacionaBuzzer();
      ativaFuncionamentoMotor = 1;
      contadorResetBuzzer = 0;
      contadorAcionamentoMotor = 0;
      escreverMemoriaEEPROM();
      lcd.clear();
    }
  }
  
}

//********************************************************************CONTADOR*****************************************************************************
void incrementaAcionamentoMotor()
{
  if (pos == 100)
  {
    flagAcionamentoMotor = 1;     
  }
  
  if (pos == 100 && flagAcionamentoMotor);
  {
    flagAcionamentoMotor = 0;
    contadorAcionamentoMotor += 1;
  }
}

//********************************************************************PRINT MONITOR SERIAL*****************************************************************************
void funcaoPrint()
{
   DateTime now = rtc.now(); //CHAMADA DE FUNÇÃO
   Serial.print("Data: "); //IMPRIME O TEXTO NO MONITOR SERIAL
   Serial.print(now.day(), DEC); //IMPRIME NO MONITOR SERIAL O DIA
   Serial.print('/'); //IMPRIME O CARACTERE NO MONITOR SERIAL
   Serial.print(now.month(), DEC); //IMPRIME NO MONITOR SERIAL O MÊS
   Serial.print('/'); //IMPRIME O CARACTERE NO MONITOR SERIAL
   Serial.print(now.year(), DEC); //IMPRIME NO MONITOR SERIAL O ANO
   Serial.print(" / Dia: "); //IMPRIME O TEXTO NA SERIAL
   Serial.print(daysOfTheWeek[now.dayOfTheWeek()]); //IMPRIME NO MONITOR SERIAL O DIA
   Serial.print(" / Horas: "); //IMPRIME O TEXTO NA SERIAL
   Serial.print(now.hour(), DEC); //IMPRIME NO MONITOR SERIAL A HORA
   Serial.print(':'); //IMPRIME O CARACTERE NO MONITOR SERIAL
   Serial.print(now.minute(), DEC); //IMPRIME NO MONITOR SERIAL OS MINUTOS
   Serial.print(':'); //IMPRIME O CARACTERE NO MONITOR SERIAL
   Serial.print(now.second(), DEC); //IMPRIME NO MONITOR SERIAL OS SEGUNDOS
   Serial.println(); //QUEBRA DE LINHA NA SERIAL
   Serial.print("distancia: ");
   Serial.print(distance); 
   Serial.print("    ");
   Serial.print("grau motor: ");
   Serial.print(pos);
   Serial.print("    ");
   Serial.print("contador: ");
   Serial.print(contadorAcionamentoMotor);
   Serial.print("    ");
   Serial.print("cont reset: ");
   Serial.print(contadorResetBuzzer);
   Serial.print("    ");
   Serial.print("temp: ");
   Serial.print(mediaTemp);
   Serial.println("");
}
//********************************************************************MLX90614*****************************************************************************
//media das ultimas 5 medições

void funcaoMediaTemp()
{
  correcaoTemp = 3.5;
  primeiraLeituraTemp = mlx.readObjectTempC()+correcaoTemp;
  delay(50);
  segundaLeituraTemp = mlx.readObjectTempC()+correcaoTemp;
  delay(50);
  terceiraLeituraTemp = mlx.readObjectTempC()+correcaoTemp;
  delay(50);
  quartaLeituraTemp = mlx.readObjectTempC()+correcaoTemp;
  delay(50);
  quintaLeituraTemp = mlx.readObjectTempC()+correcaoTemp;
  delay(50);
  mediaTemp = ((primeiraLeituraTemp+segundaLeituraTemp+terceiraLeituraTemp+quartaLeituraTemp+quintaLeituraTemp)/5);
}

//********************************************************************RTC*****************************************************************************
void funcaoRTC()
{
  DateTime now = rtc.now();  
  myFile.print("Data: "); //IMPRIME O TEXTO NO MONITOR SERIAL
  myFile.print(now.day(), DEC); //IMPRIME NO MONITOR SERIAL O DIA
  myFile.print('/'); //IMPRIME O CARACTERE NO MONITOR SERIAL
  myFile.print(now.month(), DEC); //IMPRIME NO MONITOR SERIAL O MÊS
  myFile.print('/'); //IMPRIME O CARACTERE NO MONITOR SERIAL
  myFile.print(now.year(), DEC); //IMPRIME NO MONITOR SERIAL O ANO
  myFile.print(" / Dia: "); //IMPRIME O TEXTO NA SERIAL
  myFile.print(daysOfTheWeek[now.dayOfTheWeek()]); //IMPRIME NO MONITOR SERIAL O DIA
  myFile.print(" / Horas: "); //IMPRIME O TEXTO NA SERIAL
  myFile.print(now.hour(), DEC); //IMPRIME NO MONITOR SERIAL A HORA
  myFile.print(':'); //IMPRIME O CARACTERE NO MONITOR SERIAL
  myFile.print(now.minute(), DEC); //IMPRIME NO MONITOR SERIAL OS MINUTOS
  myFile.print(':'); //IMPRIME O CARACTERE NO MONITOR SERIAL
  myFile.println(now.second(), DEC); //IMPRIME NO MONITOR SERIAL OS SEGUNDOS
}
