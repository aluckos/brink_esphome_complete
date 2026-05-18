# Brink ESPHome Complete

**Kompletna integracja rekuperatora Brink (Excelent 400 / Renovent HR) z ESPHome przez OpenTherm**

[![ESPHome](https://img.shields.io/badge/ESPHome-Compatible-blue)](https://esphome.io)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

Połączenie wszystkich projektów Brink OpenTherm w jedno, w pełni funkcjonalne repozytorium z pełną dokumentacją i przykładami.

---

## 🎯 Cechy

- ✅ **Pełna kontrola wentylacji** (0-100% + tryby pracy)
- ✅ **Monitoring temperatur** (wejście/wyjście, nawiew/wywiew)
- ✅ **Status filtra** i przypomnienie o wymianie
- ✅ **Kontrola bypass** (automatyczna/ręczna)
- ✅ **Monitoring przepływu** powietrza
- ✅ **Czujniki napięcia i prądu** (U1-U5, I1)
- ✅ **CPID/CPOD** (sterowanie wentylatorem)
- ✅ **Integracja z Home Assistant** (automatyczna discovery)
- ✅ **OTA updates** (aktualizacje przez WiFi)

---

## 📦 Wymagany sprzęt

### Podstawowy zestaw:
- **ESP8266** (Wemos D1 mini) lub **ESP32** (DevKit, WROOM)
- **OpenTherm Master Shield** (Ihor Melnyk / diyless / własny)
- **Kabel 2-przewodowy** do portu OpenTherm w Brink
- **Zasilanie 5V** (USB lub zewnętrzne)

### Opcjonalnie:
- Obudowa 3D printed
- Dodatkowe czujniki (BME280, DS18B20)

---

## 🚀 Szybki start

### 1. Instalacja ESPHome

```bash
pip install -U esphome
```

### 2. Wybór przykładowej konfiguracji

```bash
# Podstawowa (tylko monitoring + sterowanie)
esphome run examples/wemos_d1_mini_basic.yaml

# Pełna (wszystkie funkcje)
esphome run examples/wemos_d1_mini_full.yaml
```

### 3. Podłączenie do rekuperatora

1. Wyłącz zasilanie Brink
2. Podłącz 2 przewody do portu OpenTherm (polaryzacja dowolna)
3. Podłącz shield do ESP
4. Włącz zasilanie
5. Sprawdź logi w ESPHome

---

## 📖 Dokumentacja

- **[Installation Guide](docs/INSTALLATION.md)** - Szczegółowa instalacja
- **[Configuration](docs/CONFIGURATION.md)** - Wszystkie opcje konfiguracji
- **[OpenTherm Protocol](docs/OPENTHERM_PROTOCOL.md)** - Opis protokołu i TSP
- **[Migration Guide](docs/MIGRATION.md)** - Migracja z Arduino/openHAB

---

## 📁 Struktura projektu

```
brink_esphome_complete/
├── components/
│   └── brink_ventilation/        # ESPHome component (główny kod)
│       ├── __init__.py            # Inicjalizacja komponentu
│       ├── brink_ot.h             # Nagłówek C++ (OpenTherm)
│       ├── binary_sensor.py       # Binary sensors (filtr)
│       ├── number.py              # Number controls (sterowanie %)
│       ├── select.py              # Select controls (tryby)
│       ├── sensor.py              # Sensors (temperatury, przepływ)
│       ├── switch.py              # Switches (bypass, tryby)
│       └── text_sensor.py         # Text sensors (status)
├── examples/
│   ├── wemos_d1_mini_basic.yaml  # Podstawowa konfiguracja
│   └── wemos_d1_mini_full.yaml   # Pełna konfiguracja
├── docs/
│   ├── INSTALLATION.md
│   ├── CONFIGURATION.md
│   ├── OPENTHERM_PROTOCOL.md
│   └── MIGRATION.md
├── legacy/                        # Archiwum starych rozwiązań
│   ├── arduino_sketches/          # Arduino (przed ESPHome)
│   └── openhab/                   # openHAB configs
└── README.md                      # Ten plik
```

---

## 🔌 Obsługiwane encje ESPHome

### **Number** (sterowanie)
| Encja | Opis | Zakres |
|-------|------|--------|
| `ventilation_level` | Poziom wentylacji | 0-100% |

### **Sensors** (monitoring)
| Encja | Opis | Jednostka |
|-------|------|-----------|
| `supply_temp` | Temperatura nawiewu | °C |
| `exhaust_temp` | Temperatura wywiewu | °C |
| `outdoor_temp` | Temperatura zewnętrzna | °C |
| `extract_temp` | Temperatura wyciągu | °C |
| `airflow` | Przepływ powietrza | m³/h |
| `cpid` | CPID wentylatora nawiewu | - |
| `cpod` | CPOD wentylatora wywiewu | - |
| `u1` - `u5` | Napięcia czujników | V |
| `i1` | Prąd czujnika | mA |

### **Binary Sensors** (stany)
| Encja | Opis |
|-------|------|
| `filter_warning` | Status filtra (wymiana) |

### **Text Sensors** (informacje)
| Encja | Opis |
|-------|------|
| `bypass_status` | Status bypass (open/closed/auto) |

### **Switches** (przełączniki)
| Encja | Opis |
|-------|------|
| `bypass_control` | Manual bypass on/off |

### **Select** (tryby)
| Encja | Opis | Opcje |
|-------|------|-------|
| `operation_mode` | Tryb pracy | Auto/Manual/Off |

---

## 🏠 Integracja z Home Assistant

Po wkompilowaniu i uruchomieniu, urządzenie automatycznie pojawi się w Home Assistant (jeśli masz skonfigurowane ESPHome integration).

### Przykładowa karta Lovelace:

```yaml
type: entities
title: Brink Ventilation
entities:
  - entity: number.brink_ventilation_level
  - entity: sensor.brink_supply_temp
  - entity: sensor.brink_exhaust_temp
  - entity: sensor.brink_airflow
  - entity: binary_sensor.brink_filter_warning
  - entity: switch.brink_bypass_control
```

---

## 🔧 Zaawansowana konfiguracja

### Dodanie własnych czujników

```yaml
sensor:
  - platform: bme280
	address: 0x76
	temperature:
	  name: "Room Temperature"
	humidity:
	  name: "Room Humidity"
```

### Automatyzacje bypass (Home Assistant)

```yaml
automation:
  - alias: "Brink - Open bypass on hot days"
	trigger:
	  - platform: numeric_state
		entity_id: sensor.brink_outdoor_temp
		above: 25
	action:
	  - service: switch.turn_on
		target:
		  entity_id: switch.brink_bypass_control
```

---

## 🐛 Rozwiązywanie problemów

### ESP nie łączy się z WiFi
- Sprawdź nazwę sieci i hasło w YAML
- Sprawdź zasięg WiFi
- Wymuś restart ESP (przycisk RST)

### Brak danych z rekuperatora
- Sprawdź połączenie OpenTherm (2 przewody)
- Sprawdź logi ESPHome: `esphome logs <config>.yaml`
- Upewnij się że shield działa (LED powinien migać)

### Filter warning nie działa
- Zresetuj licznik w rekuperatorze (menu serwisowe)
- Sprawdź TSP 13 w logach

---

## 📜 Licencja

MIT License - możesz używać, modyfikować i dystrybuować za darmo.

---

## 🤝 Wkład

Pull requesty mile widziane! 

Jeśli znalazłeś błąd lub masz pomysł na nową funkcję:
1. Fork tego repo
2. Stwórz branch (`git checkout -b feature/new-feature`)
3. Commit (`git commit -m 'Add new feature'`)
4. Push (`git push origin feature/new-feature`)
5. Otwórz Pull Request

---

## 📞 Kontakt i wsparcie

- **Issues**: [GitHub Issues](https://github.com/A0647462_acmtemu/brink_esphome_complete/issues)
- **Dyskusja**: [GitHub Discussions](https://github.com/A0647462_acmtemu/brink_esphome_complete/discussions)

---

## 🙏 Podziękowania

- **Ihor Melnyk** - OpenTherm shield design
- **ESPHome team** - Framework
- **Społeczność Home Assistant** - Testy i feedback
- Wszyscy kontrybutorzy projektów `brink_ot`, `brink_ot2`, `brink_openhab`

---

## 📚 Historia projektu

Ten projekt łączy:
- **brink_ot** - Pierwsza wersja ESPHome (basic sensors)
- **brink_ot2** - Rozszerzona wersja (full control)
- **brink_openhab** - Arduino + openHAB (legacy)

Wszystko zostało skonsolidowane w jeden, łatwy w użyciu komponent ESPHome.

---

**Wersja: 1.0.0** | **Data: 2026-05-15** | **Autor: A0647462**
