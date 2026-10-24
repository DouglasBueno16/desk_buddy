#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

// Tela OLED 128 x 64 px
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// O endereço I2C mais comum é 0x3C (alguns módulos usam 0x3D)
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Intervalo entre consultas (10 minutos)
const unsigned long UPDATE_INTERVAL_MS = 10UL * 60UL * 1000UL;
unsigned long lastUpdate = 0;

// Consulta a API e preenche temperatura e umidade. Retorna true se deu certo.
bool fetchWeather(float &temperatura, int &umidade) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi desconectado");
    return false;
  }

  String url = String("http://api.openweathermap.org/data/2.5/weather?lat=") + OWM_LAT +
               "&lon=" + OWM_LON +
               "&units=metric&lang=pt_br&appid=" + OWM_API_KEY;

  HTTPClient http;
  http.begin(url);
  int httpCode = http.GET();

  if (httpCode != HTTP_CODE_OK) {
    Serial.printf("Erro HTTP: %d\n", httpCode);
    http.end();
    return false;
  }

  String payload = http.getString();
  http.end();

  // Filtra apenas os campos necessarios para economizar memoria
  JsonDocument filter;
  filter["main"]["temp"] = true;
  filter["main"]["humidity"] = true;

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, payload, DeserializationOption::Filter(filter));
  if (err) {
    Serial.print("Erro ao ler JSON: ");
    Serial.println(err.c_str());
    return false;
  }

  temperatura = doc["main"]["temp"].as<float>();
  umidade = doc["main"]["humidity"].as<int>();
  return true;
}

// Desenha temperatura e umidade na tela
void showWeather(float temperatura, int umidade) {
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);

  display.setTextSize(1);
  display.setCursor(5, 5);
  display.println(F("Temperatura:"));
  display.setTextSize(2);
  display.setCursor(5, 16);
  display.print(temperatura, 1);
  display.print((char)248); // simbolo de grau
  display.print("C");

  display.setTextSize(1);
  display.setCursor(5, 38);
  display.println(F("Umidade:"));
  display.setTextSize(2);
  display.setCursor(5, 49);
  display.print(umidade);
  display.print("%");

  display.display();
}

void showMessage(const char *msg) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5, 28);
  display.println(msg);
  display.display();
}

void updateWeather() {
  float temperatura;
  int umidade;
  if (fetchWeather(temperatura, umidade)) {
    Serial.printf("Temperatura: %.1f C | Umidade: %d %%\n", temperatura, umidade);
    showWeather(temperatura, umidade);
  } else {
    showMessage("Erro ao obter clima");
  }
}

void setup() {
  Serial.begin(115200);

  // Inicializa I2C nos pinos padrão do ESP32: SDA = 21, SCL = 22
  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("Falha ao inicializar a tela OLED. Verifique as conexoes e o endereco I2C!"));
    for (;;); // Trava o programa em caso de erro
  }

  // Limpa o buffer inicial
  display.clearDisplay();
  display.cp437(true); // habilita tabela de caracteres para o simbolo de grau
  showMessage("Conectando WiFi...");

  // WIFI
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi conectado");
  Serial.println("Endereco IP: ");
  Serial.println(WiFi.localIP());

  // Primeira consulta a API OpenWeather
  updateWeather();
  lastUpdate = millis();
}

void loop() {
  // Atualiza o clima periodicamente
  if (millis() - lastUpdate >= UPDATE_INTERVAL_MS) {
    lastUpdate = millis();
    updateWeather();
  }
}