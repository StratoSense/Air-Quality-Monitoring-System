# 🛰️ IoT-Based Indoor Air Quality Monitoring System

## 📘 Overview
This project is an **IoT-based Indoor Air Quality Monitoring System** designed to measure and analyze key environmental parameters such as **temperature, humidity, carbon monoxide (CO), and general air quality (using MQ135)**.  
It leverages the **ESP32 microcontroller** and connects to the **Blynk Cloud Platform** for real-time monitoring, alert notifications, and data visualization.

The system ensures that users are informed about their surrounding air conditions and can take necessary actions to maintain a healthy environment.

---

## ⚙️ System Components
| Component | Description |
|------------|-------------|
| **ESP32** | Main controller with Wi-Fi connectivity |
| **MQ7 Sensor** | Detects carbon monoxide (CO) levels |
| **MQ135 Sensor** | Measures air pollutants (CO₂, NH₃, NOx, etc.) |
| **DHT22 Sensor** | Measures temperature and humidity |
| **SSD1306 OLED Display** | Displays sensor readings locally |
| **Blynk Cloud** | Platform for IoT dashboard and alerts |

---

## 🧩 Features
- 🌡️ Real-time monitoring of temperature, humidity, and air quality  
- ☁️ Cloud data logging and visualization using **Blynk IoT**  
- 🔔 Instant notifications for poor air quality or high CO levels  
- 📱 Mobile app dashboard for remote access  
- 💡 OLED screen for local display  
- ⚡ Low-cost and energy-efficient setup  

---

## 🏗️ System Architecture
![System Diagram](images/system_diagram.png)

---

## 🔌 Circuit Connections
| Component | ESP32 Pin |
|------------|------------|
| DHT22 | D5 |
| MQ7 | A0 |
| MQ135 | A1 |
| OLED SDA | D21 |
| OLED SCL | D22 |

---

## 🧠 Working Principle
1. The **ESP32** reads data from the **DHT22**, **MQ7**, and **MQ135** sensors.  
2. It calculates the **Air Quality Index (AQI)** and categorizes the air quality as *Good*, *Moderate*, or *Poor*.  
3. Sensor data is displayed on the **OLED screen** in real time.  
4. The ESP32 sends the same data to the **Blynk Cloud** using Wi-Fi.  
5. **Blynk Dashboard** visualizes the readings and triggers notifications when the air quality crosses certain thresholds.

---

## 🖥️ Blynk Dashboard Preview
![Blynk Dashboard](images/blynk_dashboard.png)

---

## 🚀 How to Use

### 🧰 Prerequisites
Install the following libraries in Arduino IDE:
- Blynk
- Adafruit GFX
- Adafruit SSD1306
- DHT sensor library
- WiFiManager

### 🪜 Steps
1. Clone this repository:
   ```bash
   git clone https://github.com/yourusername/Air-Quality-Monitoring-System-IoT.git
   ```
2. Open the project in **Arduino IDE**.
3. In the `.ino` file, enter your Wi-Fi credentials and Blynk Auth Token:
   ```cpp
   #define BLYNK_AUTH_TOKEN "YOUR_BLYNK_TOKEN"
   #define WIFI_SSID "YOUR_WIFI_NAME"
   #define WIFI_PASS "YOUR_WIFI_PASSWORD"
   ```
4. Select the correct **ESP32 board** from Tools → Board.
5. Upload the code to your ESP32.
6. Open the **Blynk mobile app** to monitor the readings live!

---

## 📊 Sample Output
```
Temperature: 27.8°C
Humidity: 62%
CO Level: 3.5 ppm
Air Quality Index: 75 (Moderate)
Status: Air quality is acceptable.
```

---

## 🧪 Applications
- Smart home monitoring  
- Industrial safety systems  
- Air quality research  
- Educational IoT projects  

---

## 🧠 Future Enhancements
- 🗣️ Voice assistant integration (Google Assistant / Alexa)  
- ☀️ Solar-powered or battery-based model  
- 📡 Edge computing for offline AQI analysis  
- 📈 Historical data logging with graphs  

---

## 📄 License
This project is licensed under the **MIT License** — see the [LICENSE](LICENSE) file for details.

---

## 👨‍💻 Author
**Adithyan**  
🎓 MCA Student, Amrita Vishwa Vidyapeetham  
💡 Passionate about IoT, Embedded Systems, and Cloud Computing  
📧 [YourEmail@example.com]  

---

## 🌐 Links
- 📘 [Blynk Platform](https://blynk.io)  
- 💻 [Arduino IDE](https://www.arduino.cc/en/software)

---

## 🏁 Project Gallery
| Hardware Setup | Circuit Diagram |
|----------------|----------------|
| ![Setup](images/hardware_setup.jpg) | ![Circuit](docs/flowchart.png) |
