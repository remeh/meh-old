#include "info_popup.h"
#include "window.h"

InfoPopup::InfoPopup(Window* window) :
    window(window) {
    Q_ASSERT(window != NULL);
    this->setFont(Editor::getFont());
    this->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    this->setFocusPolicy(Qt::NoFocus);
    this->setAttribute(Qt::WA_ShowWithoutActivating);
}

void InfoPopup::show() {
    QPlainTextEdit::show();
    QPlainTextEdit::raise();

    QPoint wpos = this->window->pos();
    QRect cursorRect = this->window->getEditor()->cursorRect();
    int winWidth = this->window->size().width();
    int popupWidth = winWidth  - winWidth/3;
    QWidget::show();
    this->resize(qBound(400, winWidth * 2/3, winWidth * 2/3), 200);
    this->move(wpos.rx() + cursorRect.x() + 30, wpos.ry() + cursorRect.y() + 84);
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