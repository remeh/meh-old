#include <QKeyEvent>
#include <QTextCursor>

#include "qdebug.h"

#include "editor.h"
#include "window.h"

void Editor::leaderModeSelectSubMode() {
    if (this->currentBuffer == nullptr) {
        this->setSubMode(NO_SUBMODE);
        this->setMode(MODE_NORMAL);
        return;
    }

    // select the plugin to interpret the command
    // ------------------------------------------

    if (this->currentBufferExtension() == "tasks") {
        this->modeLabel->setText("TASKS");
        this->setSubMode(SUBMODE_tasks);
        return;
    }

    this->setSubMode(NO_SUBMODE);
    this->setMode(MODE_NORMAL);
    return;
}

void Editor::keyPressEventLeaderMode(QKeyEvent* event, bool ctrl, bool shift) {
    Q_ASSERT(event != nullptr);

    switch (this->subMode) {
        case SUBMODE_tasks:
            if (this->tasksPlugin != nullptr) {
                this->tasksPlugin->keyPressEvent(event, ctrl, shift);
            }
            return;
    }

    switch (event->key()) {
        case Qt::Key_Escape:
            this->setMode(MODE_NORMAL);
            this->setSubMode(NO_SUBMODE);
            return;
    }
}