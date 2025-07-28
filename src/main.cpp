#include <Servo.h>
#include <Arduino.h>

Servo servo1; // İlk disk
Servo servo2; // İkinci disk

const int servo1Pin = 2;
const int servo2Pin = 3;

// Continuous servo değerleri
const int STOP = 90;
const int CW_FAST = 0;

// Filtre pozisyon süreleri (ms) - daha doğru kalibre edilmiş
const int pos90 = 190;   // Kırmızı filtre pozisyonu
const int pos180 = 350;  // Yeşil filtre pozisyonu  
const int pos270 = 510;  // Mavi filtre pozisyonu

// Sistem durumu
bool systemActive = false;
unsigned long commandStartTime = 0;
int totalFilterTime = 0;

// Disk pozisyon takibi
int servo1CurrentPos = 0;  // 0=normal, 90=kırmızı, 180=yeşil, 270=mavi
int servo2CurrentPos = 0;  // 0=normal, 90=kırmızı, 180=yeşil, 270=mavi

// Fonksiyon prototipleri
void applyFilter(char filterCode, int duration);
void resetToNormal();
void processCommand(String command);

void rotateToPosition(Servo &servo, int duration, String name, String color, int targetPos, int servoNum) {
  Serial.print(name + " " + color + " pozisyonuna donuyor...");
  servo.write(CW_FAST);
  delay(duration);
  servo.write(STOP);
  
  // Pozisyon güncelle
  if (servoNum == 1) {
    servo1CurrentPos = targetPos;
  } else {
    servo2CurrentPos = targetPos;
  }
  
  Serial.println(" Tamamlandi");
  delay(200); // Kısa gecikme - servo stabilizasyonu için
}

void resetToNormal() {
  Serial.println("Diskler standart (N) durumuna getiriliyor...");
  
  // Servo1'i normal pozisyona getir
  int returnDegree1 = (360 - servo1CurrentPos) % 360;
  if (returnDegree1 > 0) {
    Serial.print("Disk1 normal pozisyona donuyor (");
    Serial.print(returnDegree1);
    Serial.println(" derece)...");
    
    int returnTime1 = 0;
    if (returnDegree1 == 90) returnTime1 = pos90;
    else if (returnDegree1 == 180) returnTime1 = pos180;
    else if (returnDegree1 == 270) returnTime1 = pos270;
    
    servo1.write(CW_FAST);
    delay(returnTime1);
    servo1.write(STOP);
    servo1CurrentPos = 0;
    Serial.println("Disk1 normal pozisyonda");
  } else {
    Serial.println("Disk1 zaten normal pozisyonda");
  }
  
  delay(200); // Kısa delay - servo geçişi için
  
  // Servo2'yi normal pozisyona getir
  int returnDegree2 = (360 - servo2CurrentPos) % 360;
  if (returnDegree2 > 0) {
    Serial.print("Disk2 normal pozisyona donuyor (");
    Serial.print(returnDegree2);
    Serial.println(" derece)...");
    
    int returnTime2 = 0;
    if (returnDegree2 == 90) returnTime2 = pos90;
    else if (returnDegree2 == 180) returnTime2 = pos180;
    else if (returnDegree2 == 270) returnTime2 = pos270;
    
    servo2.write(CW_FAST);
    delay(returnTime2);
    servo2.write(STOP);
    servo2CurrentPos = 0;
    Serial.println("Disk2 normal pozisyonda");
  } else {
    Serial.println("Disk2 zaten normal pozisyonda");
  }
  
  delay(200); // Kısa delay
  Serial.println("Sistem standart durumda - Her iki disk normal pozisyonda");
}

