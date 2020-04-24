#include "syntax.h"

Syntax::Syntax(QTextDocument *parent)
	: QSyntaxHighlighter(parent)
{
	HighlightingRule rule;

	keywordFormat.setFontWeight(QFont::Bold);
	const QString keywordPatterns[] = {
		QStringLiteral("\\bchar\\b"), QStringLiteral("\\bclass\\b"), QStringLiteral("\\bconst\\b"),
		QStringLiteral("\\bdouble\\b"), QStringLiteral("\\benum\\b"), QStringLiteral("\\bexplicit\\b"),
		QStringLiteral("\\bfriend\\b"), QStringLiteral("\\binline\\b"), QStringLiteral("\\bint\\b"),
		QStringLiteral("\\blong\\b"), QStringLiteral("\\bnamespace\\b"), QStringLiteral("\\boperator\\b"),
		QStringLiteral("\\bprivate\\b"), QStringLiteral("\\bprotected\\b"), QStringLiteral("\\bpublic\\b"),
		QStringLiteral("\\bshort\\b"), QStringLiteral("\\bsignals\\b"), QStringLiteral("\\bsigned\\b"),
		QStringLiteral("\\bslots\\b"), QStringLiteral("\\bstatic\\b"), QStringLiteral("\\bstruct\\b"),
		QStringLiteral("\\btemplate\\b"), QStringLiteral("\\btypedef\\b"), QStringLiteral("\\btypename\\b"),
		QStringLiteral("\\bunion\\b"), QStringLiteral("\\bunsigned\\b"), QStringLiteral("\\bvirtual\\b"),
		QStringLiteral("\\bvoid\\b"), QStringLiteral("\\bvolatile\\b"), QStringLiteral("\\bbool\\b"),
		QStringLiteral("\\bfor\\b"), QStringLiteral("\\bswitch\\b"), QStringLiteral("\\bcase\\b"),
		QStringLiteral("\\btrue\\b"), QStringLiteral("\\bfalse\\b"), QStringLiteral("\\btype\\b"),
		QStringLiteral("\\bwhile\\b"), QStringLiteral("\\bdelete\\b"), QStringLiteral("\\bnew\\b")
	};
	for (const QString &pattern : keywordPatterns) {
		rule.pattern = QRegularExpression(pattern);
		rule.format = keywordFormat;
		highlightingRules.append(rule);
	}

	classFormat.setFontWeight(QFont::Bold);
	classFormat.setForeground(Qt::gray);
	rule.pattern = QRegularExpression(QStringLiteral("\\bQ[A-Za-z]+\\b"));
	rule.format = classFormat;
	highlightingRules.append(rule);

	quotationFormat.setForeground(QColor::fromRgb(98,98,98));
	rule.pattern = QRegularExpression(QStringLiteral("\".*\""));
	rule.format = quotationFormat;
	highlightingRules.append(rule);

	// TODO(remy): add the same for instanciation in Go / Zig
	functionFormat.setFontItalic(true);
	functionFormat.setForeground(Qt::gray);
	rule.pattern = QRegularExpression(QStringLiteral("\\b[A-Za-z0-9_]+(?=\\()"));
	rule.format = functionFormat;
	highlightingRules.append(rule);

	singleLineCommentFormat.setForeground(QColor::fromRgb(98,98,98));
	rule.pattern = QRegularExpression(QStringLiteral("//[^\n]*"));
	rule.format = singleLineCommentFormat;
	highlightingRules.append(rule);

	todoFixmeNoteFormat.setForeground(Qt::red);
	rule.pattern = QRegularExpression(QStringLiteral("(TODO|NOTE|FIXME)"));
	rule.format = todoFixmeNoteFormat;
	highlightingRules.append(rule);
}

void Syntax::highlightBlock(const QString &text)
{
	for (const HighlightingRule &rule : qAsConst(highlightingRules)) {
		QRegularExpressionMatchIterator matchIterator = rule.pattern.globalMatch(text);
		while (matchIterator.hasNext()) {
			QRegularExpressionMatch match = matchIterator.next();
			setFormat(match.capturedStart(), match.capturedLength(), rule.format);
		}
	}
}
