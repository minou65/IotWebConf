/**
 * IotWebConfTab.h -- Extension of IotWebConf that adds tab support
 *   to the configuration page.
 *   https://github.com/minou65/IotWebConf
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

#ifndef IotWebConfTab_h
#define IotWebConfTab_h

#include <IotWebConf.h>
#include <map>
#include <vector>

namespace iotwebconf
{

/**
 * Structure to hold tab information
 */
struct TabInfo
{
  const char* tabName;
  ParameterGroup* group;
};

/**
 * Custom HTML format provider that adds tab support
 */
class TabHtmlFormatProvider : public HtmlFormatProvider
{
public:
  TabHtmlFormatProvider(
      std::vector<TabInfo>* tabs, int minWidth = 500, int maxWidth = 600)
      : _tabs(tabs), _minWidth(minWidth), _maxWidth(maxWidth)
  {
  }

  /**
   * Set the container width constraints
   */
  void setContainerWidth(int minWidth, int maxWidth)
  {
    _minWidth = minWidth;
    _maxWidth = maxWidth;
  }

  String getStyleInner() override
  {
    String style = HtmlFormatProvider::getStyleInner();

    // Main container - configurable width for consistency across tabs
    style += "body > div{min-width:" + String(_minWidth) +
             "px;max-width:" + String(_maxWidth) +
             "px;width:100%;box-sizing:border-box;}\n";

    // Tab container styling
    style +=
        ".tab{overflow:hidden;border-bottom:2px solid "
        "#16A1E7;background-color:#f1f1f1;margin-bottom:10px;display:flex;}\n";
    // Tab button styling
    style += ".tab button{background-color:#f1f1f1 !important;flex:1 1 "
             "0;min-width:0;border:1px solid #ccc "
             "!important;outline:none;cursor:pointer;";
    style += "padding:14px "
             "16px;transition:0.3s;font-size:16px;border-top-left-radius:5px;"
             "border-top-right-radius:5px;";
    style += "margin-right:2px;border-bottom:none !important;color:#333 "
             "!important;line-height:normal !important;width:auto "
             "!important;box-sizing:border-box;}\n";
    // Tab button hover
    style += ".tab button:hover{background-color:#ddd !important;}\n";
    // Active tab button
    style +=
        ".tab button.active{background-color:#16A1E7 !important;color:white "
        "!important;border:1px solid #16A1E7 !important;border-bottom:2px "
        "solid #fff !important;position:relative;z-index:1;}\n";
    // Tab content - ALL hidden by default
    style += ".tabcontent{display:none;padding:12px;border:1px solid "
             "#ccc;border-top:none;background-color:#fff;}\n";
    // Fieldsets consistent width
    style += "fieldset{width:100%;box-sizing:border-box;}\n";

    return style;
  }

  String getScriptInner() override
  {
    String script = HtmlFormatProvider::getScriptInner();
    script += "function openTab(evt,tabName){var i,tabcontent,tablinks;";
    script += "tabcontent=document.getElementsByClassName('tabcontent');";
    script +=
        "for(i=0;i<tabcontent.length;i++){tabcontent[i].style.display='none';}";
    script += "tablinks=document.getElementsByClassName('tablinks');";
    script += "for(i=0;i<tablinks.length;i++){tablinks[i].className=tablinks[i]"
              ".className.replace(' active','');}";
    script += "var tabElement=document.getElementById(tabName);";
    script += "if(tabElement){tabElement.style.display='block';}";
    script +=
        "if(evt&&evt.currentTarget){evt.currentTarget.className+=' active';}";
    script += "}";
    return script;
  }

private:
  std::vector<TabInfo>* _tabs;
  int _minWidth;
  int _maxWidth;
};

/**
 * Extended IotWebConf class with tab support
 */
class IotWebConfTab : public IotWebConf
{
public:
  /**
   * Create a new configuration handler with tab support.
   */
  IotWebConfTab(
      const char* thingName, DNSServer* dnsServer, WebServer* server,
      const char* initialApPassword, const char* configVersion = "init")
      : IotWebConf(
            thingName, dnsServer, server, initialApPassword, configVersion),
        _systemTabPosition(0)
  {
    _tabHtmlFormatProvider = new TabHtmlFormatProvider(&_tabs);
    setHtmlFormatProvider(_tabHtmlFormatProvider);

    // System-Parameter immer zum ersten Tab hinzufügen
    _systemTabName = "System";
  }

