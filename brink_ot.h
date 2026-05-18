#pragma once

#include "esphome.h"
#include "OpenTherm.h"

namespace esphome {
namespace brink_ventilation {

// Helper: Dekodowanie kodu MsgOperation (C0-C12)
static const char* decode_msg_operation(uint8_t code) {
  switch (code) {
    case 0xC0: return "C0: None";
    case 0xC1: return "C1: Filter dirty";
    case 0xC2: return "C2: Max ventilation";
    case 0xC3: return "C3: Absence ventilation";
    case 0xC4: return "C4: Supply air fan error";
    case 0xC5: return "C5: Exhaust air fan error";
    case 0xC6: return "C6: Frost protection mode";
    case 0xC7: return "C7: Preheater error";
    case 0xC8: return "C8: Bypass stuck open";
    case 0xC9: return "C9: Bypass stuck closed";
    case 0xCA: return "C10: T1 sensor error";
    case 0xCB: return "C11: T2 sensor error";
    case 0xCC: return "C12: T3 sensor error";
    default:   return "Unknown";
  }
}

class BrinkOpenTherm;

// Number: główny suwak wentylacji (OT ID 71)
class BrinkNumber : public number::Number {
 public:
  BrinkOpenTherm *parent_{nullptr};
  void set_parent(BrinkOpenTherm *parent) { parent_ = parent; }
  void control(float value) override;
};

// Number: U1 ventilation preset (TSP 38/39)
class BrinkU1Number : public number::Number, public Component {
 public:
  BrinkOpenTherm *parent_{nullptr};
  void set_parent(BrinkOpenTherm *parent) { parent_ = parent; }
  void control(float value) override;
};

// Number: U2 ventilation preset (TSP 40/41)
class BrinkU2Number : public number::Number, public Component {
 public:
  BrinkOpenTherm *parent_{nullptr};
  void set_parent(BrinkOpenTherm *parent) { parent_ = parent; }
  void control(float value) override;
};

// Number: U3 ventilation preset (TSP 42/43)
class BrinkU3Number : public number::Number, public Component {
 public:
  BrinkOpenTherm *parent_{nullptr};
  void set_parent(BrinkOpenTherm *parent) { parent_ = parent; }
  void control(float value) override;
};

// Select: Bypass control (Auto/Open/Closed)
class BrinkBypassSelect : public select::Select, public Component {
 public:
  BrinkOpenTherm *parent_{nullptr};
  void set_parent(BrinkOpenTherm *parent) { parent_ = parent; }
  void control(const std::string &value) override;
};

// Select: Ventilation preset (U1/U2/U3)
class BrinkPresetSelect : public select::Select, public Component {
 public:
  BrinkOpenTherm *parent_{nullptr};
  void set_parent(BrinkOpenTherm *parent) { parent_ = parent; }
  void control(const std::string &value) override;
};

class BrinkOpenTherm : public PollingComponent {
 public:
  OpenTherm *ot{nullptr};
  int pin_in{0}, pin_out{0};

  // sterowanie (ID 71)
  float target_ventilation_{25.0f};

  // pomocnicze do 2-bajtowych TSP
  uint8_t tsp_low_byte_{0};

  // --- encje ESPHome (wsk. ustawiane z pythonowych platform) ---
  // Temperatury (OT IDs 80-83)
  sensor::Sensor *t_supply_in_sensor{nullptr};    // ID 80
  sensor::Sensor *t_supply_out_sensor{nullptr};   // ID 81
  sensor::Sensor *t_exhaust_in_sensor{nullptr};   // ID 82
  sensor::Sensor *t_exhaust_out_sensor{nullptr};  // ID 83

  // przepływ (u Ciebie: TSP 52/53 jako 2 bajty)
  sensor::Sensor *current_flow_sensor{nullptr};

  // Dodatkowe TSP 2-bajtowe
  sensor::Sensor *cpid_sensor{nullptr};  // 64/65
  sensor::Sensor *cpod_sensor{nullptr};  // 66/67

  sensor::Sensor *u1_sensor{nullptr};  // 0/1
  sensor::Sensor *u2_sensor{nullptr};  // 2/3
  sensor::Sensor *u3_sensor{nullptr};  // 4/5

  // Dodatkowe TSP 1-bajtowe
  sensor::Sensor *u4_sensor{nullptr};  // 6 (wartość /2 => °C)
  sensor::Sensor *u5_sensor{nullptr};  // 7 (wartość /2 => °C)
  sensor::Sensor *i1_sensor{nullptr};  // 9 (wartość -100)

