#include "include/cef_app.h"
#include <map>

class RendererTerminal {
  public:
    RendererTerminal(CefRefPtr<CefV8Value> readCallback, CefRefPtr<CefV8Context> context, CefRefPtr<CefBrowser> browser);

    // Get ID of this RendererTerminal
    uint32 getId();

    // Write to terminal
    void write(CefString data);

    // Read from Terminal
    void read(CefString data);

    // Close the terminal
    static void destroy(uint32 terminalId);

    // Destroy All
    static void destroyAll();

    // Get terminal by id
    static RendererTerminal* getTerminal(uint32 terminalId);

  private:
    static uint32 currentId;
    static std::map<uint32,RendererTerminal*> terminalMap;
    void close();
    void open();
    CefRefPtr<CefV8Value> readCallback;
    CefRefPtr<CefV8Context> context;
    CefRefPtr<CefBrowser> browser;
    uint32 id;
};