  IotWebConfTab(
      const char* thingName, DNSServer* dnsServer, WebServerWrapper* server,
      const char* initialApPassword, const char* configVersion = "init")
      : IotWebConf(
            thingName, dnsServer, server, initialApPassword, configVersion),
        _systemTabPosition(0)
  {
    _tabHtmlFormatProvider = new TabHtmlFormatProvider(&_tabs);
    setHtmlFormatProvider(_tabHtmlFormatProvider);

    _systemTabName = "System";
  }

  ~IotWebConfTab() { delete _tabHtmlFormatProvider; }

  /**
   * Set the tab name for system parameters (ThingName, WiFi, AP Password).
   * Must be called before init()!
   */
  void setSystemTabName(const char* tabName) { _systemTabName = tabName; }

  /**
   * Set the position of the system tab
   * @param position Position index:
   *   0 = first position (default)
   *   1..n = specific position (1-based)
   *   -1 = last position
   */
  void setSystemTabPosition(int position) { _systemTabPosition = position; }

  /**
   * Set the container width constraints
   */
  void setContainerWidth(int minWidth, int maxWidth)
  {
    if (_tabHtmlFormatProvider != nullptr)
    {
      _tabHtmlFormatProvider->setContainerWidth(minWidth, maxWidth);
    }
  }

  /**
   * Add a parameter group with tab assignment.
   */
  void addParameterGroup(ParameterGroup* group, const char* tabName)
  {
    IotWebConf::addParameterGroup(group);

    TabInfo tabInfo;
    tabInfo.tabName = tabName;
    tabInfo.group = group;
    _tabs.push_back(tabInfo);

    // Don't add system tab name to unique tabs list
    if (strcmp(tabName, _systemTabName) == 0)
    {
      return;
    }

    if (_uniqueTabNames.find(tabName) == _uniqueTabNames.end())
    {
      _uniqueTabNames[tabName] = _uniqueTabsList.size();
      _uniqueTabsList.push_back(tabName);
    }
  }

  /**
   * Override for compatibility - adds to default "Configuration" tab
   */
  void addParameterGroup(ParameterGroup* group)
  {
    addParameterGroup(group, "Configuration");
  }

  /**
   * Config URL web request handler with tab support.
   */
  void handleConfig(WebRequestWrapper* webRequestWrapper)
  {
    if (webRequestWrapper->hasArg("iotSave"))
    {
      IotWebConf::handleConfig(webRequestWrapper);
      return;
    }

    if (this->getState() == OnLine)
    {
      if (!webRequestWrapper->authenticate(
              IOTWEBCONF_ADMIN_USER_NAME, getApPassword()))
      {
        webRequestWrapper->requestAuthentication();
        return;
      }
    }

    HtmlFormatProvider* hfp = getHtmlFormatProvider();
    String page = hfp->getHead();
    page.replace("{v}", String("Config ") + this->getThingName());
    page += hfp->getStyle();
    page += hfp->getScript();
    page += hfp->getHeadExtension();
    page += hfp->getHeadEnd();

    // Generate tab buttons with system tab positioning
    page += generateTabButtons();

    page += hfp->getFormStart();

    // Render tabs content with system tab at correct position
    renderTabsContent(page);

    page += hfp->getFormEnd();
    page += getUpdateLinkHtml();
    page += getConfigVersionHtml();
    page += hfp->getEnd();

    webRequestWrapper->sendHeader(
        "Cache-Control", "no-cache, no-store, must-revalidate");
    webRequestWrapper->sendHeader("Pragma", "no-cache");
    webRequestWrapper->sendHeader("Expires", "-1");
    webRequestWrapper->send(200, "text/html; charset=UTF-8", page);
  }

  void handleConfig()
  {
    StandardWebRequestWrapper webRequestWrapper =
        StandardWebRequestWrapper(this->_standardWebServerWrapper._server);
    handleConfig(&webRequestWrapper);
  }

private:
  std::vector<TabInfo> _tabs;
  std::map<String, size_t> _uniqueTabNames;
  std::vector<const char*> _uniqueTabsList;
  TabHtmlFormatProvider* _tabHtmlFormatProvider = nullptr;
  const char* _systemTabName;
  int _systemTabPosition;

