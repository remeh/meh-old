#pragma once

#include <QRegularExpression>
#include <QString>
#include <QSyntaxHighlighter>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QVector>

class Editor;

struct HighlightingRule
{
    QRegularExpression pattern;
    QTextCharFormat format;
};

class Syntax : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    Syntax(Editor *editor, QTextDocument *parent = 0);
    bool setSelection(const QString& text);
    bool setSearchText(const QString& text);

    static QList<HighlightingRule> getSharedRules();
    static QList<HighlightingRule> getCodeRules();
    static QList<HighlightingRule> getOverrideRules();

    static QColor getMainColor();

protected:
    void highlightBlock(const QString &text) override;

private:
    Editor* editor;

    QVector<HighlightingRule> highlightingRules;

    QString selection;
    QRegularExpression selectionRx;
    QTextCharFormat selectionFormat;

    QString searchText;
    QRegularExpression searchTextRx;
    QTextCharFormat searchTextFormat;
};