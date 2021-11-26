#include <QFileInfo>

#include "editor.h"
#include "line_number_area.h"
#include "window.h"

LineNumberArea::LineNumberArea(Editor* editor) : QWidget(editor), editor(editor) {
    this->gitFlags.clear();
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

void LineNumberArea::clearGitFlags() {
    this->gitFlags.clear();
}

void LineNumberArea::setGitFlag(int line, int flag) {
    if (this->gitFlags.contains(line) && this->gitFlags[line] > 0 && this->gitFlags[line] != flag) {
        this->gitFlags[line] = GIT_FLAG_BOTH;
        return;
    }
    this->gitFlags[line] = flag;
}