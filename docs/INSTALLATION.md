# Installation Guide - Brink ESPHome Complete

## Wymagania wstępne

### Software
- Python 3.9 lub nowszy
- pip (Python package manager)
- ESPHome 2023.12.0 lub nowszy
- (Opcjonalnie) Home Assistant 2023.11 lub nowszy

### Hardware
- **ESP8266** (Wemos D1 mini) LUB **ESP32** (DevKit, WROOM)
- **OpenTherm Master Shield**
- **Kabel 2-przewodowy** (minimum 0.5mm², max 50m)
- **Zasilacz 5V** (minimum 1A)

---

## Krok 1: Instalacja ESPHome

### Windows
```powershell
pip install -U esphome
```

### Linux/macOS
```bash
pip3 install -U esphome
```

### Weryfikacja instalacji
```bash
esphome version
```

Powinieneś zobaczyć: `Version: 2023.12.x`

---

## Krok 2: Przygotowanie konfiguracji

### Opcja A: Użyj gotowego przykładu

```bash
cd C:\Users\YOUR_USER\source\repos\A0647462_acmtemu\brink_esphome_complete
cp examples/wemos_d1_mini_basic.yaml my_brink.yaml
```

### Opcja B: Stwórz własną konfigurację

```bash
esphome wizard my_brink.yaml
```

Odpowiedz na pytania:
- **Device name**: `brink-ventilation`
- **WiFi SSID**: twoja sieć WiFi
- **WiFi password**: hasło do WiFi
- **Board**: `d1_mini` (ESP8266) lub `esp32dev` (ESP32)

Następnie dodaj do pliku:

```yaml
external_components:
  - source:
	  type: local
	  path: components
	components: [ brink_ventilation ]

brink_ventilation:
  id: brink_vent

number:
  - platform: brink_ventilation
	brink_ventilation_id: brink_vent
	name: "Ventilation Level"

sensor:
  - platform: brink_ventilation
	brink_ventilation_id: brink_vent
	supply_temp:
	  name: "Supply Temperature"
	exhaust_temp:
	  name: "Exhaust Temperature"
```

---

## Krok 3: Montaż sprzętu

### 3.1 OpenTherm Shield

1. **Włóż shield na Wemos D1 mini** (piny się zgadzają)
2. **Sprawdź piny**:
   - `TX` → `D4` (GPIO2)
   - `RX` → `D3` (GPIO0)
   - `VCC` → `5V`
   - `GND` → `GND`

### 3.2 Podłączenie do Brink

**⚠️ WAŻNE: Wyłącz zasilanie rekuperatora przed podłączeniem!**

1. Znajdź port **OpenTherm** na płycie głównej Brink (zazwyczaj 2-pin connector obok RJ45)
2. Podłącz **2 przewody** (polaryzacja dowolna, OpenTherm działa w obie strony)
3. Drugi koniec podłącz do shield (terminale `OT+` i `OT-`)

**Schemat podłączenia:**
```
[Brink OT Port] ←--→ [2-wire cable] ←--→ [OpenTherm Shield] ←--→ [ESP8266/ESP32]
											 ↓
										  [5V PSU]
```

### 3.3 Zasilanie

- Podłącz **zasilacz 5V** do portu micro-USB Wemos
- LUB użyj pinu `5V` i `GND` (jeśli masz zewnętrzne zasilanie)

---

## Krok 4: Pierwsza kompilacja i wgranie

### 4.1 Kompilacja

```bash
esphome compile my_brink.yaml
```

Poczekaj ~5-10 minut (pierwsza kompilacja jest długa).

### 4.2 Wgranie przez USB

1. **Podłącz ESP do komputera** (USB)
2. **Sprawdź port**:
   - Windows: Device Manager → Ports → `COM3` (lub inny)
   - Linux: `ls /dev/ttyUSB*`
3. **Wgraj firmware**:

```bash
esphome run my_brink.yaml
```

Wybierz odpowiedni port COM/ttyUSB.

### 4.3 Pierwsze uruchomienie

