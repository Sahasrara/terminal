// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#ifndef CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_
#define CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_

#include "include/cef_app.h"

/*
The CefApp interface provides access to process-specific callbacks. Important callbacks include:

    OnBeforeCommandLineProcessing which provides the opportunity to programmatically set command-line arguments. See the “Command Line Arguments” section for more information.
    OnRegisterCustomSchemes which provides an opportunity to register custom schemes. See the “”Request Handling” section for more information.
    GetBrowserProcessHandler which returns the handler for functionality specific to the browser process including the OnContextInitialized() method.
    GetRenderProcessHandler which returns the handler for functionality specific to the render process. This includes JavaScript-related callbacks and process messages. See the JavaScriptIntegration Wiki page and the “Inter-Process Communication” section for more information.

*/


// Implement application-level callbacks for the browser process.
class SimpleApp : public CefApp,
                  public CefBrowserProcessHandler,
                  public CefRenderProcessHandler,
                  public CefV8Handler {
 public:
  SimpleApp();

  // CefApp methods:
  virtual CefRefPtr<CefBrowserProcessHandler> GetBrowserProcessHandler() OVERRIDE;
  virtual CefRefPtr<CefRenderProcessHandler> GetRenderProcessHandler() OVERRIDE;

  // CefBrowserProcessHandler methods:
  virtual void OnContextInitialized() OVERRIDE;


  // CefRenderProcessHandler methods:
  virtual void OnBrowserCreated(CefRefPtr<CefBrowser> browser) OVERRIDE;
  virtual void OnBrowserDestroyed(CefRefPtr<CefBrowser> browser) OVERRIDE;
  virtual void OnContextCreated(CefRefPtr<CefBrowser> browser, 
    CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) OVERRIDE;
  virtual void OnContextReleased(CefRefPtr<CefBrowser> browser, 
    CefRefPtr<CefFrame> frame, CefRefPtr<CefV8Context> context) OVERRIDE;
  virtual bool OnProcessMessageReceived(CefRefPtr<CefBrowser> browser, 
    CefProcessId source_process, CefRefPtr<CefProcessMessage> message) OVERRIDE; 

  // Virutal on CefV8Handler
  bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, 
    const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, 
    CefString& exception) OVERRIDE;// break this into multiple handlers

 private:
  CefRefPtr<CefBrowser> browser;

  // Include the default reference counting implementation.
  IMPLEMENT_REFCOUNTING(SimpleApp);
};

#endif  // CEF_TESTS_CEFSIMPLE_SIMPLE_APP_H_
