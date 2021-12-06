#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QLocalSocket>
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <QThread>
#include <QMessageBox>

#include "command.h"
#include "completer.h"
#include "editor.h"
#include "exec.h"
#include "fileslookup.h"
#include "git.h"
#include "grep.h"
#include "info_popup.h"
#include "replace.h"
#include "statusbar.h"
#include "window.h"

Window::Window(QApplication* app, QString instanceSocket, QWidget* parent) :
    app(app),
    instanceSocket(instanceSocket),
    QWidget(parent),
    commandServer(this) {
    // widgets
    // ----------------------
    this->command = new Command(this);
    this->command->setContentsMargins(2, 2, 2, 0);
    this->command->hide();
    this->filesLookup = new FilesLookup(this);
    this->filesLookup->hide();
    this->completer = new Completer(this);
    this->completer->hide();

    // status bar
    // ----------------------
    this->statusBar = new StatusBar(this);
    this->statusBar->show();

    this->infoPopup = new InfoPopup(this);
    this->infoPopup->hide();

    // replace
    // ----------------------

    this->replace = new ReplaceWidget(this);
    this->replace->hide();

    // set base dir
    // ----------------------

    QFileInfo dir(".");
    this->setBaseDir(dir.canonicalFilePath());

    // references widget displayed at the bottom of the editor
    // ----------------------

    this->refWidget = new ReferencesWidget(this);
    this->refWidget->hide();

    // grep instance
    // ----------------------

    this->grep = new Grep(this);
    this->grep->hide();

    // prepare the lsp manager
    // ----------------------

    this->lspManager = new LSPManager(this);

    // layout
    // ----------------------

    this->setStyleSheet("background-color: #262626; color: #ffffff;");

    this->tabs = new QTabWidget();
    this->tabs->setDocumentMode(true);
    this->tabs->setTabsClosable(true);
    this->tabs->setUsesScrollButtons(true);
    this->tabs->setFocusPolicy(Qt::NoFocus);
    this->tabs->setMovable(true);
    this->tabs->setStyleSheet(QString("QTabWidget { \
} \
QTabWidget::pane { \
} \
QTabWidget QTabBar{ \
background-color: #262626;  \
border: 0px;  \
height: 34px; \
qproperty-drawBase: 0; \
} \
QTabWidget QTabBar::tab{  \
padding: 0px 5px 0px 5px;  \
height: 34px;  \
color: #136ba2;  \
border-right: 0px; \
}  \
QTabWidget QTabBar::tab:selected{  \
background-color: #1e1e1e;  \
color: #ffffff;  \
}  \
QTabWidget QTabBar::tear{  \
background-color: #1e1e1e;  \
color: #262626;  \
}  \
QTabWidget QTabBar::tab:!selected{  \
background-color:#2d2d2d;  \
color:  #969696;  \
}  \
QTabWidget QTabBar::close-button{  \
image: url(:res/close.png); \
}  \
QTabWidget QTabBar::close-button:hover{  \
image: url(:res/close-hover.png); \
}  \
QTabWidget::tab-bar { \
} \
QTabWidget QTabBar::tab:hover{  \
background-color:  #5A5B5C;  \
color:  #ffffff;  \
}"));
    this->tabs->setElideMode(Qt::ElideNone);

    this->layout = new QGridLayout();
    this->layout->setContentsMargins(0, 0, 0, 0);
    this->layout->setVerticalSpacing(3);
    this->layout->addWidget(this->tabs);
    this->layout->addWidget(this->refWidget);
    this->layout->addWidget(this->replace);
    this->layout->addWidget(this->statusBar);
    this->setLayout(layout);

    // git instance and exec instance
    // ----------------------

    this->exec = new Exec(this);

    // command server
    // --------------

    commandServer.listen(instanceSocket);

    connect(this->tabs, &QTabWidget::tabCloseRequested, this, &Window::onCloseTab);
    connect(this->tabs, &QTabWidget::currentChanged, this, &Window::onChangeTab);
    connect(&this->commandServer, &QLocalServer::newConnection, this, &Window::onNewSocketCommand);
}

Window::~Window() {
    // XXX(remy): do I have to delete all editors myself since they are owned
    // by the QTabWidget?

    commandServer.close();
    delete this->tabs;
    delete this->grep;
    delete this->exec;
    delete this->lspManager;
}

