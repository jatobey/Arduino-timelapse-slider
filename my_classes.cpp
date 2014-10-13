/*
 * slider OS 2
 * for controlling a timelapse slider and triggering the Camera
 * with an arduino, 3 buttons, 2 7-segment-displays, stepper driver
 *
 * version 2.0.0 beta1 (dd.mm.yyyy)
 * Jannik Beyerstedt, Hamburg, Germany | http://jannikbeyerstedt.de | jtByt.Pictures@gmail.com
 * CC BY-NC-SA 3.0
 */


#include "my_classes.h"


Slider::Slider (SlowImpulses *stepper) {
  stepperInstance = stepper;
  
  maxPosition = maxSteps;
  slideRunning = false;
  slideDir = 1;
}


void Slider::initCarriagePosition(unsigned int pos) {
  carriagePosition = pos;
}


void Slider::setParameters (int operationMode, int travelTime, int triggerInterval, int slidingDirection) {
  mode = operationMode;
  
  slideDir = !slidingDirection; // invert
  
  switch (mode) {
    case 1:  // TL-mode
      // TODO 2.1 - now: workaround by simply reusing the slide mode
      
    case 2:  // SL-mode
      // calculate speed (steps per interval)
      // maxSteps / travelTime [min] * 60 [sek/min] = stepsPerSecond
      stepsPerSecond = maxSteps / travelTime / 60;
          
      // duration of one interval [ms] = triggerInterval [s] * 1000 [ms/s] - triggerDuration [ms]
      intervalDuration = triggerInterval * 1000 - triggerDuration;
      
      if (stepsPerSecond > maxVelocity) {
        stepsPerSecond = maxVelocity;
      }
      
      stepsPerInterval = intervalDuration * stepsPerSecond / 1000;
      
      break;
    case 3:  // CO-mode
      // calculate speed (steps per interval)
      // maxSteps / travelTime [min] * 60 [sek/min] = stepsPerSecond
      stepsPerSecond = maxSteps / travelTime / 60;
      
      intervalDuration = 0;
      
      stepsPerInterval = 0; // TODO: another solution for keeping track of the carriage position
      
      break;
    default:
      Serial.println("ERROR: Slider::setParameters - wrong mode");
      break;
  }
  
  Serial.println("---- configuration of the slide: ----");
  Serial.print("mode ");Serial.println(mode);
  Serial.print("stepsPerSecond ");Serial.println(stepsPerSecond);
  Serial.print("intervalDuration ");Serial.println(intervalDuration);
  Serial.print("stepsPerInterval ");Serial.println(stepsPerInterval);
  Serial.print("direction ");Serial.println(slideDir);
  Serial.println("----- end config slide ----");
  
}

void Slider::startSlide () {
  digitalWrite(stepperSleep, HIGH);
  digitalWrite(stepperDir, slideDir);
  
  // NEW: with slow_impulses
  boolean stepperNoError = stepperInstance->set(stepsPerSecond, intervalDuration);
  
  if (stepperNoError) {
    // workaround for bad position tracking
    if (carriagePosition < 0) {
      carriagePosition = 0;
    }else if (carriagePosition > maxPosition) {
      carriagePosition = maxPosition;
    }
    
    slideRunning = true;
    stepperInstance->start();
    displaySymbol(slideMve);
    
  }else {
    Serial.println("ERROR: slow_impulses::set - wrong frequency");
    slideRunning = false;
  }
  
}


boolean Slider::update() {
  // enable slide an check for virtual endstop
  if (slideRunning == true && carriagePosition < maxPosition && carriagePosition >= 0 ) {
    
    if (stepperInstance->getStatus()) {
      // still running, do nothing
    }else {
      stepperInstance->stop();
        
      camTrigger();
      
      stepperInstance->start();
      
      if (slideDir == 0) { // right
        carriagePosition += stepsPerInterval;
      }else {
        carriagePosition -= stepsPerInterval;
      }
      Serial.print("---- current position: "); Serial.println(carriagePosition);
    }
      
    return true;

  }else if (slideRunning == false) {
    Serial.println("ERROR: slide has not been started");
    return false;
  }else {
    Serial.println("ERROR: virtual endstop hit - slide aborted");
    return false;
  }
}


void Slider::stopSlide () {
  stepperInstance->stop();
}


void Slider::camTrigger () {
  Serial.println("trigger");
  displaySymbol(slideTrg);
  digitalWrite(trigger, HIGH);
  delay(triggerDuration);
  digitalWrite(trigger, LOW);
}





