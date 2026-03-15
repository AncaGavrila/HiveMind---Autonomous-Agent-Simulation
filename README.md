# HiveMind---Autonomous-Agent-Simulation
HiveMind is a C++ simulation system designed to manage autonomous agents within a dynamic city grid. The project focuses on the coordination of specialized entities such as Robots, Drones, and Scooters.

## OOP Implementation
This project was developed as a comprehensive application of all Object-Oriented Programming (OOP) concepts taught throughout the course:

* **Encapsulation**: Private and protected data members are used to secure agent attributes, with access provided through public interfaces and setter/getter methods.
* **Inheritance**: A hierarchical structure is established where specialized classes (Drona, Scooter, Robot) derive from a common base class (Agent) to reuse code and define specific behaviors.
* **Polymorphism**: Virtual functions are implemented to allow the simulation engine to handle different agent types through a unified base class pointer, ensuring dynamic method dispatch at runtime.
* **Abstraction**: High-level simulation logic is decoupled from low-level data management using modular systems for mapping and agent coordination.
* **Composition**: The HiveMindCore system integrates multiple objects, including the CityMap and agent collections, to manage the simulation environment and spatial tracking.

## Technical Features
* **Grid-Based Navigation**: Implements a coordinate system to track agent positions and manage environmental obstacles.
* **Specialized Agent Logic**: Includes distinct movement and energy constraints for aerial agents (Drones) and ground-based units (Robots and Scooters).
* **Centralized Simulation Engine**: Uses HiveMindCore to orchestrate agent updates, collision detection, and overall simulation flow.

## Tech Stack
* **Language**: C++
* **Standard Library**: Utilizes the Standard Template Library (STL) for data management
* **Architectural Paradigm**: Object-Oriented Programming

## Installation and Execution
1. Compile the source code using a C++ compiler:
   ```bash
   g++ -o simulation proiect.cpp
