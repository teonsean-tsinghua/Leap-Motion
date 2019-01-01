#include <iostream>
#include <cstring>
#include <thread>
#include <unistd.h>
#include "Leap.h"
#include "converter.h"
#include "keyboardui.h"
#include "helper.h"

float TRIGGER_THRESHOLDS[10] = {
  /* Left  Thumb */ 260.0,
  /* Left  Index */ 190.0,
  /* Left  Middl */ 190.0,
  /* Left  Ring  */ 190.0,
  /* Left  Pinky */ 200.0,
  /* Right Thumb */ 260.0,
  /* Right Index */ 190.0,
  /* Right Middl */ 190.0,
  /* Right Ring  */ 190.0,
  /* Right Pinky */ 200.0
};

int LIMIT_RESULT = 20;
int FINGER_LOCKED = -1;             // Lock on triggering finger
int FINGER_TRIGGER_SPEEDS[10];      // Record downward velocity to determine trigger
bool inWordSelectionMode = false;   // Triggered by thumb to select word
int wordSelectionPosition = 0;      // Which word is selected on choice right now
int hasPrintCurrentTrigger = -1;    // After a trigger is detected but before it has printed
bool isAutocompleteOn = true;       // Toggle autocomplete of words
bool print = true;
std::string currentWord;            // currently selected word
std::string currentSentence;        // currently constructed sentences
std::vector<int> sequenceOfLetters; // Sequence of finger strokes
enum InputState { BASE, LIMIT, KEYBOARD }; // for CLI

// Converter & Keyboardui
Converter converter;
Keyboardui keyboardui;

