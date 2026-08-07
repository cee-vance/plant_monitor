# ECE/CSE 474: AI Project Context

Fill this in once and reuse it across labs. At the start of a lab, give this file to your
AI assistant (paste it in or attach it) so it knows your board, your tools, and your
conventions before it writes any code. Update it as you go, especially the "Things the AI
keeps getting wrong" list, which saves you time on later labs.

Replace every [bracketed] placeholder with your own information.

---

## Board and hardware

- Board: ESP32-S3 (DevKitC-1). [confirm your exact board and revision]
- Pinout reference: ESP32-S3 DevKitC-1 pin layout. https://docs.espressif.com/projects/esp-dev-kits/en/latest/esp32s3/_images/ESP32-S3_DevKitC-1_pinlayout_v1.1.jpg
- Peripherals I use in this course: external LEDs and 220 Ω resistors, breadboard, I2C
  LCD (PCF8574T backpack, address 0x27), tactile button, photoresistor, passive buzzer.
  [add or trim for the current lab]

## Toolchain

- IDE: Arduino IDE. 2.3.10
- ESP32 Arduino core: esptool v5.3.0
- Serial monitor baud: 9600. 

## Datasheets and references (attach or link these for the AI)

- ESP32-S3 Technical Reference Manual (TRM): https://documentation.espressif.com/esp32-s3_technical_reference_manual_en.pdf
- PCF8574T I2C LCD backpack datasheet: https://www.ti.com/lit/ds/symlink/pcf8574.pdf
- Any peripheral datasheets for the current lab: [link]

> Tip: attach the actual PDF or spec pages when you can. The AI is much more accurate
> about register offsets and bit fields when it can see the datasheet than when it guesses.

## My coding conventions

- ALL_CAPS for macros
- variables names like this var_name
- see this for code style 
- small functions

## Sample code from my past work (paste or link)

Guidelines: https://docs.google.com/document/d/1XPFq0_giamHGzyMvyvkoSTnSWu0A6n2bFp31tw0RZJg/edit?tab=t.0

Example Code:
```cpp
// Filename: lab2_part1_step2.ino
// Authors: Collin Hunt Swam Aung
// Date: 07/05/2026
// Description: This sketch is used to time how 
// long it takes to turn GPIO on/off using registers.
// Prints the results to serial monitor

// ================================ Includes ============================================
#include "driver/gpio.h"
#include "soc/io_mux_reg.h"
#include "soc/gpio_reg.h"
#include "soc/gpio_periph.h"
#include "soc/timer_group_reg.h"

// ================================ Macros ============================================
#define GPIO_PIN 5 // GPIO5
#define REPETITIONS 1000 

void setup() {
  // Set GPIO_PIN function to GPIO using MUX macro
  PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[GPIO_PIN], PIN_FUNC_GPIO);  // configure pin 5 as GPIO

  // Enable GPIO_PIN as output
  *((volatile uint32_t *)GPIO_ENABLE_REG) |= (1 << GPIO_PIN); 

    // setup the serial monitor
  Serial.begin(9600);
}

void loop() {

  Serial.println("-------------------------Timing switching using registers -------------------------\n");
  // initial counter
  int startTime = micros();
  for(int i = 0; i < REPETITIONS; i++) {
    *((volatile uint32_t *)GPIO_OUT_REG) ^= (1 << GPIO_PIN);  // ON
    *((volatile uint32_t *)GPIO_OUT_REG) ^= (1 << GPIO_PIN);  // OFF
  }
  // all done
  int endTime = micros();

  // display the results
  Serial.printf("Registers time: %d\n\n", endTime - startTime );

  delay(1000);

}
```

## Things the AI keeps getting wrong (grow this list every lab)



## How I want the AI to help me

- Be a design partner, not a code vending machine.
- Explain before you code. When I give you a design, critique it and answer my questions
  first.
- Ask me questions when a decision is mine to make (for example, poll vs interrupt).
- Keep solutions small and readable, and tell me why each value is what it is.
- Do not dump a full finished solution unless I ask for it. I need to understand and
  defend every line at the demo.