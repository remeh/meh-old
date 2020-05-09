#include <QApplication>
#include <QObject>
#include <QStringList>

#include "editor.h"
#include "window.h"

int main(int argv, char **args)
{
    QApplication app(argv, args);
    app.setCursorFlashTime(0);
    QCoreApplication::setOrganizationName("mehteor");
    QCoreApplication::setOrganizationDomain("remy.io");
    QCoreApplication::setApplicationName("meh");

    Window window;
    window.setWindowTitle(QObject::tr("meh"));
    window.resize(800, 700);
    window.show();

    QStringList arguments = QCoreApplication::arguments();
    if (arguments.size() > 0) {
        for (int i = arguments.size() - 1; i > 0; i--) {
            if (arguments.at(i).startsWith("+")) {
                continue;
            }
            window.getEditor()->selectOrCreateBuffer(arguments.at(i));
        }

        if (arguments.last().startsWith("+")) {
            bool ok = false;
            int lineNumber = arguments.last().toInt(&ok);
            if (ok) {
                window.getEditor()->goToLine(lineNumber);
            }
        }
    }

    return app.exec();
}

