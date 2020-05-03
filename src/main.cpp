#include <Arduino.h>

#include "sensesp_app.h"
#include "sensors/analog_input.h"
#include "transforms/linear.h"
#include "signalk/signalk_output.h"

// SensESP builds upon the ReactESP framework. Every ReactESP application
// defines an "app" object vs defining a "main()" method.
ReactESP app([] () {

  // Some initialization boilerplate when in debug mode...
  #ifndef SERIAL_DEBUG_DISABLED
  Serial.begin(115200);

  // A small arbitrary delay is required to let the
  // serial port catch up
  delay(100);
  Debug.setSerialEnabled(true);
  #endif


  // Create the global SensESPApp() object.
  sensesp_app = new SensESPApp();


  // The "SignalK path" identifies this sensor to the SignalK server. Leaving
  // this blank would indicate this particular sensor (or transform) does not
  // broadcast SignalK data.
  const char* sk_path = "tanks.freshWater.Starboard.currentLevel,";


  // The "Configuration path" is combined with "/config" to formulate a URL
  // used by the RESTful API for retrieving or setting configuration data.
  // It is ALSO used to specify a path to the SPIFFS file system
  // where configuration data is saved on the MCU board. It should
  // ALWAYS start with a forward slash if specified. If left blank,
  // that indicates this sensor or transform does not have any
  // configuration to save, or that you're not interested in doing
  // run-time configuration.
  const char* analog_in_config_path = "/freshWaterTank_starboard/analog_in";
  const char* linear_config_path = "/freshWaterTank_starboard/linear";
  

  // Create a sensor that is the source of our data, that will be read every 500 ms. 
  // It's a light sensor that's connected to the ESP's AnalogIn pin. When it's dark,
  // the sensor's output (as read by analogRead()) is 120, and when it's bright,
  // the output is 850, for a range of 730.
  uint read_delay = 500;
  
  auto* pAnalogInput = new AnalogInput(read_delay, analog_in_config_path);

  // A Linear transform takes its input, multiplies it by the multiplier, then adds the offset,
  // to calculate its output. In this example, we want to see the final output presented
  // as a percentage, where dark = 0% and bright = 100%. To get a percentage, we use this formula:
  // sensor output * (100 / 730) - 16.44 = output (a percentage from 0 to 100).
  // Dark = 120 * (100 / 730) + (-16.44) = 0%
  // Bright = 850 * (100 / 730) + (-16.44) = 100%
  const float multiplier = 1.0; // 100% divided by 730 = 0.137 "percent per analogRead() unit"
  const float offset = 0.;

  // Wire up the output of the analog input to the Linear transform,
  // and then output the results to the SignalK server.
  pAnalogInput -> connectTo(new Linear(multiplier, offset, linear_config_path))
               -> connectTo(new SKOutputNumber(sk_path));

  // Start the SensESP application running
  sensesp_app->enable();
});