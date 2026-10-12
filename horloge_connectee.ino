#include <WiFiS3.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include "RTC.h"
#include "ArduinoGraphics.h"
#include "Arduino_LED_Matrix.h"

// ====== MODIFY ======
const char SSID[] = "YOUR WIFI NAME";
const char PASS[] = "YOUR WIFI PW";
// ========================

const unsigned long INTERVALLE_SYNCHRO = 6UL * 60 * 60 * 1000; // resynchro toutes les 6 h

ArduinoLEDMatrix matrice;
WiFiUDP udp;
NTPClient ntp(udp, "pool.ntp.org");  // donne l'heure UTC

unsigned long derniereSynchro = 0;

// ---------- Affichage ----------
void afficher(const char* texte) {
  matrice.beginDraw();
  matrice.stroke(0xFFFFFFFF);
  matrice.textScrollSpeed(60);           // plus petit = plus rapide
  matrice.textFont(Font_5x7);
  matrice.beginText(0, 1, 0xFFFFFF);
  matrice.println(texte);
  matrice.endText(SCROLL_LEFT);          // fait défiler vers la gauche
  matrice.endDraw();
}

// ---------- Heure d'été européenne ----------
// Jour de la semaine (0 = dimanche)
int jourSemaine(int a, int m, int j) {
  static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
  if (m < 3) a--;
  return (a + a / 4 - a / 100 + a / 400 + t[m - 1] + j) % 7;
}

// Dernier dimanche d'un mois de 31 jours (mars et octobre)
int dernierDimanche(int a, int m) {
  return 31 - jourSemaine(a, m, 31);
}

// Heure d'été : du dernier dimanche de mars 01h UTC au dernier dimanche d'octobre 01h UTC
bool heureEte(RTCTime& utc) {
  int a = utc.getYear();
  int m = Month2int(utc.getMonth());
  int j = utc.getDayOfMonth();
  int h = utc.getHour();

  if (m < 3 || m > 10) return false;
  if (m > 3 && m < 10) return true;
  int dd = dernierDimanche(a, m);
  if (m == 3)  return (j > dd) || (j == dd && h >= 1);
  /* m == 10 */ return (j < dd) || (j == dd && h < 1);
}

// ---------- WiFi + NTP ----------
void connecterWiFi() {
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print("Connexion a ");
    Serial.println(SSID);
    afficher("   WiFi...   ");
    WiFi.begin(SSID, PASS);
    delay(2000);
  }
  Serial.println("WiFi connecte !");
}

bool synchroniserHeure() {
  if (WiFi.status() != WL_CONNECTED) connecterWiFi();

  for (int essai = 0; essai < 5; essai++) {
    if (ntp.forceUpdate()) {
      unsigned long epoch = ntp.getEpochTime();
      if (epoch > 1700000000UL) {          // valeur plausible (après 2023)
        RTCTime t((time_t)epoch);
        RTC.setTime(t);                    // la RTC garde l'heure UTC
        Serial.print("Heure synchronisee (UTC) : ");
        Serial.println(String(t));
        return true;
      }
    }
    delay(1000);
  }
  Serial.println("Echec de la synchro NTP");
  return false;
}

// ---------- Programme principal ----------
void setup() {
  Serial.begin(115200);
  matrice.begin();
  RTC.begin();

  connecterWiFi();
  ntp.begin();

  while (!synchroniserHeure()) {
    afficher("   NTP ?   ");
  }
  derniereSynchro = millis();
}

void loop() {
  // Resynchronisation régulière (la RTC continue toute seule entre-temps)
  if (millis() - derniereSynchro > INTERVALLE_SYNCHRO) {
    if (synchroniserHeure()) derniereSynchro = millis();
  }

  // Lecture de l'heure UTC dans la RTC, puis conversion en heure de Paris
  RTCTime utc;
  RTC.getTime(utc);
  int decalage = heureEte(utc) ? 2 : 1;    // UTC+2 l'été, UTC+1 l'hiver
  RTCTime locale((time_t)(utc.getUnixTime() + decalage * 3600L));

  char texte[16];
  snprintf(texte, sizeof(texte), "  %02d:%02d  ", locale.getHour(), locale.getMinutes());
  afficher(texte);
}