  // Bypass status jako tekst i jako liczba
  sensor::Sensor *bypass_status_sensor{nullptr};        // surowy 0/1/2
  text_sensor::TextSensor *bypass_status_text{nullptr}; // opis

  // Filtr + status komunikacji
  binary_sensor::BinarySensor *filter_status_binary{nullptr};
  text_sensor::TextSensor *status_text_sensor{nullptr};

  // OT ID 70 (VentStatus) - fault indication i ventilation mode
  binary_sensor::BinarySensor *fault_indication_binary{nullptr};    // bit 0
  binary_sensor::BinarySensor *ventilation_mode_binary{nullptr};    // bit 1

  // RPM wentylatorów (OT IDs 85-86)
  sensor::Sensor *rpm_exhaust_sensor{nullptr};  // ID 85
  sensor::Sensor *rpm_supply_sensor{nullptr};   // ID 86

  // Nowe TSP 1-bajtowe
  sensor::Sensor *msg_operation_sensor{nullptr};      // TSP 53 (C0-C12)
  text_sensor::TextSensor *msg_operation_text{nullptr}; // TSP 53 decoded
  sensor::Sensor *temp_atmo_sensor{nullptr};          // TSP 55 (-100)
  sensor::Sensor *temp_indoors_sensor{nullptr};       // TSP 56 (-100)
  binary_sensor::BinarySensor *init_status_binary{nullptr}; // TSP 57 (0/1)
  sensor::Sensor *voltage_param1_sensor{nullptr};     // TSP 58 (0-10V)
  sensor::Sensor *voltage_param2_sensor{nullptr};     // TSP 59 (0-10V)
  sensor::Sensor *frost_status_sensor{nullptr};       // TSP 68 (0-5)
  sensor::Sensor *temp2_atmo_sensor{nullptr};         // TSP 69 (-100)
  sensor::Sensor *temp2_indoors_sensor{nullptr};      // TSP 70 (-100)
  sensor::Sensor *temp_postheater_sensor{nullptr};    // TSP 71

  // Nowe TSP 2-bajtowe
  sensor::Sensor *max_vol_sensor{nullptr};            // TSP 48/49
  sensor::Sensor *min_vol_sensor{nullptr};            // TSP 50/51
  sensor::Sensor *current_input_vol_sensor{nullptr};  // TSP 60/61
  sensor::Sensor *current_output_vol_sensor{nullptr}; // TSP 62/63

  // Numbers dla U1/U2/U3 + trigger select
  number::Number *u1_number{nullptr};
  number::Number *u2_number{nullptr};
  number::Number *u3_number{nullptr};
  select::Select *ventilation_preset_select{nullptr}; // U1/U2/U3 select
  select::Select *bypass_select{nullptr}; // Bypass control

  void set_pins(int in, int out) { pin_in = in; pin_out = out; }

  // Metody do sterowania bypass
  void set_bypass_mode(const std::string &mode);
  void apply_preset(const std::string &preset);
  void write_u_preset(uint8_t preset_num, uint16_t value);

  // --- settery sensorów (muszą odpowiadać nazwom z sensor.py) ---
  void set_t_supply_in_sensor(sensor::Sensor *s) { t_supply_in_sensor = s; }
  void set_t_supply_out_sensor(sensor::Sensor *s) { t_supply_out_sensor = s; }
  void set_t_exhaust_in_sensor(sensor::Sensor *s) { t_exhaust_in_sensor = s; }
  void set_t_exhaust_out_sensor(sensor::Sensor *s) { t_exhaust_out_sensor = s; }

  void set_current_flow_sensor(sensor::Sensor *s) { current_flow_sensor = s; }

  void set_cpid_sensor(sensor::Sensor *s) { cpid_sensor = s; }
  void set_cpod_sensor(sensor::Sensor *s) { cpod_sensor = s; }

  void set_u1_sensor(sensor::Sensor *s) { u1_sensor = s; }
  void set_u2_sensor(sensor::Sensor *s) { u2_sensor = s; }
  void set_u3_sensor(sensor::Sensor *s) { u3_sensor = s; }
  void set_u4_sensor(sensor::Sensor *s) { u4_sensor = s; }
  void set_u5_sensor(sensor::Sensor *s) { u5_sensor = s; }
  void set_i1_sensor(sensor::Sensor *s) { i1_sensor = s; }

