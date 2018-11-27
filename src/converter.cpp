#include"converter.h"

const int Converter::MAXIMUM_CANDIDATE = 64;
const double Converter::THRESHOLD = 1.0 / 5120;

Converter::Trie::Trie() {
    this->ch = '\0';
    this->prob_as_word = 0;
    this->isWord = false;
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
    finger_area.push_back(std::vector<char>({'q', 'a', 'z'}));
    finger_area.push_back(std::vector<char>({'w', 's', 'x'}));
    finger_area.push_back(std::vector<char>({'e', 'd', 'c'}));
    finger_area.push_back(std::vector<char>({'r', 'f', 'v', 't', 'g', 'b'}));
    finger_area.push_back(std::vector<char>());
    finger_area.push_back(std::vector<char>());
    finger_area.push_back(std::vector<char>({'y', 'h', 'n', 'u', 'j', 'm'}));
    finger_area.push_back(std::vector<char>({'i', 'k'}));
    finger_area.push_back(std::vector<char>({'o', 'l'}));
    finger_area.push_back(std::vector<char>({'p'}));

    build_trie();

    stash[""].push_back(Candidate("", trie_root, 1));
}

Converter::~Converter() {
    delete trie_root;
}

void Converter::build_trie(TiXmlElement* ele, Trie* node) {
    const char* isWord = ele->Attribute("isWord");
    const char* prob_as_word = ele->Attribute("prob_as_word");
    std::stringstream ss;
    ss << isWord << " " << prob_as_word;
    ss >> node->isWord >> node->prob_as_word;
    for(TiXmlElement* child = ele->FirstChildElement(); child != NULL; child = child->NextSiblingElement())
    {
        std::string elemName = child->Value();
        if(elemName.length() != 1) {
            std::cerr << "XML error: element name is " << elemName << std::endl;
            exit(1);
        }
        const char* prob_s = child->Attribute("prob");
        std::stringstream ss;
        ss << prob_s;
        double prob;
        ss >> prob;
        Trie* child_node = new Trie();
        node->children[elemName[0]] = std::pair<Trie*, double>(child_node, prob);
        build_trie(child, child_node);
    }
}

void Converter::build_trie() {
    trie_root = new Trie();
    TiXmlDocument doc;
    if(!doc.LoadFile("tree.xml"))
    {
        std::cerr << doc.ErrorDesc() << std::endl;
        exit(1);
    }
    TiXmlElement* root = doc.FirstChildElement();
    if(root == NULL)
    {
        std::cerr << "Failed to load file: No root element." << std::endl;
        doc.Clear();
        exit(1);
    }
    build_trie(root, trie_root);
    doc.Clear();
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
    // for(auto each: stash[cur]) {
    //     std::cout << each.str << ": " << each.prob << std::endl;
    // }
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
            result.push_back(std::pair<std::string, double>(cand.str, cand.prob * cand.trie->prob_as_word));
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
