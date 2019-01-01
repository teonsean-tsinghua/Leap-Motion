#include"keyboardui.h"


Keyboardui::Keyboardui() {}
void Keyboardui::testMethod() {
  QVariant returnedValue;
  QVariant msg = "Message from C++";

  QMetaObject::invokeMethod(object, "myQmlFunction",
          Q_RETURN_ARG(QVariant, returnedValue),
          Q_ARG(QVariant, msg));
}

void Keyboardui::init(int argc, char** argv) {
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qml/KeyboardUI.qml")));
    if (engine.rootObjects().isEmpty()) return;
    QQmlComponent component(&engine, QUrl(QStringLiteral("qml/KeyboardUI.qml")));
    object = component.create();
    testMethod();
    test_int = 20;
    std::cout << "test_int set to 20\n";
    app.exec();
}

void Keyboardui::registerKeystroke(int fingerIndex) {
  QVariant msg = fingerIndex;
  QMetaObject::invokeMethod(object, "registerKeystroke", Q_ARG(QVariant, msg));
}
