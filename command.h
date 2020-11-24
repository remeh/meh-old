#pragma once

#include <QKeyEvent>
#include <QLineEdit>

class Window;

class Command : public QLineEdit
{
    Q_OBJECT
public:
    Command(Window* window);

    void execute(QString text);
    // warningModifiedBuffers displays a warning if we're trying to close while
    // some buffers were not saved. Return true only when there is buffers not saved.
    bool warningModifiedBuffers();
    void show();

protected:
    void keyPressEvent(QKeyEvent* event);

private slots:

private:
    void openFile(const QString& filename);

    bool areYouSure();

    // historyIdx starts with 0, use it to count backward.
    int historyIdx;

    Window* window;
};
