#ifndef HELPER_H
#define HELPER_H

#include <iostream>
#include <cstring>
#include <unistd.h>
#include "converter.h"

// Check if input string is a value number
bool isNumber(const std::string& s);

// Print the help menu
void printHelpMenu();

// test the Converter
void testConverter(Converter converter);

#endif // HELPER_H
