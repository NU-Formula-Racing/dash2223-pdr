#include <Arduino.h>

/**
 * This file is meant to show how to use the CAN library.
 * First, you need to include the correct library (ESP or Teensy) andinstantiate the CAN bus you will be using.
 *
 */

#include "esp_can.h"
// The tx and rx pins are constructor arguments to ESPCan, which default to TX = 5, RX = 4
ESPCAN can_bus{};

/**
 * @brief You also need to include the VirtualTimers library in order to use VirtualTimers and VirtualTimerGroups
 *
 */
#include "virtualTimer.h"

// Make a VirtualTimerGroup to add your timers to
VirtualTimerGroup timer_group{};

/**
 * Every CAN message, TX or RX, has signals, which need to be instantiated before the message. You should never put the
 * same signal in multiple messages. The CANSignal class is used to create these signals. The signal type, starting
 * position, length, factor, offset, and signedness of the signal are all templated arguments CANSignal<SignalType,
 * start_position, length, factor (using CANTemplateConvertFloat due to C++ limitations), offset (using
 * CANTemplateConvertFloat due to C++ limitations), is_signed> Note: you should never override the defaults for the
 * other templated arguments There are no constructor arguments
 *
 */
CANSignal<uint32_t, 32, 32, CANTemplateConvertFloat(1), CANTemplateConvertFloat(0), false> millis_tx_signal{};

/**
 * The CANTXMessage and CANRXMessage classes are used to create messages with signals in them.
 * Both of these classes take the number of signals as a templated argument (CAN*xMessage<num_signals>)
 *
 */

/**
 * CANTXMessage takes the CAN bus to transmit on, the message ID, the message size in bytes (based on the end position
 * of the highest signal), the transmit period, and the signals as arguments
 * CANTXMessage can also take a VirtualTimerGroup to add its transmit timer to
 *
 */
CANTXMessage<1> tx_message{can_bus, 0x100, 8, 100, timer_group, millis_tx_signal};

// You should make a function to do anything that needs to be periodic and run it with a VirtualTimer in a TimerGroup
void ten_ms_task()
{
    /**
     * You can use and set the CANSignals as if they were SignalType
     *
     */
    millis_tx_signal = millis() / 1000;

    // The CANRXMessage automatically gets updated on message reception from the interrupt. Not: in order for this to
    // work, you must periodically call Tick() on the can_bus
    can_bus.Tick();
}

void setup()
{
    /**
     * The CAN bus(es) need to be initialized with their baud rate
     *
     */
    can_bus.Initialize(ICAN::BaudRate::kBaud1M);

    // You can create a new timer in a VirtualTimerGroup using the AddTimer(function, time) function
    timer_group.AddTimer(1000, ten_ms_task);

    Serial.begin(9600);
    Serial.println("Started");
}

void loop() { timer_group.Tick(millis()); }