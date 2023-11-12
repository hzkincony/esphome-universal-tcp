#include "universal_tcp_component.h"

namespace esphome {
  namespace universal_tcp {
    void UniversalTcpComponent::setup() {
    }

    void UniversalTcpComponent::ensure_tcp_server() {
      if (local_port_ == 0) {
        return;
      }

      if (server_ready_) {
        return;
      }

      tcp_server_ = new AsyncServer(local_port_);
      tcp_server_->onClient([this](void* arg, AsyncClient *client) {
        ESP_LOGD(TAG, "new client has been connected to server, ip: %s",
                 client->remoteIP().toString().c_str());
        incoming_tcp_clients_.push_back(client);
        client->onDisconnect([this](void* arg, AsyncClient *client) {
          auto it = std::find(incoming_tcp_clients_.begin(), incoming_tcp_clients_.end(), client);
          if (it != incoming_tcp_clients_.end()) {
            incoming_tcp_clients_.erase(it);
          }
          delete client;
        }, nullptr);

        client->onData([this](void* arg, AsyncClient *client, void *data, size_t len) {
          ESP_LOGD(TAG, "receive data, length=%d, data=%s", len, data);
          receive_string_buffer_.append((char*)data, len);
          ESP_LOGD(TAG, "current buffer data=%s", receive_string_buffer_.c_str());
          fire_triggers();
        }, nullptr);
      }, nullptr);
      tcp_server_->begin();
      server_ready_ = true;
      ESP_LOGD(TAG, "listened");
    }

    void UniversalTcpComponent::fire_triggers() {
      // 检查缓冲区中是否含有分割符，即是否有完整的指令
      size_t pos;
      while ((pos = receive_string_buffer_.find(receive_delimiter_)) != std::string::npos) {
        // 提取完整的指令
        std::string command = receive_string_buffer_.substr(0, pos);
        receive_string_buffer_.erase(0, pos + 1); // 从缓冲区中移除这个指令

        // 对每一个完整的指令调用 triggers_ 的 trigger 方法
        if (!command.empty()) {
          // 假设 triggers_ 是一个能够响应字符串指令的对象
          for (auto& trigger : string_triggers_) {
            trigger->trigger(command);
          }
        }
      }
    }

    void UniversalTcpComponent::broadcast_to_incoming_tcp_clients(std::string data) {
      if (incoming_tcp_clients_.size() == 0) {
        ESP_LOGD(TAG, "none incoming tcp client, ignore broadcast data");
      }

      for (AsyncClient* client : incoming_tcp_clients_) {
        if (client->connected()) {
          client->add(data.c_str(), strlen(data.c_str()));
          client->add(send_delimiter_.c_str(), strlen(send_delimiter_.c_str()));
          client->send();
          ESP_LOGD(TAG, "send data to client ip=%s, data=%s",
                   client->remoteIP().toString().c_str(), data.c_str());
        }
      }
    }

    void UniversalTcpComponent::ensure_tcp_client() {
      if (remote_ip_ == "0.0.0.0" || remote_port_ == 0) {
        return;
      }

      if (tcp_client_.connected()) {
        if (client_ready_ == false) {
          ESP_LOGD(TAG, "client connected");
        }
        client_ready_ = true;
      } else {
        client_ready_ = false;
        ESP_LOGD(TAG, "client not connected");
      }

      if (tcp_client_.connecting()) {
        ESP_LOGD(TAG, "client still connecting");
        return;
      }

      if (client_ready_) {
        return;
      }

      if (tcp_client_.connect(remote_ip_.c_str(), remote_port_)) {
        ESP_LOGD(TAG, "client connecting...");
      } else {
        ESP_LOGD(TAG, "client connect failed");
      }
    }

    void UniversalTcpComponent::update() {
      if (!network::is_connected()) {
        ESP_LOGD(TAG, "network not ready");
        return;
      }

      ensure_tcp_server();
      ensure_tcp_client();

      if (client_ready_) {
        while (!send_string_buffer_.empty()) {
          std::string d = send_string_buffer_.front();
          tcp_client_.add(d.c_str(), strlen(d.c_str()));
          tcp_client_.add(send_delimiter_.c_str(), strlen(send_delimiter_.c_str()));
          tcp_client_.send();
          ESP_LOGD(TAG, "pop from queue, string data: %s", d.c_str());

          send_string_buffer_.pop();
        }
      }
    }

    void UniversalTcpComponent::send_string_data(std::string data) {
      broadcast_to_incoming_tcp_clients(data);

      if (remote_ip_ == "0.0.0.0" || remote_port_ == 0) {
        return;
      }

      if (!client_ready_) {
        if (send_string_buffer_.size() >= send_buffer_length_) {
          ESP_LOGW(TAG, "send buffer is full, discarding some data");
          send_string_buffer_.pop();
        }

        send_string_buffer_.push(data);
        ESP_LOGD(TAG, "client is not ready, push into buffer, string data: %s", data.c_str());
        return;
      }

      tcp_client_.add(data.c_str(), strlen(data.c_str()));
      tcp_client_.add(send_delimiter_.c_str(), strlen(send_delimiter_.c_str()));
      tcp_client_.send();

      ESP_LOGD(TAG, "send string data: %s", data.c_str());

    }
  }
}
