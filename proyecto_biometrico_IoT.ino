#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <Adafruit_Fingerprint.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <Keypad.h>

const byte n_rows = 3;
const byte n_cols = 3;

char keys[n_rows][n_cols] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'}
};
byte colPins[n_rows] = { D8, D7, D6 };
byte rowPins[n_cols] = { D5, D4, D3 };
#define mySerial Serial
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Keypad myKeypad = Keypad(makeKeymap(keys), rowPins, colPins, n_rows, n_cols);

const char* ssid = "Fanbercell 27-05";
//const char* ssid = "Merejildo Carvajal";
const char* password = "Multiraices-2019";
//const char* password = "Ariana2007";
//const char* serverNameData = "http://192.168.16.152:53785/api/login/testBiometrico";
//post id /marcar
//usuario
const char* serverNameData = "https://fanbercell.com/api/login/testBiometrico";

//const char* serverConfirmacionIngreso="http://192.168.16.152:53785/api/login/marcar";
const char* serverConfirmacionIngreso = "https://fanbercell.com/api/login/marcar";

//const char* serverTotalHuellas="http://192.168.16.152:53785/api/login/totalHuellas";
const char* serverTotalHuellas = "https://fanbercell.com/api/login/totalHuellas";

//const char* serverGetData="http://192.168.16.152:53785/api/login/huellaUnica";
const char* serverGetData = "https://fanbercell.com/api/login/huellaUnica";
const char* fingerprint = "82 3C 86 5D F7 20 25 2D 6D B0 AA 8C 77 40 74 35 4E 2C 32 7A";
int identificador;
volatile boolean f = false;
volatile boolean g = true;
uint8_t p;
HTTPClient http_;
void setup()
{
  finger.begin(57600);
  lcd.init();
  lcd.backlight();
  myKeypad.begin(makeKeymap(keys));
  attachInterrupt(digitalPinToInterrupt(D8), flag, RISING);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    lcd.setCursor(0, 0);
    lcd.print("Conectando wifi");
    delay(100);
  }

  while (!finger.verifyPassword())
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor");
    lcd.setCursor(0, 1);
    lcd.print("Desconectado");
    delay(0);
  }

  if (finger.verifyPassword())

  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Sensor");
    lcd.setCursor(0, 1);
    lcd.print("Conectado");
  }

}

void loop() {

  String id_usuario;
  String data = id_usuario;
  data += "/";
  String campos = "usuario/";
  obtner_huella_db(serverTotalHuellas, serverGetData);

  ingreso_de_usuario();
  if (g)
  {
    
    id_usuario = buscar_usario();
    if (id_usuario != "0")
    {
      enviar_datos_hacia_api(data, serverConfirmacionIngreso, campos, fingerprint);
    }

  }
  g = true;
  finger.emptyDatabase();
  //identificador++;
  delay(0);
}

///////////////////////////////////////////////////////////////////////////////////
//Funci�n de interrupcion para el pin digital D8 encargado de detectar si
//se presion� el numero 7
////////////////////////////////////////////////////////////////////////////////////
ICACHE_RAM_ATTR void flag()
{
  f = true;

}


/////////////////////////////////////////////////////////////////////////////////////
//Funci�n de interrupcion para el pin digital D7 encargado de detectar si
//se presion� el numero 8
////////////////////////////////////////////////////////////////////////////////////
ICACHE_RAM_ATTR void flag1()
{
  g = false;

}

//////////////////////////////////////////////////////////////////////////
//Funci�n que se encarga de guardar el dato biom�trico de la 
//base de datos hacia el disposito biom�trico
//////////////////////////////////////////////////////////////////////////
void guardar_en_el_dispositvo(const char* huella, int id)
{
  int cnt = 0;
  int huella_hex[556];
  int j = 0;

  memset(huella_hex, 0xff, 556);
  while (cnt < 1111)
  {
    String str_huella;
    str_huella += huella[cnt];
    str_huella += huella[cnt + 1];
    const char* hex_string = str_huella.c_str();

    huella_hex[j] = (int)strtol(hex_string, NULL, 16);

    cnt = cnt + 2;
    j++;
    delay(0);
  }

  p = finger.setModel();
  //delay(10);
  if (p == FINGERPRINT_OK)
  {
    uint8_t bytePacket;

    for (uint16_t m = 0; m < 556; m++)
    {
      bytePacket = huella_hex[m];
      finger.sendFormattedTemplatePackages(bytePacket);
      delay(0);
      //Serial.print(bytePacket,HEX);

    }

  }

  finger.storeModel(id);


}


