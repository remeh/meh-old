#pragma once

#include <QByteArray>
#include <QFile>
#include <QString>

class Buffer
{
public:
	Buffer();
	// Buffer creates a buffer targeting a given file.
	Buffer(QString filename);

	// readFile reads the file on disk and returns its content.
	QByteArray readFile();

protected:

private:
	QString filename;
	QByteArray data;
};
