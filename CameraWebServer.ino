#include "esp_camera.h"
#include <WiFi.h>
#include <PubSubClient.h>

// YOU CAN ADD OTHER CAMERA MODEL FROM ARDUINO EXAMPLE FILE AND DON'T FORGET TO EDIT camera_pins.h AS WELL
#define CAMERA_MODEL_AI_THINKER
#include "camera_pins.h"


//Wifi SSID and Password
const char* ssid = "WIFI-Network-name";
const char* password = "WIFI-Network-password";

void callback(char* topic, byte* payload, unsigned int length){}

void startCameraServer();
boolean matchFace = false;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();
    
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  //initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);//flip it back
    s->set_brightness(s, 1);//up the blightness just a bit
    s->set_saturation(s, -2);//lower the saturation
  }
  //drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  startCameraServer();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");
}

void loop() {
  delay(1000);
//MQTT BROKER IP
  IPAddress server(1270, 0, 0, 1);

  WiFiClient espClient;
  PubSubClient client(server, 1883, callback, espClient);
// CODE GOES HERE FOR DETECTION
  if(matchFace==true)
   {
    if (!client.connected()) {
      while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
//MQTT BROKER USER AND PASSWORD SHOULD BE HERE        
        if (client.connect("MQTT CLIENT NAME", "MQTT BROKER USER" , "MQTT BROKER PASSWORD"))
        {
          Serial.println("connected");
          client.setCallback(callback);
        } else {
          Serial.print("failed, rc=");
          Serial.print(client.state());
          Serial.println(" try again in 5 seconds");
          delay(5000);
        }
      }
  }
// MQTT PUBLISH CODE WHEN A FACE DETECTED
// I USE MOUSQITTO BROKER IN HOME ASSISTANT TO TURN ON THE DOOR REALY TO OPEN IT
      client.publish("DOOR/cmnd/POWER", "ON");
      Serial.print("UNLOCK DOOR");    
//matchFace IS BOOLEAN VARIABLE THAT HAS BEEN ADDED TO app_httpd.cpp WHEN DETECTING A FACE
      matchFace=false;
      delay(20000);
   }
}
