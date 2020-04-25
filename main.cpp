#include <QApplication>

#include "editor.h"
#include "window.h"

int main(int argv, char **args)
{
    QApplication app(argv, args);

	Window window;
    window.setWindowTitle(QObject::tr("mh"));
    window.resize(800, 700);
    window.show();

	int arguments = QCoreApplication::arguments().size();
	if (arguments > 0) {
		for (int i = arguments - 1; i > 0; i--) {
			window.getEditor()->selectOrCreateBuffer(QCoreApplication::arguments().at(i));
		}
	}

    return app.exec();
}

