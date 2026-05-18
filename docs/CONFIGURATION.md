# Configuration Guide - Brink ESPHome Complete

## Podstawowa konfiguracja

### Minimalna konfiguracja YAML

```yaml
esphome:
  name: brink-ventilation
  platform: ESP8266
  board: d1_mini

wifi:
  ssid: "YOUR_WIFI_SSID"
  password: "YOUR_WIFI_PASSWORD"

logger:

api:
  encryption:
	key: "YOUR_32BYTE_KEY_HERE"

ota:
  password: "YOUR_OTA_PASSWORD"

external_components:
  - source:
	  type: local
	  path: components
	components: [ brink_ventilation ]

brink_ventilation:
  id: brink_vent
  tx_pin: GPIO2  # D4
  rx_pin: GPIO0  # D3
```

---

## Pełna konfiguracja wszystkich encji

### Number (sterowanie)

```yaml
number:
  - platform: brink_ventilation
	brink_ventilation_id: brink_vent
	name: "Ventilation Level"
	min_value: 0
	max_value: 100
	step: 1
	mode: slider
	icon: "mdi:fan"
```

### Sensors (monitoring)

```yaml
sensor:
  - platform: brink_ventilation
	brink_ventilation_id: brink_vent

	# Temperatury
	supply_temp:
	  name: "Supply Temperature"
	  accuracy_decimals: 1
	  filters:
		- sliding_window_moving_average:
			window_size: 5
			send_every: 5

	exhaust_temp:
	  name: "Exhaust Temperature"
	  accuracy_decimals: 1

	outdoor_temp:
	  name: "Outdoor Temperature"
	  accuracy_decimals: 1

	extract_temp:
	  name: "Extract Temperature"
	  accuracy_decimals: 1

	# Przepływ powietrza
	airflow:
	  name: "Airflow"
	  unit_of_measurement: "m³/h"
	  accuracy_decimals: 0

	# CPID/CPOD (sterowanie wentylatorem)
	cpid:
	  name: "CPID Supply Fan"
	  accuracy_decimals: 0

	cpod:
	  name: "CPOD Exhaust Fan"
	  accuracy_decimals: 0

	# Napięcia czujników
	u1:
	  name: "Sensor U1"
	  unit_of_measurement: "V"
	  accuracy_decimals: 2

	u2:
	  name: "Sensor U2"
	  unit_of_measurement: "V"
	  accuracy_decimals: 2

	u3:
	  name: "Sensor U3"
	  unit_of_measurement: "V"
	  accuracy_decimals: 2

	u4:
	  name: "Sensor U4"
	  unit_of_measurement: "°C"
	  accuracy_decimals: 1

	u5:
	  name: "Sensor U5"
	  unit_of_measurement: "°C"
	  accuracy_decimals: 1

	# Prąd czujnika
	i1:
	  name: "Sensor I1"
	  unit_of_measurement: "mA"
	  accuracy_decimals: 0
```

### Binary Sensors (stany)

```yaml
binary_sensor:
  - platform: brink_ventilation
	brink_ventilation_id: brink_vent
	filter_warning:
	  name: "Filter Warning"
	  device_class: problem
	  icon: "mdi:air-filter"
```

### Text Sensors (informacje)

```yaml
text_sensor:
  - platform: brink_ventilation
	brink_ventilation_id: brink_vent
	bypass_status:
	  name: "Bypass Status"
	  icon: "mdi:valve"
```

### Switches (przełączniki)

```yaml
switch:
  - platform: brink_ventilation
	brink_ventilation_id: brink_vent
	bypass_control:
	  name: "Bypass Manual Control"
	  icon: "mdi:valve-open"
```

### Select (tryby)

```yaml
select:
  - platform: brink_ventilation
	brink_ventilation_id: brink_vent
	operation_mode:
	  name: "Operation Mode"
	  icon: "mdi:fan-auto"
	  options:
		- "Auto"
		- "Manual"
		- "Off"
```

---

## Zaawansowane funkcje

### 1. Dodatkowe czujniki (BME280 - temp/wilgotność/ciśnienie)

```yaml
i2c:
  sda: GPIO4  # D2
  scl: GPIO5  # D1
  scan: true

sensor:
  - platform: bme280
	address: 0x76
	temperature:
	  name: "Room Temperature"
	  oversampling: 16x
	humidity:
	  name: "Room Humidity"
	pressure:
	  name: "Room Pressure"
	update_interval: 60s
```

### 2. Dodatkowe czujniki (DS18B20 - temperatura)

```yaml
dallas:
  - pin: GPIO14  # D5

sensor:
  - platform: dallas
	address: 0x1234567890ABCDEF
	name: "Additional Temperature"
```

### 3. Dodanie przycisków fizycznych

```yaml
binary_sensor:
  - platform: gpio
	pin:
	  number: GPIO12  # D6
	  mode: INPUT_PULLUP
	  inverted: true
	name: "Boost Button"
	on_press:
	  then:
		- number.set:
			id: ventilation_level
			value: 100
	on_release:
	  then:
		- number.set:
			id: ventilation_level
			value: 50
```

### 4. LED status

```yaml
light:
  - platform: status_led
	name: "Status LED"
	pin:
	  number: GPIO2
	  inverted: true
```

### 5. Monitoring WiFi i uptime

```yaml
sensor:
  - platform: wifi_signal
	name: "WiFi Signal"
	update_interval: 60s

  - platform: uptime
	name: "Uptime"
	update_interval: 60s
```

