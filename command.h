#pragma once

#include <QKeyEvent>
#include <QLineEdit>

class Window;

class Command : public QLineEdit
{
    Q_OBJECT
public:
    Command(Window* window);

protected:
	void keyPressEvent(QKeyEvent* event);

private slots:

private:
	void execute(QString text);
	void openFile(const QString& filename);

	Window* window;
};
