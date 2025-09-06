from esphome import automation
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_ON_UPDATE, CONF_TRIGGER_ID

from ..homeassistant_api import HomeAssistantApi, CONF_HOMEASSISTANT_API_ID

DEPENDENCIES = ["homeassistant_api"]

weather_forecast_ns = cg.esphome_ns.namespace("weather_forecast")
WeatherForecast = weather_forecast_ns.class_(
    "WeatherForecast", cg.PollingComponent
)
Event = weather_forecast_ns.struct("Event")
UpdateTrigger = weather_forecast_ns.class_(
    "UpdateTrigger", automation.Trigger.template(cg.std_vector(Event))
)

CONF_ENTITY_ID = "entity_id"
CONF_TYPE = "type"

def forecast_type(value):
    value = cv.string_strict(value)
    if value not in ["daily", "hourly", "twice_daily"]:
        raise Invalid("Forecast type must be 'daily', 'hourly' or 'twice_daily'")
    return value

CONFIG_WEATHER_FORECAST = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(WeatherForecast),
        cv.GenerateID(CONF_HOMEASSISTANT_API_ID): cv.use_id(HomeAssistantApi),
        cv.Required(CONF_ENTITY_ID): cv.entity_id,
        cv.Required(CONF_TYPE): forecast_type,
        cv.Optional(CONF_ON_UPDATE): automation.validate_automation(
            {
                cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(UpdateTrigger),
            }
        ),
    }
).extend(cv.polling_component_schema("10min"))

CONFIG_SCHEMA = cv.Any(
    cv.ensure_list(CONFIG_WEATHER_FORECAST),
    CONFIG_WEATHER_FORECAST,
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
    cg.add(var.set_type(config[CONF_TYPE]))

    for conf in config.get(CONF_ON_UPDATE, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(trigger, [], conf)

    await cg.register_component(var, config)