///////////////////////////////////////////////////////////////////////////////////////////
////Funcion que permite tomar imagen de la huella dactilar, crear el modelo y almacenarla en un string de caracteres,
////dicho string son los caracteres hexagecimales de la huella
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////7
String ingresar_nuevo_usuario()
{


  String huella;
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Ingrese nueva");
  lcd.setCursor(0, 1);
  lcd.print("huella");
  do {
    while (mySerial.available())
    {

      mySerial.read();

    }
    p = finger.getImage();
    switch (p) {
      case FINGERPRINT_OK:
        lcd.clear();
        lcd.print("Imagen capturada");
        delay(400);

      default:
        //Serial.println("Erro desconhecido");
        break;
    }

    delay(0);
  } while (p != FINGERPRINT_OK && g);
  if (g)
  {
    p = finger.image2Tz(1);
    switch (p)
    {
      case FINGERPRINT_OK:
        lcd.clear();
        lcd.print("Imagen");
        lcd.setCursor(0,1);
        lcd.print("convertida");
        delay(400);
        break;
      default:
        break;
        //Serial.println("Erro desconhecido");

    }
    lcd.clear();
    lcd.print("Retire");
    delay(1000);
    lcd.clear();
    lcd.print("Ingrese de Nuevo");
    do {
      p = finger.getImage();
      switch (p) {
        case FINGERPRINT_OK:
          lcd.clear();
          lcd.print("Imagen capturada");
          delay(400);

        default:
          //Serial.println("Erro desconhecido");
          break;
      }
      delay(0);
    } while (p != FINGERPRINT_OK);

    p = finger.image2Tz(2);
    switch (p) {
      case FINGERPRINT_OK:

        lcd.clear();
        lcd.print("Imagen");
        lcd.setCursor(0, 1);
        lcd.print("convertida");
        delay(400);
        break;
      default:
        break;


    }
    uint16_t templateBuffer[556];
    memset(templateBuffer, 0xff, 556);  //zero out template buffer
    uint16_t index = 0;
    finger.createModel();

    //delay(20);
    //lcd.clear();
    //uint32_t starttime = millis();
    p = finger.getModel();

    while (index < 556)
    {
      if (mySerial.available())
      {
        templateBuffer[index] = mySerial.read();
        index++;
        delay(0);
      }
    }

    for (uint16_t i_ = 0; i_ < 556; i_++)
    {
      if (templateBuffer[i_] > 15)
      {
        huella += String(templateBuffer[i_], HEX);
      }
      else
      {
        huella += "0";
        huella += String(templateBuffer[i_], HEX);
      }
      delay(0);
    }


  }

  return huella;


}

//////////////////////////////////////////////////////////////////////////////////////////////////
void enviar_datos_hacia_api(String datos, const char* host, String campos_JSON, const char* fingerprint_)
{
  int n = 0;
  int m = 0;
  int j = -1;
  int k = -1;
  http_.begin(host, fingerprint_);

  DynamicJsonBuffer JSONbuffer;
  JsonObject& JSONencoder = JSONbuffer.createObject();
  while (j != datos.length() - 1)
  {
      n = j;
      m = k;

      j = datos.indexOf("/", n + 1);
      k = campos_JSON.indexOf("/", m + 1);
      JSONencoder[campos_JSON.substring(m + 1, k)] = datos.substring(n + 1, j);
      delay(0);
  
  }
  String JsonString;
  JSONencoder.printTo(JsonString);

  http_.addHeader("Content-Type", "application/json");
  int httpCode = http_.POST(JsonString);

}

