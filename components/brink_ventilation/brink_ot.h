#pragma once

#include "esphome.h"
#include "OpenTherm.h"

namespace esphome {
namespace brink_ventilation {

// Forward declarations
class BrinkOpenTherm;

// Helper: Dekodowanie kodu MsgOperation (C0-C12)
static const char* decode_msg_operation(uint8_t code) {
  switch (code) {
	case 0x00: return "OK: No errors";
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

// Async State Machine
enum class AsyncState {
  IDLE,           // Ready to send next request
  WAITING,        // Waiting for response
  PROCESSING      // Processing received response
};

class BrinkOpenTherm : public PollingComponent {
 public:
  OpenTherm *ot{nullptr};
  int pin_in{0}, pin_out{0};

  // sterowanie (ID 71)
  float target_ventilation_{25.0f};

  // pomocnicze do 2-bajtowych TSP
  uint8_t tsp_low_byte_{0};

  // status tracking
  std::string last_status_{""};

  // Async state machine
  AsyncState async_state_{AsyncState::IDLE};
  uint8_t step_{0};
  unsigned long last_response_{0};
  OpenThermResponseStatus last_response_status_{OpenThermResponseStatus::NONE};

  // --- encje ESPHome ---
  sensor::Sensor *t_supply_in_sensor{nullptr};
  sensor::Sensor *t_supply_out_sensor{nullptr};
  sensor::Sensor *t_exhaust_in_sensor{nullptr};
  sensor::Sensor *t_exhaust_out_sensor{nullptr};

  sensor::Sensor *current_flow_sensor{nullptr};
  sensor::Sensor *bypass_status_sensor{nullptr};
  text_sensor::TextSensor *bypass_status_text{nullptr};

  binary_sensor::BinarySensor *filter_status_binary{nullptr};
  text_sensor::TextSensor *status_text_sensor{nullptr};

  binary_sensor::BinarySensor *fault_indication_binary{nullptr};
  binary_sensor::BinarySensor *ventilation_mode_binary{nullptr};

  sensor::Sensor *cpid_sensor{nullptr};
  sensor::Sensor *cpod_sensor{nullptr};

  sensor::Sensor *u1_sensor{nullptr};
  sensor::Sensor *u2_sensor{nullptr};
  sensor::Sensor *u3_sensor{nullptr};
  sensor::Sensor *u4_sensor{nullptr};
  sensor::Sensor *u5_sensor{nullptr};
  sensor::Sensor *i1_sensor{nullptr};

  sensor::Sensor *rpm_exhaust_sensor{nullptr};
  sensor::Sensor *rpm_supply_sensor{nullptr};

  sensor::Sensor *msg_operation_sensor{nullptr};
  text_sensor::TextSensor *msg_operation_text{nullptr};

  sensor::Sensor *temp_atmo_sensor{nullptr};
  sensor::Sensor *temp_indoors_sensor{nullptr};
  binary_sensor::BinarySensor *init_status_binary{nullptr};
  sensor::Sensor *voltage_param1_sensor{nullptr};
  sensor::Sensor *voltage_param2_sensor{nullptr};
  sensor::Sensor *frost_status_sensor{nullptr};
  sensor::Sensor *temp2_atmo_sensor{nullptr};
  sensor::Sensor *temp2_indoors_sensor{nullptr};
  sensor::Sensor *temp_postheater_sensor{nullptr};

  sensor::Sensor *max_vol_sensor{nullptr};
  sensor::Sensor *min_vol_sensor{nullptr};
  sensor::Sensor *current_input_vol_sensor{nullptr};
  sensor::Sensor *current_output_vol_sensor{nullptr};

  number::Number *u1_number{nullptr};
  number::Number *u2_number{nullptr};
  number::Number *u3_number{nullptr};
  select::Select *ventilation_preset_select{nullptr};
  select::Select *bypass_select{nullptr};

  void set_pins(int in, int out) { pin_in = in; pin_out = out; }

  void set_bypass_mode(const std::string &mode);
  void apply_preset(const std::string &preset);
  void write_u_preset(uint8_t preset_num, uint16_t value);

  // Settery (same as sync version)
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
  void loop() override;   // ← NEW: Non-blocking OT processing
  void update() override; // ← Tylko inicjuje requesty

 protected:
  void start_next_request();
  void handle_response(unsigned long response);

  inline void publish_bypass_text_(uint8_t v) {
	if (!bypass_status_text) return;
	switch (v) {
	  case 0: bypass_status_text->publish_state("closed"); break;
	  case 1: bypass_status_text->publish_state("automatic"); break;
	  case 2: bypass_status_text->publish_state("inlet_minimum"); break;
	  default: bypass_status_text->publish_state("unknown"); break;
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

  ESP_LOGI("brink", "Async OpenTherm initialized on pins IN=%d, OUT=%d", pin_in, pin_out);
}

inline void BrinkOpenTherm::loop() {
  if (ot == nullptr || async_state_ != AsyncState::IDLE) return;

  // Process one request per loop iteration (quasi-async)
  // This keeps ESPHome responsive by not blocking for all 38 steps at once
  start_next_request();
}

inline void BrinkOpenTherm::update() {
  if (ot == nullptr) return;

  // Status połączenia
  if (status_text_sensor != nullptr) {
	std::string current_status = "connected";
	if (current_status != last_status_) {
	  status_text_sensor->publish_state(current_status);
	  last_status_ = current_status;
	}
  }

  // Restart polling cycle
  step_ = 0;
  async_state_ = AsyncState::IDLE;
}

inline void BrinkOpenTherm::start_next_request() {
  if (async_state_ != AsyncState::IDLE || !ot->isReady()) {
	return; // Busy or not ready
  }

  unsigned long request = 0;
  unsigned long response = 0;
  bool should_send = true;

  switch (step_) {
	case 0:  // WRITE: wentylacja ID 71
	  request = ot->buildRequest(OpenThermMessageType::WRITE_DATA, (OpenThermMessageID)71,
								 (unsigned int)target_ventilation_);
	  ESP_LOGD("brink", "Step %d: Writing ventilation setpoint %.0f", step_, target_ventilation_);
	  break;

	case 1:  // T1 ID 80
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)80, 0);
	  ESP_LOGD("brink", "Step %d: Reading T1 (OT ID 80)", step_);
	  break;

	case 2:  // T2 ID 81
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)81, 0);
	  ESP_LOGD("brink", "Step %d: Reading T2 (OT ID 81)", step_);
	  break;

	case 3:  // T3 ID 82
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)82, 0);
	  ESP_LOGD("brink", "Step %d: Reading T3 (OT ID 82)", step_);
	  break;

