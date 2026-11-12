/**
 * IotWebConfWebServerWrapper.h -- IotWebConf is an ESP8266/ESP32
 *   non blocking WiFi/AP web configuration library for Arduino.
 *   https://github.com/prampec/IotWebConf
 *
 * Copyright (C) 2020 Balazs Kelemen <prampec+arduino@gmail.com>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef WebServerWrapper_h
#define WebServerWrapper_h

#include <Arduino.h>
#include <IPAddress.h>

namespace iotwebconf
{

class WebRequestWrapper {
public:
  virtual ~WebRequestWrapper() = default;

  virtual const String hostHeader() const = 0;
  virtual IPAddress localIP() = 0;
  virtual uint16_t localPort() = 0;
  virtual const String uri() const = 0;
  virtual bool authenticate(const char* username, const char* password) = 0;
  virtual void requestAuthentication() = 0;
  virtual bool hasArg(const String& name) = 0;
  virtual String arg(const String name) = 0;
  virtual void sendHeader(const String& name, const String& value, bool first = false) = 0;
  virtual void setContentLength(size_t contentLength) = 0;
  virtual void send(int code, const char* content_type = nullptr,
                    const String& content = String("")) = 0;
  virtual void sendContent(const String& content) = 0;
  virtual void stop() = 0;
};

class WebServerWrapper{
public:
  virtual ~WebServerWrapper() = default;

  virtual void handleClient() = 0;
  virtual void begin() = 0;
};

} // end namespace
#endif