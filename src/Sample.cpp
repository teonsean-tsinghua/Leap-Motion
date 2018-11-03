/******************************************************************************\
* Copyright (C) 2012-2014 Leap Motion, Inc. All rights reserved.               *
* Leap Motion proprietary and confidential. Not for distribution.              *
* Use subject to the terms of the Leap Motion SDK Agreement available at       *
* https://developer.leapmotion.com/sdk_agreement, or another agreement         *
* between Leap Motion and you, your company or other organization.             *
\******************************************************************************/

#include <iostream>
#include <cstring>
#include "Leap.h"

#define TAP_THRESHOLD 300.0;
bool FINGER_LOCK[10];

using namespace Leap;

class SampleListener : public Listener {
  public:
    virtual void onInit(const Controller&);
    virtual void onConnect(const Controller&);
    virtual void onDisconnect(const Controller&);
    virtual void onExit(const Controller&);
    virtual void onFrame(const Controller&);
    virtual void onFocusGained(const Controller&);
    virtual void onFocusLost(const Controller&);
    virtual void onDeviceChange(const Controller&);
    virtual void onServiceConnect(const Controller&);
    virtual void onServiceDisconnect(const Controller&);

  private:
};

const std::string fingerNames[] = {"Thumb", "Index", "Middle", "Ring", "Pinky"};
const std::string boneNames[] = {"Metacarpal", "Proximal", "Middle", "Distal"};
const std::string stateNames[] = {"STATE_INVALID", "STATE_START", "STATE_UPDATE", "STATE_END"};

void SampleListener::onInit(const Controller& controller) {
  std::cout << "Initialized" << std::endl;
}

void SampleListener::onConnect(const Controller& controller) {
  std::cout << "Connected" << std::endl;
  controller.enableGesture(Gesture::TYPE_CIRCLE);
  controller.enableGesture(Gesture::TYPE_KEY_TAP);
  controller.enableGesture(Gesture::TYPE_SCREEN_TAP);
  controller.enableGesture(Gesture::TYPE_SWIPE);

  // config velocity
  controller.config().setFloat("Gesture.KeyTap.MinDownVelocity", 40.0);
  controller.config().setFloat("Gesture.KeyTap.HistorySeconds", .2);
  controller.config().setFloat("Gesture.KeyTap.MinDistance", 8.0);
  controller.config().save();
}

void SampleListener::onDisconnect(const Controller& controller) {
  // Note: not dispatched when running in a debugger.
  std::cout << "Disconnected" << std::endl;
}

void SampleListener::onExit(const Controller& controller) {
  std::cout << "Exited" << std::endl;
}

bool is_finger_locked() {
  for (int x=0; x<10; x++) {
    if (FINGER_LOCK[x]) return true;
  }
  return false;
}

void SampleListener::onFrame(const Controller& controller) {
  // Get the most recent frame and report some basic information
  const Frame frame = controller.frame();

  // if (frame.fingers().extended().count() != 1) return; // Only track one extended finger

  HandList hands = frame.hands();
  for (HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) {

    const Hand hand = *hl;

    const FingerList fingers = hand.fingers();
    for (FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) {
      const Finger finger = *fl;

        // allow hand to determine offset value
        int finger_lock_offset = hand.isLeft() ? 5 : 0;
        finger_lock_offset += finger.type();
        int fingerDownVelocity = finger.tipVelocity()[1];
        // if the downward velocity exceeds 150
        if (fingerDownVelocity < -150.0) {

          // and if the we haven't locked in on a finger
          if (!is_finger_locked()) {

            int finger_x_position = finger.tipPosition()[0];
            std::string letter;

            if (finger.type() == 0) {
              std::cout << "' ' " << fingerNames[finger.type()] << " Velocity: " << fingerDownVelocity << std::endl;
            } else {
              if (finger_x_position < -90) {
                letter = "a";
              } else if (finger_x_position < -70) {
                letter = "s";
              } else if (finger_x_position < -50) {
                letter = "d";
              } else if (finger_x_position < -30) {
                letter = "f";
              } else if (finger_x_position < -10) {
                letter = "g";
              } else if (finger_x_position < 10) {
                letter = "h";
              } else if (finger_x_position < 30) {
                letter = "j";
              } else if (finger_x_position < 50) {
                letter = "k";
              } else if (finger_x_position < 70) {
                letter = "l";
              } else if (finger_x_position < 90) {
                letter = ";";
              } else {
                letter = "'";
              }

              std::cout << letter << " " << fingerNames[finger.type()] << " Velocity: " << fingerDownVelocity << std::endl;
            }

            // lock the finger
            FINGER_LOCK[finger_lock_offset] = true;
          }
        } else { // else if the downwards velocity is less than 150, then and
          // we have our locked finger, then we release the lock
          if (FINGER_LOCK[finger_lock_offset]) {
            FINGER_LOCK[finger_lock_offset] = false;
          }
        }
    }
  }
}

void SampleListener::onFocusGained(const Controller& controller) {
  std::cout << "Focus Gained" << std::endl;
}

void SampleListener::onFocusLost(const Controller& controller) {
  std::cout << "Focus Lost" << std::endl;
}

void SampleListener::onDeviceChange(const Controller& controller) {
  std::cout << "Device Changed" << std::endl;
  const DeviceList devices = controller.devices();

  for (int i = 0; i < devices.count(); ++i) {
    std::cout << "id: " << devices[i].toString() << std::endl;
    std::cout << "  isStreaming: " << (devices[i].isStreaming() ? "true" : "false") << std::endl;
  }
}

void SampleListener::onServiceConnect(const Controller& controller) {
  std::cout << "Service Connected" << std::endl;
}

void SampleListener::onServiceDisconnect(const Controller& controller) {
  std::cout << "Service Disconnected" << std::endl;
}

int main(int argc, char** argv) {
  // Create a sample listener and controller
  SampleListener listener;
  Controller controller;

  // Have the sample listener receive events from the controller
  controller.addListener(listener);

  if (argc > 1 && strcmp(argv[1], "--bg") == 0)
    controller.setPolicy(Leap::Controller::POLICY_BACKGROUND_FRAMES);

  // Keep this process running until Enter is pressed
  std::cout << "Press Enter to quit..." << std::endl;
  std::cin.get();

  // Remove the sample listener when done
  controller.removeListener(listener);

  return 0;
}