void processCommand(String command) {
  if (command.length() != 4) {
    Serial.println("HATA: Komut 4 haneli olmali! (Ornek: 6M7P)");
    return;
  }
  
  // Komut ayrıştırması
  char time1 = command[0];     // İlk süre (6-9)
  char filter1 = command[1];   // İlk filtre (M,F,N,R,G,B,P,Y,C)
  char time2 = command[2];     // İkinci süre (6-9)
  char filter2 = command[3];   // İkinci filtre
  
  // Süre kontrolü
  if (time1 < '6' || time1 > '9' || time2 < '6' || time2 > '9') {
    Serial.println("HATA: Sureler 6-9 arasinda olmali!");
    return;
  }
  
  int duration1 = (time1 - '0'); // Char to int
  int duration2 = (time2 - '0');
  totalFilterTime = duration1 + duration2;
  
  if (totalFilterTime > 15) {
    Serial.println("HATA: Toplam sure 15 saniyeyi asamaz!");
    return;
  }
  
  Serial.println("=== MULTI-SPEKTRAL GORUNTULENME BASLIYOR ===");
  Serial.print("Komut: ");
  Serial.println(command);
  Serial.print("Ilk filtre: ");
  Serial.print(filter1);
  Serial.print(" (");
  Serial.print(duration1);
  Serial.println(" saniye)");
  Serial.print("Ikinci filtre: ");
  Serial.print(filter2);
  Serial.print(" (");
  Serial.print(duration2);
  Serial.println(" saniye)");
  Serial.print("Toplam sure: ");
  Serial.print(totalFilterTime);
  Serial.println(" saniye");
  Serial.println();
  
  systemActive = true;
  commandStartTime = millis();
  
  // İLK FİLTRE UYGULANIR
  Serial.print("ADIM 1: Ilk filtre (");
  Serial.print(filter1);
  Serial.println(") uygulanıyor...");
  applyFilter(filter1, duration1);
  
  // İLK FİLTRE SONRASI NORMALE DÖN
  Serial.println("ADIM 2: Ilk filtre tamamlandi - Sistem normale donuyor...");
  resetToNormal();
  
  // İKİNCİ FİLTRE UYGULANIR
  Serial.print("ADIM 3: Ikinci filtre (");
  Serial.print(filter2);
  Serial.println(") uygulanıyor...");
  applyFilter(filter2, duration2);
  
  // İKİNCİ FİLTRE SONRASI NORMALE DÖN
  Serial.println("ADIM 4: Ikinci filtre tamamlandi - Sistem normale donuyor...");
  resetToNormal();
  
  systemActive = false;
  
  unsigned long totalTime = (millis() - commandStartTime) / 1000;
  Serial.println("=== GOREV TAMAMLANDI ===");
  Serial.print("Toplam gecen sure: ");
  Serial.print(totalTime);
  Serial.println(" saniye");
  Serial.println();
}

