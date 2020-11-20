#include <QFileInfo>

#include "window.h"
#include "line_number_area.h"

#include "qdebug.h"

LineNumberArea::LineNumberArea(Window *window, Editor* editor) : QWidget(editor), window(window) {
}

void LineNumberArea::paintEvent(QPaintEvent *event) {
    this->window->getEditor()->lineNumberAreaPaintEvent(event);
}

void LineNumberArea::mousePressEvent(QMouseEvent *event) {
    int line = this->window->getEditor()->lineNumberAtY(event->y());
    if (line == -1) {
        return;
    }

    auto allDiags = this->window->getEditor()->lspManager.getDiagnostics(this->window->getEditor()->getCurrentBuffer()->getFilename());
    auto lineDiags = allDiags[line];
    if (lineDiags.size() == 0) {
        return;
    }

    for (int i = 0; i < lineDiags.size(); i++) {
        auto diag = lineDiags[i];
        if (diag.message.size() > 0) {
            QFileInfo fi = QFileInfo(diag.absFilename);
            QString message = fi.fileName() + ":" + QString::number(diag.line) + " " + diag.message;
            this->window->getStatusBar()->setMessage(message);
        }
    }
}

QSize LineNumberArea::sizeHint() const {
    return QSize(this->window->getEditor()->lineNumberAreaWidth(), 0);
}
