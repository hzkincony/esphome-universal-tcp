# Universal Tcp Component

# Core Yaml
```yaml
external_components:
  - source:
      type: git
      url: https://github.com/hzkincony/esphome-universal-tcp
      ref: v1.0.0

universal_tcp:
  id: universal_tcp1
  local_port: 8888 # esp32 will listen on this port
  delimiter: ""
  send_buffer_length: 64

interval:
  - interval: 1min
    then:
      - lambda: !lambda |-
          auto current = id(current_1).state;
          id(universal_tcp1).send_string_data("RELAY-GET_energy-255,1," + current + "," + voltage + ",OK");
```