	case 4:  // T4 ID 83
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)83, 0);
	  ESP_LOGD("brink", "Step %d: Reading T4 (OT ID 83)", step_);
	  break;

	case 5:  // FLOW low: TSP 52 (LB)
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 52 << 8);
	  ESP_LOGD("brink", "Step %d: Reading TSP 52 (Flow LB)", step_);
	  break;

	case 6:  // FLOW high: TSP 53 (HB) + MsgOperation
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 53 << 8);
	  ESP_LOGD("brink", "Step %d: Reading TSP 53 (Flow HB + MsgOp)", step_);
	  break;

	case 7:  // Filter: TSP 13
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 13 << 8);
	  ESP_LOGD("brink", "Step %d: Reading TSP 13 (Filter)", step_);
	  break;

	case 8:  // Bypass: TSP 54
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 54 << 8);
	  ESP_LOGD("brink", "Step %d: Reading TSP 54 (Bypass)", step_);
	  break;

	case 9:  // CPID low: TSP 64
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 64 << 8);
	  break;

	case 10:  // CPID high: TSP 65
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 65 << 8);
	  break;

	case 11:  // CPOD low: TSP 66
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 66 << 8);
	  break;

	case 12:  // CPOD high: TSP 67
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 67 << 8);
	  break;

	case 13:  // U1 low: TSP 38
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 38 << 8);
	  break;

	case 14:  // U1 high: TSP 39
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 39 << 8);
	  break;

	case 15:  // U2 low: TSP 40
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 40 << 8);
	  break;

	case 16:  // U2 high: TSP 41
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 41 << 8);
	  break;

	case 17:  // U3 low: TSP 42
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 42 << 8);
	  break;

	case 18:  // U3 high: TSP 43
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 43 << 8);
	  break;

	case 19:  // U4: TSP 44
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 44 << 8);
	  break;

	case 20:  // U5: TSP 45
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 45 << 8);
	  break;

	case 21:  // I1: TSP 46
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 46 << 8);
	  break;

	case 22:  // MaxVol low: TSP 48
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 48 << 8);
	  break;

	case 23:  // MaxVol high: TSP 49
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 49 << 8);
	  break;

	case 24:  // MinVol low: TSP 50
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 50 << 8);
	  break;

	case 25:  // MinVol high: TSP 51
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 51 << 8);
	  break;

	case 26:  // OT70: VentStatus
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)70, 0);
	  break;

	case 27:  // RPM Exhaust: OT85
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)85, 0);
	  break;

	case 28:  // RPM Supply: OT86
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)86, 0);
	  break;

	case 29:  // TSP 57: InitStatus
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 57 << 8);
	  break;

	// TSP 58/59 (VoltageParam1/2) - opcjonalne, wyłączone domyślnie
	// case 30/31: ...

	case 30:  // TSP 60: CurrentInputVol low
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 60 << 8);
	  break;

	case 31:  // TSP 61: CurrentInputVol high
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 61 << 8);
	  break;

	case 32:  // TSP 62: CurrentOutputVol low
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 62 << 8);
	  break;

	case 33:  // TSP 63: CurrentOutputVol high
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 63 << 8);
	  break;

	case 34:  // TSP 68: FrostStatus
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 68 << 8);
	  break;

	case 35:  // TSP 69: Temp2Atmo
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 69 << 8);
	  break;

	case 36:  // TSP 70: Temp2Indoors
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 70 << 8);
	  break;

	case 37:  // TSP 71: TempPostHeater
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 71 << 8);
	  break;

	default:
	  // Koniec cyklu, restart
	  step_ = 0;
	  async_state_ = AsyncState::IDLE;
	  should_send = false;
	  ESP_LOGD("brink", "Poll cycle complete, restarting");
	  break;
  }

  if (should_send) {
	// Send request synchronously (blocks ~100ms per request)
	// But we only do ONE per loop() call, so ESPHome stays responsive
	response = ot->sendRequest(request);

	if (ot->isValidResponse(response)) {
	  ESP_LOGV("brink", "Step %d: Response 0x%08lX", step_, response);
	  handle_response(response);
	} else {
	  ESP_LOGW("brink", "Step %d: Invalid response 0x%08lX", step_, response);
	}

	// Move to next step
	step_++;
	if (step_ >= 38) {  // Total steps
	  step_ = 0;
	}
  }
}

