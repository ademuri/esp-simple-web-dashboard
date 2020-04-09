#!/bin/bash

set -euo pipefail

platformio ci --board esp32dev --lib=. examples/dashboard --project-option="lib_deps=ESP Async WebServer"
platformio ci --board esp12e --lib=. examples/dashboard --project-option="lib_deps=ESP Async WebServer"