  void set_bypass_status_sensor(sensor::Sensor *s) { bypass_status_sensor = s; }
  void set_bypass_status_text(text_sensor::TextSensor *s) { bypass_status_text = s; }

  void set_filter_status_binary(binary_sensor::BinarySensor *s) { filter_status_binary = s; }
  void set_status_text_sensor(text_sensor::TextSensor *s) { status_text_sensor = s; }

  void set_fault_indication_binary(binary_sensor::BinarySensor *s) { fault_indication_binary = s; }
  void set_ventilation_mode_binary(binary_sensor::BinarySensor *s) { ventilation_mode_binary = s; }

  void set_rpm_exhaust_sensor(sensor::Sensor *s) { rpm_exhaust_sensor = s; }
  void set_rpm_supply_sensor(sensor::Sensor *s) { rpm_supply_sensor = s; }

  void set_msg_operation_sensor(sensor::Sensor *s) { msg_operation_sensor = s; }
  void set_temp_atmo_sensor(sensor::Sensor *s) { temp_atmo_sensor = s; }
  void set_temp_indoors_sensor(sensor::Sensor *s) { temp_indoors_sensor = s; }
  void set_init_status_binary(binary_sensor::BinarySensor *s) { init_status_binary = s; }
  void set_voltage_param1_sensor(sensor::Sensor *s) { voltage_param1_sensor = s; }
  void set_voltage_param2_sensor(sensor::Sensor *s) { voltage_param2_sensor = s; }
  void set_frost_status_sensor(sensor::Sensor *s) { frost_status_sensor = s; }
  void set_temp2_atmo_sensor(sensor::Sensor *s) { temp2_atmo_sensor = s; }
  void set_temp2_indoors_sensor(sensor::Sensor *s) { temp2_indoors_sensor = s; }
  void set_temp_postheater_sensor(sensor::Sensor *s) { temp_postheater_sensor = s; }

  void set_max_vol_sensor(sensor::Sensor *s) { max_vol_sensor = s; }
  void set_min_vol_sensor(sensor::Sensor *s) { min_vol_sensor = s; }
  void set_current_input_vol_sensor(sensor::Sensor *s) { current_input_vol_sensor = s; }
  void set_current_output_vol_sensor(sensor::Sensor *s) { current_output_vol_sensor = s; }

  void set_msg_operation_text(text_sensor::TextSensor *s) { msg_operation_text = s; }

  void set_u1_number(number::Number *n) { u1_number = n; }
  void set_u2_number(number::Number *n) { u2_number = n; }
  void set_u3_number(number::Number *n) { u3_number = n; }
  void set_ventilation_preset_select(select::Select *s) { ventilation_preset_select = s; }
  void set_bypass_select(select::Select *s) { bypass_select = s; }

  void set_ventilation_number(BrinkNumber *n) { n->set_parent(this); }

  void setup() override;
  void update() override;

 protected:
  // Prosta „kolejka” polling: odpytywanie po jednym elemencie na update()
  // żeby nie zajechać OT i zachować cykliczną komunikację.
  uint8_t step_{0};

  inline void publish_bypass_text_(uint8_t v) {
    if (!bypass_status_text) return;
    switch (v) {
      case 0:
        bypass_status_text->publish_state("closed");
        break;
      case 1:
        bypass_status_text->publish_state("automatic");
        break;
      case 2:
        bypass_status_text->publish_state("inlet_minimum");
        break;
      default:
        bypass_status_text->publish_state("unknown");
        break;
    }
  }
};

// --- IMPLEMENTACJA ---

static BrinkOpenTherm *global_brink_ot = nullptr;

static void IRAM_ATTR handleInterrupt() {
  if (global_brink_ot != nullptr && global_brink_ot->ot != nullptr) {
    global_brink_ot->ot->handleInterrupt();
  }
}

inline void BrinkOpenTherm::setup() {
  global_brink_ot = this;
  ot = new OpenTherm(pin_in, pin_out);
  ot->begin(handleInterrupt);
}

inline void BrinkNumber::control(float value) {
  this->publish_state(value);
  if (this->parent_ != nullptr) {
    this->parent_->target_ventilation_ = value;
  }
}

inline void BrinkOpenTherm::update() {
  if (ot == nullptr) return;

  // Nie używamy OT ID 0 (status boiler) - to jest dla kotłów, nie dla central wentylacyjnych!
  // Ventilation status to OT ID 70 (VentStatus) i jest odczytywany w polling loop.

  if (status_text_sensor != nullptr) status_text_sensor->publish_state("connected");

  unsigned long response = 0;

  // krokami odczytujemy kolejne parametry
  switch (step_) {
    case 0:  // WRITE: wentylacja ID 71
      ot->sendRequest(ot->buildRequest(OpenThermMessageType::WRITE_DATA, (OpenThermMessageID) 71,
                                       (unsigned int) target_ventilation_));
      step_++;
      break;

    case 1:  // T1 ID 80
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 80, 0));
      ESP_LOGD("brink", "OT ID 80 response: 0x%08lX", response);
      if (response && t_supply_in_sensor) {
        float temp = ot->getFloat(response);
        ESP_LOGD("brink", "T1 (supply_in): %.2f°C", temp);
        t_supply_in_sensor->publish_state(temp);
      } else {
        ESP_LOGW("brink", "No response for OT ID 80 (T1)");
      }
      step_++;
      break;

