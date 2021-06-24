#pragma once


#include <QKeyEvent>
#include <QList>
#include <QWidget>

class Window;
struct HighlightingRule;

// TasksPlugin is the basic definition of a TaskPlugin.
class TasksPlugin : public QWidget {
    Q_OBJECT

public:
    TasksPlugin(Window* window);
    void keyPressEvent(QKeyEvent* event, bool ctrl, bool shift);

    static QList<HighlightingRule> getSyntaxRules();

private:
    Window* window;
};
