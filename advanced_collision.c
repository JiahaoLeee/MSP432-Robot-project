
        // Advanced collision handling logic:
        // 1. Center collision (both sides are triggered):
        if (leftCount > 0 && rightCount > 0) {
            // Reverse to create space.
            Motor_Reverse();
            DelayMs(100);  // Short reversal delay
            // Turn toward the side with fewer sensors activated using a large turn.
            if (leftCount > rightCount) {
                Motor_LargeTurn_Right();
            } else if (rightCount > leftCount) {
                Motor_LargeTurn_Left();
            } else {
                // If equal, choose a default (e.g., large left turn).
                Motor_LargeTurn_Left();
            }
        }
        // 2. Collision detected only on the left side.
        else if (leftCount > 0) {
            // If multiple left sensors are triggered, treat as a front collision.
            if (leftCount >= 2) {
                Motor_Reverse();
                DelayMs(100);
                Motor_LargeTurn_Right();
            } else {
                // Single sensor activation: perform a small corrective turn.
                Motor_SmallTurn_Right();
            }
        }
        // 3. Collision detected only on the right side.
        else if (rightCount > 0) {
            if (rightCount >= 2) {
                Motor_Reverse();
                DelayMs(100);
                Motor_LargeTurn_Left();
            } else {
                Motor_SmallTurn_Left();
            }
        }
        else {
            // Fallback: if sensor activation is ambiguous, reverse slightly.
            Motor_Reverse();
        }
        
        // Clear the collision flag after handling.
        collisionOccurred = 0;
    }
