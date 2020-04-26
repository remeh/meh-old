#pragma once

#include <QRegularExpression>
#include <QString>
#include <QSyntaxHighlighter>
#include <QTextBlock>
#include <QTextCharFormat>
#include <QTextDocument>
#include <QVector>

class Syntax : public QSyntaxHighlighter
{
	Q_OBJECT

public:
	Syntax(QTextDocument *parent = 0);
	bool setSelection(const QString& text);
//void rehighlightAround(QTextBlock currentBlock);

protected:
	void highlightBlock(const QString &text) override;

private:
	struct HighlightingRule
	{
		QRegularExpression pattern;
		QTextCharFormat format;
	};
	QVector<HighlightingRule> highlightingRules;

	QString selection;
	QRegularExpression selectionRx; // TODO(remy): naming

	QTextCharFormat keywordFormat;
	QTextCharFormat classFormat;
	QTextCharFormat singleLineCommentFormat;
	QTextCharFormat quotationFormat;
	QTextCharFormat functionFormat;
	QTextCharFormat todoFixmeNoteFormat;
	QTextCharFormat selectionFormat;
	QTextCharFormat markdownTitleFormat;
};
