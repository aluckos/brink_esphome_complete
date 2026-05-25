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
  void set_parent(BrinkOpenTherm *parent) { 
    parent_ = parent;
    // Set initial traits - will be updated after reading min/max
    this->traits.set_min_value(0);
    this->traits.set_max_value(400);
    this->traits.set_step(10);
  }
  void control(float value) override;
  void setup() override;
};

// Number: U2 ventilation preset (TSP 40/41)
class BrinkU2Number : public number::Number, public Component {
 public:
  BrinkOpenTherm *parent_{nullptr};
  void set_parent(BrinkOpenTherm *parent) { 
    parent_ = parent;
    this->traits.set_min_value(0);
    this->traits.set_max_value(400);
    this->traits.set_step(10);
  }
  void control(float value) override;
  void setup() override;
};

// Number: U3 ventilation preset (TSP 42/43)
class BrinkU3Number : public number::Number, public Component {
 public:
  BrinkOpenTherm *parent_{nullptr};
  void set_parent(BrinkOpenTherm *parent) { 
    parent_ = parent;
    this->traits.set_min_value(0);
    this->traits.set_max_value(400);
    this->traits.set_step(10);
  }
  void control(float value) override;
  void setup() override;
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

  // Cached values for validation
  uint16_t min_vol_{0};
  uint16_t max_vol_{400};
  uint16_t u1_value_{0};
  uint16_t u2_value_{0};
  uint16_t u3_value_{0};

  // Async state machine
  AsyncState async_state_{AsyncState::IDLE};
  uint8_t step_{0};
  unsigned long last_response_{0};
  OpenThermResponseStatus last_response_status_{OpenThermResponseStatus::NONE};
  bool write_in_progress_{false};  // Flag to pause polling during preset writes

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

  sensor::Sensor *u1_sensor{nullptr};
  sensor::Sensor *u2_sensor{nullptr};
  sensor::Sensor *u3_sensor{nullptr};
  sensor::Sensor *u4_sensor{nullptr};
  sensor::Sensor *u5_sensor{nullptr};

  sensor::Sensor *rpm_exhaust_sensor{nullptr};
  sensor::Sensor *rpm_supply_sensor{nullptr};

  sensor::Sensor *msg_operation_sensor{nullptr};
  text_sensor::TextSensor *msg_operation_text{nullptr};

  binary_sensor::BinarySensor *init_status_binary{nullptr};
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
  void set_u1_sensor(sensor::Sensor *s) { u1_sensor = s; }
  void set_u2_sensor(sensor::Sensor *s) { u2_sensor = s; }
  void set_u3_sensor(sensor::Sensor *s) { u3_sensor = s; }
  void set_u4_sensor(sensor::Sensor *s) { u4_sensor = s; }
  void set_u5_sensor(sensor::Sensor *s) { u5_sensor = s; }
  void set_bypass_status_sensor(sensor::Sensor *s) { bypass_status_sensor = s; }
  void set_bypass_status_text(text_sensor::TextSensor *s) { bypass_status_text = s; }
  void set_filter_status_binary(binary_sensor::BinarySensor *s) { filter_status_binary = s; }
  void set_status_text_sensor(text_sensor::TextSensor *s) { status_text_sensor = s; }
  void set_fault_indication_binary(binary_sensor::BinarySensor *s) { fault_indication_binary = s; }
  void set_ventilation_mode_binary(binary_sensor::BinarySensor *s) { ventilation_mode_binary = s; }
  void set_rpm_exhaust_sensor(sensor::Sensor *s) { rpm_exhaust_sensor = s; }
  void set_rpm_supply_sensor(sensor::Sensor *s) { rpm_supply_sensor = s; }
  void set_msg_operation_sensor(sensor::Sensor *s) { msg_operation_sensor = s; }
  void set_init_status_binary(binary_sensor::BinarySensor *s) { init_status_binary = s; }
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
  void start_startup_read();
  void start_next_request();
  void handle_response();

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
  if (ot == nullptr) return;

  // Non-blocking process - check OT state machine
  ot->process();

  // Skip polling if write operation is in progress
  if (write_in_progress_) {
    return;
  }

