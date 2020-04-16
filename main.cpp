#include <QApplication>

#include "editor.h"

int main(int argv, char **args)
{
    QApplication app(argv, args);

	Buffer buffer("/home/remy/docs/code/gopath/src/github.com/DataDog/datadog-agent/.gitlab-ci.yml");

    Editor editor;
    editor.setWindowTitle(QObject::tr("mh")); // TODO(remy): move this to the App class
	editor.setFontFamily("Monospace");
	editor.setCurrentBuffer(&buffer);
    editor.show();

    return app.exec();
}

