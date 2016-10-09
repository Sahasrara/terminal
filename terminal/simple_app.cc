// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "simple_app.h"

#include <string>

#include "simple_handler.h"
#include "renderer_terminal.h"
#include "include/cef_browser.h"
#include "include/cef_command_line.h"
#include "include/views/cef_browser_view.h"
#include "include/views/cef_window.h"
#include "include/wrapper/cef_helpers.h"

namespace {
  // When using the Views framework this object provides the delegate
  // implementation for the CefWindow that hosts the Views-based browser.
  class SimpleWindowDelegate : public CefWindowDelegate {
   public:
    explicit SimpleWindowDelegate(CefRefPtr<CefBrowserView> browser_view)
        : browser_view_(browser_view) {
    }

    void OnWindowCreated(CefRefPtr<CefWindow> window) OVERRIDE {
      // Add the browser view and show the window.
      window->AddChildView(browser_view_);
      window->Show();

      // Give keyboard focus to the browser view.
      browser_view_->RequestFocus();
    }

    void OnWindowDestroyed(CefRefPtr<CefWindow> window) OVERRIDE {
      browser_view_ = NULL;
    }

    bool CanClose(CefRefPtr<CefWindow> window) OVERRIDE {
      // Allow the window to close if the browser says it's OK.
      CefRefPtr<CefBrowser> browser = browser_view_->GetBrowser();
      if (browser)
        return browser->GetHost()->TryCloseBrowser();
      return true;
    }

   private:
    CefRefPtr<CefBrowserView> browser_view_;

    IMPLEMENT_REFCOUNTING(SimpleWindowDelegate);
    DISALLOW_COPY_AND_ASSIGN(SimpleWindowDelegate);
  };
}  // namespace


CefRefPtr<CefBrowserProcessHandler> SimpleApp::GetBrowserProcessHandler() {
  return this;
}
CefRefPtr<CefRenderProcessHandler> SimpleApp::GetRenderProcessHandler() {
  return this;
}


SimpleApp::SimpleApp() {
}

std::string getExecutableBaseFolder() {
    std::string path    = "";
    pid_t       pid     = getpid();
    char        buf[20] = {
        0
    };

    sprintf(buf, "%d", pid);
    std::string _link = "/proc/";
    _link.append(buf);
    _link.append("/exe");
    char proc[512];
    int  ch = readlink(_link.c_str(), proc, 512);
    if ( ch != -1 ) {
        proc[ch] = 0;
        path     = proc;
        std::string::size_type t = path.find_last_of("/");
        path = path.substr(0, t);
    }

    return path;
}

void SimpleApp::OnBrowserCreated(CefRefPtr<CefBrowser> browser) {
  this->browser = browser;
}

void SimpleApp::OnBrowserDestroyed(CefRefPtr<CefBrowser> browser) {
  this->browser = NULL;
}

void SimpleApp::OnContextInitialized() {
  /**
   *  Browser Creation Preparations
   */
  CEF_REQUIRE_UI_THREAD();

  CefRefPtr<CefCommandLine> command_line =
      CefCommandLine::GetGlobalCommandLine();

#if defined(OS_WIN) || defined(OS_LINUX)
  // Create the browser using the Views framework if "--use-views" is specified
  // via the command-line. Otherwise, create the browser using the native
  // platform framework. The Views framework is currently only supported on
  // Windows and Linux.
  const bool use_views = command_line->HasSwitch("use-views");
#else
  const bool use_views = false;
#endif

  /**
   *  Custom Setup
   */
  // SimpleHandler implements browser-level callbacks.
  CefRefPtr<SimpleHandler> handler(new SimpleHandler(use_views));

  // Specify CEF browser settings here.
  CefBrowserSettings browser_settings;

  std::string url = "file://";
  url.append(getExecutableBaseFolder());
  url.append("/web_files/html/index.html");
  LOG(WARNING) << url;


  /**
   *  Create Browser
   */
  if (use_views) {
    // Create the BrowserView.
    CefRefPtr<CefBrowserView> browser_view = CefBrowserView::CreateBrowserView(
        handler, url, browser_settings, NULL, NULL);

    // Create the Window. It will show itself after creation.
    CefWindow::CreateTopLevelWindow(new SimpleWindowDelegate(browser_view));
  } else {
    // Information used when creating the native window.
    CefWindowInfo window_info;

#if defined(OS_WIN)
    // On Windows we need to specify certain flags that will be passed to
    // CreateWindowEx().
    window_info.SetAsPopup(NULL, "terminal");
#endif

    // Create the first browser window.
    CefBrowserHost::CreateBrowser(window_info, handler, url, browser_settings,
                                  NULL);
  }
}

