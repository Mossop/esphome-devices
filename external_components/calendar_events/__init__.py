from esphome import automation
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_ON_UPDATE, CONF_TRIGGER_ID

from ..homeassistant_api import HomeAssistantApi, CONF_HOMEASSISTANT_API_ID

DEPENDENCIES = ["homeassistant_api"]

calendar_events_ns = cg.esphome_ns.namespace("calendar_events")
CalendarEvents = calendar_events_ns.class_(
    "CalendarEvents", cg.PollingComponent
)
Event = calendar_events_ns.struct("Event")
UpdateTrigger = calendar_events_ns.class_(
    "UpdateTrigger", automation.Trigger.template(cg.std_vector(Event))
)

CONF_DURATION = "duration"
CONF_START_OFFSET = "start_offset"
CONF_CALENDARS = "calendars"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(CalendarEvents),
        cv.GenerateID(CONF_HOMEASSISTANT_API_ID): cv.use_id(HomeAssistantApi),
        cv.Required(CONF_CALENDARS): cv.ensure_list(cv.string),
        cv.Optional(CONF_START_OFFSET): cv.time_period,
        cv.Required(CONF_DURATION): cv.positive_time_period_seconds,
        cv.Optional(CONF_ON_UPDATE): automation.validate_automation(
            {
                cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(UpdateTrigger),
            }
        ),
    }
).extend(cv.polling_component_schema("60s"))

async def to_code(config):
    paren = await cg.get_variable(config[CONF_HOMEASSISTANT_API_ID])
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_homeassistant_api(paren))
    cg.add(var.set_calendars(config[CONF_CALENDARS]))
    if CONF_START_OFFSET in config:
        cg.add(var.set_start_offset(config[CONF_START_OFFSET]))
    if CONF_DURATION in config:
        cg.add(var.set_duration(config[CONF_DURATION]))

    for conf in config.get(CONF_ON_UPDATE, []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        await automation.build_automation(trigger, [], conf)

    await cg.register_component(var, config)