/////////////////////////////////////////////////////////////////////////////////////////////
void obtner_huella_db(const char* host_total, const char* host_obtener_huellas)
{
  //Object of class HTTPClient
  int total;
  http_.begin(host_total, fingerprint);
  int httpCode = http_.GET();

  if (httpCode > 0) {
    // Parsing
    const size_t bufferSize = JSON_OBJECT_SIZE(1) + 10;
    DynamicJsonBuffer jsonBuffer(bufferSize);
    JsonObject& root = jsonBuffer.parseObject(http_.getString());
    // Parameters
    total = root["total"]; // 1

  };


  for (int i = 1; i <= total; i++)
  {
    //Object of class HTTPClient
    String url_;
    url_ += host_obtener_huellas;
    url_ += "/";
    url_ += String(i);
    const char* url;
    url = url_.c_str();
    http_.begin(url, fingerprint);
    int httpCode = http_.GET();

    if (httpCode > 0) {

      const size_t bufferSize = 1170;
      DynamicJsonBuffer jsonBuffer(bufferSize);
      JsonObject& root = jsonBuffer.parseObject(http_.getString());

      const char* huella = root["huella"];

      guardar_en_el_dispositvo(huella, i);
    }

    delay(0);
  }
  //http_.end();
}

void ingreso_de_usuario()
{


  lcd.clear();
  do {
    //lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Ingresar huella");
    char c = myKeypad.getKey();
    if (f && String(c) == "7")
    {
      int i = 0;
      detachInterrupt(digitalPinToInterrupt(D8));
      String password;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Ingrese clave");

      char myKey;
      String pass;
      while (i < 4)
      {
        char myKey = NULL;

        myKey = myKeypad.getKey();
        if (myKey != NULL)
        {
          password += myKey;

          pass += "*";
          lcd.setCursor(0, 1);
          lcd.print(pass);
          i++;
          delay(0);
        }
        delay(0);
      }
      delay(300);

      attachInterrupt(digitalPinToInterrupt(D7), flag1, RISING);
      if (password == "1234")
      {
        while (g) {


          String dactilar = ingresar_nuevo_usuario();
          if (g)
          {
            
            String data_ = dactilar;
            data_ += "/";
            String campos= "huella/";

            //
            enviar_datos_hacia_api(data_, serverNameData, campos, fingerprint);
          }
        }

      }
      else
      {
        f = false;
        g = false;
        detachInterrupt(digitalPinToInterrupt(D7));
      }

      //
      attachInterrupt(digitalPinToInterrupt(D8), flag, RISING);

    }

    while (mySerial.available())
    {
      mySerial.read();
    }
    detachInterrupt(digitalPinToInterrupt(D7));
    
    if (g)
    {
      p = finger.getImage();
      switch (p) {
        case FINGERPRINT_OK:
         
        
        default:
          //Serial.println("Erro desconhecido");
          break;
      }
    }
    delay(0);

  } while (p != FINGERPRINT_OK && g);

  if (g)
  {
    p = finger.image2Tz(1);
    switch (p) {
      case FINGERPRINT_OK:
                
        break;
      default:
        break;

    }
  }

}

String buscar_usario()
{
  //int id=0;
  int p;
  p = finger.fingerFastSearch();
  lcd.clear();
  //lcd.print(p);
  //delay(1000);
  if (p == FINGERPRINT_OK) {
    lcd.print(finger.fingerID);
    return String(finger.fingerID);
    delay(0);
  }
  else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    lcd.print("Communication error");
    //return p;
  }
  else if (p == FINGERPRINT_NOTFOUND) {
    lcd.setCursor(0, 0);
    lcd.print("No estas en la");
    lcd.setCursor(0, 1);
    lcd.print("base de datos");
    delay(400);
    return "0";
  }
  else {
    lcd.print("Unknown error");
    delay(1000);
    //return p;
  }
  //return String(finger.fingerID);

}
//void mostrar info_bien_en_pantalla(String mensaje)
//{
//
//}


