#include "info_popup.h"
#include "window.h"

InfoPopup::InfoPopup(Window* window) :
    QPlainTextEdit(window),
    window(window) {
    Q_ASSERT(window != NULL);
    this->setFont(Editor::getFont());
    this->setFocusPolicy(Qt::NoFocus);
    this->setStyleSheet("border: 1px solid #999999;");
}

void InfoPopup::show() {
    QPlainTextEdit::show();
    QPlainTextEdit::raise();

    QRect cursorRect = this->window->getEditor()->cursorRect();
    int winWidth = this->window->size().width();
    int popupWidth = winWidth  - winWidth/3;
    QWidget::show();
    this->resize(qBound(600, winWidth * 3/5, winWidth * 3/5), 200);
    this->move(cursorRect.x() + 30, cursorRect.y() + 60);
}

void InfoPopup::keyPressEvent(QKeyEvent* event) {
    Q_ASSERT(event != NULL);

    switch (event->key()) {
        case Qt::Key_Escape:
            this->clear();
            this->hide();
            return;
    }
}

void InfoPopup::setMessage(const QString& message) {
    this->setPlainText(message);
    this->show();
}

void InfoPopup::moveNearMouse() {
    QPoint p = this->window->mapFromGlobal(QCursor::pos());
    this->move(p.x(), p.y() + 10);
}