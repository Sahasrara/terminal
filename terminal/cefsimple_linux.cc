// Copyright (c) 2013 The Chromium Embedded Framework Authors. All rights
// reserved. Use of this source code is governed by a BSD-style license that
// can be found in the LICENSE file.

#include "simple_app.h"

#include <X11/Xlib.h>

#include "include/base/cef_logging.h"

namespace {

int XErrorHandlerImpl(Display *display, XErrorEvent *event) {
  LOG(WARNING)
        << "X error received: "
        << "type " << event->type << ", "
        << "serial " << event->serial << ", "
        << "error_code " << static_cast<int>(event->error_code) << ", "
        << "request_code " << static_cast<int>(event->request_code) << ", "
        << "minor_code " << static_cast<int>(event->minor_code);
  return 0;
}

int XIOErrorHandlerImpl(Display *display) {
  return 0;
}

}  // namespace


// Entry point function for all processes.
int main(int argc, char* argv[]) {
  // Provide CEF with command-line arguments.
  CefMainArgs main_args(argc, argv);
  
  // CEF applications have multiple sub-processes (render, plugin, GPU, etc)
  // that share the same executable. This function checks the command-line and,
  // if this is a sub-process, executes the appropriate logic.
  int exit_code = CefExecuteProcess(main_args, NULL, NULL);
  if (exit_code >= 0) {
    // The sub-process has completed so return here.
    return exit_code;
  }

  // Install xlib error handlers so that the application won't be terminated
  // on non-fatal errors.
  XSetErrorHandler(XErrorHandlerImpl);
  XSetIOErrorHandler(XIOErrorHandlerImpl);

  // Specify CEF global settings here.
  CefSettings settings;
  settings.remote_debugging_port = 8088;
/*

    single_process Set to true to use a single process for the browser and renderer. Also configurable using the "single-process" command-line switch. See the “Processes” section for more information.
    
    browser_subprocess_path The path to a separate executable that will be launched for sub-processes. See the “Separate Sub-Process Executable” section for more information.
    
    multi_threaded_message_loop Set to true to have the browser process message loop run in a separate thread. See the “Message Loop Integration” section for more information.
    
    command_line_args_disabled Set to true to disable configuration of browser process features using standard CEF and Chromium command-line arguments. See the “Command Line Arguments” section for more information.
    
    cache_path The location where cache data will be stored on disk. If empty an in-memory cache will be used for some features and a temporary disk cache will be used for others. HTML5 databases such as localStorage will only persist across sessions if a cache path is specified.
    
    locale The locale string that will be passed to Blink. If empty the default locale of "en-US" will be used. This value is ignored on Linux where locale is determined using environment variable parsing with the precedence order: LANGUAGE, LC_ALL, LC_MESSAGES and LANG. Also configurable using the "lang" command-line switch.
    
    log_file The directory and file name to use for the debug log. If empty, the default name of "debug.log" will be used and the file will be written to the application directory. Also configurable using the "log-file" command-line switch.
    
    log_severity The log severity. Only messages of this severity level or higher will be logged. Also configurable using the "log-severity" command-line switch with a value of "verbose", "info", "warning", "error", "error-report" or "disable".
    
    resources_dir_path The fully qualified path for the resources directory. If this value is empty the cef.pak and/or devtools_resources.pak files must be located in the module directory on Windows/Linux or the app bundle Resources directory on Mac OS X. Also configurable using the "resources-dir-path" command-line switch.
    
    locales_dir_path The fully qualified path for the locales directory. If this value is empty the locales directory must be located in the module directory. This value is ignored on Mac OS X where pack files are always loaded from the app bundle Resources directory. Also configurable using the "locales-dir-path" command-line switch.
    
    remote_debugging_port Set to a value between 1024 and 65535 to enable remote debugging on the specified port. For example, if 8080 is specified the remote debugging URL will be http://localhost:8080. CEF can be remotely debugged from any CEF or Chrome browser window. Also configurable using the "remote-debugging-port" command-line switch.
*/

  // SimpleApp implements application-level callbacks for the browser process.
  // It will create the first browser instance in OnContextInitialized() after
  // CEF has initialized.
  CefRefPtr<SimpleApp> app(new SimpleApp);

  // Initialize CEF for the browser process.
  CefInitialize(main_args, settings, app.get(), NULL);

  // Run the CEF message loop. This will block until CefQuitMessageLoop() is
  // called.
  CefRunMessageLoop();

  // Shut down CEF.
  CefShutdown();

  return 0;
}
