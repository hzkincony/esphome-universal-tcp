#pragma once

#include <string>
#include <queue>

#include "esphome.h"
#include "esphome/core/component.h"
#include "AsyncTCP.h"

#define TAG "universal_tcp"

namespace esphome {
  namespace universal_tcp {
    class OnStringDataTrigger;

    class UniversalTcpComponent : public PollingComponent {
    public:
      UniversalTcpComponent() : PollingComponent(5000) {};
      void setup() override;
      void update() override;
      void send_string_data(std::string data);
      void set_remote_ip(std::string ip) {
        this->remote_ip_ = ip;
      };
      void set_remote_port(uint16_t port) {
        this->remote_port_ = port;
      };
      void set_local_port(uint16_t port) {
        this->local_port_ = port;
      };
      void set_send_delimiter(std::string delimiter) {
        this->send_delimiter_ = delimiter;
      };
      void set_send_buffer_length(uint8_t buffer_length) {
        this->send_buffer_length_ = buffer_length;
      };
      void set_receive_delimiter(std::string delimiter) {
        this->receive_delimiter_ = delimiter;
      };

      void add_string_trigger(OnStringDataTrigger *trigger) {
        this->string_triggers_.push_back(trigger);
      };
    protected:
      std::vector<OnStringDataTrigger *> string_triggers_{};
      std::string remote_ip_;
      uint16_t remote_port_;
      uint16_t local_port_;
      uint8_t send_buffer_length_;
      std::string send_delimiter_;
      std::string receive_delimiter_;
      AsyncClient tcp_client_;
      AsyncServer* tcp_server_;
      std::vector<AsyncClient*> incoming_tcp_clients_{};
      std::string receive_string_buffer_;
      std::queue<std::string> send_string_buffer_{};
      bool server_ready_ = false;
      bool client_ready_ = false;

      void ensure_tcp_server();
      void ensure_tcp_client();
      void fire_triggers();
      void broadcast_to_incoming_tcp_clients(std::string data);
    };

    class OnStringDataTrigger : public Trigger<std::string>, public Component {
      friend class UniversalTcpComponent;

    public:
      explicit OnStringDataTrigger(UniversalTcpComponent *parent)
        : parent_(parent){};

      void setup() override { this->parent_->add_string_trigger(this); }

    protected:
      UniversalTcpComponent *parent_;
    };

  }
}
