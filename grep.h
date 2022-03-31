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

    // baseDir is the directory from which the command is run.
    // target is the target in which ripgrep should look for matches.
    // Returns 0 if a grep has been started, -1 otherwise.
    int grep(const QString& string, const QString& baseDir, const QString& target);
    int grep(const QString& string, const QString& baseDir);

    // openSelection opens the needed buffer as the proper line.
    void openSelection();

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
