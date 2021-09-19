#pragma once

#include <QRegularExpression>
#include <QString>
#include <QSyntaxHighlighter>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QVector>

class Editor;

struct SyntaxRule
{
    QString word;
    QTextCharFormat format;
};

struct PluginRule
{
    QRegularExpression pattern;
    QTextCharFormat format;
};

class SyntaxHighlighter : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    SyntaxHighlighter(Editor *editor, QTextDocument *parent = 0);
    bool setSelection(const QString& text);
    bool setSearchText(const QString& text);

    static QColor getMainColor();

protected:
    void highlightBlock(const QString &text) override;

private:
    Editor* editor;
    QString filename;

    QVector<SyntaxRule> simpleWordEqualityRules;
    QVector<PluginRule> pluginRules;

    QString selection;
    QRegularExpression selectionRx;
    QTextCharFormat selectionFormat;

    QString searchText;
    QRegularExpression searchTextRx;
    QTextCharFormat searchTextFormat;

    QRegularExpression todoRx;
    QTextCharFormat todoFormat;

    QRegularExpression whitespaceEolRx;
    QTextCharFormat whitespaceEolFormat;

    QTextCharFormat commentFormat;
    QTextCharFormat quoteFormat;
    QTextCharFormat functionCallFormat;

    void setCodeRules();
    QVector<PluginRule> setMarkdownRules();

    void processWord(const QString& word, int wordStart, bool endOfLine, bool inQuote);
    void processLine(const QString& line);
    void processQuote(const QString& text, int start);
    void processComment(const QString& text, int start);
    void processFunctionCall(const QString& text, int start, bool endOfLine);
    void processRegexp(const QString& text, QRegularExpression rx, QTextCharFormat format);
};