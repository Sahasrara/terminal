#include "include/cef_app.h"
#include <thread>
#include <map>

class BrowserTerminal {
  public:
    BrowserTerminal(uint32 id, CefProcessId processId, CefRefPtr<CefBrowser> browser);

    // Get ID of this BrowserTerminal
    uint32 getId();

    // Read to browser window
    void readFromTerminal(CefString data);

    // Write to terminal
    void writeToTerminal(CefString data);

    // Close the terminal
    static void destroy(uint32 terminalId);

    // Destroy All
    static void destroyAll();

    // Get terminal by id
    static BrowserTerminal* getTerminal(uint32 terminalId);

  private:
    static std::map<uint32,BrowserTerminal*> terminalMap;
    CefRefPtr<CefBrowser> browser;
    std::thread tailer_thread;
    bool kill_flag;
    pid_t shell_pid;
    int master_fd;
    void tailMasterPseudoTerminal();
    void closeTerminal();
    void openTerminal();
    uint32 id;
    CefProcessId processId;
};