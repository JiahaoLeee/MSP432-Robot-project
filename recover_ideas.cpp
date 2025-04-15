
        // Advanced collision logic(if needed)
        if ((leftCount > 0) || (rightCount > 0)) {
            // If both sides have sensors triggered, treat it as a center collision
            Motor_Backward(3000, 3000);
            Clock_Delay1ms(1000);
            // Turn toward whichever side had fewer triggers
            if (leftCount > rightCount) {
                Motor_Right(3000, 3000);
                Clock_Delay1ms(1000);
            } else if (rightCount > leftCount) {
                Motor_Left(3000, 3000);
                Clock_Delay1ms(1000);
            } else {
                // If equal, pick a default side
                Motor_Left(3000, 3000);
                Clock_Delay1ms(1000);
            }
        } else if (leftCount > 0) {
            // Collision only on the left side => turn right
            Motor_Right(3000, 3000);
            Clock_Delay1ms(1000);
        } else if (rightCount > 0) {
            // Collision only on the right side => turn left
            Motor_Left(3000, 3000);
            Clock_Delay1ms(1000);
        } else {
            // Otherwise, do a small reverse
            Motor_Backward(3000, 3000);
            Clock_Delay1ms(1000);
        }

        // Option 2: Reverse, then Turn, then Resume
        // Motor_Backward(3000, 3000);    // Reverse to create space
        // Clock_Delay1ms(500);           // Short pause after reversing
        // Motor_Left(3000, 3000);        // Turn left to avoid obstacle (or Motor_Right)
        // Clock_Delay1ms(1000);          // Execute the turn for a set duration
        // Motor_Forward(3000, 3000);     // Resume forward motion
        
        