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
    window.setWindowTitle(QObject::tr("meh - no file"));
    window.resize(800, 700);
    window.show();

    QStringList arguments = QCoreApplication::arguments();
    if (arguments.size() > 0) {
        for (int i = arguments.size() - 1; i > 0; i--) {
            if (arguments.at(i).startsWith("+")) {
                continue;
            }

            QFileInfo fi(arguments.at(i));
            if (fi.isDir()) {
                continue;
            }

            window.getEditor()->selectOrCreateBuffer(arguments.at(i));
        }

        // special cases about the last one
        if (arguments.last().startsWith("+")) {
            bool ok = false;
            int lineNumber = arguments.last().toInt(&ok);
            if (ok) {
                window.getEditor()->goToLine(lineNumber);
            }
        } else if (arguments.size() > 1) {
            // the last one is not a +###
            // checks whether it is a directory, if it is, we want to
            // set it as the base work dir.
            QFileInfo fi(arguments.last());
            if (fi.isDir()) {
                window.setBaseDir(fi.absoluteFilePath());
                window.openListFiles();
            }
        }
    }

    return app.exec();
}

