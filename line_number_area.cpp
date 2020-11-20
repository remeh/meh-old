#include <QFileInfo>

#include "editor.h"
#include "line_number_area.h"

LineNumberArea::LineNumberArea(Editor* editor) : QWidget(editor), editor(editor) {
}

void LineNumberArea::paintEvent(QPaintEvent *event) {
    this->editor->lineNumberAreaPaintEvent(event);
}

void LineNumberArea::mousePressEvent(QMouseEvent *event) {
    int line = this->editor->lineNumberAtY(event->y());
    if (line == -1) {
        return;
    }

    this->editor->showLSPDiagnosticsOfLine(line);
}

QSize LineNumberArea::sizeHint() const {
    return QSize(this->editor->lineNumberAreaWidth(), 0);
}
