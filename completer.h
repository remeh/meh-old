#pragma once

#include <QKeyEvent>
#include <QLineEdit>
#include <QListWidget>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QWidget>

class Window;

class Completer : public QListWidget {
    Q_OBJECT

public:
    Completer(Window* window);

    void setItems(const QString& base, const QStringList& list);

protected:
    void keyPressEvent(QKeyEvent*) override;

private:
    Window* window;
    QString base;

};
