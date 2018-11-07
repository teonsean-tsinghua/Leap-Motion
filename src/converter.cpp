#include"converter.h"

const int Converter::MAXIMUM_CANDIDATE = 64;
const double Converter::THRESHOLD = 1.0 / 5120;

Converter::Trie::Trie(char ch, bool isWord) {
    this->ch = ch;
    this->isWord = isWord;
}

Converter::Trie::~Trie() {
    for(auto it = children.begin(); it != children.end(); it++) {
        delete it->second.first;
    }
}

Converter::Candidate::Candidate(std::string str, Trie* trie, double prob) {
    this->str = str;
    this->trie = trie;
    this->prob = prob;
}

bool operator < (const Converter::Candidate& a, const Converter::Candidate& b) {
    return a.prob < b .prob;
}

Converter::Converter() {
    // hard coded. can allow user to customize.
    finger_area.push_back(std::vector<char>({'q', 'a', 'z', 'w', 's', 'x'}));
    finger_area.push_back(std::vector<char>({'q', 'a', 'z', 'w', 's', 'x', 'e', 'd', 'c'}));
    finger_area.push_back(std::vector<char>({'q', 'a', 'z', 'w', 's', 'x', 'e', 'd', 'c', 'r', 'f', 'v'}));
    finger_area.push_back(std::vector<char>({'w', 's', 'x', 'e', 'd', 'c', 'r', 'f', 'v', 't', 'g', 'b'}));
    finger_area.push_back(std::vector<char>());
    finger_area.push_back(std::vector<char>());
    finger_area.push_back(std::vector<char>({'y', 'g', 'v', 'u', 'h', 'b', 'i', 'j', 'n', 'o', 'k', 'm'}));
    finger_area.push_back(std::vector<char>({'u', 'h', 'b', 'i', 'j', 'n', 'o', 'k', 'm', 'p', 'l'}));
    finger_area.push_back(std::vector<char>({'i', 'j', 'n', 'o', 'k', 'm', 'p', 'l'}));
    finger_area.push_back(std::vector<char>({'o', 'k', 'm', 'p', 'l'}));

    build_trie();

    stash[""].push_back(Candidate("", trie_root, 1));
}

Converter::~Converter() {
    delete trie_root;
}

void Converter::build_trie() {
    trie_root = new Trie('\0', false);
}

void Converter::gen_next_candidates(std::string cur, char next) {
    std::vector<Candidate>& prev_cands = stash[cur];
    std::priority_queue<Candidate> max_heap;
    for(auto& cand: prev_cands) {
        for(auto ch: finger_area[next - '0']) {
            if(cand.trie->children.count(ch)) {
                max_heap.push(Candidate(cand.str + ch, cand.trie->children[ch].first, cand.prob * cand.trie->children[ch].second));
            }
        }
    }
    cur += next;
    for(int i = std::min(MAXIMUM_CANDIDATE, (int)max_heap.size()) - 1; i >= 0; i--) {
        stash[cur].push_back(max_heap.top());
        max_heap.pop();
    }
    softmax(stash[cur]);
}

void Converter::softmax(std::vector<Candidate>& cands) {
    double sum = 0;
    for(auto& cand: cands) {
        cand.prob = exp(cand.prob);
        sum += cand.prob;
    }
    for(auto& cand: cands) {
        cand.prob = cand.prob / sum;
    }
}

void Converter::predict_word(std::string input, std::vector<std::pair<std::string, double> >& result) {
    std::vector<Candidate>& cands = stash[input];
    std::priority_queue<Candidate> to_expand;
    for(auto& cand: cands) {
        if(cand.prob >= THRESHOLD) {
            to_expand.push(cand);
        }
    }
    while(!to_expand.empty() && result.size() < MAXIMUM_CANDIDATE) {
        Candidate cand = to_expand.top();
        to_expand.pop();
        if(cand.trie->isWord) {
            result.push_back(std::pair<std::string, double>(cand.str, cand.prob));
        }
        for(auto it = cand.trie->children.begin(); it != cand.trie->children.end(); it++) {
            double prob = cand.prob * it->second.second;
            if(prob >= THRESHOLD) {
                to_expand.push(Candidate(cand.str + it->first, it->second.first, prob));
            }
        }
    }
}

std::vector<std::pair<std::string, double> > Converter::convert(std::string sequence) {
    std::vector<std::pair<std::string, double> > ret = std::vector<std::pair<std::string, double> >();
    std::string cur = "";
    for(auto ch: sequence) {
        if(!stash.count(cur + ch)) {
            gen_next_candidates(cur, ch);
        }
        cur += ch;
    }
    predict_word(cur, ret);
    return ret;
}
