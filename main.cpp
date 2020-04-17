#include <QApplication>

#include "editor.h"

int main(int argv, char **args)
{
    QApplication app(argv, args);

	Buffer* buffer = new Buffer("/Users/remy.mathieu/go/src/github.com/DataDog/datadog-agent/.gitlab-ci.yml");

    Editor editor;
    editor.setWindowTitle(QObject::tr("mh")); // TODO(remy): move this to the App class
	editor.setFontFamily("Inconsolata");
	editor.setCurrentBuffer(buffer);
    editor.show();

    return app.exec();
}

