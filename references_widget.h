#pragma once

#include <QGridLayout>
#include <QKeyEvent>
#include <QLabel>
#include <QObject>
#include <QMap>
#include <QString>
#include <QStringList>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QWidget>

class Window;

class ReferencesWidget : public QWidget {
    Q_OBJECT

public:
    ReferencesWidget(Window* window);
    void setItems(const QString& base, const QStringList& list);

    // fitContent must be called in order to resize the columns
    // when the content has been inserted.
    void fitContent();

    void show();
    void hide();
    void clear();

    void setLabelText(QString string);

    void insert(const QString& file, const QString& lineNumber, const QString& text);

    void onKeyPressEvent(QKeyEvent*);

protected:
    void keyPressEvent(QKeyEvent*) override;

private:
    Window* window;
    QTreeWidget* tree;
    QGridLayout* layout;
    QLabel* label;
    QMap<QString, QTreeWidgetItem*> data;
};
