#pragma once

#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QPlainTextEdit>
#include <QVBoxLayout>
#include <QWidget>

class Buffer;
class Window;

class StatusBar : public QWidget {
    Q_OBJECT

public:
    StatusBar(Window* window);

    void setMode(const QString& mode);
    void setBuffer(Buffer* buffer);
    void setMessage(const QString& filename);
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

public slots:
    void onFilenameClicked();
protected:
private:
    Window* window;
    QVBoxLayout* vlayout;
    QGridLayout* glayout;
    QLabel* mode;
    QPushButton* filename;
    QLabel* lineNumber;
    QPlainTextEdit* message;

    bool lspWorking;
};
