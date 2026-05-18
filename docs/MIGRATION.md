# Migration Guide - Brink ESPHome Complete

## Z Arduino (brink_openhab) do ESPHome

### Co się zmienia?

| Arduino | ESPHome |
|---------|---------|
| Kod C++ (.ino) | Konfiguracja YAML |
| MQTT ręczne | Automatyczne API |
| Serial/USB | OTA przez WiFi |
| Ręczne parsowanie | Gotowe encje |

### Krok po kroku

#### 1. Zapisz stare ustawienia

Jeśli używałeś Arduino + MQTT:
- Zapisz adresy MQTT topics
- Zapisz automaty w openHAB/Home Assistant

#### 2. Przygotuj hardware

- **Zostaw** OpenTherm shield (działa tak samo!)
- **Zostaw** połączenie z Brink (2 przewody)
- Sprawdź piny: TX na D4, RX na D3

#### 3. Stwórz konfigurację ESPHome

```yaml
esphome:
  name: brink-ventilation
  platform: ESP8266
  board: d1_mini

# Skopiuj WiFi z Arduino
wifi:
  ssid: "TWOJA_SIEC"
  password: "TWOJE_HASLO"

logger:
api:
ota:

external_components:
  - source:
	  type: local
	  path: components
	components: [ brink_ventilation ]

brink_ventilation:
  tx_pin: GPIO2  # D4 (tak samo jak Arduino)
  rx_pin: GPIO0  # D3 (tak samo jak Arduino)

number:
  - platform: brink_ventilation
	name: "Ventilation Level"

sensor:
  - platform: brink_ventilation
	supply_temp:
	  name: "Supply Temperature"
	exhaust_temp:
	  name: "Exhaust Temperature"
```

#### 4. Wgraj nowy firmware

```bash
esphome run brink_esphome.yaml
```

#### 5. Usuń MQTT z Home Assistant

ESPHome używa **native API**, więc MQTT nie jest potrzebne:
- Usuń `mqtt:` z konfiguracji Home Assistant
- Dodaj integrację **ESPHome**

---

## Z brink_ot/brink_ot2 do brink_esphome_complete

### Co się zmienia?

**Nic!** `brink_esphome_complete` to połączenie tych projektów.

### Jeśli używasz brink_ot (starsza wersja)

#### Brakujące funkcje w brink_ot:
- ❌ Brak `select` (tryby pracy)
- ❌ Brak `switch` (bypass manual)

#### Migracja:

1. **Backup starej konfiguracji**:
```bash
cp my_old_brink.yaml my_old_brink.yaml.bak
```

2. **Zmień źródło komponentu**:

**Stare (brink_ot)**:
```yaml
external_components:
  - source:
	  type: local
	  path: ../brink_ot/components
```

**Nowe (brink_esphome_complete)**:
```yaml
external_components:
  - source:
	  type: local
	  path: ../brink_esphome_complete/components
```

3. **Dodaj nowe encje**:

```yaml
select:
  - platform: brink_ventilation
	operation_mode:
	  name: "Operation Mode"

switch:
  - platform: brink_ventilation
	bypass_control:
	  name: "Bypass Control"
```

4. **Wgraj nową wersję**:
```bash
esphome run my_brink.yaml
```

---

## Mapowanie MQTT → ESPHome API

Jeśli miałeś w openHAB/Home Assistant MQTT topics, oto odpowiedniki:

| MQTT Topic (Arduino) | ESPHome Entity |
|----------------------|----------------|
| `brink/ventilation/level` | `number.brink_ventilation_level` |
| `brink/temp/supply` | `sensor.brink_supply_temp` |
| `brink/temp/exhaust` | `sensor.brink_exhaust_temp` |
| `brink/temp/outdoor` | `sensor.brink_outdoor_temp` |
| `brink/temp/extract` | `sensor.brink_extract_temp` |
| `brink/filter/warning` | `binary_sensor.brink_filter_warning` |
| `brink/bypass/status` | `text_sensor.brink_bypass_status` |
| `brink/bypass/control` | `switch.brink_bypass_control` |

### Przykład migracji automatyzacji

