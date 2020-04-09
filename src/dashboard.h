#ifndef DASHBOARD
#define DASHBOARD

#include <ESPAsyncWebServer.h>

#include <functional>
#include <sstream>

class Dashboard {
 public:
  Dashboard(AsyncWebServer *server);

  // Adds a dashboard entry. The callback is called to get the value to
  // display. The javascript refreshes the value every refresh_millis.
  void Add(std::string name, std::function<std::string()> callback,
           uint32_t refresh_millis);

  // Convenience functions that use standard conversions.
  void Add(std::string name, bool &value_ptr, uint32_t refresh_millis);

  template <typename Type>
  void Add(std::string name, Type &value_ptr, uint32_t refresh_millis);

  template <typename Type>
  void Add(std::string name, std::function<Type()> callback,
           uint32_t refresh_millis);

  // Returns the last time (in millis since startup) that the server was
  // used. If wifi sleep is enabled, the server may be very slow to respond.
  // You can use this to disable wifi sleep, WiFi.setSleep(false) when the
  // dashboard is being used.
  uint32_t last_used();

 private:
  int next_id_ = 0;
  uint32_t last_used_ = 0;

  AsyncWebServer *const server_;
};

template <typename Type>
void Dashboard::Add(std::string name, Type &value_ptr,
                    uint32_t refresh_millis) {
  Add(
      name,
      [&value_ptr]() {
        std::ostringstream stream;
        stream << value_ptr;
        return stream.str();
      },
      refresh_millis);
}

template <typename Type>
void Dashboard::Add(std::string name, std::function<Type()> callback,
                    uint32_t refresh_millis) {
  Add(
      name,
      [callback]() {
        std::ostringstream stream;
        stream << callback();
        return stream.str();
      },
      refresh_millis);
}

#endif  // DASHBOARD
