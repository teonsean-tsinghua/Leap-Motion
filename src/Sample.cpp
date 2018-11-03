/******************************************************************************\
* Copyright (C) 2012-2014 Leap Motion, Inc. All rights reserved.               *
* Leap Motion proprietary and confidential. Not for distribution.              *
* Use subject to the terms of the Leap Motion SDK Agreement available at       *
* https://developer.leapmotion.com/sdk_agreement, or another agreement         *
* between Leap Motion and you, your company or other organization.             *
\******************************************************************************/

#include <iostream>
#include <cstring>
#include <fstream>
#include <string>
#include "Leap.h"

#define TRIGGER_THRESHOLD 190.0

float TRIGGER_THRESHOLDS[10] = {
  /* Left  Thumb */ 260.0,
  /* Left  Index */ TRIGGER_THRESHOLD,
  /* Left  Middl */ TRIGGER_THRESHOLD,
  /* Left  Ring  */ TRIGGER_THRESHOLD,
  /* Left  Pinky */ 200.0,
  /* Right Thumb */ 260.0,
  /* Right Index */ TRIGGER_THRESHOLD,
  /* Right Middl */ TRIGGER_THRESHOLD,
  /* Right Ring  */ TRIGGER_THRESHOLD,
  /* Right Pinky */ TRIGGER_THRESHOLD
};

int FINGER_LOCKED = -1;
int FINGER_DOWNWARD_VELOCITIES[10];
int has_print = -1;

// Registered trigger inputs
std::vector<int> sequence;

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

// Helper classes
class WordFrequency {
  public:
    std::string word;
    int freq;

    WordFrequency(std::string w, int f) {
      word = w;
      freq = f;
    }
};
class WordTreeNode {
  public:
    std::string currentPos;
    std::vector<WordFrequency> wordFreqs;
    WordTreeNode *child_9;
    WordTreeNode *child_8;
    WordTreeNode *child_7;
    WordTreeNode *child_6;
    WordTreeNode *child_1;
    WordTreeNode *child_2;
    WordTreeNode *child_3;
    WordTreeNode *child_4;
};

// Helper functions
void load_lexicon() {
  std::string line;
  std::ifstream myfile (".//lexicon.txt");
  WordTreeNode root;

  if (myfile.is_open()) {
    while (getline(myfile,line)) {
      std::cout << line << '\n';
      size_t space_pos = line.find(' ');
      std::string word = line.substr(0, space_pos);
      int freq = stoi(line.substr(space_pos));
      std::cout << "word " << word << '\n';
      std::cout << "freq " << freq << '\n';

      WordFrequency wordFreq = WordFrequency(word, freq);
      exit(0);

    }
    myfile.close();
  }
  else std::cout << "Unable to open file";
}
void print_finger_velocities() {
  if (has_print != -1 && FINGER_LOCKED != -1) {
    for (int x=9; x>=5; x--) {
      int offset = 5;
      if (FINGER_DOWNWARD_VELOCITIES[x] <= 0) FINGER_DOWNWARD_VELOCITIES[x] = 0;
      if (FINGER_DOWNWARD_VELOCITIES[x] >= 10) offset--;
      if (FINGER_DOWNWARD_VELOCITIES[x] >= 100) offset--;
      if (FINGER_DOWNWARD_VELOCITIES[x] >= 1000) offset--;
      std::string offset_string = std::string(offset, ' ');
      if (has_print == x) {
        offset_string = std::string(offset, '*');
      }
      std::cout << x << ": " << FINGER_DOWNWARD_VELOCITIES[x] << offset_string << "|";
    }
    std::cout << "||";
    for (int x=0; x<=4; x++) {
      int offset = 5;
      if (FINGER_DOWNWARD_VELOCITIES[x] <= 0) FINGER_DOWNWARD_VELOCITIES[x] = 0;
      if (FINGER_DOWNWARD_VELOCITIES[x] >= 10) offset--;
      if (FINGER_DOWNWARD_VELOCITIES[x] >= 100) offset--;
      if (FINGER_DOWNWARD_VELOCITIES[x] >= 1000) offset--;
      std::string offset_string = std::string(offset, ' ');
      if (has_print == x) {
        offset_string = std::string(offset, '*');
      }
      std::cout << x << ": " << FINGER_DOWNWARD_VELOCITIES[x] << offset_string << "|";
    }

    // // print thumb
    // for (int x=0; x<8; x++) {
    //   int offset = 8;
    //   if (THUMB_VELO[x] < 0) offset--;
    //   if (THUMB_VELO[x] <= -10 || THUMB_VELO[x] >= 10) offset--;
    //   if (THUMB_VELO[x] <= -100 || THUMB_VELO[x] >= 100) offset--;
    //   if (THUMB_VELO[x] <= -1000 || THUMB_VELO[x] >= 1000) offset--;
    //   std::cout << x << ": " << THUMB_VELO[x] << std::string(offset, ' ') << "|";
    // }

    std::cout << "\n";
    has_print = -1;
  }
}
void update_sequence(int finger_triggered) {
  if (finger_triggered%5 == 0) { // is thumb.
    std::cout << "PRINT: ";
    for (std::vector<int>::const_iterator i = sequence.begin(); i != sequence.end(); ++i)
      std::cout << *i;
    std::cout << "-----";
    sequence.clear();
  } else {
    sequence.push_back(finger_triggered);
  }
}
std::string get_letter_from_offset(int offset) {
  switch (offset) {
    case 9: return "qaz   ";
    case 8: return "wsx   ";
    case 7: return "edc   ";
    case 6: return "rfvtgb";
    case 1: return "yhnujm";
    case 2: return "ik    ";
    case 3: return "ol    ";
    case 4: return "p     ";
    default: return "      ";
  }
}

