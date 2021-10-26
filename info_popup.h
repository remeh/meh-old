#pragma once

#include <QPlainTextEdit>
#include <QWidget>

class Editor;
class Window;

class InfoPopup : public QPlainTextEdit {
    Q_OBJECT

public:
    InfoPopup(Window* window);

    void setMessage(const QString& message);
    void show();
protected:
    void keyPressEvent(QKeyEvent* event);

private:
    Window* window;
};
