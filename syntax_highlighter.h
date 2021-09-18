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

    QVector<SyntaxRule> simpleWordEqualityRules;

    QString selection;
    QRegularExpression selectionRx;
    QTextCharFormat selectionFormat;

    QString searchText;
    QRegularExpression searchTextRx;
    QTextCharFormat searchTextFormat;

    QTextCharFormat commentFormat;
    QTextCharFormat quoteFormat;
    QTextCharFormat functionCallFormat;

    void setCodeRules();

    void processWord(const QString& word, int wordStart, bool startOfLine, bool endOfLine, bool inQuote);
    void processLine(const QString& line);
    void processQuote(const QString& text, int start);
    void processComment(const QString& text, int start);
    void processFunctionCall(const QString& text, int start);
};