    case 2:  // T2 ID 81
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 81, 0));
      ESP_LOGD("brink", "OT ID 81 response: 0x%08lX", response);
      if (response && t_supply_out_sensor) {
        float temp = ot->getFloat(response);
        ESP_LOGD("brink", "T2 (supply_out): %.2f°C", temp);
        t_supply_out_sensor->publish_state(temp);
      } else {
        ESP_LOGW("brink", "No response for OT ID 81 (T2)");
      }
      step_++;
      break;

    case 3:  // T3 ID 82
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 82, 0));
      ESP_LOGD("brink", "OT ID 82 response: 0x%08lX", response);
      if (response && t_exhaust_in_sensor) {
        float temp = ot->getFloat(response);
        ESP_LOGD("brink", "T3 (exhaust_in): %.2f°C", temp);
        t_exhaust_in_sensor->publish_state(temp);
      } else {
        ESP_LOGW("brink", "No response for OT ID 82 (T3)");
      }
      step_++;
      break;

    case 4:  // T4 ID 83
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 83, 0));
      ESP_LOGD("brink", "OT ID 83 response: 0x%08lX", response);
      if (response && t_exhaust_out_sensor) {
        float temp = ot->getFloat(response);
        ESP_LOGD("brink", "T4 (exhaust_out): %.2f°C", temp);
        t_exhaust_out_sensor->publish_state(temp);
      } else {
        ESP_LOGW("brink", "No response for OT ID 83 (T4)");
      }
      step_++;
      break;

    // --- TSP 2-bajtowe (two-step: low, high) ---
    case 5:  // FLOW low: TSP 52 (LB)
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 89, 52 << 8));
      ESP_LOGD("brink", "TSP 52 response: 0x%08lX", response);
      if (response) tsp_low_byte_ = (uint8_t) (response & 0xFF);
      step_++;
      break;

    case 6:  // FLOW high: TSP 53 (HB) + MsgOperation
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 89, 53 << 8));
      ESP_LOGD("brink", "TSP 53 response: 0x%08lX", response);
      if (response) {
        uint8_t msg_op_code = (uint8_t) (response & 0xFF);
        // Flow
        if (current_flow_sensor) {
          uint16_t flow = ((uint16_t) msg_op_code << 8) | tsp_low_byte_;
          ESP_LOGD("brink", "Current flow: %d m³/h", flow);
          current_flow_sensor->publish_state(flow);
        }
        // MsgOperation sensor (raw code)
        if (msg_operation_sensor) {
          msg_operation_sensor->publish_state(msg_op_code);
        }
        // MsgOperation text (decoded)
        if (msg_operation_text) {
          const char* desc = decode_msg_operation(msg_op_code);
          ESP_LOGD("brink", "MsgOperation: 0x%02X = %s", msg_op_code, desc);
          msg_operation_text->publish_state(desc);
        }
      }
      step_++;
      break;

    case 7:  // Filter: TSP 13
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 89, 13 << 8));
      ESP_LOGD("brink", "TSP 13 (filter) response: 0x%08lX", response);
      if (response && filter_status_binary) {
        bool dirty = (response & 0xFF) == 1;
        ESP_LOGD("brink", "Filter dirty: %s", dirty ? "YES" : "NO");
        filter_status_binary->publish_state(dirty);
      }
      step_++;
      break;

    // --- bypass status: TSP 54 (not 55!) ---
    case 8:
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 89, 54 << 8));
      ESP_LOGD("brink", "TSP 54 (bypass) response: 0x%08lX", response);
      if (response) {
        uint8_t v = (uint8_t) (response & 0xFF);
        ESP_LOGD("brink", "Bypass status raw: %d (0=closed, 1=auto, 2=inlet_min)", v);
        if (bypass_status_sensor) bypass_status_sensor->publish_state(v);
        publish_bypass_text_(v);
      }
      step_++;
      break;

    // --- CPID: 64/65 (2 bytes) ---
    case 9:
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 89, 64 << 8));
      if (response) tsp_low_byte_ = (uint8_t) (response & 0xFF);
      step_++;
      break;

    case 10:
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 89, 65 << 8));
      if (response && cpid_sensor) cpid_sensor->publish_state(((uint16_t) (response & 0xFF) << 8) | tsp_low_byte_);
      step_++;
      break;

    // --- CPOD: 66/67 (2 bytes) ---
    case 11:
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 89, 66 << 8));
      if (response) tsp_low_byte_ = (uint8_t) (response & 0xFF);
      step_++;
      break;

    case 12:
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 89, 67 << 8));
      if (response && cpod_sensor) cpod_sensor->publish_state(((uint16_t) (response & 0xFF) << 8) | tsp_low_byte_);
      step_++;
      break;

    // --- U1: 0/1 (2 bytes) ---
    case 13:
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 89, 0 << 8));
      if (response) tsp_low_byte_ = (uint8_t) (response & 0xFF);
      step_++;
      break;

    case 14:
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 89, 1 << 8));
      if (response && u1_sensor) u1_sensor->publish_state(((uint16_t) (response & 0xFF) << 8) | tsp_low_byte_);
      step_++;
      break;

    // --- U2: 2/3 ---
    case 15:
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 89, 2 << 8));
      if (response) tsp_low_byte_ = (uint8_t) (response & 0xFF);
      step_++;
      break;

    case 16:
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 89, 3 << 8));
      if (response && u2_sensor) u2_sensor->publish_state(((uint16_t) (response & 0xFF) << 8) | tsp_low_byte_);
      step_++;
      break;

    // --- U3: 4/5 ---
    case 17:
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 89, 4 << 8));
      if (response) tsp_low_byte_ = (uint8_t) (response & 0xFF);
      step_++;
      break;

    case 18:
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 89, 5 << 8));
      if (response && u3_sensor) u3_sensor->publish_state(((uint16_t) (response & 0xFF) << 8) | tsp_low_byte_);
      step_++;
      break;

    // --- U4 (temp threshold atmo): index 6, value/2 => °C ---
    case 19:
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 89, 6 << 8));
      if (response && u4_sensor) u4_sensor->publish_state(((uint8_t) (response & 0xFF)) / 2.0f);
      step_++;
      break;

    // --- U5 (temp threshold indoor): index 7, value/2 => °C ---
    case 20:
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 89, 7 << 8));
      if (response && u5_sensor) u5_sensor->publish_state(((uint8_t) (response & 0xFF)) / 2.0f);
      step_++;
      break;

    // --- I1 (imbalance): index 9, value-100 ---
    case 21:
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 89, 9 << 8));
      if (response && i1_sensor) i1_sensor->publish_state(((int) ((uint8_t) (response & 0xFF))) - 100);
      step_++;
      break;

    // --- OT ID 70 (VentStatus): fault indication i ventilation mode ---
    case 22:
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 70, 0));
      ESP_LOGD("brink", "OT ID 70 (VentStatus) response: 0x%08lX", response);
      if (response) {
        uint16_t status = (response >> 8) & 0xFFFF;  // Low word zawiera status flags
        // Bit 0: Fault indication (0=OK, 1=Fault)
        if (fault_indication_binary) {
          bool fault = (status & 0x01) != 0;
          ESP_LOGD("brink", "Fault indication: %s", fault ? "YES" : "NO");
          fault_indication_binary->publish_state(fault);
        }
        // Bit 1: Ventilation mode (0=supply only, 1=supply+exhaust)
        if (ventilation_mode_binary) {
          bool mode = (status & 0x02) != 0;
          ESP_LOGD("brink", "Ventilation mode: %s", mode ? "supply+exhaust" : "supply only");
          ventilation_mode_binary->publish_state(mode);
        }
      }
      step_ = 0;  // wracamy do początku
      break;

    // --- RPM wentylatorów ---
    case 23:  // Exhaust RPM (OT ID 85)
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 85, 0));
      ESP_LOGD("brink", "OT ID 85 (RPM exhaust) response: 0x%08lX", response);
      if (response && rpm_exhaust_sensor) {
        uint16_t rpm = (response >> 8) & 0xFFFF;
        ESP_LOGD("brink", "Exhaust fan RPM: %d", rpm);
        rpm_exhaust_sensor->publish_state(rpm);
      }
      step_++;
      break;

    case 24:  // Supply RPM (OT ID 86)
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 86, 0));
      ESP_LOGD("brink", "OT ID 86 (RPM supply) response: 0x%08lX", response);
      if (response && rpm_supply_sensor) {
        uint16_t rpm = (response >> 8) & 0xFFFF;
        ESP_LOGD("brink", "Supply fan RPM: %d", rpm);
        rpm_supply_sensor->publish_state(rpm);
      }
      step_++;
      break;

    // --- TSP 53: MsgOperation ---
    case 25:
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 89, 53 << 8));
      ESP_LOGD("brink", "TSP 53 (MsgOperation) response: 0x%08lX", response);
      if (response && msg_operation_sensor) {
        uint8_t msg = (uint8_t) (response & 0xFF);
        ESP_LOGD("brink", "Operation message: C%d", msg);
        msg_operation_sensor->publish_state(msg);
      }
      step_++;
      break;

    // --- TSP 55: TempAtmo ---
    case 26:
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 89, 55 << 8));
      ESP_LOGD("brink", "TSP 55 (TempAtmo) response: 0x%08lX", response);
      if (response && temp_atmo_sensor) {
        int temp = ((int) ((uint8_t) (response & 0xFF))) - 100;
        ESP_LOGD("brink", "Temp atmosphere: %d°C", temp);
        temp_atmo_sensor->publish_state(temp);
      }
      step_++;
      break;

    // --- TSP 56: TempIndoors ---
    case 27:
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 89, 56 << 8));
      ESP_LOGD("brink", "TSP 56 (TempIndoors) response: 0x%08lX", response);
      if (response && temp_indoors_sensor) {
        int temp = ((int) ((uint8_t) (response & 0xFF))) - 100;
        ESP_LOGD("brink", "Temp indoors: %d°C", temp);
        temp_indoors_sensor->publish_state(temp);
      }
      step_++;
      break;

    // --- TSP 60/61: CurrentInputVol ---
    case 28:
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 89, 60 << 8));
      if (response) tsp_low_byte_ = (uint8_t) (response & 0xFF);
      step_++;
      break;

    case 29:
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 89, 61 << 8));
      ESP_LOGD("brink", "TSP 60/61 (CurrentInputVol) response: 0x%08lX", response);
      if (response && current_input_vol_sensor) {
        uint16_t vol = ((uint16_t) (response & 0xFF) << 8) | tsp_low_byte_;
        ESP_LOGD("brink", "Current input volume: %d m³/h", vol);
        current_input_vol_sensor->publish_state(vol);
      }
      step_++;
      break;

    // --- TSP 62/63: CurrentOutputVol ---
    case 30:
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 89, 62 << 8));
      if (response) tsp_low_byte_ = (uint8_t) (response & 0xFF);
      step_++;
      break;

    case 31:
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 89, 63 << 8));
      ESP_LOGD("brink", "TSP 62/63 (CurrentOutputVol) response: 0x%08lX", response);
      if (response && current_output_vol_sensor) {
        uint16_t vol = ((uint16_t) (response & 0xFF) << 8) | tsp_low_byte_;
        ESP_LOGD("brink", "Current output volume: %d m³/h", vol);
        current_output_vol_sensor->publish_state(vol);
      }
      step_++;
      break;

    // --- TSP 48/49: MaxVol ---
    case 32:
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 89, 48 << 8));
      if (response) tsp_low_byte_ = (uint8_t) (response & 0xFF);
      step_++;
      break;

    case 33:
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 89, 49 << 8));
      ESP_LOGD("brink", "TSP 48/49 (MaxVol) response: 0x%08lX", response);
      if (response && max_vol_sensor) {
        uint16_t vol = ((uint16_t) (response & 0xFF) << 8) | tsp_low_byte_;
        ESP_LOGD("brink", "Max volume: %d m³/h", vol);
        max_vol_sensor->publish_state(vol);
      }
      step_++;
      break;

    // --- TSP 50/51: MinVol ---
    case 34:
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 89, 50 << 8));
      if (response) tsp_low_byte_ = (uint8_t) (response & 0xFF);
      step_++;
      break;

    case 35:
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 89, 51 << 8));
      ESP_LOGD("brink", "TSP 50/51 (MinVol) response: 0x%08lX", response);
      if (response && min_vol_sensor) {
        uint16_t vol = ((uint16_t) (response & 0xFF) << 8) | tsp_low_byte_;
        ESP_LOGD("brink", "Min volume: %d m³/h", vol);
        min_vol_sensor->publish_state(vol);
      }
      step_++;
      break;

    // --- TSP 57: InitStatus ---
    case 36:
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 89, 57 << 8));
      ESP_LOGD("brink", "TSP 57 (InitStatus) response: 0x%08lX", response);
      if (response && init_status_binary) {
        bool initiated = ((response & 0xFF) == 1);
        ESP_LOGD("brink", "Appliance initiated: %s", initiated ? "YES" : "NO");
        init_status_binary->publish_state(initiated);
      }
      step_++;
      break;

    // --- TSP 58: VoltageParam1 ---
    case 37:
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 89, 58 << 8));
      ESP_LOGD("brink", "TSP 58 (VoltageParam1) response: 0x%08lX", response);
      if (response && voltage_param1_sensor) {
        float voltage = ((uint8_t) (response & 0xFF)) / 10.0f;  // 0-100 -> 0-10V
        ESP_LOGD("brink", "Voltage param1 (moisture): %.1fV", voltage);
        voltage_param1_sensor->publish_state(voltage);
      }
      step_++;
      break;

    // --- TSP 59: VoltageParam2 ---
    case 38:
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 89, 59 << 8));
      ESP_LOGD("brink", "TSP 59 (VoltageParam2) response: 0x%08lX", response);
      if (response && voltage_param2_sensor) {
        float voltage = ((uint8_t) (response & 0xFF)) / 10.0f;  // 0-100 -> 0-10V
        ESP_LOGD("brink", "Voltage param2 (CO2): %.1fV", voltage);
        voltage_param2_sensor->publish_state(voltage);
      }
      step_++;
      break;

    // --- TSP 68: FrostStatus ---
    case 39:
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 89, 68 << 8));
      ESP_LOGD("brink", "TSP 68 (FrostStatus) response: 0x%08lX", response);
      if (response && frost_status_sensor) {
        uint8_t frost = (uint8_t) (response & 0xFF);
        ESP_LOGD("brink", "Frost status: %d (0=none, 1-4=imbalance, 5=fan off)", frost);
        frost_status_sensor->publish_state(frost);
      }
      step_++;
      break;

    // --- TSP 69: Temp2Atmo ---
    case 40:
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 89, 69 << 8));
      ESP_LOGD("brink", "TSP 69 (Temp2Atmo) response: 0x%08lX", response);
      if (response && temp2_atmo_sensor) {
        int temp = ((int) ((uint8_t) (response & 0xFF))) - 100;
        ESP_LOGD("brink", "Temp2 atmosphere: %d°C", temp);
        temp2_atmo_sensor->publish_state(temp);
      }
      step_++;
      break;

    // --- TSP 70: Temp2Indoors ---
    case 41:
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 89, 70 << 8));
      ESP_LOGD("brink", "TSP 70 (Temp2Indoors) response: 0x%08lX", response);
      if (response && temp2_indoors_sensor) {
        int temp = ((int) ((uint8_t) (response & 0xFF))) - 100;
        ESP_LOGD("brink", "Temp2 indoors: %d°C", temp);
        temp2_indoors_sensor->publish_state(temp);
      }
      step_++;
      break;

    // --- TSP 71: TempPostHeater ---
    case 42:
      response = ot->sendRequest(ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID) 89, 71 << 8));
      ESP_LOGD("brink", "TSP 71 (TempPostHeater) response: 0x%08lX", response);
      if (response && temp_postheater_sensor) {
        int temp = (int) ((uint8_t) (response & 0xFF));
        ESP_LOGD("brink", "Temp postheater: %d°C", temp);
        temp_postheater_sensor->publish_state(temp);
      }
      step_ = 0;  // wracamy do początku po ostatnim case
      break;

    default:
      step_ = 0;
      break;
  }
}

