#pragma once

#include <QHBoxLayout>
#include <QList>
#include <QMouseEvent>
#include <QPushButton>
#include <QSignalMapper>
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
    void setModified(bool modified);

protected:

private slots:
    void onClicked();

private:
    Window* window;

    QList<QPushButton*> labels;
    QString fullpath;
    QHBoxLayout* layout;
    bool modified;
};
