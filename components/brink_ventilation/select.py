import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import select
from esphome.const import CONF_ID

from . import BRINK_VENTILATION_ID, BrinkOpenTherm, brink_ventilation_ns

DEPENDENCIES = ["brink_ventilation"]

BrinkBypassSelect = brink_ventilation_ns.class_("BrinkBypassSelect", select.Select, cg.Component)
BrinkPresetSelect = brink_ventilation_ns.class_("BrinkPresetSelect", select.Select, cg.Component)

CONF_TYPE = "type"

TYPES = {
    "BYPASS_CONTROL": BrinkBypassSelect,
    "VENTILATION_PRESET": BrinkPresetSelect,
}

CONFIG_SCHEMA = cv.typed_schema(
    {
        "BYPASS_CONTROL": select.select_schema(BrinkBypassSelect).extend(
            {
                cv.GenerateID(BRINK_VENTILATION_ID): cv.use_id(BrinkOpenTherm),
            }
        ).extend(cv.COMPONENT_SCHEMA),
        "VENTILATION_PRESET": select.select_schema(BrinkPresetSelect).extend(
            {
                cv.GenerateID(BRINK_VENTILATION_ID): cv.use_id(BrinkOpenTherm),
            }
        ).extend(cv.COMPONENT_SCHEMA),
    },
    key=CONF_TYPE,
    upper=True,
)


async def to_code(config):
    parent = await cg.get_variable(config[BRINK_VENTILATION_ID])
    select_type = config[CONF_TYPE]

    if select_type == "BYPASS_CONTROL":
        var = cg.new_Pvariable(config[CONF_ID])
        await select.register_select(var, config, options=["Auto", "Open", "Closed"])
        cg.add(var.set_parent(parent))
        cg.add(parent.set_bypass_select(var))
    elif select_type == "VENTILATION_PRESET":
        var = cg.new_Pvariable(config[CONF_ID])
        await select.register_select(var, config, options=["U1", "U2", "U3"])
        cg.add(var.set_parent(parent))
        cg.add(parent.set_ventilation_preset_select(var))

