#include <QApplication>

#include "window.h"

int main(int argv, char **args)
{
    QApplication app(argv, args);

	Buffer buffer("/home/remy/docs/code/gopath/src/github.com/DataDog/datadog-agent/.gitlab-ci.yml");

    Window window;
    window.setWindowTitle(QObject::tr("mh"));
	window.setFontFamily("Monospace");
	window.setCurrentBuffer(&buffer);
    window.show();

    return app.exec();
}

