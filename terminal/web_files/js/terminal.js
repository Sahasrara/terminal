// [terminal id] -> terminal div object
var terminals = {};

function openTerminal() {
  // Create Terminal Resources and get ID
  var terminalId = window.TerminalHandler.openTerminal(readCallback);

  // Create Terminal Container
  var newTerminalContainer = document.createElement('div');

  // Create Terminal Display
  var terminalDisplay = document.createElement('div');
  
  // Create Terminal Input
  var terminalInput = document.createElement('input');
  terminalInput.type = "text";

  // Create Submit Button
  var writeButton = document.createElement('button');
  writeButton.innerHTML = "Write";
  writeButton.onclick = function() {
    writeToTerminal(terminalId);
  };

  // Create Close Button
  var closeButton = document.createElement('button');
  closeButton.innerHTML = "Close";
  closeButton.onclick = function() {
    closeTerminal(terminalId);
  };

  // Add everything to the container
  newTerminalContainer.appendChild(terminalDisplay);
  newTerminalContainer.appendChild(terminalInput);
  newTerminalContainer.appendChild(writeButton);
  newTerminalContainer.appendChild(closeButton);

  // Insert the container
  document.getElementById("mainTerminalContainer").appendChild(newTerminalContainer);

  // Store new Terminal in global map
  terminals[terminalId] = {
    "container": newTerminalContainer,
    "display": terminalDisplay,
    "input": terminalInput,
    "write-button": writeButton,
    "close-button": closeButton
  };
}

function writeToTerminal(terminalId) {
  window.TerminalHandler.writeToTerminal(terminalId, terminals[terminalId]["input"].value + "\n");
}

function closeTerminal(terminalId) {
  window.TerminalHandler.closeTerminal(terminalId);
}

function readCallback(terminalId, data) {
  data = data.replace(new RegExp("\n", 'g'), "<br/>");
  terminals[terminalId]["display"].innerHTML += data;
}