**Stare (MQTT w Home Assistant)**:
```yaml
automation:
  - alias: "Set ventilation to 100%"
	trigger:
	  platform: time
		at: "08:00:00"
	action:
	  service: mqtt.publish
	  data:
		topic: "brink/ventilation/level"
		payload: "100"
```

**Nowe (ESPHome API)**:
```yaml
automation:
  - alias: "Set ventilation to 100%"
	trigger:
	  platform: time
	  at: "08:00:00"
	action:
	  service: number.set_value
	  target:
		entity_id: number.brink_ventilation_level
	  data:
		value: 100
```

---

## Migracja z openHAB do Home Assistant

### 1. Export items z openHAB

Plik `brink.items` w openHAB:
```
Number Brink_Ventilation "Ventilation [%d %%]"
Number Brink_SupplyTemp "Supply Temp [%.1f °C]"
```

### 2. Odpowiedniki w Home Assistant

Automatyczne po dodaniu integracji ESPHome:
- `number.brink_ventilation_level`
- `sensor.brink_supply_temp`

### 3. Migracja rules

**openHAB (brink_humidity.rules)**:
```javascript
rule "Boost on high humidity"
when
	Item Humidity changed
then
	if (Humidity.state > 70) {
		Brink_Ventilation.sendCommand(100)
	}
end
```

**Home Assistant (automation.yaml)**:
```yaml
automation:
  - alias: "Boost on high humidity"
	trigger:
	  platform: numeric_state
	  entity_id: sensor.bathroom_humidity
	  above: 70
	action:
	  service: number.set_value
	  target:
		entity_id: number.brink_ventilation_level
	  data:
		value: 100
```

---

## Problemy po migracji

### Problem: Duplikaty encji w Home Assistant

**Przyczyna**: Stare encje MQTT + nowe ESPHome

**Rozwiązanie**:
1. Settings → Devices & Services → MQTT
2. Znajdź stare encje `brink_*`
3. Usuń je wszystkie
4. Restart Home Assistant

### Problem: Brak połączenia ESPHome

**Rozwiązanie**:
1. Sprawdź czy `api:` jest w YAML
2. Sprawdź czy ESP i HA w tej samej sieci
3. Sprawdź firewall (port 6053)

### Problem: Dane się nie aktualizują

**Rozwiązanie**:
```yaml
brink_ventilation:
  update_interval: 5s  # Zmniejsz z 10s na 5s
```

---

## Zachowanie historii danych

Jeśli chcesz zachować stare wykresy z MQTT:

### Opcja 1: Nie usuwaj starych encji od razu
Poczekaj 30 dni, aż Home Assistant zbuduje historię nowych encji

### Opcja 2: Rename encji w ESPHome
```yaml
sensor:
  - platform: brink_ventilation
	supply_temp:
	  name: "brink_supply_temp"  # Taka sama nazwa jak MQTT
```

Home Assistant automatycznie połączy historię.

---

## Porównanie wydajności

| Metryka | Arduino + MQTT | ESPHome API |
|---------|----------------|-------------|
| Delay update | ~500ms | ~50ms |
| CPU load (ESP) | ~40% | ~15% |
| Network traffic | ~5 KB/s | ~1 KB/s |
| HA CPU load | ~5% | ~1% |
| OTA update time | N/A (manual USB) | 30-120s |

---

## Backup przed migracją

```bash
# Home Assistant
ha core backup

# openHAB (jeśli używasz)
openhab-cli backup

# Arduino code
cp *.ino ~/backups/
```

---

## Rollback (powrót do Arduino)

Jeśli coś poszło nie tak:

1. Wgraj stary firmware Arduino:
```bash
arduino-cli upload -p /dev/ttyUSB0 Brink_HR.ino
```

2. Przywróć konfigurację MQTT w Home Assistant

3. Restart Home Assistant

---

## Następne kroki

- [Installation Guide](INSTALLATION.md) - Szczegóły instalacji
- [Configuration Guide](CONFIGURATION.md) - Zaawansowana konfiguracja
- [OpenTherm Protocol](OPENTHERM_PROTOCOL.md) - Zrozumienie protokołu

---

**Gratulacje! Migracja ukończona!** 🎉

Ciesz się nowoczesnością ESPHome i Home Assistant!
