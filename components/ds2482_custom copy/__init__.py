import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ADDRESS, CONF_UPDATE_INTERVAL

CODEOWNERS = ["yourname"]

CONF_DS2482_CUSTOM_ID = "ds2482_custom_id"

ds2482_ns = cg.esphome_ns.namespace("ds2482_custom")
DS2482Component = ds2482_ns.class_("DS2482Component", cg.PollingComponent)

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_DS2482_CUSTOM_ID): cv.declare_id(DS2482Component),
        cv.Optional(CONF_ADDRESS, default=0x18): cv.i2c_address,
        cv.Optional(CONF_UPDATE_INTERVAL, default="15s"): cv.update_interval,
    }
).extend(cv.polling_component_schema("15s"))

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_DS2482_CUSTOM_ID])
    cg.add(var.set_address(config[CONF_ADDRESS]))
    await cg.register_component(var, config)