// LeapMotion Template Code
using namespace Leap;
class SampleListener : public Listener {
  public:
    virtual void onInit(const Controller&);
    virtual void onFocusGained(const Controller&);
    virtual void onFocusLost(const Controller&);
    virtual void onDeviceChange(const Controller&);
    virtual void onConnect(const Controller&);
    virtual void onDisconnect(const Controller&);
    virtual void onExit(const Controller&);
    virtual void onFrame(const Controller&);
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
void SampleListener::onFocusGained(const Controller& controller) {
  std::cout << "Focus Gained" << std::endl;
}

// determine finger index
int getFingerIndex(Hand hand, Finger finger) {
  int fingerIndex = hand.isLeft() ? 5 : 0;
  return fingerIndex + finger.type();
}
// determine finger's trigger speed
int getTriggerSpeed(Hand hand, Finger finger) {
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
int getLargestTriggerValueIndex() {
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

// print the velocity of each finger on a trigger event
void printFingerVelocities() {
  if (print) {
    for (int x=9; x>=5; x--) {
      int offset = 5;
      if (FINGER_TRIGGER_SPEEDS[x] <= 0) FINGER_TRIGGER_SPEEDS[x] = 0;
      if (FINGER_TRIGGER_SPEEDS[x] >= 10) offset--;
      if (FINGER_TRIGGER_SPEEDS[x] >= 100) offset--;
      if (FINGER_TRIGGER_SPEEDS[x] >= 1000) offset--;

      std::string offset_string = std::string(offset, ' ');
      if (hasPrintCurrentTrigger == x) {
        offset_string = std::string(offset, '*');
      }
      std::cout << x << ": " << FINGER_TRIGGER_SPEEDS[x] << offset_string << "|";
    }
    std::cout << "||";
    for (int x=0; x<=4; x++) {
      int offset = 5;
      if (FINGER_TRIGGER_SPEEDS[x] <= 0) FINGER_TRIGGER_SPEEDS[x] = 0;
      if (FINGER_TRIGGER_SPEEDS[x] >= 10) offset--;
      if (FINGER_TRIGGER_SPEEDS[x] >= 100) offset--;
      if (FINGER_TRIGGER_SPEEDS[x] >= 1000) offset--;
      std::string offset_string = std::string(offset, ' ');
      if (hasPrintCurrentTrigger == x) {
        offset_string = std::string(offset, '*');
      }
      std::cout << x << ": " << FINGER_TRIGGER_SPEEDS[x] << offset_string << "|";
    }
    std::cout << "\n";
  }
  hasPrintCurrentTrigger = -1;
}
// print word choices
void printSequenceAndWordChoices() {
  // get word candidates from sequence

  // TODO: remove redundant work
  std::string input_string;
  for (int i = 0; i < sequenceOfLetters.size(); i++) {
    input_string += std::to_string(sequenceOfLetters[i]);
  }
  std::vector<std::pair<std::string, double> > candidates = converter.convert(input_string);
  if (candidates.size() == 0) {
    std::cout << "(no results)" << std::endl;
    return;
  }

  int count = 0;
  int list_length = (LIMIT_RESULT < candidates.size() ? LIMIT_RESULT : candidates.size()) +1;
  if (wordSelectionPosition < 0) wordSelectionPosition += list_length;

  std::vector<std::string> candidatesForVisualization;
  for(auto each: candidates) {
    candidatesForVisualization.push_back(each.first);

    // TODO: set a more consistent model

    if (count >= LIMIT_RESULT) break;
    if (wordSelectionPosition == count++) {
      std::cout << ">";
      currentWord = each.first;
    }
    std::cout << each.first << ": " << each.second << std::endl;
  }
  if (wordSelectionPosition == list_length-1) {
    std::cout << ">";
    currentWord = "";
  }
  std::cout << "DEL" << std::endl;

  keyboardui.setWordCandidates(candidatesForVisualization, wordSelectionPosition);

}

// After a keystroke is registered, handle the appropriate trigger
void handleKeystrokeEvent(int fingerIndex) {
  // If keystroke is thumb, we select left and right
  if (fingerIndex%5 == 0) {
    inWordSelectionMode = true;
    std::cout << "RESULTS:" << std::endl;
    if (fingerIndex == 0) wordSelectionPosition++;
    if (fingerIndex == 5) wordSelectionPosition--;
    printSequenceAndWordChoices();
    std::cout << std::endl;
  } else { // else if fingers, onto the next word
    if (inWordSelectionMode) {
      inWordSelectionMode = false;
      wordSelectionPosition = 0;
      sequenceOfLetters.clear();
      if (currentWord != "") {
        currentSentence += currentWord + " ";
      }
      currentWord = "";
      std::cout << "CURRENT SENTENCE: " << currentSentence << std::endl;
      keyboardui.updateDisplay(currentSentence);
    }
    sequenceOfLetters.push_back(fingerIndex);
    if (isAutocompleteOn) {
      printSequenceAndWordChoices();
      std::cout << std::endl;
    }
  }

  // UI registers Keystroke
  keyboardui.registerKeystroke(fingerIndex);
}
// read from keyboard input [looped]
void runKeyboardInputMode(){
  std::cout << "Keyboard Input Mode: \n";
  for (std::string line; std::getline(std::cin, line);) {
    std::vector<std::pair<std::string, double> > re = converter.convert(line);
    for(auto each: re) {
      std::cout << each.first << ": " << each.second << std::endl;
    }
  }
}
// run commands from stdin [looped]
void runStdinInterface() {
  int numberInput;
  std::string user_input;
  while (true) {
    std::cin >> user_input;
    if (user_input == "HELP") {
      printHelpMenu();
    } else if (user_input == "LIMIT") {
      LIMIT_RESULT = getNumberFromStdin();
    } else if (user_input == "AUTOCOMPLETE") {
      isAutocompleteOn = !isAutocompleteOn;
      std::string on_off = isAutocompleteOn == 0 ? "ON" : "OFF";
      std::cout << "AUTOCOMPLETE " << on_off << std::endl;
    } else if (user_input == "CURRENT") {
      std::cout << "PRINTING CURRENT SENTENCE: " << currentSentence << std::endl;
    } else if (user_input == "CLEAR") {
      std::cout << "CURRENT SENTENCE CLEARED" << std::endl;
      currentSentence = "";
    } else if (user_input == "PRINT") {
      print = !print;
      std::string on_off = print == 0 ? "ON" : "OFF";
      std::cout << "PRINT " << on_off << std::endl;
    } else if (user_input == "KEYBOARD") {
      runKeyboardInputMode();
    } else if (user_input == "QUIT") {
      std::cout << "GOODBYE!" << std::endl;
      exit(0);
    } else {
      std::cout << "INVALID COMMAND: " << user_input << std::endl;
      std::cout << "<HELP> to see valid commands" << std::endl;
    }
  }
}

// For each frame, determine the velocity of each finger. If a certain finger is
// not currently locked, and a finger exceeds a threshold, lock that finger,
// register it as a trigger, and evoke handleKeystrokeEvent() and printFingerVelocities().
void SampleListener::onFrame(const Controller& controller) {
  const Frame frame = controller.frame();
  HandList hands = frame.hands();

  for (HandList::const_iterator hl = hands.begin(); hl != hands.end(); ++hl) {
    const Hand hand = *hl;
    const FingerList fingers = hand.fingers();
    for (FingerList::const_iterator fl = fingers.begin(); fl != fingers.end(); ++fl) {
      const Finger finger = *fl;
      int fingerIndex = getFingerIndex(hand, finger);
      int triggerSpeed = getTriggerSpeed(hand, finger);

      // For current frame iteration, store all finger velocities
      FINGER_TRIGGER_SPEEDS[fingerIndex] = triggerSpeed;

      // If currently locked finger decreases speed, we release the lock:
      if (FINGER_LOCKED == fingerIndex)
        if (triggerSpeed <= TRIGGER_THRESHOLDS[fingerIndex]-100)
          FINGER_LOCKED = -1;
    }
  }

  int fingerIndex = getLargestTriggerValueIndex();
  if (fingerIndex == -1) return;
  int largestTriggerSpeed = FINGER_TRIGGER_SPEEDS[fingerIndex];

  // if finger is not locked and one trigger speed is high
  if (FINGER_LOCKED == -1 &&
      largestTriggerSpeed > TRIGGER_THRESHOLDS[fingerIndex]) {
      FINGER_LOCKED = fingerIndex;
      hasPrintCurrentTrigger = fingerIndex;
      handleKeystrokeEvent(fingerIndex);
      printFingerVelocities();
  }
}

int executeLeapMotion(int argc, char** argv) {
  printHelpMenu();
  runStdinInterface(); // blocking
  return 0;
}

int main(int argc, char** argv) {
  std::cout << "Initializing converter...\n";
  Converter converter;
  testConverter(converter);
  std::cout << "Converter initialized.\n";

  // Create a sample listener and controller
  SampleListener listener;
  Controller controller;
  controller.addListener(listener);
  if (argc > 1 && strcmp(argv[1], "--bg") == 0)
    controller.setPolicy(Leap::Controller::POLICY_BACKGROUND_FRAMES);

  // Keep this process running until Enter is pressed
  // std::cout << "Press Enter to quit..." << std::endl;
  // std::cin.get();

  // thread for stdin interface
  std::thread executionThread (executeLeapMotion, argc, argv);
  keyboardui.init(argc, argv);

  // Remove the sample listener when done
  controller.removeListener(listener);

  return 0;
}