inline void BrinkOpenTherm::handle_response(unsigned long response) {
  // Process response based on current step
  switch (step_) {
	case 0:  // Write ventilation - no data to process
	  ESP_LOGD("brink", "Ventilation setpoint written");
	  break;

	case 1:  // T1
	  if (ot->isValidResponse(response) && t_supply_in_sensor) {
		float temp = ot->getFloat(response);
		ESP_LOGD("brink", "T1: %.2f°C", temp);
		t_supply_in_sensor->publish_state(temp);
	  }
	  break;

	case 2:  // T2
	  if (ot->isValidResponse(response) && t_supply_out_sensor) {
		float temp = ot->getFloat(response);
		ESP_LOGD("brink", "T2: %.2f°C", temp);
		t_supply_out_sensor->publish_state(temp);
	  }
	  break;

	case 3:  // T3
	  if (ot->isValidResponse(response) && t_exhaust_in_sensor) {
		float temp = ot->getFloat(response);
		ESP_LOGD("brink", "T3: %.2f°C", temp);
		t_exhaust_in_sensor->publish_state(temp);
	  }
	  break;

	case 4:  // T4
	  if (ot->isValidResponse(response) && t_exhaust_out_sensor) {
		float temp = ot->getFloat(response);
		ESP_LOGD("brink", "T4: %.2f°C", temp);
		t_exhaust_out_sensor->publish_state(temp);
	  }
	  break;

	case 5:  // Flow LB
	  if (ot->isValidResponse(response)) {
		tsp_low_byte_ = (uint8_t)(response & 0xFF);
	  }
	  break;

	case 6:  // Flow HB + MsgOp
	  if (ot->isValidResponse(response)) {
		uint8_t msg_op_code = (uint8_t)(response & 0xFF);

		if (current_flow_sensor) {
		  uint16_t flow = ((uint16_t)msg_op_code << 8) | tsp_low_byte_;
		  ESP_LOGD("brink", "Flow: %d m³/h", flow);
		  current_flow_sensor->publish_state(flow);
		}

		if (msg_operation_sensor) {
		  msg_operation_sensor->publish_state(msg_op_code);
		}

		if (msg_operation_text) {
		  const char* desc = decode_msg_operation(msg_op_code);
		  msg_operation_text->publish_state(desc);
		}
	  }
	  break;

	case 7:  // Filter
	  if (ot->isValidResponse(response) && filter_status_binary) {
		bool dirty = (response & 0xFF) == 1;
		ESP_LOGD("brink", "Filter dirty: %s", dirty ? "YES" : "NO");
		filter_status_binary->publish_state(dirty);
	  }
	  break;

	case 8:  // Bypass
	  if (ot->isValidResponse(response)) {
		uint8_t v = (uint8_t)(response & 0xFF);
		ESP_LOGD("brink", "Bypass: %d", v);
		if (bypass_status_sensor) bypass_status_sensor->publish_state(v);
		publish_bypass_text_(v);
	  }
	  break;

	case 9:  // CPID LB
	  if (ot->isValidResponse(response)) {
		tsp_low_byte_ = (uint8_t)(response & 0xFF);
	  }
	  break;

	case 10:  // CPID HB
	  if (ot->isValidResponse(response) && cpid_sensor) {
		uint16_t cpid = ((uint16_t)(response & 0xFF) << 8) | tsp_low_byte_;
		cpid_sensor->publish_state(cpid);
	  }
	  break;

	case 11:  // CPOD LB
	  if (ot->isValidResponse(response)) {
		tsp_low_byte_ = (uint8_t)(response & 0xFF);
	  }
	  break;

	case 12:  // CPOD HB
	  if (ot->isValidResponse(response) && cpod_sensor) {
		uint16_t cpod = ((uint16_t)(response & 0xFF) << 8) | tsp_low_byte_;
		cpod_sensor->publish_state(cpod);
	  }
	  break;

	case 13:  // U1 LB
	  if (ot->isValidResponse(response)) {
		tsp_low_byte_ = (uint8_t)(response & 0xFF);
	  }
	  break;

	case 14:  // U1 HB
	  if (ot->isValidResponse(response) && u1_sensor) {
		uint16_t u1 = ((uint16_t)(response & 0xFF) << 8) | tsp_low_byte_;
		u1_sensor->publish_state(u1);
	  }
	  break;

	case 15:  // U2 LB
	  if (ot->isValidResponse(response)) {
		tsp_low_byte_ = (uint8_t)(response & 0xFF);
	  }
	  break;

	case 16:  // U2 HB
	  if (ot->isValidResponse(response) && u2_sensor) {
		uint16_t u2 = ((uint16_t)(response & 0xFF) << 8) | tsp_low_byte_;
		u2_sensor->publish_state(u2);
	  }
	  break;

	case 17:  // U3 LB
	  if (ot->isValidResponse(response)) {
		tsp_low_byte_ = (uint8_t)(response & 0xFF);
	  }
	  break;

	case 18:  // U3 HB
	  if (ot->isValidResponse(response) && u3_sensor) {
		uint16_t u3 = ((uint16_t)(response & 0xFF) << 8) | tsp_low_byte_;
		u3_sensor->publish_state(u3);
	  }
	  break;

	case 19:  // U4
	  if (ot->isValidResponse(response) && u4_sensor) {
		uint8_t u4_raw = (uint8_t)(response & 0xFF);
		float u4 = u4_raw / 2.0f;
		u4_sensor->publish_state(u4);
	  }
	  break;

	case 20:  // U5
	  if (ot->isValidResponse(response) && u5_sensor) {
		uint8_t u5_raw = (uint8_t)(response & 0xFF);
		float u5 = u5_raw / 2.0f;
		u5_sensor->publish_state(u5);
	  }
	  break;

	case 21:  // I1
	  if (ot->isValidResponse(response) && i1_sensor) {
		uint8_t i1_raw = (uint8_t)(response & 0xFF);
		int i1 = (int)i1_raw - 100;
		i1_sensor->publish_state(i1);
	  }
	  break;

	case 22:  // MaxVol LB
	  if (ot->isValidResponse(response)) {
		tsp_low_byte_ = (uint8_t)(response & 0xFF);
	  }
	  break;

	case 23:  // MaxVol HB
	  if (ot->isValidResponse(response) && max_vol_sensor) {
		uint16_t max_vol = ((uint16_t)(response & 0xFF) << 8) | tsp_low_byte_;
		max_vol_sensor->publish_state(max_vol);
	  }
	  break;

	case 24:  // MinVol LB
	  if (ot->isValidResponse(response)) {
		tsp_low_byte_ = (uint8_t)(response & 0xFF);
	  }
	  break;

	case 25:  // MinVol HB
	  if (ot->isValidResponse(response) && min_vol_sensor) {
		uint16_t min_vol = ((uint16_t)(response & 0xFF) << 8) | tsp_low_byte_;
		min_vol_sensor->publish_state(min_vol);
	  }
	  break;

	case 26:  // VentStatus (OT70)
	  if (ot->isValidResponse(response)) {
		uint16_t status = ot->getUInt(response);
		if (fault_indication_binary) {
		  fault_indication_binary->publish_state(status & 0x1);
		}
		if (ventilation_mode_binary) {
		  ventilation_mode_binary->publish_state((status & 0x2) >> 1);
		}
	  }
	  break;

	case 27:  // RPM Exhaust
	  if (ot->isValidResponse(response) && rpm_exhaust_sensor) {
		uint16_t rpm = response & 0xFFFF;
		if (rpm > 0 && rpm < 10000) {  // Sanity check
		  rpm_exhaust_sensor->publish_state(rpm);
		}
	  }
	  break;

	case 28:  // RPM Supply
	  if (ot->isValidResponse(response) && rpm_supply_sensor) {
		uint16_t rpm = response & 0xFFFF;
		if (rpm > 0 && rpm < 10000) {
		  rpm_supply_sensor->publish_state(rpm);
		}
	  }
	  break;

	case 29:  // InitStatus
	  if (ot->isValidResponse(response) && init_status_binary) {
		bool init = (response & 0xFF) == 1;
		init_status_binary->publish_state(init);
	  }
	  break;

	case 30:  // CurrentInputVol LB
	  if (ot->isValidResponse(response)) {
		tsp_low_byte_ = (uint8_t)(response & 0xFF);
	  }
	  break;

	case 31:  // CurrentInputVol HB
	  if (ot->isValidResponse(response) && current_input_vol_sensor) {
		uint16_t vol = ((uint16_t)(response & 0xFF) << 8) | tsp_low_byte_;
		current_input_vol_sensor->publish_state(vol);
	  }
	  break;

	case 32:  // CurrentOutputVol LB
	  if (ot->isValidResponse(response)) {
		tsp_low_byte_ = (uint8_t)(response & 0xFF);
	  }
	  break;

	case 33:  // CurrentOutputVol HB
	  if (ot->isValidResponse(response) && current_output_vol_sensor) {
		uint16_t vol = ((uint16_t)(response & 0xFF) << 8) | tsp_low_byte_;
		current_output_vol_sensor->publish_state(vol);
	  }
	  break;

	case 34:  // FrostStatus
	  if (ot->isValidResponse(response) && frost_status_sensor) {
		uint8_t frost = (uint8_t)(response & 0xFF);
		frost_status_sensor->publish_state(frost);
	  }
	  break;

	case 35:  // Temp2Atmo
	  if (ot->isValidResponse(response) && temp2_atmo_sensor) {
		uint8_t temp_raw = (uint8_t)(response & 0xFF);
		int temp = (int)temp_raw - 100;
		temp2_atmo_sensor->publish_state(temp);
	  }
	  break;

	case 36:  // Temp2Indoors
	  if (ot->isValidResponse(response) && temp2_indoors_sensor) {
		uint8_t temp_raw = (uint8_t)(response & 0xFF);
		int temp = (int)temp_raw - 100;
		temp2_indoors_sensor->publish_state(temp);
	  }
	  break;

	case 37:  // TempPostHeater
	  if (ot->isValidResponse(response) && temp_postheater_sensor) {
		uint8_t temp = (uint8_t)(response & 0xFF);
		temp_postheater_sensor->publish_state(temp);
	  }
	  break;
  }
}

