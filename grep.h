#pragma once

#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QTreeWidget>
#include <QWidget>

class Window;

class Grep : public QWidget {
	Q_OBJECT

public:
	Grep(Window* window);

	// TODO(remy): comment me
	void grep(const QString& string, const QString& baseDir);

	// openSelection opens the needed buffer as the proper line.
	void openSelection();

	// TODO(remy): comment me
	void readAndAppendResult(const QString& result);

	// TODO(remy): remove entry from the tree

	void show();
	void hide();

public slots:
	void onErrorOccurred();
	void onResults();
	void onFinished();

protected:
	void keyPressEvent(QKeyEvent* event);

private:
	Window* window;

	QLabel* label;
	QTreeWidget* tree;
	QGridLayout* layout;

	QProcess* process;
	
	int resultsCount;
};
