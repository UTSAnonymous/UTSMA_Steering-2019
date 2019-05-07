/*
   UTSME Autonomous 2019 Steering control
   by Hein Wai Leong

   CanBus Library Fork from pawelsky / FlexCAN_Library
   https://github.com/pawelsky/FlexCAN_Library
   Arduino library for CAN on Teensy 3.1, 3.2, 3.5 and 3.6
   By Pawelsky

*/

#include "FlexCAN.h"

#define UART_SERIAL Serial1;

// Canbus setup --------------------------
FlexCAN CANbus(1000000);
const int CANID_STEERING_RX = 0x33;
const int CANID_STEERING_TX = 0x34;
const int CANID_AUTONOMOUS_ENABLED = 0x88;

// Pin Setup -----------------------------
int led = 13;
const int Relay = 14;
int stop_button = 15;

// Steering data -------------------------
int rawByteData = 0;

bool autonomous_stop = false;

static CAN_message_t txmsg_error_steering, rxmsg, tx_autonomous;

#include "FlexCAN.h"

#ifndef __MK20DX256__
#error "Teensy 3.2 is required to run"
#endif

void setup() {
    delay(2000);
    Serial.begin(9600);
    Serial1.begin(19200);
    
    Serial.println(F("Teensy 3.2 - BSPD"));

    //initialize pins
    pinMode(Relay, OUTPUT);
    pinMode(led, OUTPUT);
    pinMode(stop_button, INPUT_PULLUP);


    //CANBUS Setup
    //Can Mask
    CAN_filter_t canMask;
    canMask.id = 0xFFFFFF;
    canMask.rtr = 0;
    canMask.ext = 0;

    //Can Filter
    CAN_filter_t canFilter;

    //Begin CANBUS
    CANbus.begin(canMask);

    canFilter.id = CANID_STEERING_RX;
    CANbus.setFilter(canFilter, 0);
    CANbus.setFilter(canFilter, 1);
    CANbus.setFilter(canFilter, 2);
    CANbus.setFilter(canFilter, 3);
    CANbus.setFilter(canFilter, 4);
    CANbus.setFilter(canFilter, 5);
    CANbus.setFilter(canFilter, 6);
    CANbus.setFilter(canFilter, 7);

    txmsg_error_steering.id = CANID_STEERING_TX;

}
int test = 1;
void loop() {

    if(CANbus.read(rxmsg)) {

        // check if the msg id belongs to steering_rx
        if(rxmsg.id == CANID_STEERING_RX) 
        {

            // check for the enable bit
            if(rxmsg.buf[0] == 1 && digitalRead(stop_button) == LOW)         //-----------------// autonomous enable bit is true
            {    
                digitalWrite(led, HIGH);
                //enable relay 
                digitalWrite(Relay, HIGH);
                
                //get data from canbus
                rawByteData = rxmsg.buf[1];
                rawByteData -= 128;

                //send data to motor controller using UART
                if(rawByteData > -2 && rawByteData < 2)      // straight
                {
                    Serial1.write(170);
                    Serial1.write(224);
                    Serial1.write(1);
                }
                else if(rawByteData <= -2)                     // left
                {
                    // turning anticlockwise (set target low resolution reverse)
                    Serial1.write(170);
                    Serial1.write(224);
                    Serial1.write(abs(rawByteData));
                }
                else if(rawByteData >= 2)                     // right
                {
                    // turning clockwise    (set target low resolution forward)
                    Serial1.write(170);
                    Serial1.write(225);
                    Serial1.write(abs(rawByteData));
                }
            }
            else                         //---------------// autonomous enable bit is false
            {
                //disable relay
                digitalWrite(led, LOW);
                digitalWrite(Relay, LOW);
            }
        }
    }

    // if the button is pressed
    if(digitalRead(stop_button) == HIGH){
        if( autonomous_stop == false){
            
            digitalWrite(Relay, LOW);
            digitalWrite(led, LOW);
    
            // if stop button is pressed, send autonomous bit as high
            for( int i = 0; i < 10; i++){
                tx_autonomous.id = CANID_AUTONOMOUS_ENABLED;
                tx_autonomous.len = 4;
                tx_autonomous.buf[0] = 0;
                tx_autonomous.buf[1] = 0;
                tx_autonomous.buf[2] = 0;
                tx_autonomous.buf[3] = 0;
                CANbus.write(tx_autonomous);

                delay(20);
            }

            autonomous_stop = true;
        }
    }

    //if the button is not pressed
    if(digitalRead(stop_button) == LOW){
        if( autonomous_stop == true){
            
            digitalWrite(Relay, LOW);
            digitalWrite(led, LOW);
    
            // if stop button is pressed, send autonomous bit as high
            for( int i = 0; i < 10; i++){
                tx_autonomous.id = CANID_AUTONOMOUS_ENABLED;
                tx_autonomous.len = 4;
                tx_autonomous.buf[0] = 1;
                tx_autonomous.buf[1] = 1;
                tx_autonomous.buf[2] = 1;
                tx_autonomous.buf[3] = 1;
                CANbus.write(tx_autonomous);

                delay(20);
            }

            autonomous_stop = false;
        }
    }


}
