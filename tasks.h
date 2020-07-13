#pragma once

#include <QKeyEvent>
#include <QWidget>

class Window;

class TasksPlugin : public QWidget {
    Q_OBJECT

public:
    TasksPlugin(Window* window);
    void keyPressEvent(QKeyEvent* event, bool ctrl, bool shift);

private:
    Window* window;
};
