# 🚀 Air Defense System Simulation (C++ & OpenCV)


## 📜 Overview
This project is a **Real-time Air Defense System Simulation** built from scratch using **C++** and **OpenCV**. It transforms a standard webcam feed into a military-grade Heads-Up Display (HUD).

The system uses **Computer Vision** algorithms to detect and track enemy targets (objects of a specific color) and features a fully functional **Missile Guidance System** based on vector mathematics.

## ✨ Key Features

* **🎯 Computer Vision Tracking:**
    * Utilizes **HSV Color Space** for robust object detection (immune to lighting changes).
    * Noise reduction using morphological operations (Erosion/Dilation).
    * Contour analysis to identify the largest threat.
* **🖥️ Military HUD Interface:**
    * **Unit 777** Egyptian branding theme.
    * Dynamic **Compass Tape** & **Altitude Indicator**.
    * Active **Scanning Radar** visualization.
    * Real-time system logs and status updates.
* **🚀 Missile Physics & Logic:**
    * **Guidance Algorithm:** Uses Vector Math to calculate the trajectory towards the moving target frame-by-frame.
    * **Particle System:** Simulates missile smoke trails and impact explosions.
* **👁️ Thermal Vision Mode:**
    * Simulated IR/Thermal view for the AI processing layer.

## 🛠️ Tech Stack

* **Language:** C++ (Standard 17)
* **Library:** OpenCV 4.x
* **Build System:** CMake

## ⚙️ Installation & Setup

1.  **Prerequisites:**
    * C++ Compiler (MSVC or MinGW).
    * CMake installed.
    * OpenCV installed and configured in your system Path.

2.  **Clone the Repository:**
    ```bash
    git clone [https://github.com/YourUsername/Air-Defense-System.git](https://github.com/YourUsername/Air-Defense-System.git)
    cd Air-Defense-System
    ```

3.  **Build with CMake:**
    * Open the project in **CLion** or **Visual Studio**.
    * Update the `CMakeLists.txt` file to point to your OpenCV build directory if necessary:
        ```cmake
        set(OpenCV_DIR "C:/path/to/opencv/build")
        ```
    * Build and Run.

## 🎮 How to Use

1.  **Launch the System:** Run the compiled executable.
2.  **Acquire Target:** Hold a **Blue Object** (or the color you configured) in front of the camera.
3.  **Wait for Lock:** The system will draw a bounding box and display `LOCK`.
4.  **Fire:** Press **`SPACE`** to launch a missile.
5.  **Impact:** Watch the missile track the target and explode upon impact.
6.  **Exit:** Press `ESC`.

## 🧠 How It Works (The Math)

### 1. The Vision Pipeline
We convert the frame from **BGR** to **HSV** to isolate the target's color. A binary mask is created, processed to remove noise, and then contours are analyzed to find the target's center `(x, y)`.

### 2. Missile Guidance (Vector Math)
When the missile is fired, we calculate the direction vector in every frame:
```cpp
Point diff = targetPos - missilePos; // Vector from missile to target
missilePos += diff * SPEED_FACTOR;   // Move missile along that vector