  // Check if we're waiting and response is ready
  if (async_state_ == AsyncState::WAITING && ot->isReady()) {
	last_response_ = ot->getLastResponse();
	last_response_status_ = ot->getLastResponseStatus();
	async_state_ = AsyncState::PROCESSING;
	handle_response();
  }

  // If idle and OT ready, try to send next request
  if (async_state_ == AsyncState::IDLE && ot->isReady()) {
	start_next_request();
  }
}

inline void BrinkOpenTherm::update() {
  // Empty - polling is driven by loop()
}

inline void BrinkOpenTherm::start_next_request() {
  if (async_state_ != AsyncState::IDLE || !ot->isReady()) {
	return; // Busy or not ready
  }

  unsigned long request = 0;
  bool should_send = true;

  switch (step_) {
	case 0:  // WRITE: wentylacja ID 71 (ALWAYS WORKING)
	  request = ot->buildRequest(OpenThermMessageType::WRITE_DATA, (OpenThermMessageID)71,
								 (unsigned int)target_ventilation_);
	  ESP_LOGD("brink", "Step %d: Writing ventilation setpoint %.0f", step_, target_ventilation_);
	  break;

	case 1:  // FLOW low: TSP 52 (LB) - ALWAYS WORKING
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 52 << 8);
	  ESP_LOGD("brink", "Step %d: Reading TSP 52 (Flow LB)", step_);
	  break;

	case 2:  // FLOW high: TSP 53 (HB) + MsgOperation - ALWAYS WORKING
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 53 << 8);
	  ESP_LOGD("brink", "Step %d: Reading TSP 53 (Flow HB + MsgOp)", step_);
	  break;

	case 3:  // T1 ID 80 - ALWAYS WORKING
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)80, 0);
	  ESP_LOGD("brink", "Step %d: Reading T1 (OT ID 80)", step_);
	  break;

	case 4:  // T3 ID 82 - ALWAYS WORKING
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)82, 0);
	  ESP_LOGD("brink", "Step %d: Reading T3 (OT ID 82)", step_);
	  break;

	case 5:  // Filter: TSP 13
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 13 << 8);
	  ESP_LOGD("brink", "Step %d: Reading TSP 13 (Filter)", step_);
	  break;

	case 6:  // Bypass: TSP 54
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 54 << 8);
	  ESP_LOGD("brink", "Step %d: Reading TSP 54 (Bypass)", step_);
	  break;

	case 7:  // MAX_VOL: TSP 48 LB
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 48 << 8);
	  ESP_LOGD("brink", "Step %d: Reading TSP 48 (MAX_VOL LB)", step_);
	  break;

	case 8:  // MAX_VOL: TSP 49 HB
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 49 << 8);
	  ESP_LOGD("brink", "Step %d: Reading TSP 49 (MAX_VOL HB)", step_);
	  break;

	case 9:  // MIN_VOL: TSP 50 LB
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 50 << 8);
	  ESP_LOGD("brink", "Step %d: Reading TSP 50 (MIN_VOL LB)", step_);
	  break;

	case 10:  // MIN_VOL: TSP 51 HB
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 51 << 8);
	  ESP_LOGD("brink", "Step %d: Reading TSP 51 (MIN_VOL HB)", step_);
	  break;

	case 11:  // VentStatus: OT 70 (Fault indication + Ventilation mode)
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)70, 0);
	  ESP_LOGD("brink", "Step %d: Reading OT 70 (VentStatus)", step_);
	  break;

	case 12:  // CURRENT_INPUT_VOL: TSP 60 LB
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 60 << 8);
	  ESP_LOGD("brink", "Step %d: Reading TSP 60 (CURRENT_INPUT_VOL LB)", step_);
	  break;

	case 13:  // CURRENT_INPUT_VOL: TSP 61 HB
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 61 << 8);
	  ESP_LOGD("brink", "Step %d: Reading TSP 61 (CURRENT_INPUT_VOL HB)", step_);
	  break;

	case 14:  // CURRENT_OUTPUT_VOL: TSP 62 LB
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 62 << 8);
	  ESP_LOGD("brink", "Step %d: Reading TSP 62 (CURRENT_OUTPUT_VOL LB)", step_);
	  break;

	case 15:  // CURRENT_OUTPUT_VOL: TSP 63 HB
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 63 << 8);
	  ESP_LOGD("brink", "Step %d: Reading TSP 63 (CURRENT_OUTPUT_VOL HB)", step_);
	  break;

	case 16:  // U1: TSP 38 LB
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 38 << 8);
	  ESP_LOGD("brink", "Step %d: Reading TSP 38 (U1 LB)", step_);
	  break;

	case 17:  // U1: TSP 39 HB
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 39 << 8);
	  ESP_LOGD("brink", "Step %d: Reading TSP 39 (U1 HB)", step_);
	  break;

	case 18:  // U2: TSP 40 LB
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 40 << 8);
	  ESP_LOGD("brink", "Step %d: Reading TSP 40 (U2 LB)", step_);
	  break;

	case 19:  // U2: TSP 41 HB
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 41 << 8);
	  ESP_LOGD("brink", "Step %d: Reading TSP 41 (U2 HB)", step_);
	  break;

	case 20:  // U3: TSP 42 LB
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 42 << 8);
	  ESP_LOGD("brink", "Step %d: Reading TSP 42 (U3 LB)", step_);
	  break;

	case 21:  // U3: TSP 43 HB
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)89, 43 << 8);
	  ESP_LOGD("brink", "Step %d: Reading TSP 43 (U3 HB)", step_);
	  break;

	// === EXPERIMENTAL - może nie działać ===
	#ifdef BRINK_ENABLE_EXPERIMENTAL

	case 22:  // RPM Exhaust: OT85
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)85, 0);
	  ESP_LOGD("brink", "Step %d: Reading RPM Exhaust (OT ID 85)", step_);
	  break;

	case 23:  // RPM Supply: OT86
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)86, 0);
	  ESP_LOGD("brink", "Step %d: Reading RPM Supply (OT ID 86)", step_);
	  break;

	case 24:  // T2 ID 81
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)81, 0);
	  ESP_LOGD("brink", "Step %d: Reading T2 (OT ID 81)", step_);
	  break;

	case 25:  // T4 ID 83
	  request = ot->buildRequest(OpenThermMessageType::READ_DATA, (OpenThermMessageID)83, 0);
	  ESP_LOGD("brink", "Step %d: Reading T4 (OT ID 83)", step_);
	  break;

	#endif  // BRINK_ENABLE_EXPERIMENTAL

	default:
	  // Koniec cyklu, restart
	  step_ = 0;
	  async_state_ = AsyncState::IDLE;
	  should_send = false;
	  ESP_LOGD("brink", "Poll cycle complete, restarting from step 0");
	  break;
  }

  if (should_send && ot->sendRequestAync(request)) {
	async_state_ = AsyncState::WAITING;
	ESP_LOGV("brink", "Request sent for step %d, waiting for response", step_);
  } else if (should_send) {
	ESP_LOGW("brink", "Failed to send request for step %d, OT not ready", step_);
  }
}

