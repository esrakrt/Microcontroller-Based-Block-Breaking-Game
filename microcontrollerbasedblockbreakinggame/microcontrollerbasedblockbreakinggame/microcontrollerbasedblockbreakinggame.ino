#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define LED_D1 53
#define LED_D2 52
#define LED_D3 51
#define OLED_RESET 4
#define BUTTON_START 46
#define BUTTON_STOP 45
#define BUTTON_SELECT 47
#define BUTTON_COLOR_TOGGLE 49 // Renk değiştirme butonu için pin tanımlaması
#define POT_PIN A0
#define LDR_PIN A1 // Işık sensörü için pin tanımlaması
#define LIGHT_THRESHOLD 100 // Işık seviyesi eşiği
// 7 segment display pinleri
#define SEG_A 22
#define SEG_B 23
#define SEG_C 24
#define SEG_D 25
#define SEG_E 26
#define SEG_F 27
#define SEG_G 28
#define DIGIT_1 30
#define DIGIT_2 29

Adafruit_SSD1306 display(128, 64, &Wire, OLED_RESET);

int menu = 0;
bool invertedColors = false; // Renklerin normal mi yoksa ters mi olduğunu takip etmek için bir değişken
int lives = 3; // Çubuğun can sayısını takip etmek için bir değişken 
int level = 1; // Hangi seviyede olduğunuzu takip etmek için bir değişken 
bool startSelected = false;
int ballX = 64, ballY = 50; // Topun başlangıç konumu
int ballSpeedX = 4, ballSpeedY = -4; // Topun hızı
// Skor
int score = 0;
// "O" harfinin konumunu ve hızını saklayan yapı 
struct LetterO {
  int x, y;
  int speedY;
};

// Kırılan her blok için bir "O" harfi oluşturur
LetterO lettersO[8][2];
// 7 segment display için sayıları tanımlama
byte numbers[10] = {B11111100, // 0
                     B01100000, // 1
                     B11011010, // 2
                     B11110010, // 3
                     B01100110, // 4
                     B10110110, // 5
                     B10111110, // 6
                     B11100000, // 7
                     B11111110, // 8
                     B11110110  // 9
                    };


// Blokların konumlarını ve durumlarını saklayan bir dizi
bool blocks[8][3];
bool newLevelBlocks[8][3];

void setup() {
  randomSeed(analogRead(0));
  pinMode(BUTTON_START, INPUT_PULLUP);
  pinMode(BUTTON_STOP, INPUT_PULLUP);
  pinMode(BUTTON_SELECT, INPUT_PULLUP);
  pinMode(LED_D1, OUTPUT);
  pinMode(LED_D2, OUTPUT);
  pinMode(LED_D3, OUTPUT);
  pinMode(BUTTON_COLOR_TOGGLE, INPUT_PULLUP); // Renk değiştirme butonunu bir giriş olarak ayarla
  pinMode(LDR_PIN, INPUT); // Işık sensörünü bir giriş olarak ayarla

  digitalWrite(LED_D1, HIGH); // Başlangıçta tüm LED'ler yanar
  digitalWrite(LED_D2, HIGH);
  digitalWrite(LED_D3, HIGH);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3D);
  display.clearDisplay();
  display.display();
  display.setTextColor(WHITE, BLACK);

  // İlk seviye için blokların konumlarını ayarla
  for (int i = 0; i < 8; i++) {
    blocks[i][0] = true;
    // "O" harflerinin başlangıç konumlarını ve hızlarını ayarla
    lettersO[i][0] = {0, display.height() + 1, 0}; // Y konumunu ekranın dışına ayarla
  }

  // İkinci seviye için blokların konumlarını ayarla
  for (int i = 0; i < 8; i++) {
    if (i < 2 || i > 5) {
      newLevelBlocks[i][0] = true;
      newLevelBlocks[i][1] = true;
    } else {
      newLevelBlocks[i][0] = false;
      newLevelBlocks[i][1] = false;
    }
  }
  // Dördüncü seviye için blokların konumlarını ayarla
  if (level == 4) {
    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 2; j++) {
        // Satranç tahtası gibi bir düzenleme yap
        if ((i + j) % 2 == 0) {
          newLevelBlocks[i][j] = true;
        } else {
          newLevelBlocks[i][j] = false;
        }
      }
    }
  }
  
  // 7 segment display pinlerini çıkış olarak ayarla
  pinMode(SEG_A, OUTPUT);
  pinMode(SEG_B, OUTPUT);
  pinMode(SEG_C, OUTPUT);
  pinMode(SEG_D, OUTPUT);
  pinMode(SEG_E, OUTPUT);
  pinMode(SEG_F, OUTPUT);
  pinMode(SEG_G, OUTPUT);
  pinMode(DIGIT_1, OUTPUT);
  pinMode(DIGIT_2, OUTPUT);


  display.setCursor(0, 0);
  display.println("Baslat");
  display.println("Cikis");
  display.display();
}