void Window::onNewSocketCommand() {
    QLocalSocket* socket = this->commandServer.nextPendingConnection();
    if (socket == nullptr) {
        return;
    }
    socket->waitForDisconnected(500);
    QByteArray data = socket->readAll();
    socket->close();

    if (this->app) {
        this->app->alert(this, 10000);
        this->activateWindow();
        this->raise();
    }

    if (data.startsWith("open ")) {
        data.remove(0, 5);
        QStringList list = QString(data).split("###");
        for (int i = 0; i < list.size(); i++) {
            QString filename = list.at(i);
            this->setCurrentEditor(filename);
        }
    }
}

void Window::closeEvent(QCloseEvent* event) {
    if (!this->areYouSure()) {
        event->ignore();
    }
}

bool Window::areYouSure(const QString& message) {
    if (this->getEditor() != nullptr) {
        if (this->getEditor()->getBuffer() == nullptr) {
            return true;
        }
        if (this->getEditor()->getBuffer()->isGitTempFile()) {
            return true;
        }
    }

    QMessageBox msgBox;
    msgBox.setText(message);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Cancel);
    return msgBox.exec() == QMessageBox::Yes;
}

void Window::onCloseTab(int index) {
    if (index < 0) { return; }
    Editor* editor = this->getEditor(index);
    if (editor != nullptr) {
        this->closeEditor(editor->getId());
    }
}

void Window::onChangeTab(int index) {
    if (index < 0) { return; }
    Editor* editor = this->getEditor(index);
    if (editor != nullptr) {
        this->setCurrentEditor(editor->getId());
    }
}

bool Window::areYouSure() {
    return this->areYouSure("Are you sure to close the app?");
}

void Window::openCommand() {
    this->command->show();
    this->command->setFocus();
}

void Window::closeCommand() {
    this->command->hide();
    if (this->getEditor()) {
        this->getEditor()->setMode(MODE_NORMAL);
    }
}

void Window::openListFiles() {
    this->filesLookup->showFiles("");
}

void Window::openListBuffers() {
    this->filesLookup->showBuffers();
}

void Window::closeList() {
    this->filesLookup->hide();
}

void Window::openReplace() {
    this->replace->show();
}

void Window::closeReplace() {
    this->replace->hide();
}

void Window::paintEvent(QPaintEvent* event) {
    if (this->getEditor() == nullptr) {
        return;
    }
    // repaint the editor and its line number area
    this->getEditor()->update();
}

void Window::openCompleter(const QString& base, const QList<CompleterEntry> entries) {
    this->completer->clear();
    this->completer->setItems(base, entries);
    this->completer->show();
    this->completer->setFocus();
    if (entries.size() > 0) {
        this->completer->setCurrentItem(this->completer->topLevelItem(0));
    }
    this->completer->scroll(0, 0);
}

void Window::closeCompleter() {
    this->completer->hide();
}

void Window::closeInfoPopup() {
    this->infoPopup->hide();
}

void Window::openGrep(const QString& string) {
    this->openGrep(string, "");
}

void Window::openGrep(const QString& string, const QString& target) {
    if (target.size() > 0) {
        this->grep->grep(string, this->baseDir, target);
    } else {
        this->grep->grep(string, this->baseDir);
    }
    this->grep->show();
}

void Window::closeGrep() {
    this->grep->hide();
}

void Window::focusGrep() {
    this->grep->focus();
}

void Window::setCommand(const QString& text) {
    this->command->setText(text);
}

void Window::setBaseDir(const QString& dir) {
    QFileInfo info(dir);
    this->baseDir = info.canonicalFilePath();
    if (!this->baseDir.endsWith("/")) {
        this->baseDir += "/";
    }
}

void Window::resizeEvent(QResizeEvent* event) {
    if (this->getEditor() != nullptr) {
        this->getEditor()->onWindowResized(event);
    }
}

// buffers
// -----------------

// XXX(remy): merge newEditor methods

Editor* Window::newEditor(QString name, QByteArray content) {
    // we want to create a new Editor with this buffer.
    Editor* editor = new Editor(this);
    Buffer* buffer = new Buffer(editor, name, content);
    editor->setBuffer(buffer);
    int tabIdx = this->tabs->addTab(editor, editor->getId());
    this->tabs->setCurrentIndex(tabIdx);

    if (this->getEditor() != nullptr && this->getEditor()->getId() != editor->getId()) {
        this->setCurrentEditor(editor->getId());
    }

    return editor;
}

