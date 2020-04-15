#include "buffer.h"

Buffer::Buffer() {
}

Buffer::Buffer(QString filename) : filename(filename) {
}

QByteArray Buffer::readFile() {
	QFile file(filename);
	if (!file.exists()) {
		// TODO(remy): doesn't exist.
	}
	file.open(QIODevice::ReadWrite);
	return file.readAll();
}