void loop() {
  if (digitalRead(BUTTON_START) == LOW) {
    menu--;
    if (menu < 0) menu = 1;
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Baslat");
    display.println("Cikis");
    display.drawRect(0, menu * 8, display.width(), 8, WHITE); // Seçilen seçeneği çerçevele
    display.display();
    delay(200);
  } else if (digitalRead(BUTTON_STOP) == LOW) {
    menu++;
    if (menu > 1) menu = 0;
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Baslat");
    display.println("Cikis");
    display.drawRect(0, menu * 8, display.width(), 8, WHITE); // Seçilen seçeneği çerçevele
    display.display();
    delay(200);
  }

   if (digitalRead(BUTTON_SELECT) == LOW) {
    if (menu == 0) {
      startSelected = true;
    } else if (menu == 1) {
      startSelected = false;
    }
    display.clearDisplay();
    display.setCursor(0, 0);
    if (startSelected) {
      display.println("Oyun basladi iyi         eglenceler!");
    } else {
      display.println("Oyunumuza            gosterdiginiz ilgi   icin tesekkurler!");
    }
    display.display();
    delay(1000);
  }
  
  // Işık sensörünün okumasını al
  int lightLevel = analogRead(LDR_PIN);

  // Eğer ışık seviyesi belirlenen eşiğin altındaysa, ekranın renklerini tersine çevir
  if (lightLevel < LIGHT_THRESHOLD) {
    display.invertDisplay(true); // Ekranın renklerini tersine çevir
  } else {
    display.invertDisplay(false); // Ekranın renklerini normal hale getir
  }
  if (startSelected) {
  display.clearDisplay();
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 2; j++) {
      // Eğer blok aktifse, onu çiz
      if (blocks[i][j]) {
        display.fillRect(i * 16, j * 8, 14, 6, WHITE);
      }
    }
  }
    // Potansiyometrenin değerini oku
    int potValue = analogRead(POT_PIN);
    // Potansiyometrenin değerini çubuğun x konumuna dönüştür
    int barX = map(potValue, 0, 1023, 0, display.width() - display.width()/4);
    // Ekranın en altında beyaz bir çubuk çiz
    display.fillRect(barX, display.height()-4, display.width()/4, 4, WHITE); // Çubuğu incelt

    // Topun konumunu güncelle
    ballX += ballSpeedX;
    ballY += ballSpeedY;
    // Top çubuğun altına düştüyse, çubuk bir can kaybeder
    if (ballY >= display.height()) {
  lives--;
  if (lives == 2) {
    digitalWrite(LED_D3, LOW); // Üçüncü LED'i kapat
  } else if (lives == 1) {
    digitalWrite(LED_D2, LOW); // İkinci LED'i kapat
  } else if (lives == 0) {
  // Oyunu bitir ve kırılan blok sayısını göster
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Oyun bitti!");
  display.println("Kirilan blok sayisi: " + String(score));
  display.display();
  delay(3000); // 3 saniye beklet

  // Her şeyi sıfırla
  score = 0;
  lives = 3;
  level = 1;
  startSelected = false;
  menu = 0;
  ballX = 64;
  ballY = 50;
  ballSpeedX = 2;
  ballSpeedY = -2;

  // LED'leri tekrar aç
  digitalWrite(LED_D1, HIGH);
  digitalWrite(LED_D2, HIGH);
  digitalWrite(LED_D3, HIGH);

  // Blokları tekrar oluştur
  for (int i = 0; i < 8; i++) {
    blocks[i][0] = true;
  }

  // İlk giriş ekranını aç
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Baslat");
  display.println("Cikis");
  display.display();

  // Oyun döngüsünden çık
  return;
}


  // Topun konumunu çubuğun üzerine ayarla ve yönünü değiştir
  ballY = display.height()-10; // Çubuğun biraz daha üzerine ayarla
  ballSpeedY = -abs(ballSpeedY); // Topun yönünü yukarı doğru değiştir

  // Topun konumunu çubuğun üzerine ayarla
  ballX = barX + display.width()/8;
  ballY = display.height()-10; // Çubuğun biraz daha üzerine ayarla

  // Topu 3 saniye boyunca gizle
  display.clearDisplay();
  display.display();
  delay(500); // 3 saniye beklet
}


    // Top ekranın kenarlarına çarptıysa yönünü değiştir
    if (ballX <= 0 || ballX >= display.width() - 4) ballSpeedX *= -1;
    if (ballY <= 0) ballSpeedY *= -1;

    // Top çubuğa çarptıysa yönünü değiştir
    if (ballY >= display.height() - 8 && ballX >= barX && ballX <= barX + display.width()/4) ballSpeedY *= -1;

    

    // Topu çiz
    display.fillCircle(ballX, ballY, 2, WHITE);

    display.display();
    // Topun bloklara çarpmasını kontrol et
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 2; j++) {
      if (blocks[i][j] && ballX >= i * 16 && ballX <= i * 16 + 14 && ballY >= j * 8 && ballY <= j * 8 + 6) {
        // Top bloğa çarptıysa, bloğu kaldır ve topun yönünü değiştir
        blocks[i][j] = false;
        ballSpeedY = abs(ballSpeedY); // Topun yönünü her zaman aşağı doğru değiştir

        // Skoru artır ve 7 segment display'i güncelle
        score++;
        update7Segment(score);

        // %10 ihtimalle "O" harfini bloğun altında oluştur ve hızını topun hızına ayarla
        if (random(10) < 1) { // %10 ihtimal
          lettersO[i][j] = {i * 16, (j * 8) + 6, ballSpeedY}; // Y konumunu bloğun altına ayarla
        } else {
          lettersO[i][j] = {0, display.height() + 1, 0}; // "O" harfi oluşturulmazsa, konum ve hızı sıfırla
        }
      }
    }
  }

