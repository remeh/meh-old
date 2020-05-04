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

protected:
    void keyPressEvent(QKeyEvent* event);

private slots:

private:
    void openFile(const QString& filename);

    Window* window;
};
