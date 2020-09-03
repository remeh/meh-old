#pragma once

#include <QGridLayout>
#include <QKeyEvent>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QWidget>

#include "references_widget.h"

class Window;

class Grep : public QWidget {
    Q_OBJECT

public:
    Grep(Window* window);

    // TODO(remy): comment me
    // baseDir is the directory from which the command is run.
    // target is the target in which ripgrep should look for matches.
    void grep(const QString& string, const QString& baseDir, const QString& target);
    void grep(const QString& string, const QString& baseDir);

    // openSelection opens the needed buffer as the proper line.
    void openSelection();

    // TODO(remy): comment me
    void readAndAppendResult(const QString& result);

    void show();
    void hide();

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
