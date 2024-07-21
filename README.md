# Universal Tcp Component
more information, you can check with KinCony's webpage: https://www.kincony.com

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

interval:
  - interval: 1min
    then:
      - lambda: !lambda |-
          auto current = id(current_1).state;
          id(universal_tcp1).send_string_data("RELAY-GET_energy-255,1," + current + "," + voltage + ",OK");
```
