#include <SPI.h>
#include <mcp_can.h>

const int SPI_CS_PIN = 10;
MCP_CAN CAN(SPI_CS_PIN);

void setup() {
  Serial.begin(115200);
  while (!Serial);

  while (CAN_OK != CAN.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ)) {
    Serial.println("MCP2515 Baslatilamadi...");
    delay(1000);
  }

  CAN.setMode(MCP_NORMAL);
  Serial.println("Arduino Çift Yönlü CAN Haberleşmesi Başlatıldı!");
}

void loop() {
  // 1. STM32'ye İstek Gönder (ID: 0x123)
  byte txData[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
  
  if (CAN.sendMsgBuf(0x123, 0, 8, txData) == CAN_OK) {
    Serial.println("\n[TX] -> STM32'ye istek gönderildi. Yanıt bekleniyor...");
  } else {
    Serial.println("[TX Hata] -> Mesaj gönderilemedi!");
  }

  // 2. STM32'den Yanıt Gelmesini Bekle (500ms Zaman Aşımı)
  unsigned long rxTimer = millis();
  bool responseReceived = false;

  while (millis() - rxTimer < 500) {
    if (CAN_MSGAVAIL == CAN.checkReceive()) {
      long unsigned int rxId;
      unsigned char len = 0;
      unsigned char rxBuf[8];

      CAN.readMsgBuf(&rxId, &len, rxBuf);

      Serial.print("[RX] <- STM32 Yanıt Verdi! ID: 0x");
      Serial.print(rxId, HEX);
      Serial.print(" | Veri: ");

      for (int i = 0; i < len; i++) {
        if (rxBuf[i] < 0x10) Serial.print("0");
        Serial.print(rxBuf[i], HEX);
        Serial.print(" ");
      }
      Serial.println();

      responseReceived = true;
      break; // Yanıt alındı, döngüden çık
    }
  }

  if (!responseReceived) {
    Serial.println("[RX Zaman Aşıımı] -> STM32 yanıt vermedi.");
  }

  delay(2000); // 2 saniyede bir periyodik sorgu
}
