#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// ======================================================
// CONFIGURACIÓN WIFI
// ======================================================

const char* WIFI_SSID = "ARTEFACTOS";
const char* WIFI_PASSWORD = "s^3sQ#9teskVgnb";


// ======================================================
// CONFIGURACIÓN MQTT
// ======================================================


const char* MQTT_SERVER = "192.168.56.1";
const int MQTT_PORT = 1883;

// Topics utilizados por el dashboard React.
const char* TOPIC_TEMPERATURA = "clima/esp32/temperatura";
const char* TOPIC_HUMEDAD = "clima/esp32/humedad";
const char* TOPIC_LUZ = "clima/esp32/luz";


// ======================================================
// CONFIGURACIÓN DE SENSORES
// ======================================================

// (Estudiante)
// Definir los pines y objetos necesarios para los sensores.

WiFiClient espClient;
PubSubClient client(espClient);

#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

const int ldr = 32; 
int valLDR = 0; 
int condicion = 0;

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

unsigned long lastMsg = 0;
int valueToSend = 0;

// Enviar una lectura cada 2 segundos.
const unsigned long INTERVALO_ENVIO = 2000;

unsigned long ultimoEnvio = 0;

void conectarWiFi() {

  Serial.print("Conectando a WiFi");

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi conectado");

  Serial.print("IP del ESP32: ");
  Serial.println(WiFi.localIP());
}

void conectarMQTT() {

  while (!mqttClient.connected()) {

    Serial.print("Conectando a Mosquitto...");

    // Crear un Client ID diferente para este ESP32.
    String clientId = "ESP32-";
    clientId += String(
      (uint32_t)(ESP.getEfuseMac() & 0xFFFFFFFF),
      HEX
    );

    if (mqttClient.connect(clientId.c_str())) {

      Serial.println(" conectado");

    } else {

      Serial.print(" error, rc=");
      Serial.print(mqttClient.state());
      Serial.println(". Reintentando en 2 segundos...");

      delay(2000);
    }
  }
}


// ======================================================
// LEER Y PUBLICAR SENSORES
// ======================================================

void publicarSensores() {

  
  //(Estudiante)
  //1. Obtener la temperatura.
  float temperatura = dht.readTemperature();
  Serial.print(F("%  |  Temperatura: "));
  Serial.print(temperatura);
  Serial.println(F("°C"));
  
  
  //2. Obtener la humedad.
  float humedad = dht.readHumidity();
  Serial.print(F("Humedad: "));
  Serial.print(humedad);
  
      

  //3. Obtener la lectura del sensor de iluminación.
  valLDR = analogRead(ldr);
  Serial.print("Valor de luz: ");
  condicion= (valLDR*100)/4095;

  Serial.println(condicion);


  //4. Preparar los valores para poder publicarlos mediante MQTT.

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;   

    char msgLuz[10];
    char msgTemp[10];
    char msgHum[10];

    snprintf(msgLuz, sizeof(msgLuz), "%d", condicion);
    dtostrf(temperatura, 4, 2, msgTemp); // dtostrf se usa para números con decimales (float)
    snprintf(msgHum, sizeof(msgHum), "%d", humedad);


  /*5. Publicar cada dato en su topic correspondiente:

         TOPIC_TEMPERATURA
         TOPIC_HUMEDAD
         TOPIC_LUZ*/

  //6. Mostrar las mediciones en el Monitor Serial.
  

}

void setup() {

  Serial.begin(115200);

  // (Estudiante)
  // Inicializar los sensores y configurar los pines
  // necesarios.
   dht.begin();

   pinMode(ldr, INPUT);

  conectarWiFi();

  mqttClient.setServer(
    MQTT_SERVER,
    MQTT_PORT
  );
}

void loop() {

  // Si se pierde WiFi, volver a conectarse.
  if (WiFi.status() != WL_CONNECTED) {
    conectarWiFi();
  }

  // Si se pierde MQTT, volver a conectarse.
  if (!mqttClient.connected()) {
    conectarMQTT();
  }

  mqttClient.loop();


  // Publicar sensores cada 2 segundos.
  if (millis() - ultimoEnvio >= INTERVALO_ENVIO) {

    ultimoEnvio = millis();

    publicarSensores();
  }
}