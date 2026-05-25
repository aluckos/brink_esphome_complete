import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import number
from esphome.const import CONF_ID, CONF_TYPE

from . import BRINK_VENTILATION_ID, BrinkOpenTherm

brink_ventilation_ns = cg.esphome_ns.namespace("brink_ventilation")
BrinkNumber = brink_ventilation_ns.class_("BrinkNumber", number.Number)
BrinkU1Number = brink_ventilation_ns.class_("BrinkU1Number", number.Number, cg.Component)
BrinkU2Number = brink_ventilation_ns.class_("BrinkU2Number", number.Number, cg.Component)
BrinkU3Number = brink_ventilation_ns.class_("BrinkU3Number", number.Number, cg.Component)

TYPES = {
    "VENTILATION": BrinkNumber,
    "U1": BrinkU1Number,
    "U2": BrinkU2Number,
    "U3": BrinkU3Number,
}

CONFIG_SCHEMA = cv.typed_schema(
    {
        "VENTILATION": number.number_schema(BrinkNumber).extend(
            {cv.GenerateID(BRINK_VENTILATION_ID): cv.use_id(BrinkOpenTherm)}
        ).extend(cv.COMPONENT_SCHEMA),
        "U1": number.number_schema(BrinkU1Number).extend(
            {cv.GenerateID(BRINK_VENTILATION_ID): cv.use_id(BrinkOpenTherm)}
        ).extend(cv.COMPONENT_SCHEMA),
        "U2": number.number_schema(BrinkU2Number).extend(
            {cv.GenerateID(BRINK_VENTILATION_ID): cv.use_id(BrinkOpenTherm)}
        ).extend(cv.COMPONENT_SCHEMA),
        "U3": number.number_schema(BrinkU3Number).extend(
            {cv.GenerateID(BRINK_VENTILATION_ID): cv.use_id(BrinkOpenTherm)}
        ).extend(cv.COMPONENT_SCHEMA),
    },
    key=CONF_TYPE,
    upper=True,
)

async def to_code(config):
    parent = await cg.get_variable(config[BRINK_VENTILATION_ID])
    number_type = config[CONF_TYPE]

    if number_type == "VENTILATION":
        var = await number.new_number(config, min_value=0, max_value=100, step=1)
        cg.add(var.set_parent(parent))
        cg.add(parent.set_ventilation_number(var))
    elif number_type == "U1":
        var = cg.new_Pvariable(config[CONF_ID])
        await number.register_number(var, config, min_value=0, max_value=100, step=5)
        cg.add(var.set_parent(parent))
        cg.add(parent.set_u1_number(var))
        # Publish initial value to 100 if not set from device
        cg.add(var.publish_state(100))
    elif number_type == "U2":
        var = cg.new_Pvariable(config[CONF_ID])
        await number.register_number(var, config, min_value=0, max_value=100, step=5)
        cg.add(var.set_parent(parent))
        cg.add(parent.set_u2_number(var))
        # Publish initial value to 200 if not set from device
        cg.add(var.publish_state(200))
    elif number_type == "U3":
        var = cg.new_Pvariable(config[CONF_ID])
        await number.register_number(var, config, min_value=0, max_value=100, step=5)
        cg.add(var.set_parent(parent))
        cg.add(parent.set_u3_number(var))
        # Publish initial value to 300 if not set from device
        cg.add(var.publish_state(300))
