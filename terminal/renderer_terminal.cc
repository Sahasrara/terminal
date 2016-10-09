#include "renderer_terminal.h"

#include "include/wrapper/cef_helpers.h"///TODO REMOVE
// Initialize currentId
uint32 RendererTerminal::currentId = 0;
// Initialize Map
std::map<uint32, RendererTerminal*> RendererTerminal::terminalMap;

RendererTerminal::RendererTerminal(CefRefPtr<CefV8Value> readCallback, 
  CefRefPtr<CefV8Context> context, CefRefPtr<CefBrowser> browser) {
  // Initialize this terminal object
  this->readCallback = readCallback;
  this->context = context;
  this->browser = browser;
  this->id = RendererTerminal::currentId++;

  // Open Terminal
  this->open();

  // Store the terminal in the global map
  RendererTerminal::terminalMap[this->id] = this;
}

void RendererTerminal::open() {
  // Create the message object.
  CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("openTerminal");

  // Retrieve the argument list object.
  CefRefPtr<CefListValue> args = msg->GetArgumentList();

  // Populate the argument values.
  args->SetInt(0, this->id);

  // Send Message
  this->browser->SendProcessMessage(PID_BROWSER, msg);
}

// Get ID of this RendererTerminal
uint32 RendererTerminal::getId() {
  return this->id;
}

// Write to terminal
void RendererTerminal::write(CefString data) {
  // Create the message object.
  CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("writeToTerminal");

  // Retrieve the argument list object.
  CefRefPtr<CefListValue> args = msg->GetArgumentList();

  // Populate the argument values.
  args->SetInt(0, this->id);
  args->SetString(1, data);

  // Send Message
  this->browser->SendProcessMessage(PID_BROWSER, msg);
}

// Read from terminal
void RendererTerminal::read(CefString data) {
  CefRefPtr<CefV8Value> retval;
  CefRefPtr<CefV8Exception> exception;
  CefRefPtr<CefV8Context> context = this->context;
  // Construct arguments to function
  CefV8ValueList args;
  args.push_back(CefV8Value::CreateUInt(this->id));
  args.push_back(CefV8Value::CreateString(data));
  this->readCallback->ExecuteFunctionWithContext(context, NULL, args);
}

// Close the terminal
void RendererTerminal::close() {
  // Create the message object.
  CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("closeTerminal");

  // Retrieve the argument list object.
  CefRefPtr<CefListValue> args = msg->GetArgumentList();

  // Populate the argument values.
  args->SetInt(0, this->id);

  // Send Message
  this->browser->SendProcessMessage(PID_BROWSER, msg);
}

void RendererTerminal::destroy(uint32 terminalId) {
  // Remove from Global Map
  std::map<uint32,RendererTerminal*>::iterator it = RendererTerminal::terminalMap.find(terminalId);
  if (it != RendererTerminal::terminalMap.end()) {
    RendererTerminal* terminal = std::get<1>(*it);
    RendererTerminal::terminalMap.erase(it);
    terminal->close();
    delete terminal;
  }
}

void RendererTerminal::destroyAll() {
  LOG(INFO) << "destory all";
  std::map<uint32,RendererTerminal*>::iterator it;
  for(it = RendererTerminal::terminalMap.begin(); it != RendererTerminal::terminalMap.end();) {
    RendererTerminal* terminal = std::get<1>(*it);
    terminal->close();
    delete terminal;
    RendererTerminal::terminalMap.erase(it++);
  }
}

// Get terminal by id
RendererTerminal* RendererTerminal::getTerminal(uint32 terminalId) {
  RendererTerminal* terminal = NULL;

  // Find RendererTerminal in Map
  std::map<uint32,RendererTerminal*>::iterator it = 
    RendererTerminal::terminalMap.find(terminalId);
  if (it != RendererTerminal::terminalMap.end()) {
    terminal = std::get<1>(*it);
  }

  return terminal;
}