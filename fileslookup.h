#pragma once

#include <QEvent>
#include <QFileInfo>
#include <QFrame>
#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QWidget>

#define FILESLOOKUP_DATA_LABEL 0 << 2 // label to display
#define FILESLOOKUP_DATA_ID    1 << 2 // id in the list of buffers
#define FILESLOOKUP_DATA_TYPE  2 << 2 // possible values: directory, file, buffer

class Window;

class FilesLookup : public QFrame {
    Q_OBJECT

public:
    FilesLookup(Window* window);

    // TODO(remy): comment me
    void lookupDir(QString filepath);

    // TODO(remy): comment me
    void lookupBuffers();

    // showLists is used by external objects to open the files lookup dialog and show a list of files.
    void showList(QList<QString> files);

    // resetFiltered resets the list with filtered results: they again contain all entries.
    void resetFiltered();

    // refreshList refreshes the content of the list.
    void refreshList();

    // openSelection opens the given selection: if it is a files, it opens
    // the buffer, if it is a directory, it opens the directory and refreshes
    // the list of the FilesLookup.
    // Returns true if we're done and we can close the FilesLookup.
    bool openSelection();

    // TODO(remy): comment me
    void filter();

    void show();
    void showFiles();
    void showBuffers();
    void hide();
    void setLabel(const QString& text) { this->label->setText(text); }

public slots:
    void onEditChanged();
    void onItemDoubleClicked();

protected:
    void keyPressEvent(QKeyEvent* event);

private:
    Window* window;

    QString base;

    QLineEdit* edit;
    QLabel* label;
    QListWidget* list;
    QGridLayout* layout;

    QSet<QString> directories;
    QSet<QString> filteredDirs;
    QSet<QString> filenames;
    QSet<QString> filteredFiles;
    QSet<QString> buffers;
    QSet<QString> filteredBuffers;
};
