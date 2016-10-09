#include "browser_terminal.h"
#include "include/wrapper/cef_helpers.h"

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/select.h>

void execsh(int master_fd, int slave_fd);

// Initialize Map
std::map<uint32, BrowserTerminal*> BrowserTerminal::terminalMap;

BrowserTerminal::BrowserTerminal(uint32 id, CefProcessId processId, CefRefPtr<CefBrowser> browser) {
  // Initialize this terminal object
  this->id = id;
  this->processId = processId;
  this->browser = browser;
  this->kill_flag = false;

  // Open Terminal
  this->openTerminal();

  // Store the terminal in the global map
  BrowserTerminal::terminalMap[this->id] = this;
}

void BrowserTerminal::openTerminal() {
  // Open the Master Clone Device /dev/ptmx, return the fd
  int master_fd = posix_openpt(O_RDWR);
  LOG(INFO) << "pt master fd = " << master_fd;

  // Change the permissions and ownership of the slave device
  int grant_success = grantpt(master_fd);
  LOG(INFO) << "Grant success = " << grant_success;

  // Unlock the slave pseudoterminal device corresponding to master_fd
  int unlock_success = unlockpt(master_fd);
  LOG(INFO) << "Unlock success = " << unlock_success;

  // Grab the name of the slave device
  char *slave_name = ptsname(master_fd);
  LOG(INFO) << "Slave name = " << slave_name;

  // Open the slave pseudoterminal device
  int slave_fd = open(slave_name, O_RDWR);
  LOG(INFO) << "Slave fd = " << slave_fd;

  // Exec shell
  switch (this->shell_pid = fork()) {
    case -1:
      printf("Failed to fork\n");
      break;
    case 0:
      // Child
      execsh(master_fd, slave_fd);
      break;
    default:
      // Parent
      close(slave_fd);
  }

  // Store Terminal Resources
  this->master_fd = master_fd;

  // Spawn Thread for Tailing Master Pseudo-Termianl
  this->tailer_thread = std::thread(&BrowserTerminal::tailMasterPseudoTerminal, this);
  LOG(INFO) << "Tailer thread started";
}

// Get ID of this BrowserTerminal
uint32 BrowserTerminal::getId() {
  return this->id;
}

// Write to browser window
void BrowserTerminal::readFromTerminal(CefString data) {
  // Create the message object.
  CefRefPtr<CefProcessMessage> msg = CefProcessMessage::Create("readFromTerminal");

  // Retrieve the argument list object.
  CefRefPtr<CefListValue> args = msg->GetArgumentList();

  // Populate the argument values.
  args->SetInt(0, this->id);
  args->SetString(1, data);

  // Send Message
  this->browser->SendProcessMessage(this->processId, msg);
}

// Write to terminal
void BrowserTerminal::writeToTerminal(CefString data) {
  //TODO
  std::string str = data.ToString();
  LOG(INFO) << "Writing to terminal: " << str;
  ssize_t written = write(this->master_fd, str.c_str(), str.length());
  LOG(INFO) << "Wrote " << written << " to terminal";
}

// Close the terminal
void BrowserTerminal::closeTerminal() {
  // Kill Tailing Thread
  this->kill_flag = true;
  this->tailer_thread.join();
  LOG(INFO) << "Tailer finished";

  // Close Terminal
  close(this->master_fd);

  // Kill Child
  kill(this->shell_pid, SIGKILL);
}

void BrowserTerminal::destroy(uint32 terminalId) {
  // Remove from Global Map
  std::map<uint32,BrowserTerminal*>::iterator it = BrowserTerminal::terminalMap.find(terminalId);
  if (it != BrowserTerminal::terminalMap.end()) {
    BrowserTerminal* terminal = std::get<1>(*it);
    BrowserTerminal::terminalMap.erase(it);
    terminal->closeTerminal();
    delete terminal;
  }
}

void BrowserTerminal::destroyAll() {
  std::map<uint32,BrowserTerminal*>::iterator it;
  for(it = BrowserTerminal::terminalMap.begin(); it != BrowserTerminal::terminalMap.end();) {
    BrowserTerminal* terminal = std::get<1>(*it);
    terminal->closeTerminal();
    delete terminal;
    BrowserTerminal::terminalMap.erase(it++);
  }
}

// Get terminal by id
BrowserTerminal* BrowserTerminal::getTerminal(uint32 terminalId) {
  BrowserTerminal* terminal = NULL;

  // Find BrowserTerminal in Map
  std::map<uint32,BrowserTerminal*>::iterator it = 
    BrowserTerminal::terminalMap.find(terminalId);
  if (it != BrowserTerminal::terminalMap.end()) {
    terminal = std::get<1>(*it);
  }

  return terminal;
}

/**
 *  OS Terminal Helpers
 */
void execsh(int master_fd, int slave_fd) { // REFACTOR THIS, LOOK AT ST 
  setsid(); /* create a new process group */
  dup2(slave_fd, STDIN_FILENO); 
  dup2(slave_fd, STDOUT_FILENO);
  dup2(slave_fd, STDERR_FILENO);
  ioctl(slave_fd, TIOCSCTTY, NULL); /* make this the controlling terminal for this process */
  close(slave_fd);
  close(master_fd);

  char *args[3];
  char *envshell = getenv("SHELL");
  args[0] = envshell;
  args[1] = (char*) "-i";
  args[2] = NULL;

  setenv("TERM", "xterm-256color", 1);
  unsetenv("COLUMNS");
  unsetenv("LINES");
  unsetenv("TERMCAP");

  signal(SIGCHLD, SIG_DFL);
  signal(SIGHUP, SIG_DFL);
  signal(SIGINT, SIG_DFL);
  signal(SIGQUIT, SIG_DFL);
  signal(SIGTERM, SIG_DFL);
  signal(SIGALRM, SIG_DFL);

  execvp(args[0], args);
  exit(EXIT_FAILURE);
}

//todo, on failed read (file closed), call exit process
void BrowserTerminal::tailMasterPseudoTerminal() {
  LOG(INFO) << "Tailer thread running for terminal: " << this->master_fd;
  char buffer[4096];

  // Create FD set for read availability monitoring
  fd_set read_fd_set;

  // Create Select Timeout
  struct timeval timeout;
  timeout.tv_sec = 1;
  timeout.tv_usec = 0;

  while (!this->kill_flag) {
    FD_ZERO(&read_fd_set);
    FD_SET(this->master_fd, &read_fd_set);
    int retval = select(this->master_fd + 1, &read_fd_set, NULL, NULL, &timeout);
    if (retval > 0) {
      size_t bytes_read = read(this->master_fd, buffer, sizeof(buffer) - 1);// -1 for null terminator
      if (bytes_read > 0) {
        if (buffer[bytes_read-1] != '\0') {
          LOG(WARNING) << "Adding null terminator";
          buffer[++bytes_read] = '\0';
        }
        std::string str(buffer, bytes_read);
        LOG(INFO) << str;
        this->readFromTerminal(str);
      }
    } else if (retval == -1) {
      LOG(WARNING) << "Failed to select terminal master";//TODO
    }
    // Reset read flag
    FD_ZERO(&read_fd_set);
  }
  LOG(INFO) << "Exiting tailer";
}