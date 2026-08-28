# \# 🌿 Plant Monitoring and Control System

# 

# \[!\[Demo Video](https://img.youtube.com/vi/iw1WFmKk7-g/0.jpg)](https://youtu.be/iw1WFmKk7-g)

# 

# \---

# 

# \## 🧩 Overview

# 

# This project implements a \*\*real-time plant monitoring and control system\*\* on an \*\*ESP32-S3\*\* microcontroller using \*\*FreeRTOS\*\*.  

# It continuously reads environmental data, logs sensor values, displays status information, and controls actuators such as a pump, vent, and servo based on configurable thresholds.

# 

# The system architecture emphasizes \*\*task isolation\*\*, \*\*queue-based communication\*\*, and \*\*fault-tolerant operation\*\*.  

# Each subsystem runs as an independent FreeRTOS task, coordinated through shared queues and configuration structures.

# 

# \---

# 

# \## 🏗️ System Architecture

# 

# !\[Plant Monitoring and Control System Architecture](./cse474_final_project_system_diagram.jpg)

# 

# \### Core Components

# 

# | Subsystem | Description |

# |------------|--------------|

# | \*\*Sensors Task\*\* | Reads all sensors (temperature, humidity, light, moisture, sound, height) and publishes data to `plant\_data\_queue`. |

# | \*\*Controls Task\*\* | Uses `global\_config` and `plant\_data\_t` to drive actuators (pump, vent, servo) based on threshold or schedule mode. |

# | \*\*Logging Task\*\* | Logs sensor data, performs relevance checks, and triggers alerts via `displayNowQueue`. |

# | \*\*Display Tasks\*\* | `regular\_display\_task` rotates screens; `display\_now\_task` handles alerts and suspends rotation when active. |

# | \*\*User Config Task\*\* | Handles button input and serial commands to update configuration parameters (`config\_msg\_t`). |

# | \*\*Global Config\*\* | Stores thresholds, modes, and timing parameters shared across all tasks. |

# 

# \---

# 

# \## ⚙️ Features

# 

# \- \*\*Multi-tasking architecture\*\* using FreeRTOS  

# \- \*\*Sensor fusion\*\* for environmental monitoring  

# \- \*\*Hysteresis-based watering control\*\*  

# \- \*\*Fault detection and persistent alerts\*\*  

# \- \*\*Configurable thresholds via UI wizard\*\*  

# \- \*\*LCD display with alert override\*\*  

# \- \*\*Modular codebase\*\* (`sensors.cpp`, `actuators.cpp`, `controls.cpp`, `logging.cpp`, `globals.cpp`)  

# 

# \---

# 

# \## 🧠 Task Interaction Summary

# 

# | Task | Input | Output | Notes |

# |------|--------|---------|-------|

# | \*\*Sensors Task\*\* | Hardware sensors | `plant\_data\_queue` | Runs at 50 Hz |

# | \*\*Controls Task\*\* | `global\_config`, `plant\_data\_queue` | Actuator commands | Drives pump/vent |

# | \*\*Logging Task\*\* | `plant\_data\_queue` | `displayNowQueue` | Triggers alerts |

# | \*\*Display Tasks\*\* | `displayNowQueue`, `global\_config` | LCD output | Handles alerts and rotation |

# | \*\*User Config Task\*\* | Button ISR, serial input | `global\_config` | Updates thresholds/modes |

# 

# \---

# 

# \## 🧪 Verification \& Validation

# 

# Each subsystem was tested for:

# \- \*\*Verification:\*\* Successful build and initialization  

# \- \*\*Validation:\*\* Correct runtime behavior under normal, boundary, and fault conditions  

# \- \*\*Performance:\*\* Task deadlines met under load  

# 

# All tests passed during the in-person demo, confirming stable operation and correct fault handling.

# 

# \---

# 

# \## 🛠️ What Was Fixed

# 

# | Issue | Cause | Fix | Evidence |

# |-------|--------|------|-----------|

# | Multiple definition errors | Duplicate includes in `.ino` | Removed redundant includes and cleaned header guards | Build passed with 0 warnings |

# | Truncated source files | Copy/paste corruption | Reconstructed full files (`logging.cpp`, `sensors.cpp`, `actuators.cpp`) | All modules compiled and ran correctly |

# | Control loop oscillation | Blocking delay in task | Replaced with non-blocking timing and retuned hysteresis | Stable watering behavior |

# | UI debounce issues | Missing debounce logic | Added ISR timing and clamping | Buttons responded correctly |

# | Alert repetition | Fault flags not latched | Added persistent fault flags | Alerts persisted until fault cleared |

# 

# \---

# 

# \## 🧾 License

# This project is released under the MIT License.  

# Feel free to use, modify, and build upon it for educational or research purposes.

# 

# \---

# 

# \## 👤 Author

# \*\*C.V. Hunt\*\*  

# EE 474 — Introduction to Embedded Systems  

# University of Washington, Summer 2026



