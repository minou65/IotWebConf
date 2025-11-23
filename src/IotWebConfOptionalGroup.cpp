/**
 * IotWebConfOptionalGroup.cpp -- IotWebConf is an ESP8266/ESP32
 *   non blocking WiFi/AP web configuration library for Arduino.
 *   https://github.com/prampec/IotWebConf
 *s
 * Copyright (C) 2020 Balazs Kelemen <prampec+arduino@gmail.com>
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#include "IotWebConfOptionalGroup.h"

namespace iotwebconf
{

OptionalParameterGroup::OptionalParameterGroup(const char* id, const char* label, bool defaultVisible)
  : ParameterGroup(id, label)
{
  this->_defaultActive = defaultVisible;
}


int OptionalParameterGroup::getStorageSize()
{
  return ParameterGroup::getStorageSize() + 1;
}

void OptionalParameterGroup::applyDefaultValue()
{
  this->_active = this->_defaultActive;
  ParameterGroup::applyDefaultValue();
}

void OptionalParameterGroup::storeValue(
  std::function<void(SerializationData* serializationData)> doStore)
{
  // -- Store active flag.
  byte data[1];
  data[0] = (byte)this->_active;
  SerializationData serializationData;
  serializationData.length = 1;
  serializationData.data = data;
  doStore(&serializationData);

  // -- Store other items.
  ParameterGroup::storeValue(doStore);
}
void OptionalParameterGroup::loadValue(
  std::function<void(SerializationData* serializationData)> doLoad)
{
  // -- Load activity.
  byte data[1];
  SerializationData serializationData;
  serializationData.length = 1;
  serializationData.data = data;
  doLoad(&serializationData);
  this->_active = (bool)data[0];
  
  // -- Load other items.
  ParameterGroup::loadValue(doLoad);
}

void OptionalParameterGroup::renderHtml(bool dataArrived, WebRequestWrapper* webRequestWrapper)
{
    if (this->label != nullptr)
    {
      String content = getStartTemplate();
      content.replace("{b}", this->label);
      content.replace("{i}", this->getId());
      content.replace("{v}", this->_active ? "active" : "inactive");
      if (this->_active)
      {
        content.replace("{cb}", "hide");
        content.replace("{cf}", "");
      }
      else
      {
        content.replace("{cb}", "");
        content.replace("{cf}", "hide");
      }
      webRequestWrapper->sendContent(content);
    }
    ConfigItem* current = this->_firstItem;
    while (current != nullptr)
    {
      if (current->visible)
      {
        current->renderHtml(dataArrived, webRequestWrapper);
      }
      current = this->getNextItemOf(current);
    }
    if (this->label != nullptr)
    {
      String content = getEndTemplate();
      content.replace("{b}", this->label);
      content.replace("{i}", this->getId());
      webRequestWrapper->sendContent(content);
    }
}

bool OptionalParameterGroup::renderHtml(bool dataArrived, WebRequestWrapper* webRequestWrapper, HtmlChunkCallback outputCallback) {
    // Serial.println("OptionalParameterGroup::renderHtml called");

    ConfigItem* current_ = _currentItem ? _currentItem : this->_firstItem;

    if (!_startTemplateSend && this->label != nullptr) {
        //Serial.println("   outputting start template");
        String content_ = getStartTemplate();
        content_.replace("{b}", this->label);
        content_.replace("{i}", this->getId());
        content_.replace("{v}", this->_active ? "active" : "inactive");
        if (this->_active) {
            content_.replace("{cb}", "hide");
            content_.replace("{cf}", "");
        }
        else {
            content_.replace("{cb}", "");
            content_.replace("{cf}", "hide");
        }
        bool completed_ = outputCallback(content_.c_str(), content_.length());
        if (!completed_) {
            //Serial.println("Rendering interrupted during start template, saving state.");
            return completed_;
		}
		_startTemplateSend = true;
    }

   
    while (current_ != nullptr) {
		//Serial.print("   rendering item: "); Serial.println(current_->getId());
        if (current_->visible) {
            bool completed_ = current_->renderHtml(dataArrived, webRequestWrapper, outputCallback);
            if (!completed_) {
                //Serial.println("Rendering interrupted, saving state.");
                _currentItem = current_;
                return completed_;
            }
        }
        current_ = this->getNextItemOf(current_);
    }

    if (this->label != nullptr) {
		//Serial.println("   outputting end template");
        String content_ = getEndTemplate();
        content_.replace("{b}", this->label);
        content_.replace("{i}", this->getId());
        bool completed_ = outputCallback(content_.c_str(), content_.length());
        if (!completed_) {
            //Serial.println("Rendering interrupted during end template, saving state.");
            return completed_;
		}
    }

    _currentItem = nullptr;
    _startTemplateSend = false;
    //Serial.println("OptionalParameterGroup::renderHtml completed");
    return true;
}

void OptionalParameterGroup::update(WebRequestWrapper* webRequestWrapper)
{
  // -- Get active variable
  String activeId = String(this->getId());
  activeId += 'v';
  if (webRequestWrapper->hasArg(activeId))
  {
    String activeStr = webRequestWrapper->arg(activeId);
    this->_active = activeStr.equals("active");
  }

  // Update other items.
  ParameterGroup::update(webRequestWrapper);
}

void OptionalParameterGroup::debugTo(Stream* out)
{
  out->print('(');
  out->print(this->_active ? "active" : "inactive");
  out->print(')');

  // Print rest.
  ParameterGroup::debugTo(out);
}

///////////////////////////////////////////////////////////////////////////////

String ChainedParameterGroup::getStartTemplate()
{
  String result = OptionalParameterGroup::getStartTemplate();

  if ((this->_prevGroup != nullptr) && (!this->_prevGroup->isActive()))
  {
    result.replace("{cb}", "hide");
  }
  return result;
};

String ChainedParameterGroup::getEndTemplate()
{
  String result;
  if (this->_nextGroup == nullptr)
  {
    result = OptionalParameterGroup::getEndTemplate();
  }
  else
  {
    result = FPSTR(IOTWEBCONF_HTML_FORM_CHAINED_GROUP_NEXTID);
    result.replace("{in}", this->_nextGroup->getId());
    result += OptionalParameterGroup::getEndTemplate();
  }
  return result;
};


}