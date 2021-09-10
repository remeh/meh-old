#include <QTextBlock>

#include "editor.h"
#include "tasks.h"
#include "syntax.h"

QColor Syntax::getMainColor() {
    return QColor::fromRgb(46,126,184); // blue
}

Syntax::Syntax(Editor* editor, QTextDocument *parent) :
  QSyntaxHighlighter(parent), editor(editor)
{
    QList<QString> languages{
        ".go", ".java", ".py", ".rs", ".rb", ".zig", ".c", ".cpp", ".h", ".hpp",
        ".scala"
    };

    selectionFormat.setForeground(QColor::fromRgb(129,179,234));
    selectionFormat.setUnderlineColor(QColor::fromRgb(129,179,234));
    selectionFormat.setFontUnderline(true);
    selectionFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);

    // rules shared with everything

    for (HighlightingRule rule : getSharedRules()) {
        highlightingRules.append(rule);
    }

    QString filename = editor->getBuffer()->getFilename();

    // if it looks like source code

    for (QString language : languages) {
        if (filename.endsWith(language)) {
            for (HighlightingRule rule : getCodeRules()) {
                highlightingRules.append(rule);
            }
        }
    }

    // rules to overriding all others (TODO and comments color for example)
    for (HighlightingRule rule : getOverrideRules()) {
        highlightingRules.append(rule);
    }

    // specific syntax rules

    if (filename.endsWith(".tasks")) {
        for (HighlightingRule rule : TasksPlugin::getSyntaxRules()) {
            highlightingRules.append(rule);
        }
    }

    if (filename.endsWith(".md")) {
        HighlightingRule rule;
        QTextCharFormat format;

        format.setFontWeight(QFont::Bold);
        format.setForeground(Syntax::getMainColor());
        rule.pattern = QRegularExpression(QStringLiteral("^\\s*#+[^\n]*"));
        rule.format = format;
        highlightingRules.append(rule);
    }
}

QList<HighlightingRule> Syntax::getCodeRules() {
    QList<HighlightingRule> rv;
    HighlightingRule rule;
    QTextCharFormat format;

    // code syntax rules

    format.setForeground(Syntax::getMainColor());
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
        QStringLiteral("\\bdef\\b"), QStringLiteral("\\bend\\b"), // ruby
        QStringLiteral("package\\b"), QStringLiteral("import\\b")
    };
    for (const QString &pattern : keywordPatterns) {
        rule.pattern = QRegularExpression(pattern);
        rule.format = format;
        rv.append(rule);
    }

    format = QTextCharFormat();
    format.setForeground(Qt::gray);
    rule.pattern = QRegularExpression(QStringLiteral("\\bQ[A-Za-z]+\\b"));
    rule.format = format;
    rv.append(rule);

    // TODO(remy): add the same for instanciation in Go / Zig
    format = QTextCharFormat();
    format.setForeground(QColor::fromRgb(160, 160, 170));
    rule.pattern = QRegularExpression(QStringLiteral("\\b[A-Za-z0-9_]+(?=\\()"));
    rule.format = format;
    rv.append(rule);

    format = QTextCharFormat();
    format.setBackground(Qt::red);
    rule.pattern = QRegularExpression(QStringLiteral("( |\t)+$"));
    rule.format = format;
    rv.append(rule);

    // diff markups
    // ------------

    QTextCharFormat diffIn, diffSeparator, diffOut;
    diffIn.setForeground(QColor::fromRgb(153,215,0));
    diffSeparator.setForeground(QColor::fromRgb(255,255,255));
    diffOut.setForeground(QColor::fromRgb(250,50,50));
    rule.pattern = QRegularExpression(QStringLiteral("^<<<<<<< .*"));
    rule.format = diffIn;
    rv.append(rule);
    rule.pattern = QRegularExpression(QStringLiteral("^=======$"));
    rule.format = diffSeparator;
    rv.append(rule);
    rule.pattern = QRegularExpression(QStringLiteral("^>>>>>>> .*"));
    rule.format = diffOut;
    rv.append(rule);

    rule.pattern = QRegularExpression(QStringLiteral("^\\+.*"));
    rule.format = diffIn;
    rv.append(rule);
    rule.pattern = QRegularExpression(QStringLiteral("^\\-.*"));
    rule.format = diffOut;
    rv.append(rule);

    return rv;
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

    return rv;
}

QList<HighlightingRule> Syntax::getOverrideRules() {
    QList<HighlightingRule> rv;
    HighlightingRule rule;
    QTextCharFormat format;

    format = QTextCharFormat();
    format.setForeground(Qt::gray);
    rule.pattern = QRegularExpression(QStringLiteral("^\\s*#+[^\n]*"));
    rule.format = format;
    rv.append(rule);

    format = QTextCharFormat();
    format.setForeground(QColor::fromRgb(98,98,98));
    rule.pattern = QRegularExpression(QStringLiteral("(^|\\s)//[^\n]*"));
    rule.format = format;
    rv.append(rule);

    format = QTextCharFormat();
    format.setForeground(QColor::fromRgb(250, 50, 50));
    rule.pattern = QRegularExpression(QStringLiteral("(TODO|NOTE|FIXME|XXX)"));
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
