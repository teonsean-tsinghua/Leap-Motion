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
#include "converter.h"

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
  /* Right Pinky */ 200.0
};

int LIMIT_RESULT = 20;
int FINGER_LOCKED = -1;
int FINGER_TRIGGER_SPEEDS[10];
int wordSelectionPosition = 0;
int hasPrint = -1;
bool is_autocomplete_on = false;
bool has_started = false;
bool show = true;
std::string current_word;
std::string sentence;
// Registered trigger inputs
std::vector<int> sequence;

// Converter
Converter converter;

enum InputState { BASE, LIMIT, MOCK_INPUT };

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

// Helper functions
// Check if input string is a value number
bool is_number(const std::string& s) {
  std::string::const_iterator it = s.begin();
  while (it != s.end() && std::isdigit(*it)) ++it;
  return !s.empty() && it == s.end();
}

// print
void print_finger_velocities() {
  if (hasPrint != -1 && FINGER_LOCKED != -1) {
    for (int x=9; x>=5; x--) {
      int offset = 5;
      if (FINGER_TRIGGER_SPEEDS[x] <= 0) FINGER_TRIGGER_SPEEDS[x] = 0;
      if (FINGER_TRIGGER_SPEEDS[x] >= 10) offset--;
      if (FINGER_TRIGGER_SPEEDS[x] >= 100) offset--;
      if (FINGER_TRIGGER_SPEEDS[x] >= 1000) offset--;
      if (show) {
        std::string offset_string = std::string(offset, ' ');
        if (hasPrint == x) {
          offset_string = std::string(offset, '*');
        }
        std::cout << x << ": " << FINGER_TRIGGER_SPEEDS[x] << offset_string << "|";
      }
    }
    if (show) std::cout << "||";
    for (int x=0; x<=4; x++) {
      int offset = 5;
      if (FINGER_TRIGGER_SPEEDS[x] <= 0) FINGER_TRIGGER_SPEEDS[x] = 0;
      if (FINGER_TRIGGER_SPEEDS[x] >= 10) offset--;
      if (FINGER_TRIGGER_SPEEDS[x] >= 100) offset--;
      if (FINGER_TRIGGER_SPEEDS[x] >= 1000) offset--;
      if (show) {
        std::string offset_string = std::string(offset, ' ');
        if (hasPrint == x) {
          offset_string = std::string(offset, '*');
        }
        std::cout << x << ": " << FINGER_TRIGGER_SPEEDS[x] << offset_string << "|";
      }
    }

    std::cout << "\n";
    hasPrint = -1;
  }
}
void print_results() {
  std::string input_string;
  for (int i = 0; i < sequence.size(); i++) {
    input_string += std::to_string(sequence[i]);
  }

  std::vector<std::pair<std::string, double> > re = converter.convert(input_string);
  int count = 0;
  int list_length = (LIMIT_RESULT < re.size() ? LIMIT_RESULT : re.size()) +1;
  if (wordSelectionPosition < 0) wordSelectionPosition += list_length;
  if (re.size() == 0) {
    std::cout << "(no results)" << std::endl;
    return;
  }
  for(auto each: re) {
    // if (each.first.length() != input_string.length()) {
    //   continue;
    // }
    if (count >= LIMIT_RESULT) break;
    if ((wordSelectionPosition-1) % list_length == count++) {
      std::cout << ">";
      current_word = each.first;
    }

    std::cout << each.first << ": " << each.second << std::endl;
  }
  if (wordSelectionPosition == 0 || wordSelectionPosition == list_length) {
    std::cout << ">";
    current_word = "";
  }
  std::cout << "DEL" << std::endl;
}
void print_help() {
  std::cout << "SAMPLE COMMANDS:" << std::endl <<
               "<HELP>" << std::endl <<
               "<LIMIT " << LIMIT_RESULT << ">" << std::endl <<
               "<AUTOCOMPLETE>" << std::endl <<
               "<SHOW_LONGER>" << std::endl <<
               "<CLEAR>" << std::endl <<
               "<PRINT>" << std::endl <<
               "<QUIT>" << std::endl;
}

