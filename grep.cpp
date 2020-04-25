#include <QByteArray>
#include <QTreeWidgetItem>

#include "grep.h"
#include "window.h"

Grep::Grep(Window* window) :
	QWidget(window),
	window(window) {
	Q_ASSERT(window != nullptr);

	this->label = new QLabel(this);
	this->tree = new QTreeWidget(this);
	this->tree->setSortingEnabled(true);
	this->tree->setColumnCount(3);
	QStringList columns;
	columns << "File" << "Line #" << "Line";
	this->tree->setHeaderLabels(columns);

	this->setFocusPolicy(Qt::StrongFocus);

	this->layout = new QGridLayout();
	this->layout->setContentsMargins(0, 0, 0, 0);
	this->layout->addWidget(this->label);
	this->layout->addWidget(this->tree);
	this->setLayout(layout);

	this->process = nullptr;
}

void Grep::show() {
	this->label->show();
	this->tree->show();
	this->label->setFocus();
	QWidget::show();
}

void Grep::hide() {
	if (this->process != nullptr) {
		this->process->kill();
		delete this->process;
		this->process = nullptr;
	}
	this->tree->clear();
	this->label->hide();
	this->tree->hide();
	QWidget::hide();	
}

void Grep::onResults() {
	QByteArray data = this->process->readLine();
	for (; data.size() > 0; data = this->process->readLine()) {
		this->readAndAppendResult(data);
		this->resultsCount++;
	}
	this->label->setText(" " + QString::number(this->resultsCount) + " results");
}

void Grep::onErrorOccurred() {
	delete this->process;
	this->process = nullptr;
}

void Grep::onFinished() {
	if (this->process) {
		delete this->process;
		this->process = nullptr;
	}
	
	// TODO(remy): show that it has finished (ui wise)
}

void Grep::keyPressEvent(QKeyEvent* event) {
	switch (event->key()) {
		case Qt::Key_Escape:
			this->window->closeGrep();
			this->window->getEditor()->setMode(MODE_NORMAL);
			return;
	}

	#ifdef Q_OS_MAC
		bool ctrl = event->modifiers() & Qt::MetaModifier;
	#else
		bool ctrl = event->modifiers() & Qt::ControlModifier;
	#endif

	switch (event->key()) {
		case Qt::Key_Return:
			{
				QTreeWidgetItem* currentItem = this->tree->currentItem();
				if (currentItem == nullptr) {
					return;
				}
				QString file = currentItem->text(0);
				QString lineNumber = currentItem->text(1);
				this->window->getEditor()->selectOrCreateBuffer(file);
				this->window->getEditor()->goToLine(lineNumber.toInt());
				return;
			}
		case Qt::Key_N:
			if (!ctrl) {
				return;
			}
			__attribute__ ((fallthrough));
		case Qt::Key_J:
			{
				QTreeWidgetItem* currentItem = this->tree->currentItem();
				if (currentItem == nullptr) {
					currentItem = this->tree->topLevelItem(0);
					this->tree->setCurrentItem(currentItem);
					return;
				}
				currentItem = this->tree->itemBelow(currentItem);
				if (currentItem != nullptr) {
					this->tree->setCurrentItem(currentItem);
				}
			}
			return;
		case Qt::Key_Backspace:
		case Qt::Key_X:
			{
				QTreeWidgetItem* currentItem = this->tree->currentItem();
				if (currentItem != nullptr) {
					delete currentItem;
				}
			}
			return;
		case Qt::Key_P:
			if (!ctrl) {
				return;
			}
			__attribute__ ((fallthrough));
		case Qt::Key_K:
			{
				QTreeWidgetItem* currentItem = this->tree->currentItem();
				if (currentItem == nullptr) {
					currentItem = this->tree->topLevelItem(0);
					this->tree->setCurrentItem(currentItem);
					return;
				}
				currentItem = this->tree->itemAbove(currentItem);
				if (currentItem != nullptr) {
					this->tree->setCurrentItem(currentItem);
				}
			}
			return;
	}
}

void Grep::readAndAppendResult(const QString& result) {
	// first two are the file and the line number
	QStringList parts = result.split(":").mid(0, 2);
	// the rest is the result
	QString line = result.section(":", 2);
	parts << line.trimmed();
	new QTreeWidgetItem(this->tree, parts);
}

// TODO(remy): support case insensitive
void Grep::grep(const QString& string, const QString& baseDir = ".") {
	if (this->process != nullptr) {
		this->process->kill();
		delete this->process;
		this->process = nullptr;
	}
	this->resultsCount = 0;
	this->tree->clear();
	this->process = new QProcess(this);
	QFileInfo info(baseDir);
	this->process->setWorkingDirectory(info.absoluteFilePath());
	QStringList list; list << "-n" << string << ".";
	this->process->start("rg", list);
	connect(this->process, &QProcess::readyReadStandardOutput, this, &Grep::onResults);
	connect(this->process, &QProcess::errorOccurred, this, &Grep::onErrorOccurred);
	connect(this->process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Grep::onFinished);
}