Po wgraniu:
1. ESP zrestartuje się automatycznie
2. Połączy się z WiFi
3. Pojawi się w ESPHome Dashboard (jeśli używasz)

---

## Krok 5: Weryfikacja połączenia

### 5.1 Logi przez USB

```bash
esphome logs my_brink.yaml
```

Szukaj:
```
[I][opentherm:XXX]: OpenTherm connected
[I][sensor:XXX]: Supply temp: 22.5°C
```

### 5.2 Logi przez WiFi (OTA)

```bash
esphome logs my_brink.yaml --device brink-ventilation.local
```

---

## Krok 6: Integracja z Home Assistant

### Automatyczna (jeśli HA i ESP w tej samej sieci)

1. Otwórz Home Assistant
2. Przejdź do **Settings → Devices & Services**
3. Kliknij **ESPHome** integration
4. Urządzenie `brink-ventilation` powinno się automatycznie pojawić
5. Kliknij **Configure** → **Submit**

### Ręczna

1. **Settings → Devices & Services → Add Integration**
2. Wybierz **ESPHome**
3. Wpisz: `brink-ventilation.local` (lub IP: `192.168.x.x`)
4. Kliknij **Submit**

---

## Krok 7: Test funkcjonalności

### Test 1: Sterowanie wentylacją

W Home Assistant:
1. Znajdź `number.brink_ventilation_level`
2. Ustaw wartość `50`
3. Rekuperator powinien zmienić obroty

### Test 2: Odczyt temperatur

Sprawdź czy wartości są realistyczne:
- `sensor.brink_supply_temp` → ~22°C (temperatura w domu)
- `sensor.brink_exhaust_temp` → ~20°C (wywiew)
- `sensor.brink_outdoor_temp` → temperatura zewnętrzna

### Test 3: Status filtra

Jeśli filtr jest brudny:
- `binary_sensor.brink_filter_warning` → `ON`

---

## Rozwiązywanie problemów

### Problem: ESP nie łączy się z WiFi

**Rozwiązanie:**
1. Sprawdź SSID i hasło w YAML
2. Sprawdź czy WiFi jest 2.4GHz (ESP8266 nie obsługuje 5GHz)
3. Spróbuj Access Point mode:

```yaml
wifi:
  ap:
	ssid: "Brink Fallback"
	password: "12345678"
```

### Problem: Brak komunikacji OpenTherm

**Rozwiązanie:**
1. Sprawdź połączenie 2-przewodowe
2. Sprawdź piny shield (TX/RX)
3. Zweryfikuj w logach: `[E][opentherm:XXX]: No response`
4. Zmień piny w YAML:

```yaml
brink_ventilation:
  tx_pin: GPIO2  # D4
  rx_pin: GPIO0  # D3
```

### Problem: Błąd kompilacji

**Rozwiązanie:**
```bash
esphome clean my_brink.yaml
esphome compile my_brink.yaml
```

### Problem: OTA update nie działa

**Rozwiązanie:**
1. Sprawdź czy ESP i komputer są w tej samej sieci
2. Sprawdź firewall
3. Użyj IP zamiast `.local`:

```bash
esphome run my_brink.yaml --device 192.168.1.100
```

---

## Aktualizacje OTA (Over-The-Air)

Po pierwszym wgraniu przez USB, kolejne można przez WiFi:

```bash
esphome run my_brink.yaml
```

ESPHome automatycznie wykryje urządzenie w sieci i wgra nową wersję.

---

## Bezpieczeństwo

### Zabezpieczenie OTA hasłem

```yaml
ota:
  password: "super_secret_password"
```

### Wyłączenie API logging

```yaml
logger:
  level: INFO
  logs:
	wifi: WARN
```

---

## Następne kroki

- [Configuration Guide](CONFIGURATION.md) - Zaawansowana konfiguracja
- [OpenTherm Protocol](OPENTHERM_PROTOCOL.md) - Zrozumienie protokołu
- [Migration Guide](MIGRATION.md) - Migracja z Arduino/openHAB

---

**Gotowe! Twój rekuperator Brink działa z ESPHome!** 🎉
