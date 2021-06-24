#include <QTextBlock>

#include "editor.h"
#include "tasks.h"
#include "syntax.h"

Syntax::Syntax(Editor* editor, QTextDocument *parent) :
  QSyntaxHighlighter(parent), editor(editor)
{
    HighlightingRule rule;

    selectionFormat.setUnderlineColor(QColor::fromRgb(153,215,0));
    selectionFormat.setFontUnderline(true);
    selectionFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);

    // specific syntax rules

    if (editor->getBuffer()->getFilename().endsWith(".tasks")) {
        for (HighlightingRule rule : TasksPlugin::getSyntaxRules()) {
            highlightingRules.append(rule);
        }
        for (HighlightingRule rule : getSharedRules()) {
            highlightingRules.append(rule);
        }
        return;
    }

    // code syntax rules

    keywordFormat.setFontWeight(QFont::Bold);
    keywordFormat.setForeground(QColor::fromRgb(46,126,184)); // blue
    const QString keywordPatterns[] = {
        QStringLiteral("\\bchar\\b"), QStringLiteral("\\bclass\\b"), QStringLiteral("\\bconst\\b"),
        QStringLiteral("\\bdouble\\b"), QStringLiteral("\\benum\\b"), QStringLiteral("\\bexplicit\\b"),
        QStringLiteral("\\bfriend\\b"), QStringLiteral("\\binline\\b"), QStringLiteral("\\bu?int(\\d){0,3}\\b"),
        QStringLiteral("\\bu?long(\\d){0,3}\\b"), QStringLiteral("\\bnamespace\\b"), QStringLiteral("\\boperator\\b"),
        QStringLiteral("\\bprivate\\b"), QStringLiteral("\\bprotected\\b"), QStringLiteral("\\bpublic\\b"),
        QStringLiteral("\\bslots\\b"), QStringLiteral("\\bstatic\\b"), QStringLiteral("struct\\b"),
        QStringLiteral("\\bif\\b"), QStringLiteral("\\belse\\b"), QStringLiteral("const\\b"),
        QStringLiteral("\\bvar\\b"), QStringLiteral("\\breturn\\b"), QStringLiteral("\\bnil\\b"),
        QStringLiteral("\\bvoid\\b"), QStringLiteral("\\bstring\\b"), QStringLiteral("\\bbool\\b"),
        QStringLiteral("func\\b"), QStringLiteral("\\bselect\\b"), QStringLiteral("\\brange\\b"),
        QStringLiteral("\\bfor\\b"), QStringLiteral("\\bswitch\\b"), QStringLiteral("\\bcase\\b"),
        QStringLiteral("\\btrue\\b"), QStringLiteral("\\bfalse\\b"), QStringLiteral("\\btype\\b"),
        QStringLiteral("\\bwhile\\b"), QStringLiteral("\\bdelete\\b"), QStringLiteral("\\bnew\\b"),
        QStringLiteral("package\\b"), QStringLiteral("import\\b")
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


    // TODO(remy): add the same for instanciation in Go / Zig
    functionFormat.setForeground(QColor::fromRgb(160, 160, 170));
    rule.pattern = QRegularExpression(QStringLiteral("\\b[A-Za-z0-9_]+(?=\\()"));
    rule.format = functionFormat;
    highlightingRules.append(rule);

    trailingWhiteSpaces.setBackground(Qt::red);
    rule.pattern = QRegularExpression(QStringLiteral("( |\t)+$"));
    rule.format = trailingWhiteSpaces;
    highlightingRules.append(rule);

    // diff markups
    // ------------

    QTextCharFormat diffIn, diffSeparator, diffOut;
    diffIn.setForeground(QColor::fromRgb(153,215,0));
    diffSeparator.setForeground(QColor::fromRgb(255,255,255));
    diffOut.setForeground(QColor::fromRgb(250,50,50));
    rule.pattern = QRegularExpression(QStringLiteral("^<<<<<<< .*"));
    rule.format = diffIn;
    highlightingRules.append(rule);
    rule.pattern = QRegularExpression(QStringLiteral("^=======$"));
    rule.format = diffSeparator;
    highlightingRules.append(rule);
    rule.pattern = QRegularExpression(QStringLiteral("^>>>>>>> .*"));
    rule.format = diffOut;
    highlightingRules.append(rule);

    rule.pattern = QRegularExpression(QStringLiteral("^\\+.*"));
    rule.format = diffIn;
    highlightingRules.append(rule);
    rule.pattern = QRegularExpression(QStringLiteral("^\\-.*"));
    rule.format = diffOut;
    highlightingRules.append(rule);

    for (HighlightingRule rule : getSharedRules()) {
        highlightingRules.append(rule);
    }
}

QList<HighlightingRule> Syntax::getSharedRules() {
    QList<HighlightingRule> rv;

    HighlightingRule rule;
    QTextCharFormat format;

    format.setForeground(Qt::gray);
    format.setFontItalic(true);
    rule.pattern = QRegularExpression(QStringLiteral("\"(.*?)\""));
    rule.format = format;
    rv.append(rule);
    rule.pattern = QRegularExpression(QStringLiteral("'(.*?)'"));
    rv.append(rule);
    rule.pattern = QRegularExpression(QStringLiteral("`(.*?)`"));
    rv.append(rule);

    format = QTextCharFormat();
    format.setFontWeight(QFont::Bold);
    format.setForeground(Qt::gray);
    rule.pattern = QRegularExpression(QStringLiteral("^\\s*#+[^\n]*"));
    rule.format = format;
    rv.append(rule);

    format = QTextCharFormat();
    format.setForeground(QColor::fromRgb(250, 50, 50));
    rule.pattern = QRegularExpression(QStringLiteral("(TODO|NOTE|FIXME|XXX)"));
    rule.format = format;
    rv.append(rule);

    format = QTextCharFormat();
    format.setForeground(QColor::fromRgb(98,98,98));
    rule.pattern = QRegularExpression(QStringLiteral("(^|\\s)//[^\n]*"));
    rule.format = format;
    rv.append(rule);

    return rv;
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

    if (this->selection.size() > 0 && this->selectionRx.isValid()) {
        QRegularExpressionMatchIterator matchIterator = selectionRx.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), selectionFormat);
        }
    }
}

bool Syntax::setSelection(const QString& text) {
    if (this->selection == text) {
        return false;
    }

    if (this->selection != text && text.size() > 0) {
        this->selectionRx = QRegularExpression("(" + text + ")");
    }
    this->selection = text;

    return true;
}

// void Syntax::rehighlightAround(QTextBlock currentBlock) {
//     QVector<QTextBlock> blocks;
//
//     blocks.append(currentBlock);
//     int blocksAround = 10;
//
//     QTextBlock block = currentBlock;
//     for (int i = 0; i < blocksAround; i++) {
//         block = block.previous();
//         blocks.prepend(block);
//         if (block == this->document()->firstBlock()) {
//             break;
//         }
//     }
//
//     block = currentBlock;
//     for (int i = 0; i < blocksAround; i++) {
//         block = block.next();
//         blocks.append(block);
//         if (block == this->document()->lastBlock()) {
//             break;
//         }
//     }
//
//     for (int i = 0; i < blocks.size(); i++) {
//         this->highlightBlock(blocks.at(i).text());
//         this->rehighlightBlock(blocks.at(i));
//     }
// }
