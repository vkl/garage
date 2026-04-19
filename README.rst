Garage Parking Helper
==========================

Overview
********

A Zephyr-based garage parking helper system featuring Bluetooth connectivity, 
ultrasonic distance sensing, and addressable RGB LED status indicators.

The project includes:

#. **Bluetooth Control**: BLE service for remote garage monitoring and control
#. **Distance Sensing**: HC-SR04 ultrasonic sensor for detecting vehicle position
#. **Visual Feedback**: WS2812 addressable LED strip (32 pixels) for status indication
#. **Persistent Storage**: Non-Volatile Storage (NVS) for configuration parameters
#. **Configurable Thresholds**: Adjustable distance thresholds for vehicle detection

Features
********

- Bluetooth Low Energy (BLE) peripheral device named "GARAGE"
- Real-time distance measurement via HC-SR04 sensor
- Addressable RGB LED strip for visual status feedback
- Flash-based configuration storage with NVS
- System logging and debug capabilities
- Configurable low/high threshold settings (stored in NVS)

Hardware Requirements
*********************

- **Board**: nRF52 series (e.g., nRF52 DK)
- **Sensor**: HC-SR04 Ultrasonic Distance Sensor
- **LED**: WS2812 addressable RGB LED strip
- **Storage**: Flash partition for NVS configuration

Building and Running
********************

Build the project for nRF52 board:

.. code-block:: bash

   west build -b nrf52dk_nrf52832

Flash to device:

.. code-block:: bash

   west flash

Project Structure
*****************

::

   src/
   ├── main.c                 # Main application and sensor handling
   ├── ws2812.c/h             # WS2812 LED strip control
   └── services/
       └── ble_service.c/h    # Bluetooth service implementation

Key Components
**************

**HC-SR04 Sensor**: Measures distance to detect vehicle position

**WS2812 LED Strip**: Provides visual feedback with configurable colors based on distance to vehicle

**BLE Service**: Allows remote monitoring and control via Bluetooth peripheral

**Configuration**: Threshold values are stored in flash memory for persistence across reboots

Bluetooth Service
*****************

The device advertises as "GARAGE" and provides:

- Service UUID: d4864824-54b3-43a1-bc20-978fc376c275
- RX Characteristic UUID: a6e8c460-7eaa-416b-95d4-9dcc084fcf6a