---

## Optymalizacja wydajności

### Redukcja częstotliwości odczytu

```yaml
brink_ventilation:
  update_interval: 10s  # Domyślnie 5s
```

### Filtrowanie danych (zmniejszenie szumu)

```yaml
sensor:
  - platform: brink_ventilation
	supply_temp:
	  name: "Supply Temperature"
	  filters:
		# Średnia krocząca z 10 odczytów
		- sliding_window_moving_average:
			window_size: 10
			send_every: 10
		# Filtr delta (wysyła tylko przy zmianie >0.5°C)
		- delta: 0.5
		# Filtr throttle (max raz na 30s)
		- throttle: 30s
```

### Redukcja logowania

```yaml
logger:
  level: INFO
  logs:
	sensor: WARN
	brink_ventilation: DEBUG  # Tylko dla debugowania
```

---

## Home Assistant - zaawansowane integracje

### Automatyzacja 1: Boost na 30 minut

```yaml
automation:
  - alias: "Brink - Boost mode for 30 min"
	trigger:
	  - platform: state
		entity_id: input_boolean.brink_boost
		to: 'on'
	action:
	  - service: number.set_value
		target:
		  entity_id: number.brink_ventilation_level
		data:
		  value: 100
	  - delay: '00:30:00'
	  - service: number.set_value
		target:
		  entity_id: number.brink_ventilation_level
		data:
		  value: 50
	  - service: input_boolean.turn_off
		target:
		  entity_id: input_boolean.brink_boost
```

### Automatyzacja 2: Bypass na gorące dni

```yaml
automation:
  - alias: "Brink - Open bypass when hot"
	trigger:
	  - platform: numeric_state
		entity_id: sensor.brink_outdoor_temp
		above: 25
	  - platform: time
		at: "22:00:00"
	condition:
	  - condition: numeric_state
		entity_id: sensor.brink_outdoor_temp
		above: 20
	action:
	  - service: switch.turn_on
		target:
		  entity_id: switch.brink_bypass_control
```

### Automatyzacja 3: Przypomnienie o wymianie filtra

```yaml
automation:
  - alias: "Brink - Filter warning notification"
	trigger:
	  - platform: state
		entity_id: binary_sensor.brink_filter_warning
		to: 'on'
	action:
	  - service: notify.mobile_app
		data:
		  title: "Rekuperator Brink"
		  message: "Wymień filtry w rekuperatorze!"
		  data:
			priority: high
```

### Automatyzacja 4: Nocna redukcja (cichsza praca)

```yaml
automation:
  - alias: "Brink - Night mode"
	trigger:
	  - platform: time
		at: "23:00:00"
	action:
	  - service: number.set_value
		target:
		  entity_id: number.brink_ventilation_level
		data:
		  value: 30

  - alias: "Brink - Day mode"
	trigger:
	  - platform: time
		at: "07:00:00"
	action:
	  - service: number.set_value
		target:
		  entity_id: number.brink_ventilation_level
		data:
		  value: 50
```

---

## Konfiguracja OpenTherm - custom piny

Jeśli używasz niestandardowego podłączenia:

```yaml
brink_ventilation:
  tx_pin: GPIO14  # D5 (zamiast D4)
  rx_pin: GPIO12  # D6 (zamiast D3)
```

---

## ESP32 - różnice w konfiguracji

### Podstawowa konfiguracja ESP32

```yaml
esphome:
  name: brink-ventilation
  platform: ESP32
  board: esp32dev

brink_ventilation:
  tx_pin: GPIO17
  rx_pin: GPIO16
```

### Zalety ESP32 vs ESP8266

| Funkcja | ESP8266 | ESP32 |
|---------|---------|-------|
| Szybkość CPU | 80 MHz | 240 MHz |
| RAM | 80 KB | 520 KB |
| WiFi | 2.4 GHz | 2.4 GHz + BLE |
| Piny GPIO | 11 | 34 |
| OTA speed | Wolne (~2 min) | Szybkie (~30s) |

---

## Kalibracja czujników

Jeśli temperatury są niedokładne:

```yaml
sensor:
  - platform: brink_ventilation
	supply_temp:
	  name: "Supply Temperature"
	  filters:
		- calibrate_linear:
			- 0.0 -> 0.0
			- 20.0 -> 19.5  # Jeśli czujnik pokazuje 20°C a jest 19.5°C
			- 25.0 -> 24.7
```

---

## Backup i restore konfiguracji

### Backup

```bash
esphome compile my_brink.yaml
# Plik .bin będzie w: .esphome/build/brink-ventilation/.pioenvs/brink-ventilation/firmware.bin
```

### Restore (przez USB)

```bash
esphome run my_brink.yaml --device /dev/ttyUSB0
```

---

## Monitorowanie błędów

### Logi w Home Assistant

```yaml
logger:
  level: DEBUG
  logs:
	brink_ventilation: DEBUG
	opentherm: DEBUG
```

### Status connectivity

```yaml
binary_sensor:
  - platform: status
	name: "Brink Online Status"
```

---

## Następne kroki

- [OpenTherm Protocol](OPENTHERM_PROTOCOL.md) - Zrozumienie protokołu
- [Migration Guide](MIGRATION.md) - Migracja z Arduino/openHAB
- Przykłady zaawansowanych automatyzacji w Home Assistant

---

**Potrzebujesz pomocy?** Otwórz issue na GitHub!
