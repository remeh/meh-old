#include <QApplication>
#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QLocalSocket>
#include <QObject>
#include <QStringList>
#include <QStyleFactory>
#include <QThread>

#include <stdio.h>

#include "buffer.h"
#include "editor.h"
#include "git.h"
#include "window.h"

#include "qdebug.h"

int main(int argv, char **args)
{
    QApplication app(argv, args);
    app.setCursorFlashTime(0);
    app.setWheelScrollLines(5);
    app.setStyle(QStyleFactory::create("Fusion"));
    QCoreApplication::setOrganizationName("mehteor");

    QCoreApplication::setOrganizationDomain("remy.io");
    QCoreApplication::setApplicationName("meh");
    QStringList arguments = QCoreApplication::arguments();

    // -n flag: to use a different instance than /tmp/meh.sock
    // --------------

    QString instanceFile = "";
    if (!arguments.empty() && arguments.size() > 1 &&
        arguments.at(1).startsWith("-n")) {
        instanceFile = arguments.at(1).right(-2);
        arguments.removeFirst();
    }

    instanceFile = QString("/tmp/meh") + instanceFile + ".sock";

    // if there is an existing instance, send it the command to open a file
    // instead of creating a new window
    // ---------------

    if (!arguments.empty() && QFile::exists(instanceFile) &&
         arguments.size() >= 2 && arguments.at(1) != "-n" &&
         !Git::isGitTempFile(arguments.at(1)) && arguments.at(1) != "-") {

        QLocalSocket socket;
        socket.connectToServer(instanceFile);

        if (socket.state() != QLocalSocket::ConnectedState) {
            qDebug() << "An error happened while connecting to " << instanceFile <<
                socket.errorString();
            qDebug() << "Will create a new instance instead.";
        } else {
            arguments.removeFirst();
            if (arguments.empty()) {
                arguments.append("/tmp/meh-notes");
            }
            for (int i = 0; i < arguments.size(); i++) {
                QFileInfo fi(arguments.at(i));
                if (fi.exists()) {
                  arguments[i] = fi.canonicalFilePath();
                } else {
                  arguments[i] = QDir::currentPath() + "/" + arguments.at(i);
                }
            }
            QString data = "open " + arguments.join("###");
            socket.write(data.toLatin1());
            socket.flush();
            socket.close();
            return 0;
        }
    }

    if (arguments.size() >= 2 && arguments.at(1) == "-n") {
        arguments.remove(1);
    }

	qDebug() << "Creating a new instance:" << instanceFile;

    Window window(&app, instanceFile);
    window.setWindowTitle(QObject::tr("meh - no file"));
    window.resize(800, 700);
    window.show();

	// remove the binary name
    arguments.takeFirst();

    // special case of reading from stdin
    if (arguments.size() > 0 && arguments.at(0) == "-") {

        QByteArray content;

        QFile in;
        if (!in.open(stdin, QIODevice::ReadOnly)) {
            qWarning() << "can't read stdin";
            qWarning() << in.errorString();
        }
        content += in.readAll();
        in.close();

        window.newEditor("stdin", content);
    } else if (arguments.size() > 0) {
        for (int i = arguments.size() - 1; i >= 0; i--) {
			// ignore if this is a position
            if (arguments.at(i).startsWith("+")) {
                continue;
            }

            // ignore if this is a directory
            QFileInfo fi(arguments.at(i));
            if (fi.isDir()) {
                continue;
            }

            window.newEditor(arguments.at(i), arguments.at(i));
        }

        // special cases about the last one
        if (arguments.last().startsWith("+")) {
            bool ok = false;
            int lineNumber = arguments.last().toInt(&ok);
            if (ok) {
                window.getEditor()->goToLine(lineNumber);
            }
        } else {
            // the last one is not a +###
            // checks whether it is a directory, if it is, we want to
            // set it as the base work dir.
            QFileInfo fi(arguments.last());
            if (fi.isDir()) {
                window.setBaseDir(fi.canonicalFilePath());
                window.openListFiles();
            }
        }
    } else {
        window.newEditor("notes", QString("/tmp/meh-notes"));
    }

    return app.exec();
}

