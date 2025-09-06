import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome.components.http_request import CONF_HTTP_REQUEST_ID, HttpRequestComponent

DEPENDENCIES = ["http_request"]

homeassistant_api_ns = cg.esphome_ns.namespace("homeassistant_api")
HomeAssistantApi = homeassistant_api_ns.class_(
    "HomeAssistantApi", cg.Component
)

CONF_HOMEASSISTANT_API_ID = "homeassistant_api_id"

CONF_TOKEN = "token"
CONF_URL = "url"

CONFIG_API = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(HomeAssistantApi),
        cv.GenerateID(CONF_HTTP_REQUEST_ID): cv.use_id(HttpRequestComponent),
        cv.Required(CONF_URL): cv.url,
        cv.Required(CONF_TOKEN): cv.string,
    }
).extend(cv.COMPONENT_SCHEMA)

CONFIG_SCHEMA = cv.Any(
    cv.ensure_list(CONFIG_API),
    CONFIG_API,
)

async def to_code(config):
    if isinstance(config, list):
        for conf in config:
            await to_code(conf)
        return

    paren = await cg.get_variable(config[CONF_HTTP_REQUEST_ID])
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_http_request(paren))
    cg.add(var.set_url(config[CONF_URL]))
    cg.add(var.set_token(config[CONF_TOKEN]))
    await cg.register_component(var, config)