// Her çizim döngüsünde "O" harflerini çiz
for (int i = 0; i < 8; i++) {
  for (int j = 0; j < 2; j++) {
    // Eğer "O" harfi ekranın içindeyse ve y hızı 0'dan farklıysa, "O" harfini çiz
    if (!blocks[i][j] && lettersO[i][j].y < display.height() && lettersO[i][j].y > 0 && lettersO[i][j].speedY != 0) {
      // "O" harfini çiz
      display.setCursor(lettersO[i][j].x, lettersO[i][j].y);
      display.print("*");

      // "O" harfinin konumunu güncelle
      lettersO[i][j].y += lettersO[i][j].speedY;
    // Eğer "O" harfi çubuğa çarptıysa, "O" harfini kaldır ve can sayısını artır
      if (lettersO[i][j].y >= display.height() - 8 && lettersO[i][j].x >= barX && lettersO[i][j].x <= barX + display.width()/4) {
        lettersO[i][j] = {0, display.height() + 1, 0}; // "O" harfinin konum ve hızını sıfırla
        if (lives < 3) { // Can sayısı 3'ten küçükse, can sayısını artır
          lives++;
          if (lives == 2) {
            digitalWrite(LED_D2, HIGH); // İkinci LED'i aç
          } else if (lives == 3) {
            digitalWrite(LED_D3, HIGH); // Üçüncü LED'i aç
          }
        }
      }
    }
  }
}