/**
 * Register JavaScript Functions
 */
void SimpleApp::OnContextCreated(CefRefPtr<CefBrowser> browser,
  CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) {
  // Retrieve the context's window object.
  CefRefPtr<CefV8Value> window = context->GetGlobal();
  
  // Create an instance of CefV8Handler object.
  CefRefPtr<CefV8Handler> handler = this;

  // Register all Functions.
  CefRefPtr<CefV8Value> openTerminal = CefV8Value::CreateFunction(
    "openTerminal", handler);
  CefRefPtr<CefV8Value> writeToTerminal = CefV8Value::CreateFunction(
    "writeToTerminal", handler);
  CefRefPtr<CefV8Value> closeTerminal = CefV8Value::CreateFunction(
    "closeTerminal", handler);

  // Create a new object to hold our methods.
  CefRefPtr<CefV8Value> terminalHandler = CefV8Value::CreateObject(NULL);

  // Add the functions to the TerminalHandler.
  terminalHandler->SetValue("openTerminal", openTerminal, 
    V8_PROPERTY_ATTRIBUTE_NONE);
  terminalHandler->SetValue("writeToTerminal", writeToTerminal, 
    V8_PROPERTY_ATTRIBUTE_NONE);
  terminalHandler->SetValue("closeTerminal", closeTerminal, 
    V8_PROPERTY_ATTRIBUTE_NONE);

  // Add the object to windows JS: window.TerminalHandler.
  window->SetValue("TerminalHandler", terminalHandler, 
    V8_PROPERTY_ATTRIBUTE_NONE);
}

/**
 *  Destroy all Terminal Assets
 */
void SimpleApp::OnContextReleased(CefRefPtr<CefBrowser> browser, 
      CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) {

  // Destroy All Terminal
  RendererTerminal::destroyAll();
}

/**
 *  Handle Browser Messages
 */
bool SimpleApp::OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, 
    CefProcessId source_process, CefRefPtr<CefProcessMessage> message) {
  bool handled = false;
  const std::string& message_name = message->GetName();
  CefRefPtr<CefListValue> arguments = message->GetArgumentList();
  if (message_name == "readFromTerminal") {
    if (arguments->GetSize() == 2) {
      // Grab Arguments
      uint32 terminalId = arguments->GetInt(0);
      CefString data = arguments->GetString(1);

      // Find the Terminal
      RendererTerminal *terminal = RendererTerminal::getTerminal(terminalId);

      // Sent data from terminal to javascript
      terminal->read(data);

      handled = true;
    }
  }
  return handled;
}

/**
 *  Handle JavaScript Functions
 */
// JS Handler Code (break into multiple handlers later)
bool SimpleApp::Execute(const CefString& name, CefRefPtr<CefV8Value> object, 
  const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, 
  CefString& exception) {
  bool handled = false;

  if (name == "openTerminal") {
    if (arguments.size() == 1 && arguments[0]->IsFunction()) {
      // Create Terminal
      RendererTerminal* terminal = new RendererTerminal(arguments[0], 
        CefV8Context::GetCurrentContext(), this->browser);

      // Return Terminal ID
      retval = CefV8Value::CreateUInt(terminal->getId());

      LOG(WARNING) << "openTerminal";
      handled = true;
    }   
  } else if (name == "writeToTerminal") {
    if (arguments.size() == 2 && arguments[0]->IsUInt() && 
      arguments[1]->IsString()) {
      // Arg[0] = terminalId
      uint32 terminalId = arguments[0]->GetUIntValue();
      // Arg[1] = data
      CefString data = arguments[1]->GetStringValue();

      // Find the Terminal
      RendererTerminal *terminal = RendererTerminal::getTerminal(terminalId);

      if (terminal != NULL) {
        // Write to the Terminal
        terminal->write(data);
      }

      LOG(WARNING) << "writeToTerminal";
      handled = true;
    }
  } else if (name == "closeTerminal") {
    if (arguments.size() == 1 && arguments[0]->IsUInt()) {
      // Arg[0] = terminalId
      uint32 terminalId = arguments[0]->GetUIntValue();

      // Destroy the terminal
      RendererTerminal::destroy(terminalId);

      LOG(WARNING) << "closeTerminal";
      handled = true;
    }
  }
  return handled;
}