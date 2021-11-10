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
        completion(completion), infos(infos) {}
    CompleterEntry(const QString& completion, const QString& infos, bool isFunc) :
        completion(completion), infos(infos), isFunc(isFunc) {}
    QString completion;
    QString infos;
    bool isFunc;
};

class Completer : public QTreeWidget {
    Q_OBJECT

public:
    Completer(Window* window);

    void setItems(const QString& base, const QList<CompleterEntry> list);
    void show();

protected:
    void keyPressEvent(QKeyEvent*) override;

private:
    Window* window;
    QString base;

    void fitContent();
};
