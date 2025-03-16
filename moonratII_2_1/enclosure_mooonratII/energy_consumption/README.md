# Energy Consumption and Battery Life Report for the Incubator

## 1. Introduction
This document analyzes the energy consumption of an incubator prototype powered by a 12 V circuit. The goal is to determine the energy consumption in watt-hours (Wh) and estimate the operational battery life when using a 6000 mAh, 12 V battery. The prototype includes a display, temperature sensors, and a resistive heating pad controlled by an Arduino R4 WiFi.

---

## 2. System Data

**Battery:**
- Capacity: 6000 mAh (6 Ah)
- Voltage: 12 V

**Current Measurements:**
- **Heating Pad at 100%:** 510 mA  
- **Heating Pad at 0%:** 290 mA  

**Heating Pad Resistance:**
- Measured value: 6.2 Ω

---

## 3. Energy Consumption Calculations

### **Power Consumption at 100% (Resistor Active):**

$P_{\text{100\%}} = V \times I = 12 \, V \times 0.510 \, A = 6.12 \, W$

### **Power Consumption at 0% (Resistor Inactive):**

$P_{\text{0\%}} = V \times I = 12 \, V \times 0.290 \, A = 3.48 \, W$

### **Average Consumption with a 50% Duty Cycle:**

Assuming the resistor is active 50% of the time:

$P_{\text{avg}} = 0.5 \times 6.12 \, W + 0.5 \times 3.48 \, W = 4.80 \, W$

---

## 4. Calculation of the System's Total Resistance

Given that the system has multiple components drawing current, it is helpful to calculate the **equivalent resistance** of the entire circuit using **Ohm's Law**, \( R = \frac{V}{I} \), for each state:

### **Equivalent Resistance at 100% Resistor Activation:**

$R_{\text{100\%}} = \frac{V}{I_{\text{100\%}}} = \frac{12 \, V}{0.510 \, A} \approx 23.53 \, \Omega$

### **Equivalent Resistance at 0% Resistor Activation:**

$R_{\text{0\%}} = \frac{V}{I_{\text{0\%}}} = \frac{12 \, V}{0.290 \, A} \approx 41.38 \, \Omega$

This shows that the circuit includes the heating pad's resistance and internal resistances from other components (e.g., display electronics, sensors, and Arduino). The difference between the two states suggests that the total system resistance decreases when the resistor is active due to the increased current flow.

---

## 5. Available Energy from the Battery

First, we convert the battery capacity from mAh to Ah:

$6,000 \, mAh = 6 \, Ah$

The total energy available is:

$E_{\text{battery}} = 12 \, V \times 6 \, Ah = 72 \, Wh$

---

## 6. Battery Life Calculation

We divide the available energy by the average power consumption to determine the battery life:

$\text{Battery Life} = \frac{E_{\text{battery}}}{P_{\text{avg}}} = \frac{72 \, Wh}{4.80 \, W} = 15 \, \text{hours}$

---

## 7. Verification of the Heating Pad Resistance

The theoretical current through the 6.2 Ω heating pad would be:

$I = \frac{V}{R} = \frac{12 \, V}{6.2 \, \Omega} \approx 1.94 \, A$

Given that your actual measurement is **510 mA**, it is evident that the heating pad is not operating at full power continuously. This could be due to **pulse-width modulation (PWM)** control or system logic designed to maintain a stable temperature without overheating.

# Some plots
## Battery Full 
![alt text](battery_full.png)
## Heating Pad at 100%
![alt text](heatpad100.png)
## Heating Pad at 0%
![alt text](heatpad0.png)
## Average
![alt text](average.png)
## All
![alt text](all.png)
