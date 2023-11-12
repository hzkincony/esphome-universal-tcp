import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import uart
from esphome.const import (
    CONF_ID,
    CONF_TRIGGER_ID,
)

DEPENDENCIES = ['network']
AUTO_LOAD = ['async_tcp']

universal_tcp_ns = cg.esphome_ns.namespace('universal_tcp')
UniversalTcpComponent = universal_tcp_ns.class_('UniversalTcpComponent', cg.PollingComponent)
OnStringDataTrigger = universal_tcp_ns.class_("OnStringDataTrigger",
                                 automation.Trigger.template(cg.std_string, cg.Component))

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(UniversalTcpComponent),
    cv.Optional("remote_ip", default="0.0.0.0"): cv.ipv4,
    cv.Optional("remote_port", default=0): cv.int_range(0, 65535),
    cv.Optional("local_port", default=0): cv.int_range(0, 65535),
    cv.Optional("send_buffer_length", default=20): cv.int_range(0, 1024),
    cv.Optional("send_delimiter", default=""): cv.string,
    cv.Optional("receive_delimiter", default=""): cv.string,
    cv.Optional("on_string_data"): automation.validate_automation(
        {
            cv.GenerateID(CONF_TRIGGER_ID): cv.declare_id(OnStringDataTrigger),
        }
    ),
}).extend(cv.COMPONENT_SCHEMA)

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    cg.add(var.set_remote_ip(str(config["remote_ip"])))
    cg.add(var.set_remote_port(config["remote_port"]))
    cg.add(var.set_local_port(config["local_port"]))
    cg.add(var.set_send_buffer_length(config["send_buffer_length"]))
    cg.add(var.set_send_delimiter(config["send_delimiter"]))
    cg.add(var.set_receive_delimiter(config["receive_delimiter"]))
    yield cg.register_component(var, config)

    for conf in config.get("on_string_data", []):
        trigger = cg.new_Pvariable(conf[CONF_TRIGGER_ID], var)
        #yield cg.register_component(trigger, conf)
        cg.add(var.add_string_trigger(trigger))
        yield automation.build_automation(trigger, [(cg.std_string, "data")], conf)
