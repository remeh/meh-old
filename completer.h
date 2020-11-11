#pragma once

#include <QKeyEvent>
#include <QLineEdit>
#include <QTreeWidget>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QWidget>

class Window;

class CompleterEntry {
public:
    CompleterEntry(const QString& completion, const QString& infos) :
        completion(completion), infos(infos) {

    }
    QString completion;
    QString infos;
};

class Completer : public QTreeWidget {
    Q_OBJECT

public:
    Completer(Window* window);

    void setItems(const QString& base, const QList<CompleterEntry> list);

protected:
    void keyPressEvent(QKeyEvent*) override;

private:
    Window* window;
    QString base;

};
