#pragma once

#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QWidget>

class Window;

class StatusBar : public QWidget {
    Q_OBJECT

public:
    StatusBar(Window* window);

    void setMode(const QString& mode);
    void setFilename(const QString& filename);
    void setLineNumber(int lineNumber);
    void setModified(bool);

public slots:
    void onFilenameClicked();
protected:
private:
    Window* window;
    QGridLayout* layout;
    QLabel* mode;
    QPushButton* filename;
    QLabel* lineNumber;
};
