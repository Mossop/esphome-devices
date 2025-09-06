from esphome import automation
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_ON_UPDATE, CONF_TRIGGER_ID

from ..homeassistant_api import HomeAssistantApi, CONF_HOMEASSISTANT_API_ID

DEPENDENCIES = ["homeassistant_api"]

flights_ns = cg.esphome_ns.namespace("flights")
Flights = flights_ns.class_(
    "Flights", cg.PollingComponent
)
Event = flights_ns.struct("Event")
UpdateTrigger = flights_ns.class_(
    "UpdateTrigger", automation.Trigger.template(cg.std_vector(Event))
)

CONF_ENTITY_ID = "entity_id"
CONF_TYPE = "type"

CONFIG_FLIGHTS = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(Flights),
        cv.GenerateID(CONF_HOMEASSISTANT_API_ID): cv.use_id(HomeAssistantApi),
        cv.Required(CONF_ENTITY_ID): cv.entity_id,
        cv.Optional(CONF_ON_UPDATE): automation.validate_automation(
            {
                cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(UpdateTrigger),
            }
        ),
    }
).extend(cv.polling_component_schema("60s"))

CONFIG_SCHEMA = cv.Any(
    cv.ensure_list(CONFIG_FLIGHTS),
    CONFIG_FLIGHTS,
)

async def to_code(config):
    if isinstance(config, list):
        for conf in config:
            await to_code(conf)
        return

    paren = await cg.get_variable(config[CONF_HOMEASSISTANT_API_ID])
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_homeassistant_api(paren))
    cg.add(var.set_entity_id(config[CONF_ENTITY_ID]))

    for conf in config.get(CONF_ON_UPDATE, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(trigger, [], conf)

    await cg.register_component(var, config)
