# Smart EV Charging & Renewable Energy Management System

## Overview

This project is an ESP32-based simulation of a smart EV charging station integrated with renewable energy management.

The system manages three EV charging ports while considering vehicle state of charge, station power capacity, renewable energy availability, temperature, and emergency conditions.

Power is dynamically allocated between connected vehicles while maintaining a maximum station capacity of 20 kW. The system also integrates simulated solar and grid energy sources and automatically reduces charging power when the available energy falls.

A browser-based dashboard provides real-time monitoring through MQTT communication.

## Features

- Three independent EV charging ports
- Individual vehicle SOC monitoring
- EV connection and disconnection control
- Maximum station capacity of 20 kW
- SOC-based charging priority
- Dynamic power allocation
- Simulated solar and grid energy sources
- Automatic charging reduction during low renewable generation
- Over-temperature protection
- Emergency-stop protection
- Buzzer and fault indication
- ESP32 Wi-Fi connectivity
- MQTT telemetry
- Live browser dashboard
- Real-time system operating states

## Operating States

The system can operate in several states:

- NORMAL — sufficient energy is available and vehicles are charging normally
- PRIORITY — charging power is prioritised according to vehicle SOC
- ENERGY LIMITED — available solar and grid power is insufficient for full charging demand
- FAULT — charging is stopped due to a protection condition

## Energy Sources

The simulated station uses two energy sources:

| Energy Source | Maximum Power |
|---|---:|
| Solar | 8 kW |
| Grid | 12 kW |
| Total Station Capacity | 20 kW |

The ESP32 energy-management logic calculates the available power and distributes it between connected vehicles.

## Charging Priority

Vehicles with a lower state of charge receive higher charging priority.

When total charging demand exceeds the available station power, the controller dynamically reduces and reallocates charging power while maintaining the station power limit.

## Protection Features

### Over-Temperature Protection

Charging is disabled when the simulated system temperature exceeds the configured safety threshold.

### Emergency Stop

The emergency-stop input immediately disables charging operations.

### Fault Indication

Fault conditions are indicated through the system status and buzzer output.

## System Architecture

```text
              SOLAR            GRID
             0–8 kW          0–12 kW
                │               │
                └───────┬───────┘
                        ▼
                ┌───────────────┐
                │ ESP32 ENERGY  │
                │  MANAGEMENT   │
                │    SYSTEM     │
                └───────┬───────┘
                        │
          ┌─────────────┼─────────────┐
          ▼             ▼             ▼
        EV01           EV02           EV03
      SOC/Power      SOC/Power      SOC/Power
          │             │             │
          └─────────────┼─────────────┘
                        │
               Priority Allocation
                        │
          ┌─────────────┼──────────────┐
          ▼             ▼              ▼
    Temperature       E-STOP          OLED
    Protection
                        │
                        ▼
                     Wi-Fi
                        │
                       MQTT
                        │
                        ▼
                 LIVE DASHBOARD
