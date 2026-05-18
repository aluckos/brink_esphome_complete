# OpenTherm Protocol - Brink ESPHome Complete

## Czym jest OpenTherm?

**OpenTherm** to otwarty protokół komunikacji używany w systemach HVAC (ogrzewanie, wentylacja, klimatyzacja). W przypadku rekuperatora Brink, OpenTherm pozwala na:
- Odczyt temperatur
- Sterowanie poziomem wentylacji
- Monitoring statusu (filtry, bypass, błędy)
- Kontrolę trybu pracy

---

## Architektura komunikacji

```
[ESPHome Device] ←--→ [OpenTherm Shield] ←--→ [Brink Rekuperator]
	 Master                Bridge                   Slave
```

- **Master** = ESPHome (Twoje urządzenie) - wysyła zapytania
- **Slave** = Brink (rekuperator) - odpowiada na zapytania

---

## Protokół transmisji

### Parametry fizyczne
- **Napięcie**: 15-18V DC (generowane przez shield)
- **Typ**: Manchester encoding
- **Prędkość**: 1000 bps (1 bit/ms)
- **Polaryzacja**: nie ma znaczenia (sygnał symetryczny)

### Ramka danych (32 bity)

```
| Parity (1) | Msg Type (3) | Data ID (8) | Data Value (16) | Stop (4) |
```

**Przykład ramki**:
```
0b 1 001 00001000 0000000001100100
   | |   |        |
   | |   |        +-- Data Value = 100 (0x64)
   | |   +----------- Data ID = 8 (Write Data)
   | +--------------- Message Type = 1 (Read-Data)
   +----------------- Parity = 1 (odd parity)
```

---

## OpenTherm Data IDs (OT ID)

### Standardowe OT IDs używane przez Brink

| OT ID | Typ | Nazwa | Opis |
|-------|-----|-------|------|
| 71 | W | Ventilation Level | Poziom wentylacji (0-100%) |
| 80 | R | Supply Temp | Temperatura nawiewu |
| 81 | R | Exhaust Temp | Temperatura wywiewu |
| 82 | R | Outdoor Temp | Temperatura zewnętrzna |
| 83 | R | Extract Temp | Temperatura wyciągu |

**R** = Read (tylko odczyt)  
**W** = Write (zapis)  
**RW** = Read/Write (odczyt i zapis)

---

## TSP (Transparent Slave Parameters)

Brink wykorzystuje **TSP** do dostępu do specyficznych parametrów rekuperatora, które nie są w standardzie OpenTherm.

### TSP Protokół

```
OT ID 10 (TSP Index) + OT ID 11 (TSP Value)
```

### Najważniejsze TSP używane w ESPHome

| TSP | Typ | Nazwa | Opis | Format |
|-----|-----|-------|------|--------|
| 0/1 | R | U1 | Napięcie czujnika 1 | 2 bajty → mV |
| 2/3 | R | U2 | Napięcie czujnika 2 | 2 bajty → mV |
| 4/5 | R | U3 | Napięcie czujnika 3 | 2 bajty → mV |
| 6 | R | U4 | Temperatura czujnika 4 | 1 bajt → °C/2 |
| 7 | R | U5 | Temperatura czujnika 5 | 1 bajt → °C/2 |
| 9 | R | I1 | Prąd czujnika 1 | 1 bajt → (x-100) mA |
| 13 | R | Filter Status | Status filtra | 0=OK, 1=wymień |
| 52/53 | R | Airflow | Przepływ powietrza | 2 bajty → m³/h |
| 55 | R | Bypass Status | Status bypass | 0=closed, 1=open, 2=auto |
| 64/65 | R | CPID | PID nawiew | 2 bajty |
| 66/67 | R | CPOD | PID wywiew | 2 bajty |

---

## Przykład komunikacji

### 1. Odczyt temperatury nawiewu (OT ID 80)

**Request (Master → Slave)**:
```
0b 1 000 01010000 0000000000000000
   | |   |        |
   | |   |        +-- Data Value = 0 (Read request)
   | |   +----------- Data ID = 80 (Supply Temp)
   | +--------------- Message Type = 0 (Read-Data)
   +----------------- Parity = 1
```

**Response (Slave → Master)**:
```
0b 1 100 01010000 0000000011011000
   | |   |        |
   | |   |        +-- Data Value = 216 (21.6°C)
   | |   +----------- Data ID = 80
   | +--------------- Message Type = 4 (Read-Ack)
   +----------------- Parity = 1
```

Temperatura = `216 / 10 = 21.6°C`

### 2. Zapis poziomu wentylacji (OT ID 71)

