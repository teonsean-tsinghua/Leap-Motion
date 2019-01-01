#include"helper.h"

bool isNumber(const std::string& s) {
  std::string::const_iterator it = s.begin();
  while (it != s.end() && std::isdigit(*it)) ++it;
  return !s.empty() && it == s.end();
}
void printHelpMenu() {
  std::cout << "SAMPLE COMMANDS:" << std::endl <<
               "show the <HELP> menu" << std::endl <<
               "show <LIMIT ##> of results" << std::endl <<
               "print the <CURRENT> sentence" << std::endl <<
               "turn <PRINT> on/off" << std::endl <<
               "use the <KEYBOARD> for input" << std::endl <<
               "<CLEAR> the current sentence" << std::endl <<
               "turn <AUTOCOMPLETE> on/off" << std::endl <<
               "<QUIT> the application" << std::endl;
}
void testConverter(Converter converter) {
  std::vector<std::pair<std::string, double> > re = converter.convert("18");
  int x=0;
  for(auto each: re) {
    if (x++ > 4) return;
    std::cout << each.first << ": " << each.second << std::endl;
  }
}

int getNumberFromStdin(){
  while (true) {
    std::string limit;
    std::cin >> limit;
    if (!isNumber(limit) || std::atoi(limit.c_str()) <= 0) {
      std::cout << "Please enter positive number. You entered: "
        << limit << std::endl;
    } else {
      std::cout << "DISPLAYING ONLY " << limit << " RESULTS" << std::endl;
      return std::atoi(limit.c_str());
    }
  }
}
