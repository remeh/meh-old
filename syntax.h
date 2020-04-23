#pragma once

#include <QRegularExpression>
#include <QString>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QVector>

class Syntax : public QSyntaxHighlighter
{
	Q_OBJECT

public:
	Syntax(QTextDocument *parent = 0);

	void setSelectedWord(const QString& word);
	void noSelectedWord() { delete this->selectedWord; s = ""; this->selectedWord = nullptr; }
	QString s;

protected:
	void highlightBlock(const QString &text) override;

private:
	struct HighlightingRule
	{
		QRegularExpression pattern;
		QTextCharFormat format;
	};
	QVector<HighlightingRule> highlightingRules;

	HighlightingRule* selectedWord;

	QRegularExpression commentStartExpression;
	QRegularExpression commentEndExpression;

	QTextCharFormat keywordFormat;
	QTextCharFormat classFormat;
	QTextCharFormat singleLineCommentFormat;
	QTextCharFormat multiLineCommentFormat;
	QTextCharFormat quotationFormat;
	QTextCharFormat functionFormat;
	QTextCharFormat todoFixmeNoteFormat;
};