// ===== Implementacje sterowania =====

void BrinkBypassSelect::control(const std::string &value) {
  if (parent_) {
    parent_->set_bypass_mode(value);
  }
  publish_state(value);
}

void BrinkPresetSelect::control(const std::string &value) {
  if (parent_) {
    parent_->apply_preset(value);
  }
  publish_state(value);
}

void BrinkOpenTherm::set_bypass_mode(const std::string &mode) {
  ESP_LOGI("brink", "Setting bypass mode: %s", mode.c_str());

  uint8_t tsp_value = 0;
  if (mode == "Auto") tsp_value = 0;
  else if (mode == "Open") tsp_value = 1;
  else if (mode == "Closed") tsp_value = 2;

  // Zapis do TSP 76 (Bypass Control)
  unsigned long request = ot->buildRequest(OpenThermMessageType::WRITE_DATA, (OpenThermMessageID) 89, (76 << 8) | tsp_value);
  unsigned long response = ot->sendRequest(request);

  if (response) {
    ESP_LOGI("brink", "Bypass mode set successfully: %s (TSP 76 = %d)", mode.c_str(), tsp_value);
  } else {
    ESP_LOGW("brink", "Failed to set bypass mode: %s", mode.c_str());
  }
}

void BrinkOpenTherm::apply_preset(const std::string &preset) {
  ESP_LOGI("brink", "Applying ventilation preset: %s", preset.c_str());

  uint8_t preset_number = 0;
  if (preset == "U1") preset_number = 1;
  else if (preset == "U2") preset_number = 2;
  else if (preset == "U3") preset_number = 3;

  // Zapis do TSP 72 (Ventilation Preset)
  unsigned long request = ot->buildRequest(OpenThermMessageType::WRITE_DATA, (OpenThermMessageID) 89, (72 << 8) | preset_number);
  unsigned long response = ot->sendRequest(request);

  if (response) {
    ESP_LOGI("brink", "Preset %s applied successfully (TSP 72 = %d)", preset.c_str(), preset_number);
  } else {
    ESP_LOGW("brink", "Failed to apply preset: %s", preset.c_str());
  }
}