Editor* Window::newEditor(QString name, QString filename) {
    QFileInfo fi(filename);
    if (fi.isDir()) {
        this->setBaseDir(fi.canonicalFilePath());
        this->openListFiles();
        return nullptr;
    }

    // we want to create a new Editor with this buffer.
    Editor* editor = new Editor(this);
    Buffer* buffer = new Buffer(editor, name, filename);
    editor->setBuffer(buffer);

    QString label;
    QString dirName = fi.dir().dirName();
    if (dirName != ".") {
        label = dirName + "/" + fi.fileName();
    } else {
        label = fi.fileName();
    }

    label = QString("  ") + label;

    int tabIdx = this->tabs->addTab(editor, label);
    if (this->getEditor() != nullptr && this->getEditor()->getId() != editor->getId()) {
        this->setCurrentEditor(editor->getId());
    }
    return editor;
}

void Window::save() {
    Q_ASSERT(this->getEditor() != nullptr);
    this->getEditor()->save();
}

void Window::saveAll() {
    // save other buffers
    for (Editor* editor : this->getEditors()) {
        editor->save();
    }
}

bool Window::hasBuffer(const QString& id) {
    return this->getEditor(id) != nullptr;
}

Editor* Window::getEditor() {
    if (this->tabs == nullptr || this->tabs->count() == 0) {
        return nullptr;
    }

    // retrieve the editor from the currently opened tab
    QWidget* page = this->tabs->currentWidget();
    if (page == nullptr) {
        return nullptr;
    }
    return static_cast<Editor*>(page);
}

// XXX(remy): factorize getEditor and getEditorTabIndex

Editor* Window::getEditor(const QString& id) {
    for (int i = 0; i < this->tabs->count(); i++) {
        QWidget* page = this->tabs->widget(i);
        if (page == nullptr) {
            qDebug() << "Window::closeEditor: nullptr page, should never happen";
            continue;
        }
        Editor* ed = static_cast<Editor*>(page);
        if (ed == nullptr) {
            qDebug() << "Window::closeEditor: can't case page, should never happen";
            continue;
        }
        if (ed->getId() == id) {
            return ed;
        }
    }
    return nullptr;
}

Editor* Window::getEditor(int tabIndex) {
    QList<Editor*> editors = this->getEditors();
    return editors.at(tabIndex);
}

QList<Editor*> Window::getEditors() {
    QList<Editor*> list;
    for (int i = 0; i < this->tabs->count(); i++) {
        QWidget* page = this->tabs->widget(i);
        if (page == nullptr) {
            qDebug() << "Window::closeEditor: nullptr page, should never happen";
            continue;
        }
        Editor* ed = static_cast<Editor*>(page);
        if (ed == nullptr) {
            qDebug() << "Window::closeEditor: can't cast page, should never happen";
            continue;
        }
        list.append(ed);
    }
    return list;
}

int Window::getEditorTabIndex(Editor* editor) {
    Q_ASSERT(editor != nullptr);
    return this->getEditorTabIndex(editor->getId());
}

int Window::getEditorTabIndex(const QString& id) {
    for (int i = 0; i < this->tabs->count(); i++) {
        QWidget* page = this->tabs->widget(i);
        if (page == nullptr) {
            qDebug() << "Window::closeEditor: nullptr page, should never happen";
            continue;
        }
        Editor* ed = static_cast<Editor*>(page);
        if (ed == nullptr) {
            qDebug() << "Window::closeEditor: can't cast page, should never happen";
            continue;
        }
        if (ed->getId() == id) {
            return i;
        }
    }
    return -1;
}

Editor* Window::setCurrentEditor(QString id) {
    int tabIndex = this->getEditorTabIndex(id);
    Editor* currentEditor = this->getEditor();
    if (tabIndex >= 0) {
        this->tabs->setCurrentIndex(tabIndex);
        Editor* editor = this->getEditor();
        this->setWindowTitle(QString("meh - ") + this->tabs->tabText(this->tabs->currentIndex()).trimmed());
        this->statusBar->setEditor(editor);
        editor->update();
        editor->setFocus();

        if (currentEditor != nullptr && this->previousEditorId != currentEditor->getId()) {
            this->previousEditorId = QString(currentEditor->getId());
        }
        return editor;
    }

    // opens it as a file
    return this->newEditor(id, id);
}

void Window::closeCurrentEditor() {
    Editor* editor = this->getEditor();
    if (editor == nullptr) {
        return;
    }

    return this->closeEditor(editor->getId());
}

