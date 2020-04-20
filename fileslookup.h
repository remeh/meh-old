#pragma once

#include <QEvent>
#include <QFileInfo>
#include <QGridLayout>
#include <QKeyEvent>
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

	QLineEdit* edit;
	QListWidget* list;
	QGridLayout* layout;

	QSet<QString> directories;
	QSet<QString> filteredDirs;
	QSet<QString> filenames;
	QSet<QString> filteredFiles;
};