void BrinkOpenTherm::write_u_preset(uint8_t preset_num, uint16_t value) {
  ESP_LOGI("brink", "Writing U%d preset value: %d", preset_num, value);

  uint8_t tsp_low = 38 + (preset_num - 1) * 2;   // U1: TSP 38/39, U2: TSP 40/41, U3: TSP 42/43
  uint8_t tsp_high = tsp_low + 1;

  uint8_t low_byte = value & 0xFF;
  uint8_t high_byte = (value >> 8) & 0xFF;

  // Zapisz low byte
  unsigned long req_low = ot->buildRequest(OpenThermMessageType::WRITE_DATA, (OpenThermMessageID) 89, (tsp_low << 8) | low_byte);
  unsigned long resp_low = ot->sendRequest(req_low);

  if (!resp_low) {
    ESP_LOGW("brink", "Failed to write U%d low byte (TSP %d)", preset_num, tsp_low);
    return;
  }

  // Zapisz high byte
  unsigned long req_high = ot->buildRequest(OpenThermMessageType::WRITE_DATA, (OpenThermMessageID) 89, (tsp_high << 8) | high_byte);
  unsigned long resp_high = ot->sendRequest(req_high);

  if (resp_high) {
    ESP_LOGI("brink", "U%d preset written successfully: %d (TSP %d/%d)", preset_num, value, tsp_low, tsp_high);
  } else {
    ESP_LOGW("brink", "Failed to write U%d high byte (TSP %d)", preset_num, tsp_high);
  }
}

void BrinkU1Number::control(float value) {
  if (parent_) {
    parent_->write_u_preset(1, (uint16_t) value);
  }
  publish_state(value);
}

void BrinkU2Number::control(float value) {
  if (parent_) {
    parent_->write_u_preset(2, (uint16_t) value);
  }
  publish_state(value);
}

void BrinkU3Number::control(float value) {
  if (parent_) {
    parent_->write_u_preset(3, (uint16_t) value);
  }
  publish_state(value);
}

}  // namespace brink_ventilation
}  // namespace esphome