  String generateTabButtons()
  {
    String buttons = "<div class='tab'>\n";

    // Calculate system tab position
    int systemPos = _systemTabPosition;
    size_t totalCustomTabs = _uniqueTabsList.size();

    // Clamp position to valid range
    if (systemPos < 0)
    {
      systemPos = totalCustomTabs; // End
    }
    if (systemPos > (int)totalCustomTabs)
    {
      systemPos = totalCustomTabs;
    }

    int customTabsAdded = 0;
    bool systemTabAdded = false;

    // Generate all tabs in order
    for (int pos = 0; pos <= (int)totalCustomTabs; pos++)
    {
      // Check if system tab should be at this position
      if (pos == systemPos && !systemTabAdded)
      {
        buttons += "<button type='button' class='tablinks";
        if (pos == 0)
          buttons += " active"; // First tab is active
        buttons += "' onclick='openTab(event,\"";
        buttons += _systemTabName;
        buttons += "\");'>";
        buttons += _systemTabName;
        buttons += "</button>\n";
        systemTabAdded = true;
      }

      // Add custom tab if available and we haven't added all yet
      if (customTabsAdded < (int)totalCustomTabs)
      {
        buttons += "<button type='button' class='tablinks";
        if (pos == 0 && !systemTabAdded)
          buttons += " active";
        buttons += "' onclick='openTab(event,\"";
        buttons += _uniqueTabsList[customTabsAdded];
        buttons += "\");'>";
        buttons += _uniqueTabsList[customTabsAdded];
        buttons += "</button>\n";
        customTabsAdded++;
      }
    }

    buttons += "</div>\n";
    return buttons;
  }

  void renderTabsContent(String& page)
  {
    // Calculate system tab position
    int systemPos = _systemTabPosition;
    size_t totalCustomTabs = _uniqueTabsList.size();

    // Clamp position to valid range
    if (systemPos < 0)
    {
      systemPos = totalCustomTabs;
    }
    if (systemPos > (int)totalCustomTabs)
    {
      systemPos = totalCustomTabs;
    }

    int customTabsAdded = 0;
    bool systemTabAdded = false;

    // Render all tabs in order
    for (int pos = 0; pos <= (int)totalCustomTabs; pos++)
    {
      // Check if system tab should be at this position
      if (pos == systemPos && !systemTabAdded)
      {
        // System tab
        page += "<div id='" + String(_systemTabName) + "' class='tabcontent'";
        if (pos == 0)
        {
          page += " style='display:block;'";
        }
        else
        {
          page += " style='display:none;'";
        }
        page += ">\n";

        // Render system parameters
        renderParameterGroup(page, getSystemParameterGroup());

        // Render all custom groups belonging to system tab
        for (size_t j = 0; j < _tabs.size(); j++)
        {
          if (strcmp(_tabs[j].tabName, _systemTabName) == 0)
          {
            renderParameterGroup(page, _tabs[j].group);
          }
        }

        page += "</div>\n";
        systemTabAdded = true;
      }

      // Add custom tab if available
      if (customTabsAdded < (int)totalCustomTabs)
      {
        const char* currentTab = _uniqueTabsList[customTabsAdded];

        // Determine if this tab should be visible on load
        int actualPosition = pos;
        if (systemTabAdded && systemPos < pos)
        {
          actualPosition--; // Adjust for system tab already added
        }

        page += "<div id='" + String(currentTab) + "' class='tabcontent'";
        if (actualPosition == 0 && !systemTabAdded)
        {
          page += " style='display:block;'";
        }
        else
        {
          page += " style='display:none;'";
        }
        page += ">\n";

        // Render all custom groups belonging to this tab
        for (size_t j = 0; j < _tabs.size(); j++)
        {
          if (strcmp(_tabs[j].tabName, currentTab) == 0)
          {
            renderParameterGroup(page, _tabs[j].group);
          }
        }

        page += "</div>\n";
        customTabsAdded++;
      }
    }
  }

  void renderParameterGroup(String& page, ParameterGroup* group)
  {
    // Simple synchronous rendering - for large forms consider chunked rendering
    class StringAppender : public WebRequestWrapper
    {
    public:
      String* target;
      StringAppender(String* t) : target(t) {}

      void send(int, const char*, const String& content = String("")) override
      {
        *target += content;
      }
      void sendContent(const String& content) override { *target += content; }
      void sendHeader(const String&, const String&, bool = false) override {}
      void setContentLength(const size_t) override {}
      void stop() override {}
      const String hostHeader() const override { return ""; }
      IPAddress localIP() override { return IPAddress(0, 0, 0, 0); }
      uint16_t localPort() override { return 0; }
      const String uri() const override { return ""; }
      bool authenticate(const char*, const char*) override { return true; }
      void requestAuthentication() override {}
      bool hasArg(const String&) override { return false; }
      String arg(const String) override { return ""; }
    };

    StringAppender appender(&page);
    group->renderHtml(false, &appender);
  }

  friend TabHtmlFormatProvider;
};

} // namespace iotwebconf

using iotwebconf::IotWebConfTab;

#endif