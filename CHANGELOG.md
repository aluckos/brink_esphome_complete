# Changelog - Brink ESPHome Complete

## [1.0.0] - 2026-05-15

### 🎉 Pierwsza oficjalna wersja - Konsolidacja projektów

Połączenie wszystkich wcześniejszych projektów Brink w jeden spójny komponent ESPHome.

### ✨ Dodano
- **Pełny komponent ESPHome** dla Brink Renovent HR / Excelent 400
- **Wszystkie platformy**:
  - `number` - sterowanie poziomem wentylacji (0-100%)
  - `sensor` - temperatury (supply, exhaust, outdoor, extract)
  - `sensor` - przepływ powietrza (m³/h)
  - `sensor` - CPID/CPOD (PID wentylatorów)
  - `sensor` - napięcia czujników (U1-U5)
  - `sensor` - prąd czujników (I1)
  - `binary_sensor` - status filtra (wymiana)
  - `text_sensor` - status bypass
  - `switch` - manual bypass control
  - `select` - tryby pracy (Auto/Manual/Off)
- **Przykłady konfiguracji**:
  - `wemos_d1_mini_basic.yaml` - podstawowa konfiguracja
  - `wemos_d1_mini_full.yaml` - pełna konfiguracja wszystkich encji
- **Kompletna dokumentacja**:
  - `README.md` - główna dokumentacja projektu
  - `INSTALLATION.md` - szczegółowa instalacja
  - `CONFIGURATION.md` - zaawansowana konfiguracja
  - `OPENTHERM_PROTOCOL.md` - opis protokołu OpenTherm i TSP
  - `MIGRATION.md` - migracja z Arduino/openHAB
- **Legacy support**:
  - Archiwum Arduino sketches (3 wersje)
  - Archiwum openHAB configs (items, rules, sitemap)
- **Pliki projektu**:
  - `.gitignore` - ignorowanie build artifacts
  - `LICENSE` - licencja MIT
  - `CHANGELOG.md` - ten plik

### 🔄 Zmigrowano z
- **brink_ot2** (2024-2025):
  - Wszystkie komponenty Python (sensor, number, select, switch, text_sensor, binary_sensor)
  - Nagłówek C++ (`brink_ot.h`)
  - Przykłady YAML
- **brink_ot** (2023-2024):
  - Starsza wersja komponentu (jako referencyjna)
- **brink_openhab** (2022-2023):
  - Arduino sketches (`Brink_HR.ino`, `Brink_HR_bypass.ino`, `Brink_HR_bypass2.ino`)
  - openHAB configs (`brink.items`, `Brink_humidity.rules`)

### 🛠️ Naprawiono
- Usunięto duplikacje kodu między projektami
- Ujednolicono nazewnictwo encji
- Poprawiono handling błędów OpenTherm
- Optymalizacja częstotliwości zapytań (zmniejszenie obciążenia ESP)

### 📚 Dokumentacja
- Dodano kompletną dokumentację instalacji i konfiguracji
- Dodano przewodnik migracji z Arduino i openHAB
- Dodano szczegółowy opis protokołu OpenTherm
- Dodano przykłady automatyzacji Home Assistant

### 🎯 Znane ograniczenia
- Wymaga ESPHome 2023.12.0 lub nowszego
- ESP8266: OTA może trwać do 2 minut (zalecany ESP32)
- Tylko 2.4GHz WiFi (ESP8266 nie obsługuje 5GHz)

---

## [0.3.0] - brink_ot2 (2025-03-10)

### Dodano
- Platforma `select` dla trybów pracy
- Platforma `switch` dla manual bypass
- Rozszerzone sensory (U4, U5, I1)

### Zmieniono
- Optymalizacja zapytań OpenTherm
- Zmniejszenie RAM usage o 20%

---

## [0.2.0] - brink_ot (2024-06-15)

### Dodano
- Pierwsze wydanie ESPHome component
- Podstawowe sensory (temperatury)
- Number control (ventilation level)
- Binary sensor (filter warning)

---

## [0.1.0] - brink_openhab (2023-01-20)

### Dodano
- Arduino sketch dla Brink + MQTT
- openHAB items i rules
- Bypass workaround (okresowe reconnect)

---

## Plan na przyszłość (v1.1.0+)

### Planowane funkcje
- [ ] ESP32-C3 support (RISC-V)
- [ ] Bluetooth proxy integration (Home Assistant)
- [ ] Webserver lokalny (bez HA)
- [ ] Zaawansowane tryby (Night mode, Eco mode, Party mode)
- [ ] Automatyczna kalibracja czujników
- [ ] Statystyki zużycia energii
- [ ] Predykcyjna wymiana filtra (ML)
- [ ] Integration z czujnikami CO2
- [ ] Dashboard Grafana (metrics export)

### Możliwe rozszerzenia
- [ ] Support dla Brink Flair (inny model)
- [ ] Support dla innych rekuperatorów OpenTherm
- [ ] Mobile app (Flutter)
- [ ] Voice control (Alexa, Google Assistant)

---

## Wkład społeczności

Zapraszamy do:
- Pull requestów (nowe funkcje, poprawki)
- Zgłaszania błędów (GitHub Issues)
- Testowania na różnych modelach Brink
- Udoskonaleń dokumentacji

---

## Kontakt

- **GitHub**: [brink_esphome_complete](https://github.com/A0647462_acmtemu/brink_esphome_complete)
- **Issues**: [Report a bug](https://github.com/A0647462_acmtemu/brink_esphome_complete/issues)
- **Discussions**: [Community forum](https://github.com/A0647462_acmtemu/brink_esphome_complete/discussions)

---

**Format wersji**: [Major.Minor.Patch] - YYYY-MM-DD  
**Licencja**: MIT
