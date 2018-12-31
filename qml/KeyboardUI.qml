import QtQuick 2.5
import QtQuick.Window 2.0
import QtScxml 5.8

Window {
    id: window
    visible: true
    width: 480
    height: 240

    function myQmlFunction(msg) {
        console.log("KeyboardUI.qml receives", msg)

        return "Message from KeyboardUI.qml saying hi!"
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
            text: "0"
            color: "white"
            font.pixelSize: window.height * 3 / 32
            font.family: "Open Sans Regular"
            fontSizeMode: Text.Fit
        }
    }

    // keystrokes
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
            id: wordSelection
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            height: parent.height * 3 / 8 - 10
            border.color: "white"
            border.width: 5
            color: "#F5F5F5"
            anchors.topMargin: -5
        }

        // keystrokes
        Repeater {
            id: digits
            model: ["Q\nA\nZ", "W\nS\nX", "E\nD\nC", "R  T\nF  G\nV  B", "Y  U\nH  J\nN  M", "I\nK", "O\nL", "P"]
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
                onClicked: statemachine.submitEvent(eventName)
            }
        }

        // arrows
        Repeater {
            id: arrows
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
                onClicked: window.myQmlFunction("this is arrows")
            }
        }
    }
}
