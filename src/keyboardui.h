#ifndef KEYBOARDUI_H
#define KEYBOARDUI_H

// QT
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QString>
#include <QStringList>
#include <QList>
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
    void clearWordCandidates();
    void setWordCandidates(std::vector<std::string> candidates, int wordSelectionPosition, bool inWordSelectionMode);
    void updateDisplay(std::string currentSentence);
    // ~Keyboard();
    // std::vector<std::pair<std::string, double> > convert(std::string sequence);
};

// std::string getTime();

#endif // KEYBOARDUI_H
