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

// #define TAP_THRESHOLD -80.0

float TAP_THRESHOLD = -150.0;

int FINGER_LOCKED = -1;
int FINGER_DOWNWARD_VELOCITIES[10];
int has_print = -1;
std::vector<int> sequence;
int THUMB_VELO[8];

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

// Helper objectivities
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
      int offset = 8;
      if (FINGER_DOWNWARD_VELOCITIES[x] >= 0) FINGER_DOWNWARD_VELOCITIES[x] = 0;
      if (FINGER_DOWNWARD_VELOCITIES[x] < 0) offset--;
      if (FINGER_DOWNWARD_VELOCITIES[x] <= -10 || FINGER_DOWNWARD_VELOCITIES[x] >= 10) offset--;
      if (FINGER_DOWNWARD_VELOCITIES[x] <= -100 || FINGER_DOWNWARD_VELOCITIES[x] >= 100) offset--;
      if (FINGER_DOWNWARD_VELOCITIES[x] <= -1000 || FINGER_DOWNWARD_VELOCITIES[x] >= 1000) offset--;
      std::string offset_string = std::string(offset, ' ');
      if (has_print == x) {
        offset_string = std::string(offset, '*');
      }
      // std::cout << x << ": " << FINGER_DOWNWARD_VELOCITIES[x] << offset_string << "|";
    }
    std::cout << "||";
    for (int x=0; x<=4; x++) {
      int offset = 8;
      if (FINGER_DOWNWARD_VELOCITIES[x] >= 0) FINGER_DOWNWARD_VELOCITIES[x] = 0;
      if (FINGER_DOWNWARD_VELOCITIES[x] < 0) offset--;
      if (FINGER_DOWNWARD_VELOCITIES[x] <= -10 || FINGER_DOWNWARD_VELOCITIES[x] >= 10) offset--;
      if (FINGER_DOWNWARD_VELOCITIES[x] <= -100 || FINGER_DOWNWARD_VELOCITIES[x] >= 100) offset--;
      if (FINGER_DOWNWARD_VELOCITIES[x] <= -1000 || FINGER_DOWNWARD_VELOCITIES[x] >= 1000) offset--;
      std::string offset_string = std::string(offset, ' ');
      if (has_print == x) {
        offset_string = std::string(offset, '*');
      }
      // std::cout << x << ": " << FINGER_DOWNWARD_VELOCITIES[x] << offset_string << "|";

    }
    // print thumb
    for (int x=0; x<8; x++) {
      int offset = 8;
      if (THUMB_VELO[x] < 0) offset--;
      if (THUMB_VELO[x] <= -10 || THUMB_VELO[x] >= 10) offset--;
      if (THUMB_VELO[x] <= -100 || THUMB_VELO[x] >= 100) offset--;
      if (THUMB_VELO[x] <= -1000 || THUMB_VELO[x] >= 1000) offset--;
      std::cout << x << ": " << THUMB_VELO[x] << std::string(offset, ' ') << "|";
    }

    std::cout << "\n";
    has_print = -1;
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
  // Get the most recent frame and report some basic information
  const Frame frame = controller.frame();

  HandList hands = frame.hands();
  for (HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) {

    const Hand hand = *hl;

    const FingerList fingers = hand.fingers();
    for (FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) {
      const Finger finger = *fl;

        // allow hand to determine offset value
        int finger_lock_offset = hand.isLeft() ? 5 : 0;
        finger_lock_offset += finger.type();
        // Bone bone = finger.bone(3); // get the distal bone

        // separate thumb/fingers
        int fingerVelocity = finger.tipVelocity()[1]; // finger Velocity
        if (finger.type() == 0) { // thumb velocity

          int x_vel = hand.isLeft() ? finger.tipVelocity()[0] : -finger.tipVelocity()[0];
          x_vel = std::min(x_vel, 0);
          int y_vel = finger.tipVelocity()[1];
          y_vel = std::min(y_vel, 0);

          fingerVelocity = -std::sqrt(x_vel*x_vel + y_vel*y_vel);

          int offset_thumb = hand.isLeft() ? 0 : 3;
          THUMB_VELO[offset_thumb] = finger.tipVelocity()[0];
          THUMB_VELO[offset_thumb+1] = finger.tipVelocity()[1];
          THUMB_VELO[offset_thumb+2] = finger.tipVelocity()[2];

          if (hand.isLeft()) {
            THUMB_VELO[6] = fingerVelocity;
          } else {
            THUMB_VELO[7] = fingerVelocity;
          }
        }

        FINGER_DOWNWARD_VELOCITIES[finger_lock_offset] = fingerVelocity;

        // if the downward velocity exceeds 150
        // and if the we haven't locked in on a finger
        if (fingerVelocity < TAP_THRESHOLD && FINGER_LOCKED == -1) {
            int finger_x_position = finger.tipPosition()[0];
            std::string letter;

            switch (finger_lock_offset) {
              case 9: letter = "qaz   ";
              break;
              case 8: letter = "wsx   ";
              break;
              case 7: letter = "edc   ";
              break;
              case 6: letter = "rfvtgb";
              break;
              case 1: letter = "yhnujm";
              break;
              case 2: letter = "ik    ";
              break;
              case 3: letter = "ol    ";
              break;
              case 4: letter = "p     ";
              break;
              default: letter = "      ";
            }

            // std::cout << letter << " " << fingerNames[finger.type()] << " Velocity: " << fingerVelocity << std::endl;

            // lock the finger
            FINGER_LOCKED = finger_lock_offset;
            has_print = finger_lock_offset;

            if (finger.type() == 0) { // is thumb.
              std::cout << "PRINT: ";
              for (std::vector<int>::const_iterator i = sequence.begin(); i != sequence.end(); ++i)
                std::cout << *i;
              std::cout << "---------\n";
              sequence.clear();
            } else {
              sequence.push_back(finger_lock_offset);
            }

        } else if (fingerVelocity >= -150.0 && FINGER_LOCKED == finger_lock_offset) {

          // if the downwards velocity is less than 150, and we are currently
          // examining our locked finger, then we release the lock
          FINGER_LOCKED = -1;
        }
    }
  }
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
  std::cout << "Press Enter to quit..." << std::endl;
  std::cin.get();

  // int threshold;
  // while (true) {
  //   std::cin >> threshold;
  //   if (threshold == -1) {
  //     break;
  //   }
  //   std::cout << "The value you entered is " << threshold << "\n";
  //   TAP_THRESHOLD = threshold * 1.0;
  //   std::cout << "TAP_THRESHOLD is " << TAP_THRESHOLD << "\n";
  // }

  // Remove the sample listener when done
  controller.removeListener(listener);

  return 0;
}
