#pragma once

#include <QTextEdit>

QT_BEGIN_NAMESPACE
class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
QT_END_NAMESPACE

class Window : public QTextEdit
{
    Q_OBJECT
public:
    Window(QWidget *parent = nullptr);

protected:

private slots:

private:

};
