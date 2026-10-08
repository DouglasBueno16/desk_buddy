#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Tela OLED 128 x 64 px
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// O endereço I2C mais comum é 0x3C (alguns módulos usam 0x3D)
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

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

  // Configura o texto
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(5, 5);
  display.println(F("Temperatura!"));

  display.setTextSize(2);
  display.setCursor(10, 32);
  display.println(F("ONLINE!"));

  // Envia as informacoes para a tela
  display.display();
}

void loop() {
  // Por enquanto, nada no loop principal
}