#pragma once

#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QWidget>

#include "breadcrumb.h"

class Editor;
class Window;

class StatusBar : public QWidget {
    Q_OBJECT

public:
    StatusBar(Window* window);

    void setMode(const QString& mode);
    void setEditor(Editor* editor);
    void setMessage(const QString& message);
    void setLineNumber(int lineNumber);
    void setModified(bool);

    void showMessage() {
        this->message->show();
    }
    void hideMessage() {
		this->message->setPlainText("");
		this->message->hide();
	}

	void setLspRunning(bool running);

protected:
private:
    Window* window;
    Breadcrumb* breadcrumb;
    QVBoxLayout* vlayout;
    QGridLayout* glayout;
    QLabel* mode;
    QLabel* lineNumber;
    QPlainTextEdit* message;

    bool lspWorking;
};