**Request (Master → Slave)**:
```
0b 1 001 01000111 0000000001100100
   | |   |        |
   | |   |        +-- Data Value = 100 (100%)
   | |   +----------- Data ID = 71 (Ventilation Level)
   | +--------------- Message Type = 1 (Write-Data)
   +----------------- Parity = 1
```

**Response (Slave → Master)**:
```
0b 1 101 01000111 0000000001100100
   | |   |        |
   | |   |        +-- Data Value = 100 (confirmed)
   | |   +----------- Data ID = 71
   | +--------------- Message Type = 5 (Write-Ack)
   +----------------- Parity = 1
```

### 3. Odczyt TSP (np. filtr - TSP 13)

**Krok 1**: Ustaw TSP Index (OT ID 10)
```
Write OT ID 10 = 13
```

**Krok 2**: Odczytaj TSP Value (OT ID 11)
```
Read OT ID 11 = 0 (filtr OK) lub 1 (wymień filtr)
```

---

## Implementacja w ESPHome (C++)

### Przykład odczytu temperatury (sensor.py)

```python
async def to_code(config):
	parent = await cg.get_variable(config[CONF_BRINK_VENTILATION_ID])

	if CONF_SUPPLY_TEMP in config:
		sens = await sensor.new_sensor(config[CONF_SUPPLY_TEMP])
		cg.add(parent.set_supply_temp_sensor(sens))
```

### Przykład odczytu w C++ (brink_ot.h)

```cpp
void BrinkOT::read_supply_temp() {
  unsigned long response = sendRequest(0x50, 0x0000);  // OT ID 80
  if (response != 0) {
	float temp = (float)((response & 0xFFFF)) / 256.0;
	supply_temp_sensor_->publish_state(temp);
  }
}
```

### Przykład zapisu wentylacji

```cpp
void BrinkOT::set_ventilation(float value) {
  uint16_t level = (uint16_t)(value * 100);  // 0.0-1.0 → 0-100
  unsigned long response = sendRequest(0x47, level);  // OT ID 71
}
```

---

## Timing i częstotliwość zapytań

### Zalecane interwały

| Parametr | Interwał | Powód |
|----------|----------|-------|
| Temperatury | 5s | Zmieniają się powoli |
| Wentylacja | 1s | Szybka reakcja |
| Przepływ | 10s | Stabilny parametr |
| Filtry | 60s | Rzadko się zmienia |

### Limity protokołu

- **Max 10 zapytań/sekundę** (protokół OpenTherm)
- **Timeout response**: 1000ms
- **Retry**: 3 próby w przypadku braku odpowiedzi

---

## Debugowanie komunikacji

### Włącz debug logging w ESPHome

```yaml
logger:
  level: DEBUG
  logs:
	brink_ventilation: DEBUG
	opentherm: VERBOSE
```

### Typowe błędy

| Błąd | Przyczyna | Rozwiązanie |
|------|-----------|-------------|
| `No response` | Brak połączenia OT | Sprawdź przewody |
| `Invalid parity` | Zakłócenia | Sprawdź ekranowanie kabla |
| `Unknown data ID` | Niewspierany OT ID | Użyj TSP |
| `Write rejected` | Rekuperator w trybie serwisowym | Wyłącz tryb serwisowy |

### Przykład logów (prawidłowe działanie)

```
[D][brink_ventilation:123]: Sending OT request: ID=80, Value=0
[V][opentherm:045]: TX: 0x10500000
[V][opentherm:067]: RX: 0x940000D8
[D][brink_ventilation:145]: Supply temp: 21.6°C
```

---

## Rozszerzenia protokołu

### Custom TSP (tylko Brink)

Brink używa własnych TSP powyżej numeru 100:
- TSP 100-199: Parametry serwisowe
- TSP 200-255: Konfiguracja zaawansowana

⚠️ **Ostrożnie**: Zmiany w TSP >100 mogą uszkodzić ustawienia rekuperatora!

---

## Źródła i dokumentacja

- [OpenTherm Specification v2.2](http://www.opentherm.eu/request-details/?post_ids=1325)
- [Brink Service Manual](https://www.brinkclimatesystems.nl/)
- [ESPHome OpenTherm Component](https://esphome.io/components/climate/opentherm.html)

---

## Następne kroki

- [Installation Guide](INSTALLATION.md) - Montaż i uruchomienie
- [Configuration Guide](CONFIGURATION.md) - Zaawansowana konfiguracja
- [Migration Guide](MIGRATION.md) - Migracja z Arduino/openHAB

---

**Masz pytania o protokół?** Otwórz issue na GitHub!