inline void BrinkOpenTherm::handle_response() {
  // Publish communication status
  if (status_text_sensor) {
	const char* status_cstr = ot->statusToString(last_response_status_);
	std::string status_str = status_cstr ? status_cstr : "UNKNOWN";
	if (status_str != last_status_) {
	  status_text_sensor->publish_state(status_str);
	  last_status_ = status_str;
	}
  }

  // Check response status
  if (last_response_status_ != OpenThermResponseStatus::SUCCESS) {
	ESP_LOGW("brink", "Step %d: Response status: %s", 
			 step_, ot->statusToString(last_response_status_));
	step_++;
	async_state_ = AsyncState::IDLE;
	// DON'T call start_next_request() immediately - let loop() check isReady()
	return;
  }

  unsigned long response = last_response_;

  // Process response based on current step
  switch (step_) {
	case 0:  // Write ventilation - no data to process
	  ESP_LOGD("brink", "Ventilation setpoint written");
	  break;

	case 1:  // Flow LB (TSP 52)
	  if (ot->isValidResponse(response)) {
		tsp_low_byte_ = (uint8_t)(response & 0xFF);
	  }
	  break;

	case 2:  // Flow HB + MsgOp (TSP 53)
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

	case 3:  // T1 - ALWAYS WORKING
	  ESP_LOGD("brink", "T1 response: valid=%d, sensor=%p", ot->isValidResponse(response), t_supply_in_sensor);
	  if (ot->isValidResponse(response)) {
		float temp = ot->getFloat(response);
		ESP_LOGD("brink", "T1: %.2f°C", temp);
		if (t_supply_in_sensor) {
		  t_supply_in_sensor->publish_state(temp);
		}
	  }
	  break;

	case 4:  // T3 - ALWAYS WORKING
	  ESP_LOGD("brink", "T3 response: valid=%d, sensor=%p", ot->isValidResponse(response), t_exhaust_in_sensor);
	  if (ot->isValidResponse(response)) {
		float temp = ot->getFloat(response);
		ESP_LOGD("brink", "T3: %.2f°C", temp);
		if (t_exhaust_in_sensor) {
		  t_exhaust_in_sensor->publish_state(temp);
		}
	  }
	  break;

	case 5:  // Filter
	  if (ot->isValidResponse(response) && filter_status_binary) {
		bool dirty = (response & 0xFF) == 1;
		ESP_LOGD("brink", "Filter dirty: %s", dirty ? "YES" : "NO");
		filter_status_binary->publish_state(dirty);
	  }
	  break;

	case 6:  // Bypass
	  if (ot->isValidResponse(response)) {
		uint8_t v = (uint8_t)(response & 0xFF);
		ESP_LOGD("brink", "Bypass: %d", v);
		if (bypass_status_sensor) bypass_status_sensor->publish_state(v);
		publish_bypass_text_(v);
	  }
	  break;

	// === Periodic slow reads (every ~30 min) ===
	case 7:  // MAX_VOL LB
	  if (ot->isValidResponse(response)) {
		tsp_low_byte_ = (uint8_t)(response & 0xFF);
		ESP_LOGI("brink", "MAX_VOL LB: 0x%02X", tsp_low_byte_);
	  }
	  break;

	case 8:  // MAX_VOL HB
	  if (ot->isValidResponse(response) && max_vol_sensor) {
		uint16_t max_vol = ((uint16_t)(response & 0xFF) << 8) | tsp_low_byte_;
		max_vol_ = max_vol;  // Cache for validation
		ESP_LOGI("brink", "MAX_VOL: %d m³/h", max_vol);
		max_vol_sensor->publish_state(max_vol);
	  }
	  break;

	case 9:  // MIN_VOL LB
	  if (ot->isValidResponse(response)) {
		tsp_low_byte_ = (uint8_t)(response & 0xFF);
		ESP_LOGI("brink", "MIN_VOL LB: 0x%02X", tsp_low_byte_);
	  }
	  break;

	case 10:  // MIN_VOL HB
	  if (ot->isValidResponse(response) && min_vol_sensor) {
		uint16_t min_vol = ((uint16_t)(response & 0xFF) << 8) | tsp_low_byte_;
		min_vol_ = min_vol;  // Cache for validation
		ESP_LOGI("brink", "MIN_VOL: %d m³/h", min_vol);
		min_vol_sensor->publish_state(min_vol);
	  }
	  break;

	case 11:  // VentStatus (OT70) - Fault indication + Ventilation mode
	  if (ot->isValidResponse(response)) {
		uint16_t status = ot->getUInt(response);
		ESP_LOGD("brink", "VentStatus: 0x%04X (Fault=%d, VentMode=%d)", 
				 status, status & 0x1, (status & 0x2) >> 1);
		if (fault_indication_binary) {
		  fault_indication_binary->publish_state(status & 0x1);
		}
		if (ventilation_mode_binary) {
		  ventilation_mode_binary->publish_state((status & 0x2) >> 1);
		}
	  }
	  break;

	case 12:  // CURRENT_INPUT_VOL LB
	  if (ot->isValidResponse(response)) {
		tsp_low_byte_ = (uint8_t)(response & 0xFF);
	  }
	  break;

	case 13:  // CURRENT_INPUT_VOL HB
	  if (ot->isValidResponse(response) && current_input_vol_sensor) {
		uint16_t input_vol = ((uint16_t)(response & 0xFF) << 8) | tsp_low_byte_;
		ESP_LOGI("brink", "CURRENT_INPUT_VOL: %d m³/h", input_vol);
		current_input_vol_sensor->publish_state(input_vol);
	  }
	  break;

	case 14:  // CURRENT_OUTPUT_VOL LB
	  if (ot->isValidResponse(response)) {
		tsp_low_byte_ = (uint8_t)(response & 0xFF);
	  }
	  break;

	case 15:  // CURRENT_OUTPUT_VOL HB
	  if (ot->isValidResponse(response) && current_output_vol_sensor) {
		uint16_t output_vol = ((uint16_t)(response & 0xFF) << 8) | tsp_low_byte_;
		ESP_LOGI("brink", "CURRENT_OUTPUT_VOL: %d m³/h", output_vol);
		current_output_vol_sensor->publish_state(output_vol);
	  }
	  break;

	case 16:  // U1 LB
	  if (ot->isValidResponse(response)) {
		tsp_low_byte_ = (uint8_t)(response & 0xFF);
		ESP_LOGD("brink", "U1 LB: 0x%02X", tsp_low_byte_);
	  }
	  break;

	case 17:  // U1 HB
	  if (ot->isValidResponse(response) && u1_sensor) {
		uint8_t high_byte = (uint8_t)(response & 0xFF);
		ESP_LOGD("brink", "U1 HB: 0x%02X", high_byte);
		uint16_t u1 = ((uint16_t)high_byte << 8) | tsp_low_byte_;
		u1_value_ = u1;  // Cache for validation
		ESP_LOGI("brink", "U1: %d m³/h (raw: HB=0x%02X, LB=0x%02X)", u1, high_byte, tsp_low_byte_);
		u1_sensor->publish_state(u1);
	  }
	  break;

	case 18:  // U2 LB
	  if (ot->isValidResponse(response)) {
		tsp_low_byte_ = (uint8_t)(response & 0xFF);
		ESP_LOGD("brink", "U2 LB: 0x%02X", tsp_low_byte_);
	  }
	  break;

	case 19:  // U2 HB
	  if (ot->isValidResponse(response) && u2_sensor) {
		uint8_t high_byte = (uint8_t)(response & 0xFF);
		ESP_LOGD("brink", "U2 HB: 0x%02X", high_byte);
		uint16_t u2 = ((uint16_t)high_byte << 8) | tsp_low_byte_;
		u2_value_ = u2;  // Cache for validation
		ESP_LOGI("brink", "U2: %d m³/h (raw: HB=0x%02X, LB=0x%02X)", u2, high_byte, tsp_low_byte_);
		u2_sensor->publish_state(u2);
	  }
	  break;

	case 20:  // U3 LB
	  if (ot->isValidResponse(response)) {
		tsp_low_byte_ = (uint8_t)(response & 0xFF);
		ESP_LOGD("brink", "U3 LB: 0x%02X", tsp_low_byte_);
	  }
	  break;

	case 21:  // U3 HB
	  if (ot->isValidResponse(response) && u3_sensor) {
		uint8_t high_byte = (uint8_t)(response & 0xFF);
		ESP_LOGD("brink", "U3 HB: 0x%02X", high_byte);
		uint16_t u3 = ((uint16_t)high_byte << 8) | tsp_low_byte_;
		u3_value_ = u3;  // Cache for validation
		ESP_LOGI("brink", "U3: %d m³/h (raw: HB=0x%02X, LB=0x%02X)", u3, high_byte, tsp_low_byte_);
		u3_sensor->publish_state(u3);
	  }
	  #ifndef BRINK_ENABLE_EXPERIMENTAL
	  // If experimental not enabled, reset after U3
	  step_ = -1;  // Will become 0 after step_++ below
	  #endif
	  break;

	#ifdef BRINK_ENABLE_EXPERIMENTAL

	case 22:  // RPM Exhaust
	  if (ot->isValidResponse(response) && rpm_exhaust_sensor) {
		uint16_t rpm = (uint16_t)(response & 0xFFFF);
		ESP_LOGD("brink", "RPM Exhaust: %d", rpm);
		rpm_exhaust_sensor->publish_state(rpm);
	  }
	  break;

	case 23:  // RPM Supply
	  if (ot->isValidResponse(response) && rpm_supply_sensor) {
		uint16_t rpm = (uint16_t)(response & 0xFFFF);
		ESP_LOGD("brink", "RPM Supply: %d", rpm);
		rpm_supply_sensor->publish_state(rpm);
	  }
	  break;

	case 24:  // T2
	  if (ot->isValidResponse(response) && t_supply_out_sensor) {
		float temp = ot->getFloat(response);
		ESP_LOGD("brink", "T2: %.2f°C", temp);
		t_supply_out_sensor->publish_state(temp);
	  }
	  break;

	case 25:  // T4
	  if (ot->isValidResponse(response) && t_exhaust_out_sensor) {
		float temp = ot->getFloat(response);
		ESP_LOGD("brink", "T4: %.2f°C", temp);
		t_exhaust_out_sensor->publish_state(temp);
	  }
	  break;

	#endif  // BRINK_ENABLE_EXPERIMENTAL
  }

  // Move to next step
  step_++;
  async_state_ = AsyncState::IDLE;

  // Let loop() handle next request when isReady()
}