// After keystroke is registered, trigger the appropriate action
void trigger_action(int fingerIndex) {
  if (fingerIndex%5 == 0) { // is thumb
    std::cout << "RESULTS:" << std::endl;
    if (fingerIndex == 0) wordSelectionPosition += 4;
    if (fingerIndex == 5) wordSelectionPosition--;
    print_results();
    std::cout << std::endl;
  } else {
    if (wordSelectionPosition > 0) {
      sequence.clear();
      wordSelectionPosition = 0;
      if (current_word != "") {
        sentence += current_word + " ";
      }
      current_word = "";
      std::cout << "CURRENT SENTENCE: " << sentence << std::endl;
    }
    sequence.push_back(fingerIndex);
    // if (is_autocomplete_on) 
    print_results();
  }
}
// run command line functions
void command_line_interface() {
  int numberInput;
  std::string user_input;
  InputState input_state = BASE;
  while (true) {
    if (input_state == BASE) {
        std::cin >> user_input;
      if (user_input == "HELP") {
        print_help();
      } else if (user_input == "LIMIT") {
        input_state = LIMIT;
      } else if (user_input == "AUTOCOMPLETE") {
        is_autocomplete_on = !is_autocomplete_on;
        std::string on_off = is_autocomplete_on == 0 ? "ON" : "OFF";
        std::cout << "AUTOCOMPLETE " << on_off << std::endl;
      } else if (user_input == "PRINT") {
        std::cout << "PRINTING CURRENT SENTENCE: " << sentence << std::endl;
      } else if (user_input == "CLEAR") {
        std::cout << "CURRENT SENTENCE CLEARED" << std::endl;
        sentence = "";
      } else if (user_input == "SHOW") {
        show = !show;
        std::string on_off = show == 0 ? "ON" : "OFF";
        std::cout << "SHOW TRIGGER DETAILS " << on_off << std::endl;
      } else if (user_input == "MOCK_INPUT") {
        input_state = MOCK_INPUT;
      } else if (user_input == "QUIT") {
        std::cout << "GOODBYE!" << std::endl;
        exit(0);
      } else {
        std::cout << "INVALID COMMAND: " << user_input << std::endl;
        std::cout << "<HELP> to see valid commands" << std::endl;
      }
    } else if (input_state == LIMIT) {
      std::string limit;
      std::cin >> limit;
      if (!is_number(limit) || std::atoi(limit.c_str()) <= 0) {
        std::cout << "Please enter positive for limit. You entered: "
          << limit << std::endl;
      } else {
        LIMIT_RESULT = std::atoi(limit.c_str());
        std::cout << "DISPLAYING ONLY " << limit << " RESULTS" << std::endl;
        input_state = BASE;
      }
    } else if (input_state == MOCK_INPUT) {
      std::string mock_input;
      std::cin >> mock_input;
      if (!is_number(mock_input) || std::atoi(mock_input.c_str()) <= 0) {
        std::cout << "Please enter positive number for mock. You entered: "
          << mock_input << std::endl;
      } else {
        std::vector<std::pair<std::string, double>> re = converter.convert(mock_input);
        for(auto each: re) {
          std::cout << each.first << ": " << each.second << std::endl;
        }
        input_state = BASE;
      }
    }
  }
}
// determine finger index
int get_finger_index(Hand hand, Finger finger) {
  int fingerIndex = hand.isLeft() ? 5 : 0;
  return fingerIndex + finger.type();
}
// determine finger's trigger speed
int get_trigger_speed(Hand hand, Finger finger) {
  if (finger.type() == 0) { // for thumbs, add horizontal and vertical
    int x_speed = hand.isLeft() ? -finger.tipVelocity()[0] : finger.tipVelocity()[0];
    x_speed = std::max(x_speed, 0);
    int y_speed = -finger.tipVelocity()[1];
    y_speed = std::max(y_speed, 0);
    return  std::sqrt(x_speed*x_speed + y_speed*y_speed);
  } else { // for non-thumbs, add vertical
    return -finger.tipVelocity()[1];
  }
}
// determine largest trigger speed and index
int get_largest_trigger_value_index() {
  int largestTriggerSpeed = 0;
  int fingerIndex = -1;
  for (int x=0; x<10; x++) {
    if (FINGER_TRIGGER_SPEEDS[x] > largestTriggerSpeed) {
      largestTriggerSpeed = FINGER_TRIGGER_SPEEDS[x];
      fingerIndex = x;
    }
  }
  return fingerIndex;
}

// Most of the code start here
void SampleListener::onFrame(const Controller& controller) {
  const Frame frame = controller.frame();

  HandList hands = frame.hands();
  for (HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) {
    const Hand hand = *hl;
    const FingerList fingers = hand.fingers();
    for (FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) {
      const Finger finger = *fl;

      int fingerIndex = get_finger_index(hand, finger);
      int triggerSpeed = get_trigger_speed(hand, finger);

      // update all hand speeds
      FINGER_TRIGGER_SPEEDS[fingerIndex] = triggerSpeed;

      // if currently locked finger decreases speed:
      if (FINGER_LOCKED == fingerIndex)
        if (triggerSpeed <= TRIGGER_THRESHOLDS[fingerIndex]-100)
          FINGER_LOCKED = -1; // release lock
    }
  }

  int fingerIndex = get_largest_trigger_value_index();
  int largestTriggerSpeed = FINGER_TRIGGER_SPEEDS[fingerIndex];

  // if finger is not locked and one trigger speed is high
  if (FINGER_LOCKED == -1 &&
      largestTriggerSpeed > TRIGGER_THRESHOLDS[fingerIndex]) {

      FINGER_LOCKED = fingerIndex;
      hasPrint = fingerIndex;
      trigger_action(fingerIndex);
  }

  print_finger_velocities();
}
void SampleListener::onFocusGained(const Controller& controller) {
  std::cout << "Focus Gained" << std::endl;
  has_started = true;
}

int main(int argc, char** argv) {
  std::cout << "Converter initialized.\n";

  // Quick sanity check
  std::vector<std::pair<std::string, double> > re = converter.convert("62888");
  for(auto each: re) {
    std::cout << each.first << ": " << each.second << std::endl;
  }

  // Create a sample listener and controller
  SampleListener listener;
  Controller controller;
  controller.addListener(listener);
  if (argc > 1 && strcmp(argv[1], "--bg") == 0)
    controller.setPolicy(Leap::Controller::POLICY_BACKGROUND_FRAMES);

  print_help();
  std::cout << "Press Enter to quit..." << std::endl;
  std::cin.get();

  command_line_interface();

  // Remove the sample listener when done
  controller.removeListener(listener);

  return 0;
}
