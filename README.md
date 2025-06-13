# Concurrent LED Control

## Overview

Concurrent_LED_Control is a FreeRTOS-based firmware project for the STM32 microcontroller. It demonstrates multitasking and synchronization concepts by controlling multiple LEDs and reading a user button input. The project uses FreeRTOS tasks, task notifications, and mutexes to safely toggle LEDs based on button presses.

## Features

- Reads user button state with a dedicated task
- Controls multiple LEDs (Green, Orange, Red, Blue) with separate tasks
- Uses task notifications to communicate button events
- Employs mutex for safe concurrent LED toggling
- Demonstrates FreeRTOS synchronization primitives: semaphores and notifications

## Hardware

- STM32 microcontroller (e.g., STM32F4 series)
- User button connected to a GPIO pin
- LEDs connected to GPIO pins (Green, Orange, Red, Blue)

## How to Build

1. Use STM32CubeMX or STM32CubeIDE to configure your microcontroller clock and GPIOs as in the project.  
2. Import the source code files into your project workspace.  
3. Make sure FreeRTOS middleware is included and configured.  
4. Build the project and flash it to your STM32 device.

## How it Works

- **task_a** monitors the user button and sends notifications on state changes.  
- **task_b** controls the Green LED based on button state.  
- **task_c** toggles the Orange LED 10 times when notified.  
- **task_d** toggles the Red LED continuously when the button is pressed, using a mutex for safe access.  
- **task_e** toggles the Blue LED only when toggling is allowed (button released), also protected by mutex.