// Control methods (same as sync version)
inline void BrinkNumber::control(float value) {
  this->publish_state(value);
  if (this->parent_ != nullptr) {
	this->parent_->target_ventilation_ = value;
  }
}

inline void BrinkU1Number::control(float value) {
  this->publish_state(value);
  if (this->parent_ != nullptr) {
	this->parent_->write_u_preset(1, (uint16_t)value);
  }
}

inline void BrinkU2Number::control(float value) {
  this->publish_state(value);
  if (this->parent_ != nullptr) {
	this->parent_->write_u_preset(2, (uint16_t)value);
  }
}

inline void BrinkU3Number::control(float value) {
  this->publish_state(value);
  if (this->parent_ != nullptr) {
	this->parent_->write_u_preset(3, (uint16_t)value);
  }
}

inline void BrinkBypassSelect::control(const std::string &value) {
  this->publish_state(value);
  if (this->parent_ != nullptr) {
	this->parent_->set_bypass_mode(value);
  }
}

inline void BrinkPresetSelect::control(const std::string &value) {
  this->publish_state(value);
  if (this->parent_ != nullptr) {
	this->parent_->apply_preset(value);
  }
}

inline void BrinkOpenTherm::set_bypass_mode(const std::string &mode) {
  uint8_t val = 1;  // default: auto
  if (mode == "closed") val = 0;
  else if (mode == "auto") val = 1;
  else if (mode == "inlet_minimum") val = 2;

  if (ot && ot->isReady()) {
	unsigned long req = ot->buildRequest(OpenThermMessageType::WRITE_DATA, 
										 (OpenThermMessageID)89, (54 << 8) | val);
	ot->sendRequestAync(req);
	ESP_LOGI("brink", "Setting bypass mode: %s (val=%d)", mode.c_str(), val);
  }
}

