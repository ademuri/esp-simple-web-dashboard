#include "dashboard.h"

#include <string>

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    html {
     font-family: Arial;
     font-size: 2.0rem;
    }
    .label {
      font-style: italic;
    }
    .error {
      background-color: red;
    }
    .activity {
      font-size: 0.5rem;
      margin: 0.4rem;
    }
  </style>
</head>
<body>
  <h2>ESP32 Dashboard</h2>
  <div>
    %LIST%
  </div>
</body>
<script>
function showActivity(id) {
  document.getElementById('activity-' + id).innerHTML = '^';
  setTimeout(function() {
    document.getElementById('activity-' + id).innerHTML = '';
  }, 500);
}

function getValue(id) {
  var xhttp = new XMLHttpRequest();
  xhttp.timeout = 10 * 1000;
  xhttp.onload = function() {
    document.getElementById("value-" + id).innerHTML = this.responseText;
    document.getElementById("value-" + id).classList.remove('error');
    showActivity(id);
  };
  let errorFn = function() {
    document.getElementById("value-" + id).classList.add('error');
  };
  xhttp.onerror = errorFn;
  xhttp.onabort = errorFn;
  xhttp.ontimeout = errorFn;
  xhttp.open("GET", "/get-" + id, true);
  xhttp.send();
}

%SCRIPT%

</script>
</html>)rawliteral";

String list;
String script;

String processor(const String &var) {
  if (var == "LIST") {
    return list;
  } else if (var == "SCRIPT") {
    return script;
  } else {
    Serial.println("Warning: unhandled template string: " + var);
  }

  return String();
}

void Dashboard::Add(std::string name, std::function<std::string()> callback,
                    uint32_t refresh_millis) {
  // Add element to template
  std::ostringstream list_stream;
  list_stream << "<div><span class='label'>" << name
              << "</span>: <span id='value-" << std::dec << next_id_
              << "'></span><span class='activity' id='activity-" << next_id_
              << "'></span></div>\n";
  list += list_stream.str().c_str();

  std::ostringstream script_stream;
  script_stream << "getValue('" << std::dec << next_id_ << "');\n";
  script_stream << "setInterval(getValue.bind(undefined, '" << std::dec
                << next_id_ << "'), " << refresh_millis << ");\n";
  script += script_stream.str().c_str();

  // Register endpoint for getting value
  String path = String("/get-") + String(next_id_);
  server_->on(path.c_str(), HTTP_GET,
              [callback, this](AsyncWebServerRequest *request) {
                last_used_ = millis();
                request->send_P(200, "text/plain", callback().c_str());
              });

  next_id_++;
}

void Dashboard::Add(std::string name, bool &value_ptr,
                    uint32_t refresh_millis) {
  Add(
      name, [&value_ptr]() { return value_ptr ? "true" : "false"; },
      refresh_millis);
}

uint32_t Dashboard::last_used() { return last_used_; }

Dashboard::Dashboard(AsyncWebServer *server) : server_(server) {
  server->on("/", HTTP_GET, [this](AsyncWebServerRequest *request) {
    last_used_ = millis();
    request->send_P(200, "text/html", index_html, processor);
  });
}
