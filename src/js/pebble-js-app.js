var channel;
var server;
var hasListener;
var connected;
var websocket;

/* from https://raw.github.com/Skipstone/Skipstone/master/js/appmessage.js */
var appMessageQueue = {
  queue: [],
  numTries: 0,
  maxTries: 5,
  add: function(obj) {
    this.queue.push(obj);
  },
  clear: function() {
    this.queue = [];
  },
  isEmpty: function() {
    return this.queue.length === 0;
  },
  nextMessage: function() {
    return this.isEmpty() ? {} : this.queue[0];
  },
  send: function() {
    if (this.queue.length > 0) {
      var ack = function() {
        appMessageQueue.numTries = 0;
        appMessageQueue.queue.shift();
        appMessageQueue.send();
      };
      var nack = function() {
        appMessageQueue.numTries++;
        appMessageQueue.send();
      };
      if (this.numTries >= this.maxTries) {
        console.log('Failed sending AppMessage: ' + JSON.stringify(this.nextMessage()));
        ack();
      }
      console.log('Sending AppMessage: ' + JSON.stringify(this.nextMessage()));
      Pebble.sendAppMessage(this.nextMessage(), ack, nack);
    }
  }
};

/* from https://raw.github.com/Skipstone/Skipstone/master/js/appmessage.js */
var wsMessageQueue = {
  queue: [],
  add: function(obj) {
    this.queue.push(obj);
  },
  addFront: function(obg) {

  },
  clear: function() {
    this.queue = [];
  },
  isEmpty: function() {
    return this.queue.length === 0;
  },
  nextMessage: function() {
    return this.isEmpty() ? {} : this.queue[0];
  },
  send: function() {
    if (connected === false) {
      connectWebsocket();
      return;
    }

    while (this.queue.length > 0) {
      websocketSend(this.nextMessage());
      wsMessageQueue.queue.shift();
    }
  }
};


function connectWebsocket() { 
  if (websocket !== undefined && websocket.readyState === WebSocket.CONNECTING) {
    console.log("websocket already connecting\n");
    return;
  } else if (websocket !== undefined && websocket !== null) {
    websocket.close();
    websocket = null;
  }

  connected = false;
  hasListener = false;
  websocket = new WebSocket(server);
  websocket.onopen = function(event) {
    connected = true;
    console.log("websocket opened");
    onOpen(event); 
  };
  websocket.onclose = function(event) {
    connected = false;
    console.log("websocket closed");
    onClose(event); 
  };
  websocket.onmessage = function(event) {
    connected = true;
    console.log("message received");
    onMessage(event); 
  };
  websocket.onerror = function(event) {
    connected = false;
    console.log("websocket errored");
    onError(event); 
  };
}

function isset(obj) {
  return (typeof obj != "undefined");
}

function handleWebStatus(status) {
  //nothing for now
}

function handleWebMessage(message) {
  //nothing for now
}

function onOpen(event) {
  var message = formatRegistrationMsg(event);
  
  //has to be at front of queue, this bypasses it.
  websocketSend(message);

  wsMessageQueue.send();

  appMessageQueue.add({_connected:true});
  appMessageQueue.send();
}

function formatRegistrationMsg(event) {
  var msg = JSON.stringify({
    "type": "registration",
    "registration": {
      "isListener": false,
      "channel": channel,
    }
  });
  return msg;
}

/* connection closed by server */
function onClose(event) {
  hasListener = false;

  appMessageQueue.add({_connected:false});
  appMessageQueue.send();
}

/* message is received */
function onMessage(event) {
  var msg = JSON.parse(event.data);

  switch (msg.type) {
    case "status":
      handleWebStatus(msg.status);
      break;
    case "message":
      handleWebMessage(msg.message);
      break;
    case "info":
      handleInfo(msg.info);
      break;
    default:
      console.log("Unknown message type\n----------------");
      console.log(msg);
      console.log("----------------\n");
      break;
  }

  if (typeof(msg.status) != "undefined") {
    if (msg.status.connected === false) {
      hasListener = false;

      //get rid of this later on, possibly go to menu and only show one table view 
      //cell with a failed to connect cell
      Pebble.showSimpleNotificationOnPebble('SimonRemote', msg.status.message);

    } else {
      hasListener = true;
    }
  }
}

/* server crashes */
function onError(event) {
  hasListener = false;
}

function handleInfo(msg) {
  console.log(JSON.stringify(msg));
  appMessageQueue.add(msg);
  appMessageQueue.send();
}

function formatCommand(msg) {
  var app = msg._app;
  var command = msg._command;

  if (typeof(app) == "undefined" || typeof(command) == "undefined") 
    return;

  var message = JSON.stringify({
    "idTo": 0,
    "type": "message",
    "message": {
      "app": app,
      "command": command
    }
  });
  return message;
}

function websocketSend(message) {
  console.log('Sending websocket message: ' + JSON.stringify(message));
  websocket.send(message);
}


Pebble.addEventListener("ready", function() {
  console.log("javascript ready");
  server = "ws://simon-server.tyhoff.com/";
  connected = false;
  channel = window.localStorage.getItem('code') || "";
  console.log("channel: " + channel);
  if (channel === "") {
    Pebble.showSimpleNotificationOnPebble('SimonRemote', "Please add channel in app settings.");
  }
  connectWebsocket();
});

Pebble.addEventListener("appmessage", function(e) {
  console.log('AppMessage received from Pebble: ' + JSON.stringify(e.payload));
  
  // if not connect, connect, then send all queued messages
  var command = formatCommand(e.payload);
  wsMessageQueue.add(command);
  if (connected === false) {
    connectWebsocket();
  } else {
    wsMessageQueue.send();
  }
});

Pebble.addEventListener("showConfiguration", function (e) {
	Pebble.openURL("http://simonremote.github.io/pebbleconfig.html");
});

Pebble.addEventListener("webviewclosed", function (e) {
	if (!e.response) return;
	var payload = JSON.parse(e.response);
  channel = payload.code;
  window.localStorage.setItem("code",channel);
  connectWebsocket();
});