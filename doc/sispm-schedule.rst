SiS-PM and Gembird Devices
==========================

Supported Models
----------------

This document describes the USB message format for:

* SiS-PM (Gembird SiS-PM)
* mSiS-PM (Gembird mSiS-PM)
* Standard Gembird power supplies with scheduling support

Note: EG-PMS2 devices use a different format documented in eg-pms2.rst

Device Information
-------------------

These devices use a microcontroller (typically Atmel AVR) to store and manage
scheduling data. The scheduling buffer is transferred via USB control transfers.

Schedule Format
===============

The schedule is stored in a 39 byte (0x27) buffer plus 1 padding byte (total 0x28).

Byte 0: Socket Identifier
--------------------------

**Value:** ``3 * socket_number + 1``

Where socket_number is device-specific:

* mSiS-PM_OLD: socket_number = 0, so byte 0 = 1
* mSiS-PM_FLASH: socket_number = 1, so byte 0 = 4
* SiS-PM (4-socket): socket_number = 1-4, so byte 0 = 4, 7, 10, or 13

Bytes 1-4: Initial Timestamp (Double Word, LSB)
-----------------------------------------------

Unix timestamp (seconds since 1970-01-01) of when the schedule was programmed.

Used as reference point for calculating scheduled action times.

Bytes 5-36: Schedule Actions (16 words total)
---------------------------------------------

Each 2-byte word encodes one scheduled action:

* **Bit 15 (MSB):** Action to take
  - 0 = Turn OFF
  - 1 = Turn ON

* **Bits 14-0:** Time interval in minutes

Special Values:

* **0x3FFF (all action bits set):** Empty/unused slot - no action scheduled
* **0xBFFF (ON + action bits set):** Empty/unused slot

When the time value reaches its maximum (0x3FFE or 0x7FFF depending on bit 15),
the device can extend the value using continuation words marked with flag 0x4000.

Time Intervals
~~~~~~~~~~~~~~

* Time values represent **minutes** from the previous action
* Maximum single value: 0x3FFE (16,382 minutes ≈ 11.3 days)
* For longer intervals, continuation words can extend the duration:
  - Continuation words have bit 14 set (0x4000 flag)
  - Each continuation adds up to 0x3FFF minutes

Example multi-word time interval::

  Word with 0x3FFE | ON flag (16,382 min interval)
  Continuation word: 0x4000 | 15000 (adds 15,000 min)
  Continuation word: 0x4000 | 5000  (adds 5,000 min)
  Next action word with new interval

Bytes 37-38: Initial Wait Time
------------------------------

**Type:** 2-byte word (LSB)

Time in minutes to wait before the first scheduled action.

Like the action time intervals, this can extend with continuation words
marked with flag 0x4000 if the value exceeds 0xFD21 (64,801 minutes).

Special value:

* **0xFD21:** Maximum base value; extension continues if needed
* **0x0001:** Minimum wait (1 minute)
* **-1 (0xFFFF):** Delete all schedule entries

Byte 39: Padding
----------------

Unused, filled with 0x00.

Empty Buffer Format
-------------------

A factory-fresh or cleared schedule buffer for mSiS-PM_OLD (socket 0) contains::

  Offset  Value    Description
  ------  -----    -----------------
  0x00    0x01     Socket identifier (socket 0: 3*0+1=1)
  0x01    0x00     Timestamp (all zeros)
  0x02    0x00
  0x03    0x00
  0x04    0x00
  0x05    0xFF     All action slots empty (0x3FFF words)
  0x06    0x3F     (repeated for slots 1-16)
  ...
  0x25    0x01     Initial wait = 1 minute
  0x26    0x00
  0x27    0x00     Padding

For other devices, byte 0 would be different (e.g., 0x04 for mSiS-PM_FLASH socket 1, or 0x04/0x07/0x0A/0x0D for SiS-PM sockets 1-4).

USB Control Transfer
====================

Getting Schedule
----------------

**Request Type:** 0x21 (Device-to-Host)
**Request:** 0x01
**Value:** ``(0x03 << 8) | (3 * socket) + 1``
**Index:** 0
**Length:** 0x28 bytes
**Timeout:** 5000 ms

Setting Schedule
----------------

**Request Type:** 0x21 (Host-to-Device)
**Request:** 0x09
**Value:** ``(0x03 << 8) | (3 * socket) + 1``
**Index:** 0
**Length:** 0x27 bytes (header + first 38 bytes only)
**Timeout:** 5000 ms

Note: The transfer excludes the last padding byte.

Example Schedule
================

Scenario: Turn ON at 9:00 AM, OFF at 5:00 PM, daily repeat

The device stores this as:

1. Initial timestamp when programmed
2. Wait time until next action
3. ON action at calculation time
4. OFF action 8 hours (480 minutes) later
5. Loop continuation for next day

Time calculations are cumulative from the initial timestamp, making the format
efficient for storing repeating daily schedules.
