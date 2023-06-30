#include <ESP8266WiFi.h>
#include <NewPingESP8266.h>
#include <PubSubClient.h>

#define pinAir      D3
#define pinNutrisi  D4
#define airTrigger  D5
#define airEcho     D6
#define nutrisiT    D7
#define nutrisiE    D8
#define jarak       200

#define MSG_BUFFER_SIZE (50)

const char* ssid = "";
const char* password = "";
const char* mqtt_server = "test.mosquitto.org";

unsigned long lastMsg = 0;
char msg[MSG_BUFFER_SIZE];
float cm, tinggiA, tinggiN;

NewPingESP8266 sonarA(airTrigger, airEcho, jarak);
NewPingESP8266 sonarN(nutrisiT, nutrisiE, jarak);

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi(){
  delay(10);

    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED){
      delay(500);
      Serial.print(".");
    }

    randomSeed(micros());

    Serial.println("");
    Serial.println("WiFi Connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length){
  Serial.print("Pesan Diterima dari [");
  Serial.print(topic);
  Serial.print("] ");
//  for (int i = 0; i < length; i++){
//    Serial.print((char)payload[i]);
//  }
  payload[length] = '\0';
  String pesan = (char*)payload;
  Serial.println(pesan);

  if (pesan == "AirOn"){
    digitalWrite(pinAir, LOW);
    snprintf (msg, MSG_BUFFER_SIZE, "{\"siram_air\": 1}");
    Serial.println(msg);
    client.publish("topik/", msg); 
    Serial.println("Air aktif");
  }else if(pesan == "AirOff"){
    digitalWrite(pinAir, HIGH);
    snprintf (msg, MSG_BUFFER_SIZE, "{\"siram_air\": 0}");
    Serial.println(msg);
    client.publish("topik/", msg); 
    Serial.println("Air Nonaktif");
  }

  if (pesan == "NutrisiOn"){
    digitalWrite(pinNutrisi, LOW);
    snprintf (msg, MSG_BUFFER_SIZE, "{\"siram_nutrisi\": 1}");
    Serial.println(msg);
    client.publish("topik/", msg);
    Serial.println("Nutrisi aktif");
  }else if(pesan == "NutrisiOff"){
    digitalWrite(pinNutrisi, HIGH);
    snprintf (msg, MSG_BUFFER_SIZE, "{\"siram_nutrisi\": 0}");
    Serial.println(msg);
    client.publish("topik/", msg); 
    Serial.println("Nutrisi Nonaktif");
  }
}

void reconnect(){
  while (!client.connected()){
    Serial.print("Attempting MQTT connection...");

    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);

    if (client.connect(clientId.c_str())){
      Serial.println("connected");

      client.publish("parkir/1", "test");

      client.subscribe("topik/");
    }else{
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 second");
      delay(5000);
    }
  }
}

void cek_air(){

  cm = sonarA.ping_cm();
  tinggiA = jarak - cm;

  Serial.print("Tinggi Air: ");
  Serial.print(tinggiA);
  Serial.println(" cm");

  snprintf (msg, MSG_BUFFER_SIZE, "{\"TinggiAir\": %.1f}", tinggiA);
  Serial.println(msg);
  client.publish("topik/", msg);  
}

void cek_nutrisi(){

  cm = sonarA.ping_cm();
  tinggiN = jarak - cm;

  Serial.print("Tinggi Nutrisi: ");
  Serial.print(tinggiN);
  Serial.println(" cm");

  snprintf (msg, MSG_BUFFER_SIZE, "{\"TinggiNutrisi\": %.1f}", tinggiN);
  Serial.println(msg);
  client.publish("topik/", msg);  
}

void setup() {
  pinMode(pinAir, OUTPUT);
  pinMode(pinNutrisi, OUTPUT);
  digitalWrite(pinAir, HIGH);
  digitalWrite(pinNutrisi, HIGH);
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()){
    reconnect();
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 10000){
    lastMsg = now;

    cek_air();
    delay(10);
    cek_nutrisi();
  }
}
