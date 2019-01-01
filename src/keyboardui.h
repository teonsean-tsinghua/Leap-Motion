#ifndef KEYBOARDUI_H
#define KEYBOARDUI_H

// QT
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QDebug>
#include <QObject>
#include <iostream>
#include <cstring>
#include <unistd.h>

class Keyboardui {
private:
    QObject *object;
    int test_int = 0;

public:
    Keyboardui();
    void init(int argc, char** argv);
    void testMethod();
    void registerKeystroke(int fingerIndex);
    // ~Keyboard();
    // std::vector<std::pair<std::string, double> > convert(std::string sequence);

};

// std::string getTime();

#endif // KEYBOARDUI_H
