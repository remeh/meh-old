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

    return app.exec();
}

