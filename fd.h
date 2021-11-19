#pragma once

#include <QGridLayout>
#include <QKeyEvent>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QWidget>

#include "references_widget.h"

class Window;

class FD : public QWidget {
    Q_OBJECT

public:
    FD(Window* window);

    void run(const QString& string);

    // TODO(remy): comment me
    void readAndAppendResult(const QString& result);

    void show();
    void hide();
    void focus();

public slots:
    void onErrorOccurred();
    void onResults();
    void onFinished();

protected:
    void keyPressEvent(QKeyEvent* event);

private:
    Window* window;

    ReferencesWidget *refWidget;
    QGridLayout *layout;

    QProcess* process;
    QString buff;
    int resultsCount;
};
