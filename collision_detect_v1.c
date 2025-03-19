//---------------------------------------------------------------------
// FSM Collision Handling Function
//
// This function should be periodically called by the main FSM loop. It
// examines the stored bump sensor data (collisionData) and then decides on the
// appropriate reaction. The advanced logic here handles multiple simultaneous
// collisions with these guidelines:
//
//   • Center collision (both left and right sensors active):
//       - Reverse first.
//       - Then choose the turn direction based on which side has more sensors activated.
//   • Single-side collisions:
//       - Turn away from the obstacle (e.g., if left sensors are active, turn right).
//   • Corner collisions or ambiguous cases:
//       - Reverse or perform a slight turn as a fallback.
// After handling the collision, the function clears the collision flag.
//--------------------------------------------------





#include <stdint.h>
#include "msp.h"

// Global variables
volatile uint8_t collisionData = 0;    // Stores the bump sensor bitmask
volatile uint8_t collisionOccurred = 0;  // Flag set when a collision is detected


void Motor_Stop(void);
void Motor_Reverse(void);
void Motor_Turn_Left(void);
void Motor_Turn_Right(void);

// Prototype for FSM collision handling function.
void HandleCollision(void);

// Bump Sensor Initialization and Interrupt Setup
//
// Pin Mapping (using negative logic with pull‑ups):
//   Bump5 -> P4.7, Bump4 -> P4.6, Bump3 -> P4.5,
//   Bump2 -> P4.3, Bump1 -> P4.2, Bump0 -> P4.0
//-------------------------------------------------------------------

void Bump_Init(void) {
    // Configure bump sensor pins as inputs with pull-ups
    P4->DIR &= ~((1<<7) | (1<<6) | (1<<5) | (1<<3) | (1<<2) | (1<<0));
    P4->REN |=  ((1<<7) | (1<<6) | (1<<5) | (1<<3) | (1<<2) | (1<<0));
    P4->OUT |=  ((1<<7) | (1<<6) | (1<<5) | (1<<3) | (1<<2) | (1<<0));

    // Configure interrupts for falling edge (sensor press event)
    P4->IES |= ((1<<7) | (1<<6) | (1<<5) | (1<<3) | (1<<2) | (1<<0));

    // Clear any pending interrupt flags for bump sensors
    P4->IFG &= ~((1<<7) | (1<<6) | (1<<5) | (1<<3) | (1<<2) | (1<<0));

    // Enable interrupts on the bump sensor pins
    P4->IE |= ((1<<7) | (1<<6) | (1<<5) | (1<<3) | (1<<2) | (1<<0));

    // Enable Port 4 interrupts in the NVIC (adjust priority as needed)
    NVIC->ISER[1] = 1 << (PORT4_IRQn & 31);
}

//----------------------------------------------------------------------
// Bump Sensor Read Function
//
// Reads the state of the bump sensors and returns a bitmask where each bit
// represents a sensor (pressed sensors are set). Note that the bit positions
// in the returned byte have been shifted for convenience.
//----------------------------------------------------------------------

uint8_t Bump_Read(void) {
    uint8_t val = P4->IN;
    uint8_t bump = 0;
    if ((val & BIT7) == 0) bump |= BIT5; // Bump5 (P4.7)
    if ((val & BIT6) == 0) bump |= BIT4; // Bump4 (P4.6)
    if ((val & BIT5) == 0) bump |= BIT3; // Bump3 (P4.5)
    if ((val & BIT3) == 0) bump |= BIT2; // Bump2 (P4.3)
    if ((val & BIT2) == 0) bump |= BIT1; // Bump1 (P4.2)
    if ((val & BIT0) == 0) bump |= BIT0; // Bump0 (P4.0)
    return bump;
}

//----------------------------------------------------------------------
// Port 4 Interrupt Service Routine (ISR)
//
// This ISR handles bump sensor interrupts. Instead of directly controlling
// motor behavior, it simply stores the sensor data and sets a flag for the FSM.
// It also changes the edge detection for each pressed sensor to rising edge
// (to capture the release event).
//----------------------------------------------------------------------

void PORT4_IRQHandler(void) {
    // Identify bump sensor pins that triggered the interrupt.
    uint8_t status = P4->IFG & ((1<<7) | (1<<6) | (1<<5) | (1<<3) | (1<<2) | (1<<0));
    // Clear the interrupt flags.
    P4->IFG &= ~status;

    // Read the current bump sensor state.
    uint8_t bumpData = Bump_Read();

    // If any sensor is pressed, update the global collision variables.
    if (bumpData) {
        collisionData = bumpData;    // Store bump sensor data.
        collisionOccurred = 1;       // Set collision flag.

        // Switch edge detection to rising edge for pressed sensors:
        if (bumpData & BIT5) { P4->IES &= ~BIT7; } // For Bump5 (P4.7)
        if (bumpData & BIT4) { P4->IES &= ~BIT6; } // For Bump4 (P4.6)
        if (bumpData & BIT3) { P4->IES &= ~BIT5; } // For Bump3 (P4.5)
        if (bumpData & BIT2) { P4->IES &= ~BIT3; } // For Bump2 (P4.3)
        if (bumpData & BIT1) { P4->IES &= ~BIT2; } // For Bump1 (P4.2)
        if (bumpData & BIT0) { P4->IES &= ~BIT0; } // For Bump0 (P4.0)
    } else {
        // If no sensors are pressed (release event), reset edge detection.
        P4->IES |= ((1<<7) | (1<<6) | (1<<5) | (1<<3) | (1<<2) | (1<<0));
        collisionOccurred = 0; // Clear collision flag.
    }
}


void HandleCollision(void) {
    if (collisionOccurred) {
        uint8_t bumpData = collisionData;
        
        // Stop any current motion.
        Motor_Stop();
        
        // Define sensor groups:
        // Left sensors: Bump2, Bump1, Bump0 (bits BIT2, BIT1, BIT0)
        // Right sensors: Bump5, Bump4, Bump3 (bits BIT5, BIT4, BIT3)
        uint8_t leftSensors = bumpData & (BIT2 | BIT1 | BIT0);
        uint8_t rightSensors = bumpData & (BIT5 | BIT4 | BIT3);
        
        // Count active sensors on each side.
        int leftCount = 0, rightCount = 0;
        if (leftSensors & BIT2) leftCount++;
        if (leftSensors & BIT1) leftCount++;
        if (leftSensors & BIT0) leftCount++;
        if (rightSensors & BIT5) rightCount++;
        if (rightSensors & BIT4) rightCount++;
        if (rightSensors & BIT3) rightCount++;

        // Advanced decision logic:
        if (leftCount > 0 && rightCount > 0) {
            // Center collision: both sides are impacted.
            // First, reverse to create space.
            Motor_Reverse();
            
            // Then choose a turn direction:
            // Turn toward the side with fewer sensors activated.
            if (leftCount > rightCount) {
                Motor_Turn_Right();
            } else if (rightCount > leftCount) {
                Motor_Turn_Left();
            } else {
                // If equal, use a default strategy
                Motor_Turn_Left();
            }
        } else if (leftCount > 0) {
            // Collision detected only on the left side:
            // Turn right to avoid the obstacle.
            Motor_Turn_Right();
        } else if (rightCount > 0) {
            // Collision detected only on the right side:
            // Turn left to avoid the obstacle.
            Motor_Turn_Left();
        } else {
            // For any ambiguous or minor sensor activation, consider a small reverse.
            Motor_Reverse();
        }
        
        // Clear the collision flag after processing.
        collisionOccurred = 0;
    }
}
