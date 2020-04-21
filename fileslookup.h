#pragma once

#include <QEvent>
#include <QFileInfo>
#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QObject>
#include <QString>
#include <QWidget>

class Window;

class FilesLookup : public QWidget {
	Q_OBJECT

public:
	FilesLookup(Window* window);

	// TODO(remy): comment me
	void lookupDir(QString filepath);

	// resetFiltered resets the list with filetered results (they again
	// contain all entries).
	void resetFiltered();

	// refreshList refreshes the content of the list.
	void refreshList();

	// openSelection opens the given selection: if it is a files, it opens
	// the buffer, if it is a directory, it opens the directory and refreshes
	// the list of the FilesLookup.
	// Returns true if we're done and we can close the FilesLookup.
	bool openSelection();

	// TODO(remy): comment me
	void filter(QString string);

	void show();
	void hide();

public slots:
	void onEditChanged();

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
};
