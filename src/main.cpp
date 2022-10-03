#include <Arduino.h>

/**
 * This file is meant to show how to use the CAN library.
 * First, you need to include the correct library (ESP or Teensy) andinstantiate the CAN bus you will be using.
 *
 */

#if defined(ARDUINO_TEENSY40) || defined(ARDUINO_TEENSY41)
#include "teensy_can.h"
// The bus number is a template argument for Teensy: TeensyCAN<bus_num>
TeensyCAN<1> can_bus{};
#endif

#ifdef ARDUINO_ARCH_ESP32
#include "esp_can.h"
// The tx and rx pins are constructor arguments to ESPCan, which default to TX = 5, RX = 4
ESPCAN can_bus{};
#endif

/**
 * Every CAN message, TX or RX, has signals, which need to be instantiated before the message. You should never put the same signal in multiple messages. The CANSignal class is used to create these signals.
 * The signal type, starting position, length, factor, offset, and signedness of the signal are all templated arguments
 * CANSignal<SignalType, start_position, length, factor (using CANTemplateConvertFloat due to C++ limitations), offset (using CANTemplateConvertFloat due to C++ limitations), is_signed>
 * Note: you should never override the defaults for the other templated arguments
 * There are no constructor arguments
 *
 */
CANSignal<float, 0, 16, CANTemplateConvertFloat(0.01), CANTemplateConvertFloat(0), true> float_tx_signal{};
CANSignal<uint8_t, 16, 8, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0), false> uint8_t_tx_signal{};
CANSignal<bool, 24, 1, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0), false> bool_tx_signal{};
CANSignal<uint32_t, 32, 32, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0), false> millis_tx_signal{};

CANSignal<float, 0, 16, CANTemplateConvertFloat(0.01), CANTemplateConvertFloat(0), true> float_rx_signal{};
CANSignal<uint8_t, 16, 8, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0), false> uint8_t_rx_signal{};
CANSignal<bool, 24, 1, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0), false> bool_rx_signal{};
CANSignal<uint32_t, 32, 32, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0), false> millis_rx_signal{};

/**
 * The CANTXMessage and CANRXMessage classes are used to create messages with signals in them.
 * Both of these classes take the number of signals as a templated argument (CAN*xMessage<num_signals>)
 *
 */

/**
 * CANTXMessage takes the CAN bus to transmit on, the message ID, the message size in bytes (based on the end position of the highest signal), the transmit period, and the signals as arguments
 *
 */
CANTXMessage<4> tx_message{can_bus, 0x100, 8, std::chrono::milliseconds{100}, float_tx_signal, uint8_t_tx_signal, bool_tx_signal, millis_tx_signal};

/**
 * CANRXMessage takes the CAN bus to receive on, the message ID, and the signals to be received as constructor arguments
 * CANRXMessages automatically register themselves with the can_bus on construction
 *
 */
CANRXMessage<4> rx_message{can_bus, 0x200, float_rx_signal, uint8_t_rx_signal, bool_rx_signal, millis_rx_signal};

void setup()
{
  /**
   * The CAN bus(es) need to be initialized with their baud rate
   *
   */
  can_bus.RegisterRXMessage(rx_message);
  can_bus.Initialize(ICAN::BaudRate::kBaud1M);

  Serial.begin(9600);
  Serial.println("Started");
}

std::chrono::milliseconds next_tick_time = std::chrono::milliseconds(millis());
std::chrono::milliseconds kTickPeriod{10};

void loop()
{
  /**
   * You can use and set the CANSignals as if they were SignalType
   *
   */
  float_tx_signal = 5.195f;
  uint8_t_tx_signal = 132;
  bool_tx_signal = true;
  millis_tx_signal = millis();

  float test_float = float_rx_signal;
  uint8_t test_uint8_t = uint8_t_rx_signal;
  bool test_bool = bool_rx_signal;
  uint32_t test_millis = millis_rx_signal;

  can_bus.Tick();

  // The CANRXMessage automatically gets updated on message reception from the interrupt.
  // The CANTXMessage must be periodically ticked with an elapsed time in order to automatically be sent.
  while (std::chrono::milliseconds(millis()) < next_tick_time)
  {
  }
  /* Serial.print("Sent float: ");
  Serial.print(float_tx_signal);
  Serial.print(" Sent uint8_t: ");
  Serial.print(uint8_t_tx_signal);
  Serial.print(" Sent bool: ");
  Serial.print(bool_tx_signal);
  Serial.print(" Sent millis: ");*/
  Serial.println(millis_tx_signal);
  /*
    Serial.print("Received float: ");
    Serial.print(float(test_float));
    Serial.print(" Received uint8_t: ");
    Serial.print(uint8_t(test_uint8_t));
    Serial.print(" Received bool: ");
    Serial.print(bool(test_bool));
    Serial.print(" Received millis: ");
    Serial.println(uint32_t(test_millis)); */

  tx_message.Tick(kTickPeriod);
  next_tick_time =
      std::chrono::milliseconds(millis()) + kTickPeriod - (std::chrono::milliseconds(millis()) - next_tick_time);
}