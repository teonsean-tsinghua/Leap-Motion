#ifndef KEYBOARDUI_H
#define KEYBOARDUI_H

// QT
#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQmlContext>
#include <QDebug>
#include <QObject>

class Keyboardui {
private:
  //   static const int MAXIMUM_CANDIDATE;
  //   static const double THRESHOLD;
  //
  //   class Trie {
  //   public:
  //       char ch;
  //       bool isWord;
  //       double prob_as_word;
  //       std::map<char, std::pair<Trie*, double> > children;
  //
  //       Trie();
  //       ~Trie();
  //
  //   };
  //
  //   class Candidate {
  //   public:
  //       std::string str;
  //       Trie* trie;
  //       double prob;
  //
  //       Candidate(std::string str, Trie* trie, double prob);
  //
  //   };
  //
  //   Trie* trie_root;
  //   std::map<std::string, std::vector<Candidate> > stash; //TODO: need to optimize.
  //   std::vector<std::vector<char> > finger_area;
  //
  //   void build_trie();
  //   void build_trie(TiXmlElement* ele, Trie* node);
  //   void gen_next_candidates(std::string cur, char next);
  //   void predict_word(std::string input, std::vector<std::pair<std::string, double> >& result);
  //   friend bool operator < (const Candidate& a, const Candidate& b);
  //   void softmax(std::vector<Candidate>& cands);
	// void minmax(std::vector<Candidate>& cands);

public:
    Keyboardui(int argc, char** argv);
    // ~Keyboard();
    // std::vector<std::pair<std::string, double> > convert(std::string sequence);

};

// std::string getTime();

#endif // KEYBOARDUI_H
