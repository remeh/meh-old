#pragma once

#include <QHBoxLayout>
#include <QList>
#include <QMouseEvent>
#include <QPushButton>
#include <QString>
#include <QWidget>

class Window;

class Breadcrumb : public QWidget
{
	Q_OBJECT
public:
    Breadcrumb(Window* window);

    void recomputeLabels();
    void setFullpath(const QString& fullpath);
    void deleteLabels();

protected:
    void mousePressEvent(QMouseEvent*) override;

private:
    Window* window;

    QList<QPushButton*> labels;
    QString fullpath;
    QHBoxLayout* layout;
};