void applyFilter(char filterCode, int duration) {
  Serial.print("Filtre kodu: ");
  Serial.print(filterCode);
  Serial.print(" - Sure: ");
  Serial.print(duration);
  Serial.println(" saniye");
  
  // Filtre uygulama başlangıcı
  unsigned long filterStart = millis();
  
  switch(filterCode) {
    case 'M': // Maroon Red (Kırmızı + Kırmızı)
      Serial.println("MAROON RED aktif - Her iki disk kirmizi");
      rotateToPosition(servo1, pos90, "Disk1", "KIRMIZI", 90, 1);
      rotateToPosition(servo2, pos90, "Disk2", "KIRMIZI", 90, 2);
      break;
      
    case 'F': // Forest Green (Yeşil + Yeşil)
      Serial.println("FOREST GREEN aktif - Her iki disk yesil");
      rotateToPosition(servo1, pos180, "Disk1", "YESIL", 180, 1);
      rotateToPosition(servo2, pos180, "Disk2", "YESIL", 180, 2);
      break;
      
    case 'N': // Navy Blue (Mavi + Mavi)
      Serial.println("NAVY BLUE aktif - Her iki disk mavi");
      rotateToPosition(servo1, pos270, "Disk1", "MAVI", 270, 1);
      rotateToPosition(servo2, pos270, "Disk2", "MAVI", 270, 2);
      break;
      
    case 'R': // Light Red (Kırmızı + Normal)
      Serial.println("LIGHT RED aktif - Disk1 kirmizi, Disk2 normal");
      rotateToPosition(servo1, pos90, "Disk1", "KIRMIZI", 90, 1);
      break;
      
    case 'G': // Light Green (Yeşil + Normal)
      Serial.println("LIGHT GREEN aktif - Disk1 yesil, Disk2 normal");
      rotateToPosition(servo1, pos180, "Disk1", "YESIL", 180, 1);
      break;
      
    case 'B': // Light Blue (Mavi + Normal)
      Serial.println("LIGHT BLUE aktif - Disk1 mavi, Disk2 normal");
      rotateToPosition(servo1, pos270, "Disk1", "MAVI", 270, 1);
      break;
      
    case 'P': // Purple (Kırmızı + Mavi)
      Serial.println("PURPLE aktif - Disk1 kirmizi, Disk2 mavi");
      rotateToPosition(servo1, pos90, "Disk1", "KIRMIZI", 90, 1);
      rotateToPosition(servo2, pos270, "Disk2", "MAVI", 270, 2);
      break;
      
    case 'Y': // Yellow (Kırmızı + Yeşil)
      Serial.println("YELLOW aktif - Disk1 kirmizi, Disk2 yesil");
      rotateToPosition(servo1, pos90, "Disk1", "KIRMIZI", 90, 1);
      rotateToPosition(servo2, pos180, "Disk2", "YESIL", 180, 2);
      break;
      
    case 'C': // Cyan (Mavi + Yeşil)
      Serial.println("CYAN aktif - Disk1 mavi, Disk2 yesil");
      rotateToPosition(servo1, pos270, "Disk1", "MAVI", 270, 1);
      rotateToPosition(servo2, pos180, "Disk2", "YESIL", 180, 2);
      break;
      
    default:
      Serial.println("HATA: Gecersiz filtre kodu!");
      return;
  }
  
  // Toplam süre kontrolü - servo hareket süreleri dahil
  unsigned long totalElapsed = millis() - filterStart;
  unsigned long targetTime = duration * 1000;
  
  if (totalElapsed < targetTime) {
    unsigned long remaining = targetTime - totalElapsed;
    Serial.print("Filtre aktif kaliyor - Kalan sure: ");
    Serial.print(remaining / 1000.0, 1);
    Serial.println(" saniye");
    delay(remaining);
  } else {
    Serial.println("Hedef sure servo hareketleri ile tamamlandi");
  }
  
  Serial.println("Filtre suresi tamamlandi");
}

void setup() {
  Serial.begin(9600);
  
  servo1.attach(servo1Pin);
  servo2.attach(servo2Pin);
  
  // Başlangıçta standart durumda
  servo1CurrentPos = 0; // Normal pozisyon
  servo2CurrentPos = 0; // Normal pozisyon
  resetToNormal();
  
  delay(2000);
  
  Serial.println("=== MULTI-SPEKTRAL GORUNTULEME SISTEMI ===");
  Serial.println("Sistem hazir - Standart (N) durumunda");
  Serial.println();
  Serial.println("KOMUT FORMATI: [Sure1][Filtre1][Sure2][Filtre2]");
  Serial.println("Sure: 6-9 arasi rakam");
  Serial.println("Filtre: M,F,N,R,G,B,P,Y,C");
  Serial.println();
  Serial.println("ORNEKLER:");
  Serial.println("6M7P - 6sn Maroon Red, 7sn Purple");
  Serial.println("8R9C - 8sn Light Red, 9sn Cyan");
  Serial.println("7F8B - 7sn Forest Green, 8sn Light Blue");
  Serial.println();
  Serial.println("4 haneli komut girin:");
}

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readString();
    input.trim();
    input.toUpperCase();
    
    Serial.print("Alinan komut: ");
    Serial.println(input);
    
    if (input == "STATUS") {
      Serial.print("Sistem durumu: ");
      Serial.println(systemActive ? "AKTIF" : "STANDBY");
      if (systemActive) {
        unsigned long elapsed = (millis() - commandStartTime) / 1000;
        Serial.print("Gecen sure: ");
        Serial.print(elapsed);
        Serial.println(" saniye");
      }
    }
    else if (input == "RESET") {
      Serial.println("Sistem sifirlaniyor...");
      resetToNormal();
      systemActive = false;
    }
    else {
      processCommand(input);
    }
    
    Serial.println();
    Serial.println("Yeni komut bekleniyor...");
}
}
