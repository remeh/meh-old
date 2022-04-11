#include <QApplication>
#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QLocalSocket>
#include <QLoggingCategory>
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

class Argument : public QString {
public:
    Argument(const QString& str) : QString(str) {};
    bool isPosition() const { return this->startsWith("+"); }
    bool isStdin() const { return *this == QString("-"); }
    bool isNewWindowFlag() const { return startsWith("-n"); }
};


// reuseInstance may open the given arguments in an existing instance.
// Returns true if an instance has been reused, false otherwise.
bool reuseInstance(QList<Argument>& arguments, const QString& instanceSocket) {
    if (arguments.empty()) {
        return false;
    }

    if (!QFile::exists(instanceSocket)) {
        return false;
    }

    if (arguments.size() < 2) {
        return false;
    }

    // remove the binary name

    arguments.removeFirst();

    // give a look at the first parameter

    Argument firstArg = arguments.first();

    if (firstArg.isNewWindowFlag()) {
        return false;
    }

    if (Git::isGitTempFile(firstArg)) {
        return false;
    }

    if (firstArg.isStdin()) {
        return false;
    }

    // try to connect to the existing instance

    QLocalSocket socket;
    socket.connectToServer(instanceSocket);

    if (socket.state() != QLocalSocket::ConnectedState) {
        qDebug() << "An error happened while connecting to " << instanceSocket <<
            socket.errorString();
        qDebug() << "Will create a new instance instead.";
        return false;
    }

    // if there's no argument, we want to open the notes

    if (arguments.empty()) {
        arguments.append(Argument("/tmp/meh-notes"));
    }

    // analyze the arguments

    for (int i = 0; i < arguments.size(); i++) {
        Argument arg = arguments.at(i);
        QFileInfo fi(arg);
        if (arg.isPosition()) {
          // do nothing, we just keep the +XX value
        } else if (fi.exists()) {
          arguments[i] = fi.canonicalFilePath();
        } else {
          arguments[i] = QDir::currentPath() + "/" + arg;
        }
    }

    // build the message to send to the running instance

    QString data = "open ";

    for (int i = 0; i < arguments.size(); i++) {
        data += arguments.at(i);
        if (i != arguments.size() - 1) {
            data += "###";
        }
    }

    // send data on the socket

    socket.write(data.toLatin1());
    socket.flush();
    socket.close();
    return true;
}

// readFromStdin reads data from stdin and open a buffer+editor with the data.
void readFromStdin(Window& window) {
    QByteArray content;

    QFile in;
    if (!in.open(stdin, QIODevice::ReadOnly)) {
        qWarning() << "can't read stdin";
        qWarning() << in.errorString();
    }
    content += in.readAll();
    in.close();

    window.newEditor("stdin", content);
}

// buildArguments reads cli arguments and return them as a list of Argument
QList<Argument> buildArguments() {
    QList<Argument> rv;
    for (QString argument : QCoreApplication::arguments()) {
        rv.append(Argument(argument));
    }
    return rv;
}

int main(int argv, char **args)
{
    // ensure to see the debug logs from the app
    // switch qt.*.debug to true to see Qt debug logs
    QLoggingCategory::setFilterRules("*.debug=true\n"
"qt.*.debug=false");

    QApplication app(argv, args);
    app.setCursorFlashTime(0);
    app.setWheelScrollLines(5);
    app.setStyle(QStyleFactory::create("Fusion"));
    app.setWindowIcon(QIcon(":res/icon.png"));
    QCoreApplication::setOrganizationName("mehteor");
    QCoreApplication::setOrganizationDomain("remy.io");
    QCoreApplication::setApplicationName("meh");
    QList<Argument> arguments = buildArguments();

    // -n flag: to use a different instance than /tmp/meh.sock
    // --------------

    QString instanceSocket = "";
    if (!arguments.empty() && arguments.size() > 1 &&
        (arguments.at(1)).isNewWindowFlag()) {
        instanceSocket = arguments.at(1).right(-2);
        arguments.removeFirst();
    }

    instanceSocket = QString("/tmp/meh") + instanceSocket + ".sock";

	// remove the binary name

    arguments.takeFirst();

    // if there is an existing instance, send it the command to open a file
    // instead of creating a new window

    if (reuseInstance(arguments, instanceSocket)) {
        return 0;
    }

    // start creating a new window

    Window window(&app, instanceSocket);
    window.setWindowTitle(QObject::tr("meh - no file"));
    window.resize(800, 700);
    window.setStyleSheet("background-color: #262626; color: #efefef;");
    window.show();

    if (arguments.size() > 0 && arguments.at(0).isStdin()) {

        // special case of reading from stdin
        // ------------

        readFromStdin(window);
    } else if (arguments.size() > 0) {

        // these are files, let's try to open them if they're not a
        // position argument (starting with a +).
        // ------------

        for (int i = arguments.size() - 1; i >= 0; i--) {
			// ignore if this is a position
            if (arguments.at(i).isPosition()) {
                continue;
            }

            // ignore if this is a directory
            QFileInfo fi(arguments.at(i));
            if (fi.isDir()) {
                continue;
            }

            window.newEditor(arguments.at(i), arguments.at(i));
        }

        // special cases about the last argument

        if (arguments.last().isPosition()) {
            bool ok = false;
            int lineNumber = arguments.last().toInt(&ok);
            if (ok) {
                window.getEditor()->goToLine(lineNumber);
            }
        } else {
            // the last one is not a +###
            // checks whether it is a directory, if it is, we want to
            // set it as the base work dir and open the "open file" window.
            QFileInfo fi(arguments.last());
            if (fi.isDir()) {
                window.setBaseDir(fi.canonicalFilePath());
                window.openListFiles();
            }
        }
    } else {

        // no arguments, open the notes
        // ------------

        window.newEditor("notes", QString("/tmp/meh-notes"));
    }

    return app.exec();
}