// Control methods (same as sync version)
inline void BrinkNumber::control(float value) {
  this->publish_state(value);
  if (this->parent_ != nullptr) {
	this->parent_->target_ventilation_ = value;
  }
}

inline void BrinkU1Number::control(float value) {
  ESP_LOGI("brink", "U1Number::control called with value=%.0f", value);
  this->publish_state(value);
  if (this->parent_ != nullptr) {
	ESP_LOGD("brink", "Calling write_u_preset(1, %d)", (uint16_t)value);
	this->parent_->write_u_preset(1, (uint16_t)value);
  } else {
	ESP_LOGE("brink", "U1Number::control: parent is NULL!");
  }
}

inline void BrinkU2Number::control(float value) {
  ESP_LOGI("brink", "U2Number::control called with value=%.0f", value);
  this->publish_state(value);
  if (this->parent_ != nullptr) {
	ESP_LOGD("brink", "Calling write_u_preset(2, %d)", (uint16_t)value);
	this->parent_->write_u_preset(2, (uint16_t)value);
  } else {
	ESP_LOGE("brink", "U2Number::control: parent is NULL!");
  }
}

inline void BrinkU3Number::control(float value) {
  ESP_LOGI("brink", "U3Number::control called with value=%.0f", value);
  this->publish_state(value);
  if (this->parent_ != nullptr) {
	ESP_LOGD("brink", "Calling write_u_preset(3, %d)", (uint16_t)value);
	this->parent_->write_u_preset(3, (uint16_t)value);
  } else {
	ESP_LOGE("brink", "U3Number::control: parent is NULL!");
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
  ESP_LOGI("brink", "write_u_preset called: U%d = %d m³/h", preset_num, value);

  if (!ot) {
	ESP_LOGE("brink", "write_u_preset: OT is NULL!");
	return;
  }

  // Validate: value must be within min_vol and max_vol range
  if (value < min_vol_ || value > max_vol_) {
	ESP_LOGE("brink", "U%d validation failed: %d not in range [%d, %d]", 
			 preset_num, value, min_vol_, max_vol_);
	return;
  }

  // Helper: check if cached value is valid (within min/max range)
  auto is_valid_cached = [this](uint16_t cached) {
	return cached >= min_vol_ && cached <= max_vol_;
  };

  // Validate: U1 <= U2 <= U3 (only check against valid cached values)
  if (preset_num == 1) {
	// U1 must be <= U2 and <= U3
	if (is_valid_cached(u2_value_) && value > u2_value_) {
	  ESP_LOGE("brink", "U1 validation failed: %d > U2 (%d)", value, u2_value_);
	  return;
	}
	if (is_valid_cached(u3_value_) && value > u3_value_) {
	  ESP_LOGE("brink", "U1 validation failed: %d > U3 (%d)", value, u3_value_);
	  return;
	}
  } else if (preset_num == 2) {
	// U2 must be >= U1 and <= U3
	if (is_valid_cached(u1_value_) && value < u1_value_) {
	  ESP_LOGE("brink", "U2 validation failed: %d < U1 (%d)", value, u1_value_);
	  return;
	}
	if (is_valid_cached(u3_value_) && value > u3_value_) {
	  ESP_LOGE("brink", "U2 validation failed: %d > U3 (%d)", value, u3_value_);
	  return;
	}
  } else if (preset_num == 3) {
	// U3 must be >= U1 and >= U2
	if (is_valid_cached(u1_value_) && value < u1_value_) {
	  ESP_LOGE("brink", "U3 validation failed: %d < U1 (%d)", value, u1_value_);
	  return;
	}
	if (is_valid_cached(u2_value_) && value < u2_value_) {
	  ESP_LOGE("brink", "U3 validation failed: %d < U2 (%d)", value, u2_value_);
	  return;
	}
  }

  ESP_LOGI("brink", "Validation passed for U%d = %d", preset_num, value);

  // Set flag to pause async polling loop
  write_in_progress_ = true;

  // Wait for OT to be ready (max 2 seconds)
  int wait_count = 0;
  const int max_wait = 40;  // 40 * 50ms = 2 seconds
  while (!ot->isReady() && wait_count < max_wait) {
    delay(50);
    wait_count++;
  }

  if (!ot->isReady()) {
    ESP_LOGE("brink", "write_u_preset: Timeout waiting for OT ready");
    write_in_progress_ = false;
    return;
  }

  if (wait_count > 0) {
    ESP_LOGD("brink", "Waited %d ms for OT ready", wait_count * 50);
  }

  uint8_t tsp_base = 38 + (preset_num - 1) * 2;  // U1=38/39, U2=40/41, U3=42/43
  uint8_t low_byte = value & 0xFF;
  uint8_t high_byte = (value >> 8) & 0xFF;

  // Write low byte using synchronous sendRequest (blocks until response)
  unsigned long req_low = ot->buildRequest(OpenThermMessageType::WRITE_DATA,
										   (OpenThermMessageID)89, 
										   (tsp_base << 8) | low_byte);
  unsigned long resp_low = ot->sendRequest(req_low);

  if (!resp_low) {
	ESP_LOGE("brink", "Failed to write U%d low byte (TSP %d)", preset_num, tsp_base);
	write_in_progress_ = false;
	return;
  }
  ESP_LOGD("brink", "Wrote TSP %d (LB) = 0x%02X", tsp_base, low_byte);

  // Write high byte using synchronous sendRequest
  unsigned long req_high = ot->buildRequest(OpenThermMessageType::WRITE_DATA,
											(OpenThermMessageID)89,
											((tsp_base + 1) << 8) | high_byte);
  unsigned long resp_high = ot->sendRequest(req_high);

  if (resp_high) {
	ESP_LOGI("brink", "Successfully wrote U%d preset: %d m³/h (TSP %d/%d)", 
			 preset_num, value, tsp_base, tsp_base + 1);
  } else {
	ESP_LOGE("brink", "Failed to write U%d high byte (TSP %d)", preset_num, tsp_base + 1);
  }

  // Clear flag to resume async polling
  write_in_progress_ = false;
}

// Setup methods for U1/U2/U3 numbers (after BrinkOpenTherm is fully defined)
inline void BrinkU1Number::setup() {
  // Update range after parent reads min/max from device
  if (parent_ && parent_->min_vol_ > 0 && parent_->max_vol_ > 0) {
    this->traits.set_min_value(parent_->min_vol_);
    this->traits.set_max_value(parent_->max_vol_);
  }
}

inline void BrinkU2Number::setup() {
  if (parent_ && parent_->min_vol_ > 0 && parent_->max_vol_ > 0) {
    this->traits.set_min_value(parent_->min_vol_);
    this->traits.set_max_value(parent_->max_vol_);
  }
}

inline void BrinkU3Number::setup() {
  if (parent_ && parent_->min_vol_ > 0 && parent_->max_vol_ > 0) {
    this->traits.set_min_value(parent_->min_vol_);
    this->traits.set_max_value(parent_->max_vol_);
  }
}

}  // namespace brink_ventilation
}  // namespace esphome
