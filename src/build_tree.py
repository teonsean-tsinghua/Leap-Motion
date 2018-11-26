import xml.etree.ElementTree as ET

class Trie:
    def __init__(self):
        self.isWord = False
        self.cnt_total = 0
        self.cnt_as_word = 0
        self.prob_as_word = 0
        self.children = {}
    
    def setProb(self, parent_cnt):
        self.prob = self.cnt_total / parent_cnt
        self.prob_as_word = self.cnt_as_word / self.cnt_total
        for k, v in self.children.items():
            v.setProb(self.cnt_total)

    def validate(self):
        sum = 0
        cnt = 0
        for k, v in self.children.items():
            v.validate()
            sum += v.prob
            cnt += v.cnt_total
        if len(self.children) == 0:
            return
        if not cnt + self.cnt_as_word == self.cnt_total:
            print('error cnt:', cnt + self.cnt_as_word, self.cnt_total)
            exit(1)
        if sum + self.prob_as_word < 0.999999999999999:
            print('error sum:', sum + self.prob_as_word)
            exit(1)

    def toXml(self, node):
        node.attrib = {'isWord': '1' if self.isWord else '0', 'prob': str(self.prob), 'prob_as_word': str(self.prob_as_word)}
        for k, v in self.children.items():
            child = ET.SubElement(node, k)
            v.toXml(child)

root = Trie()
with open('lexicon.txt', 'r') as f:
    for line in f:
        if len(line) == 0:
            continue
        line = line.split(' ')
        word = line[0]
        cnt = int(line[1])
        cur = root
        cur.cnt_total += cnt
        for ch in word:
            if ch not in cur.children:
                cur.children[ch] = Trie()
            cur = cur.children[ch]
            cur.cnt_total += cnt
        cur.isWord = True
        cur.cnt_as_word += cnt
root.setProb(root.cnt_total)
root.validate()
root_xml = ET.Element('root')
root.toXml(root_xml)
tree = ET.ElementTree(root_xml)
tree.write('tree.xml')