This project was created with the intent to test/learn some features related to ESP32 programming using C++.

It performs no useful tasks but I belive it covers ome interesting concepts such as:

1 - Direct LED Matrix control using SPI (instead of the usual library MD_Parola.

2 - Multi-task scheduling using both of ESP32's cores

3 - Touch interface

4 - Deep sleep and wakeup

5 - Interrupt

6 - Timers

7 - Queues

8 - Mutex for synchrozination

9 - Modern C++ constructs such lambda functions and std::ranges.

How it works:

Whe the MCU starts it creates 5 tasks, a timer, a queue and a semaphore.

4 tasks are used to light up leds in the Matrix following some pattern. After the tasks are started, they wait for a notification to tell them to proceed and send data to the LED Matrix - Tasks don't send data though the SPI bus simultaneously.

1 task is used to read the Queue with data about the pins being touched. This task, gets data from the queue and sends it to the respective task.

After startup if none of the pins 12, 13, 14 or 15 is touched the MCU goes into deep sleep.

After startup, if a pin is touched before the MCU goes into deep sleep, an interrupt is created and a "message" is sent to the Queue.

When a pin is touched the timer is reset and starts to count again.

If the MCU is sleeping and is awaken by the touch of a pin, the task relative to that pin is executed and the timer starts to count again.
