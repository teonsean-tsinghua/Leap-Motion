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
  for(auto each: re) {
    std::cout << each.first << ": " << each.second << std::endl;
  }
}