void Window::closeEditor(const QString& id) {
    // XXX(remy): is there any Qt signals to connect?

    // TODO(remy): we are going two times through the list of tabs here
    // we may want to do something more optimized. I've done it this way
    // for the time being since it avoid code duplication and is way more readable.
    Editor* editor = this->getEditor(id);

    Editor* currentEditor = this->getEditor();
    QString currentEditorId = "";
    if (currentEditor != nullptr) {
        currentEditorId = currentEditor->getId();
    }

    if (editor == nullptr) {
        qDebug() << "Window::closeEditor:" << "can't find editor with id:" << id;
    }

    if (editor->getBuffer()->modified) {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Unsaved buffer");
            msgBox.setText("The buffer you want to close has modifications. Close anyway?");
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
            msgBox.setDefaultButton(QMessageBox::Cancel);
            if (msgBox.exec() == QMessageBox::Cancel) {
                return;
            }
    }

    int tabIdx = this->getEditorTabIndex(id);

    this->tabs->removeTab(tabIdx);
    editor->deleteLater(); // since we use removeTab, it's not done by the QTabWidget

    if (id != currentEditorId) {
        // do not change current tab if the one just closed isn't the current one.
        return;
    }

    if (this->checkpoints.size() > 0) {
        if (this->lastCheckpoint()) {
            return;
        }
    }

    if (this->getEditor() != nullptr) {
        this->setCurrentEditor(this->getEditor()->getId());
        return;
    }

    this->newEditor("notes", QString("/tmp/meh-notes"));
}

// checkpoints
// -----------

void Window::saveCheckpoint() {
    if (this->getEditor() == nullptr || this->getEditor()->getBuffer() == nullptr) {
        return;
    }

    QString id = this->getEditor()->getId();
    int position = this->getEditor()->textCursor().position();
    Checkpoint c(id, position);

    if (this->checkpoints.isEmpty()) {
        this->checkpoints.append(c);
        return;
    }

    if (!this->checkpoints.isEmpty()) {
        Checkpoint last = this->checkpoints.last();
        if (last.id != id || last.position != position) {
            this->checkpoints.append(c);
        }
    }
}

bool Window::lastCheckpoint() {
    while (!this->checkpoints.isEmpty()) {
        Checkpoint c = this->checkpoints.takeLast();
        Editor* editor = this->getEditor(c.id);
        if (editor != nullptr) {
            this->setCurrentEditor(c.id);
            QTextCursor cursor = editor->textCursor();
            cursor.setPosition(c.position);
            editor->setTextCursor(cursor);
            // we've restored one, we can leave
            return true;
        }
    }
    return false;
}

// lsp
// -------------

void Window::showLSPDiagnosticsOfLine(const QString& buffId, int line) {
    auto allDiags = this->getLSPManager()->getDiagnostics(buffId);
    auto lineDiags = allDiags[line];
    if (lineDiags.size() == 0) {
        return;
    }

    for (int i = 0; i < lineDiags.size(); i++) {
        auto diag = lineDiags[i];
        if (diag.message.size() > 0) {
            QFileInfo fi = QFileInfo(diag.absFilename);
            QString message = fi.fileName() + ":" + QString::number(diag.line) + " " + diag.message;
            this->getStatusBar()->setMessage(message);
        }
    }
}

void Window::lspInterpretMessages(const QByteArray& data) {
    QList<QJsonDocument> list = LSPReader::readMessage(data);
    if (list.size() == 0) {
        return;
    }

    for (int i = 0; i < list.size(); i++) {
        this->lspInterpret(list[i]);
    }
}