inline void BrinkOpenTherm::apply_preset(const std::string &preset) {
  uint8_t val = 1;  // default U1
  if (preset == "U1") val = 1;
  else if (preset == "U2") val = 2;
  else if (preset == "U3") val = 3;

  if (ot && ot->isReady()) {
	unsigned long req = ot->buildRequest(OpenThermMessageType::WRITE_DATA,
										 (OpenThermMessageID)89, (47 << 8) | val);
	ot->sendRequestAync(req);
	ESP_LOGI("brink", "Applying preset: %s (val=%d)", preset.c_str(), val);
  }
}

inline void BrinkOpenTherm::write_u_preset(uint8_t preset_num, uint16_t value) {
  if (!ot || !ot->isReady()) return;

  uint8_t tsp_base = 38 + (preset_num - 1) * 2;  // U1=38/39, U2=40/41, U3=42/43

  // Write low byte
  unsigned long req_low = ot->buildRequest(OpenThermMessageType::WRITE_DATA,
										   (OpenThermMessageID)89, 
										   (tsp_base << 8) | (value & 0xFF));
  ot->sendRequestAync(req_low);

  delay(100);  // Small delay between writes

  // Write high byte
  unsigned long req_high = ot->buildRequest(OpenThermMessageType::WRITE_DATA,
											(OpenThermMessageID)89,
											((tsp_base + 1) << 8) | ((value >> 8) & 0xFF));
  ot->sendRequestAync(req_high);

  ESP_LOGI("brink", "Writing U%d preset: %d m³/h", preset_num, value);
}

}  // namespace brink_ventilation
}  // namespace esphome