void SampleListener::onInit(const Controller& controller) {
  std::cout << "Initialized" << std::endl;
  // load_lexicon();
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
void SampleListener::onFrame(const Controller& controller) {
  // return;

  // Get the most recent frame and report some basic information
  const Frame frame = controller.frame();

  HandList hands = frame.hands();
  for (HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) {
    const Hand hand = *hl;
    const FingerList fingers = hand.fingers();
    for (FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) {

      const Finger finger = *fl;
      // allow hand to determine offset value
      int fingerTriggerOffset = hand.isLeft() ? 5 : 0;
      fingerTriggerOffset += finger.type();

      // separate thumb/fingers
      int fingerTriggerSpeed = -finger.tipVelocity()[1]; // finger trigger speed
      if (finger.type() == 0) { // thumb trigger speed
        int x_speed = hand.isLeft() ? -finger.tipVelocity()[0] : finger.tipVelocity()[0];
        x_speed = std::max(x_speed, 0);
        int y_speed = -finger.tipVelocity()[1];
        y_speed = std::max(y_speed, 0);
        fingerTriggerSpeed = std::sqrt(x_speed*x_speed + y_speed*y_speed);
      }


      FINGER_DOWNWARD_VELOCITIES[fingerTriggerOffset] = fingerTriggerSpeed;

      if (fingerTriggerSpeed <= TRIGGER_THRESHOLDS[fingerTriggerOffset]-100
        && FINGER_LOCKED == fingerTriggerOffset) {
        // if the downwards velocity is less than 150, and we are currently
        // examining our locked finger, then we release the lock
        FINGER_LOCKED = -1;
      }
    }
  }

  int fingerTriggerSpeed = 0;
  int fingerTriggerOffset = -1;
  for (int x=0; x<10; x++) {
    if (FINGER_DOWNWARD_VELOCITIES[x] > fingerTriggerSpeed) {
      fingerTriggerSpeed = FINGER_DOWNWARD_VELOCITIES[x];
      fingerTriggerOffset = x;
    }
  }

  // if the downward velocity exceeds 150
  // and if the we haven't locked in on a finger
  if (fingerTriggerSpeed > TRIGGER_THRESHOLDS[fingerTriggerOffset] && FINGER_LOCKED == -1) {

      // std::string letter = get_letter_from_offset(fingerTriggerOffset);

      // lock the finger
      FINGER_LOCKED = fingerTriggerOffset;
      has_print = fingerTriggerOffset;

      // Check is thumb to trigger next word
      update_sequence(fingerTriggerOffset);

  } else if (fingerTriggerSpeed > FINGER_DOWNWARD_VELOCITIES[fingerTriggerOffset]
    && FINGER_LOCKED == fingerTriggerOffset) {

    // if the downwards velocity is greater than the current, and we are
    // currently examining our locked finger, update the largest
    // FINGER_DOWNWARD_VELOCITIES[fingerTriggerOffset] = fingerTriggerSpeed;
    // std::cout << "fingerTriggerSpeed: " << fingerTriggerSpeed << "\n";

  }
  // else if (fingerTriggerSpeed <= TRIGGER_THRESHOLDS[fingerTriggerOffset]-100
  //   && FINGER_LOCKED == fingerTriggerOffset) {
  //   // if the downwards velocity is less than 150, and we are currently
  //   // examining our locked finger, then we release the lock
  //   FINGER_LOCKED = -1;
  // }

  print_finger_velocities();
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
  // std::cout << "Press Enter to quit..." << std::endl;
  // std::cin.get();

  std::cout << "TRIGGER_THRESHOLD is " << TRIGGER_THRESHOLD << "\n";
  int numberInput;
  while (true) {
    std::cin >> numberInput;
    if (numberInput == -1) {
      for (int x=0; x<10; x++) {
        std::cout << x << " position threshold: " << TRIGGER_THRESHOLDS[x] << "\n";
      }
      continue;
    }
    int triggerPosition = numberInput%10;
    std::cout << "Press Enter threshold:" << std::endl;
    std::cin >> numberInput;
    if (numberInput == -1) {
      for (int x=0; x<10; x++) {
        std::cout << x << " position threshold: " << TRIGGER_THRESHOLDS[x] << "\n";
      }
      continue;
    }
    int threshold = numberInput;

    std::cout << "triggerPosition (mod 10): " << triggerPosition << '\n';
    std::cout << "threshold: " << threshold << '\n';
    TRIGGER_THRESHOLDS[triggerPosition] = threshold;
  }

  // Remove the sample listener when done
  controller.removeListener(listener);

  return 0;
}