void Window::lspInterpret(QJsonDocument json) {
    if (json.isNull() || json.isEmpty()) {
        return;
    }

    LSPAction action = this->lspManager->getExecutedAction(json["id"].toInt());
    if (action.requestId == 0) {
        if (json["method"].isNull()) {
            return;
        }
        // showMessage
        // -----------
        if (json["method"].toString() == "window/showMessage") {
            if (!json["params"].isNull()) {
                const QString& msg = json["params"]["message"].toString();
                if (msg.size() > 0) {
                    this->getStatusBar()->setMessage(msg);
                }
            }
        // publishDiagnostics
        // ------------------
        } else if (json["method"] == "textDocument/publishDiagnostics") {
            if (!json["params"].isNull() && !json["params"]["diagnostics"].isNull()) {
                if (!json["params"]["diagnostics"].isArray()) {
                    qWarning() << "lspInterpret: \"diagnostics\" is not an array";
                    return;
                }
                if (json["params"]["uri"].isNull()) {
                    qWarning() << "lspInterpret: no \"uri\" field in diagnostic";
                    return;
                }

                auto diags = json["params"]["diagnostics"].toArray();
                const QString& uri = QFileInfo(json["params"]["uri"].toString().replace("file://", "")).canonicalFilePath();

                this->lspManager->clearDiagnostics(uri);

                for (int i = 0; i < diags.size(); i++) {
                    QJsonObject diag = diags[i].toObject();
                    const QString& msg = diag["message"].toString();
                    if (!diag["range"].isNull() && !diag["range"].toObject()["start"].isNull()
                            && !diag["range"].toObject()["start"].toObject()["line"].isNull()) {
                        int line = diag["range"].toObject()["start"].toObject()["line"].toInt();
                        LSPDiagnostic diag;
                        diag.line = line + 1;
                        diag.message = msg;
                        diag.absFilename = uri;
                        this->lspManager->addDiagnostic(uri, diag);
                    }
                }
                this->repaint();
            }
        }
        return;
    }

    switch (action.action) {
        case LSP_ACTION_DECLARATION:
        case LSP_ACTION_DEFINITION:
            {
                // TODO(remy): deal with multiple results
                int line = json["result"][0]["range"]["start"]["line"].toInt();
                int column = json["result"][0]["range"]["start"]["character"].toInt();
                QString file = json["result"][0]["uri"].toString();
                if (file.isEmpty()) {
                    this->getStatusBar()->setMessage("Nothing found.");
                    return;
                }
                file.remove(0,7); // remove the file://
                this->saveCheckpoint();
                this->setCurrentEditor(file);
                this->getEditor()->goToLine(line + 1);
                this->getEditor()->goToColumn(column);
                return;
            }
        case LSP_ACTION_COMPLETION:
            {
                if (json["result"].isNull()) {
                    this->getStatusBar()->setMessage("Nothing found.");
                    return;
                }

                if (action.buffer->getId() != this->getEditor()->getId()) {
                    qDebug() << "debug: received an lsp response for another editor than the current one";
                    return;
                }

                LSP* lsp = this->lspManager->getLSP(this->getEditor()->getId()) ;
                if (lsp == nullptr) {
                    this->getStatusBar()->setMessage("Nothing found.");
                    return;
                }

                auto entries = lsp->getEntries(json);
                if (entries.size() == 0) {
                    this->getStatusBar()->setMessage("Nothing found.");
                    return;
                }

                const QString& base = this->getEditor()->getWordUnderCursor();
                this->openCompleter(base, entries);
                return;
            }
        case LSP_ACTION_HOVER:
            {
                if (json["result"].isNull() || json["result"]["contents"].isNull()) {
                    this->getInfoPopup()->setMessage("Nothing found.");
                    return;
                }

                auto contents = json["result"]["contents"].toObject();
                this->getInfoPopup()->setMessage(contents["value"].toString());
                return;
            }
        case LSP_ACTION_SIGNATURE_HELP:
            {
                if (!json["result"].isNull() && !json["result"]["signatures"].isNull() && json["result"]["signatures"].toArray().size() > 0) {
                    QString message;
                    QJsonArray signatures = json["result"]["signatures"].toArray();
                    for (int i = 0; i < signatures.size(); i++) {
                        QJsonValue signature = signatures[i];
                        message.append(signature["label"].toString()).append("\n");
                        message.append(signature["documentation"].toString());
                    }
                    this->getInfoPopup()->setMessage(message);
                    return;
                }
                this->getInfoPopup()->setMessage("Nothing found.");
                return;
            }
        case LSP_ACTION_REFERENCES:
            {
                // TODO(remy): error management
                this->getRefWidget()->clear();
                this->getRefWidget()->hide();
                QJsonArray list = json["result"].toArray();
                for (int i = 0; i < list.size(); i++) {
                    QJsonObject entry = list[i].toObject();
                    int line = entry["range"].toObject()["start"].toObject()["line"].toInt();
                    line += 1;
                    QString file = entry["uri"].toString();
                    if (file.startsWith("file://")) {
                        file = file.remove(0, 7);
                    }

                    const QString targetLine = this->getEditor()->getOneLine(file, line);
                    this->getRefWidget()->insert(file, QString::number(line), targetLine);
                }
                this->getRefWidget()->fitContent();
                this->getRefWidget()->show();
                this->getRefWidget()->setFocus();
                return;
            }
    }
}