display.display();

// Eğer tüm bloklar kırıldıysa, yeni seviyeye geç
if (allBlocksBroken()) {
  // Sonraki bölüm açılıyor mesajını göster
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Sonraki bolum        aciliyor...");
  display.display();
  delay(1000); // 5 saniye beklet

  level++; // Seviyeyi artır

  // Topun hızını %20 artır
  ballSpeedX *= 1.2;
  ballSpeedY *= 1.2;
  if (level == 2) {
    // İkinci seviye için blokların konumlarını ayarla
    
    for (int i = 0; i < 4; i++) {
      blocks[i][0] = true; // İlk satırın sol yarısında bloklar oluştur
    }
    for (int i = 4; i < 8; i++) {
      blocks[i][1] = true; // İkinci satırın sağ yarısında bloklar oluştur
    }
  } else if (level == 3) {
    // Üçüncü seviye için blokların konumlarını ayarla
    for (int i = 0; i < 8; i += 2) { // Blokların yan yana olmaması için ikişer ikişer artır
      for (int j = 0; j < 2; j++) {
        blocks[i][j] = true;
      }
    }
  } else if (level == 4) {
    // Dördüncü seviye için blokların konumlarını ayarla
    for (int i = 0; i < 8; i++) {
      for (int j = 0; j < 2; j++) {
        // Satranç tahtası gibi bir düzenleme yap
        if ((i + j) % 2 == 0) {
          blocks[i][j] = true;
        } else {
          blocks[i][j] = false;
        }
      }
    }
  } else if (level == 5) {
    for (int i = 0; i < 8; i++) {
      if (i < 2 || i > 5) {
        blocks[i][0] = true;
        blocks[i][1] = true;
      } else {
        blocks[i][0] = false;
        blocks[i][1] = false;
      }
    }
  }
  // Topun konumunu çubuğun üzerine ayarla
  ballX = barX + display.width()/8;
  ballY = display.height()-10; // Çubuğun biraz daha üzerine ayarla
}

  }
  
}
// Tüm blokların kırılıp kırılmadığını kontrol eden bir fonksiyon
bool allBlocksBroken() {
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 2; j++) {
      if (blocks[i][j]) {
        return false; // Eğer herhangi bir blok aktifse, tüm bloklar kırılmamış demektir
      }
    }
  }
  return true; // Eğer bu noktaya geldiysek, tüm bloklar kırılmış demektir
}
void update7Segment(int number) {
  int digit1 = number / 10;
  int digit2 = number % 10;

  digitalWrite(DIGIT_1, LOW);
  digitalWrite(DIGIT_2, HIGH);
  displayNumber(digit2);
  displayNumber(digit1);

  delay(1);

  digitalWrite(DIGIT_1, HIGH);
  digitalWrite(DIGIT_2, LOW);
  displayNumber(digit2);
  
}

void displayNumber(int number) {
  byte numToDisplay = numbers[number];

  digitalWrite(SEG_A, numToDisplay & B10000000);
  digitalWrite(SEG_B, numToDisplay & B01000000);
  digitalWrite(SEG_C, numToDisplay & B00100000);
  digitalWrite(SEG_D, numToDisplay & B00010000);
  digitalWrite(SEG_E, numToDisplay & B00001000);
  digitalWrite(SEG_F, numToDisplay & B00000100);
  digitalWrite(SEG_G, numToDisplay & B00000010);
}