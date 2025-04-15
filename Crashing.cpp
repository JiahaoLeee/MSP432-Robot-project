//----------------------------------------------------------------------
// Bump Sensor Initialization and Interrupt Setup
//
// Pin Mapping (using negative logic with pullups):
//   Bump5 -> P4.7, Bump4 -> P4.6, Bump3 -> P4.5,
//   Bump2 -> P4.3, Bump1 -> P4.2, Bump0 -> P4.0
//----------------------------------------------------------------------

void Bump_Init(void) {
    // Configure bump sensor pins as inputs with pull-ups
    P4->DIR &= ~0xED;   // bits 7,6,5,3,2,0 are set as inputs
    P4->REN |=  0xED;   // enable pull resistors on these pins
    P4->OUT |=  0xED;   // select pull-ups
    // Configure interrupts for falling edge (sensor press event)
    P4->IES |= 0xED;
    // Clear any pending interrupt flags for bump sensors
    P4->IFG &= ~0xED;
    // Enable interrupts on the bump sensor pins
    P4->IE |= 0xED;

    // Enable Port 4 interrupts in the NVIC (adjust priority as needed)
    NVIC->ISER[1] = 1 << (PORT4_IRQn & 31);
}

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

// Global crash counter for logging (e.g., IoT Dashboard)
volatile uint32_t crashCount = 0;

//----------------------------------------------------------------------
// HandleCollision (Crash Handling)
// Immediately stops the robot, pauses for 0.5 seconds
//----------------------------------------------------------------------

void HandleCollision(void) {
    if (collisionOccurred) {
        collisionOccurred = 0;  // Clear the collision flag
        uint8_t bumpData = collisionData;

        // Log this crash (for subsequent IoT dashboard reporting)
        crashCount++;

        // 1) Stop immediately and pause for 0.5 seconds (500 ms).
        Motor_Stop();
        Clock_Delay1ms(500);


    }
}
