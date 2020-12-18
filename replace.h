#pragma once

#include <QKeyEvent>
#include <QLabel>
#include <QObject>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QWidget>

class Window;

class ReplaceWidget : public QWidget {
    Q_OBJECT

public:
    ReplaceWidget(Window* window);

    void show();
    void clear();

    void replace();

protected:
    void keyPressEvent(QKeyEvent*) override;

private:
    Window* window;
    QVBoxLayout* layout;
    QLabel* searchFor;
    QLineEdit* searchForEdit;
    QLabel* replaceWith;
    QLineEdit* replaceWithEdit;
};
