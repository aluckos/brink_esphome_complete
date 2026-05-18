import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_TYPE,
    DEVICE_CLASS_TEMPERATURE,
    STATE_CLASS_MEASUREMENT,
    UNIT_CELSIUS,
)

from . import BRINK_VENTILATION_ID, BrinkOpenTherm

TYPES = {
    # Temperatury (OT 80-83)
    "T_SUPPLY_IN": "t_supply_in",
    "T_SUPPLY_OUT": "t_supply_out",
    "T_EXHAUST_IN": "t_exhaust_in",
    "T_EXHAUST_OUT": "t_exhaust_out",
    # Przepływ
    "CURRENT_FLOW": "current_flow",
    # Ciśnienia CPID/CPOD
    "CPID": "cpid",
    "CPOD": "cpod",
    # U1-U3: kroki wydatku
    "U1": "u1",
    "U2": "u2",
    "U3": "u3",
    # U4/U5: progi bypassu
    "U4": "u4",
    "U5": "u5",
    # I1: imbalance
    "I1": "i1",
    # Bypass status
    "BYPASS_STATUS": "bypass_status",
}

CONFIG_SCHEMA = cv.All(
    sensor.sensor_schema(
        state_class=STATE_CLASS_MEASUREMENT,
    ).extend(
        {
            cv.GenerateID(BRINK_VENTILATION_ID): cv.use_id(BrinkOpenTherm),
            cv.Required(CONF_TYPE): cv.one_of(*TYPES, upper=True),
        }
    )
)

async def to_code(config):
    parent = await cg.get_variable(config[BRINK_VENTILATION_ID])
    sensor_type = config[CONF_TYPE]

    var = await sensor.new_sensor(config)

    # Wywołaj odpowiednią metodę set_XXX_sensor w C++
    func_name = f"set_{TYPES[sensor_type]}_sensor"
    func = getattr(parent, func_name)
    cg.add(func(var))
