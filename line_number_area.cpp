#include <QFileInfo>

#include "editor.h"
#include "line_number_area.h"
#include "window.h"

LineNumberArea::LineNumberArea(Editor* editor) : QWidget(editor), editor(editor) {
}

void LineNumberArea::paintEvent(QPaintEvent *event) {
    this->editor->lineNumberAreaPaintEvent(event);
}

void LineNumberArea::mousePressEvent(QMouseEvent *event) {
    int line = this->editor->lineNumberAtY(event->position().y());
    if (line == -1) {
        return;
    }

    this->editor->getWindow()->showLSPDiagnosticsOfLine(this->editor->getId(), line);
}

QSize LineNumberArea::sizeHint() const {
    return QSize(this->editor->lineNumberAreaWidth(), 0);
}
