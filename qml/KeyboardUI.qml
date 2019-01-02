import QtQuick 2.5
import QtQuick.Window 2.0
import QtScxml 5.8

Window {
    id: window
    visible: true
    width: 800
    height: 420

    function myQmlFunction(msg) {
        console.log("KeyboardUI.qml receives", msg)
        return "Message from KeyboardUI.qml saying hi!"
    }
    function registerKeystroke(fingerIndex) {
        var qmlIndex = 0;

        // map to qt coordinates
        if (fingerIndex%5 == 0) { // thumb
            arrowButtons.itemAt(1-fingerIndex/5).pressed = true
            delay(200, function() {
                arrowButtons.itemAt(1-fingerIndex/5).pressed = false
            })
            return
        } else if (fingerIndex === 1) {
            qmlIndex = 4
        } else if (fingerIndex === 2) {
            qmlIndex = 5
        } else if (fingerIndex === 3) {
            qmlIndex = 6
        } else if (fingerIndex === 4) {
            qmlIndex = 7
        } else if (fingerIndex === 6) {
            qmlIndex = 3
        } else if (fingerIndex === 7) {
            qmlIndex = 2
        } else if (fingerIndex === 8) {
            qmlIndex = 1
        } else if (fingerIndex === 9) {
            qmlIndex = 0
        }

        letterButtons.itemAt(qmlIndex).pressed = true
        delay(200, function() {
            letterButtons.itemAt(qmlIndex).pressed = false
        })

        return "FingerIndex: " + fingerIndex + " triggered!"
    }
    function clearWordCandidates() {
        candidates.model = ["","","","",""];
    }
    function setWordCandidates(words, index) {
        candidates.model = words;
        if (index != -1) highlightCandidate(index)
    }
    function highlightCandidate(highlightedIndex){
        clearHighlight()
        candidates.itemAt(highlightedIndex).pressed = true
    }
    function clearHighlight(){
        var i;
        for (i = 0; i < 5; i++)
            candidates.itemAt(i).pressed = false
    }
    function updateDisplay(msg){
      resultText.text = msg
      clearWordCandidates()
    }

    // results area
    Rectangle {
        id: resultArea
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        height: parent.height * 3 / 8 - 10
        border.color: "white"
        border.width: 5
        color: "#4285F5"
        Text {
            id: resultText
            anchors.leftMargin: keystrokes.implicitMargin
            anchors.rightMargin: keystrokes.implicitMargin
            anchors.fill: parent
            horizontalAlignment: Text.AlignRight
            verticalAlignment: Text.AlignVCenter
            text: ""
            color: "white"
            font.pixelSize: window.height * 2 / 32
            font.family: "Open Sans Regular"
        }
    }

    // buttons
    Item {
        id: keystrokes
        anchors.top: resultArea.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        property real implicitMargin: {
            var ret = 0;
            for (var i = 0; i < visibleChildren.length; ++i) {
                var child = visibleChildren[i];
                ret += (child.implicitMargin || 0);
            }
            return ret / visibleChildren.length;
        }
        height: 120

        // word choices
        Rectangle {
            id: wordCandidates
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: parent.height * 3 / 8 - 10
            border.color: "white"
            border.width: 5
            color: "#F5F5F5"
            anchors.topMargin: -5
            Repeater {
                id: candidates
                model: ["","","","",""]
                Button {
                    x: index * width
                    width: parent.width/candidates.model.length
                    height: parent.height - 30
                    color: pressed ? "#bad1ff" : "#F5F5F5"
                    border.color: "white"
                    border.width: 5
                    text: modelData
                    fontHeight: 0.3
                }
            }
        }

        // letterButtons
        Repeater {
            id: letterButtons
            model: ["Q\nA\nZ", "W\nS\nX", "E\nD\nC", "R  T\nF  G\nV  B", "Y  U\nH   J\nN  M", "I\nK", "O\nL", "P"]
            Button {
                x: index <= 3 ? index * (parent.width / 10) :
                   index == 4 ? (index+1) * (parent.width / 10) : (index+2) * (parent.width / 10)
                y: parent.height / 5
                width: index == 3 || index == 4 ? parent.width / 5 : parent.width / 10
                height: parent.height / 5 * 3
                color: pressed ? "#4285F5" : "#F5F5F5"
                border.color: "white"
                border.width: 5
                text: modelData
                fontHeight: 0.2
            }
        }

        // arrowButtons
        Repeater {
            id: arrowButtons
            model: ["<", ">"]
            Button {
                x: index * width
                y: parent.height / 5 * 4
                width: parent.width / 2
                height: parent.height / 5
                color: pressed ? "#4285F5" : "#F5F5F5"
                border.color: "white"
                border.width: 5
                text: modelData
                fontHeight: 0.7
                onClicked: window.registerKeystroke(3)
            }
        }
    }

    // Timer for delay
    Timer {
        id: timer
    }
    function delay(delayTime, cb) {
        timer.interval = delayTime;
        timer.repeat = false;
        timer.triggered.connect(cb);
        timer.start();
    }
}
