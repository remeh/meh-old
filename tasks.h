#pragma once


#include <QKeyEvent>
#include <QList>
#include <QWidget>

class Window;
struct PluginRule;

// TasksPlugin is the basic definition of a TaskPlugin.
class TasksPlugin : public QWidget {
    Q_OBJECT

public:
    TasksPlugin(Window* window);
    void keyPressEvent(QKeyEvent* event, bool ctrl, bool shift);

    static QList<PluginRule> getSyntaxRules();

private:
    Window* window;
};
