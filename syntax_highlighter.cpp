#include <QTextBlock>

#include "editor.h"
#include "tasks.h"
#include "syntax_highlighter.h"

QColor SyntaxHighlighter::getMainColor() {
    return QColor::fromRgb(46,126,184); // blue
}

SyntaxHighlighter::SyntaxHighlighter(Editor* editor, QTextDocument *parent) :
  QSyntaxHighlighter(parent), editor(editor)
{
    if (editor == nullptr) {
        return;
    }

    this->filename = editor->getBuffer()->getFilename();

    QList<QString> languages{
        ".go", ".java", ".py", ".rs", ".rb", ".zig", ".c", ".cpp", ".h", ".hpp",
        ".scala"
    };

    todoRx = QRegularExpression(QStringLiteral("(TODO|NOTE|FIXME|XXX)"));
    whitespaceEolRx = QRegularExpression(QStringLiteral("( |\t)+$"));

    selectionFormat.setForeground(QColor::fromRgb(129,179,234));
    selectionFormat.setUnderlineColor(QColor::fromRgb(129,179,234));
    selectionFormat.setFontUnderline(true);
    selectionFormat.setUnderlineStyle(QTextCharFormat::SingleUnderline);

    searchTextFormat.setForeground(Qt::white);
    searchTextFormat.setBackground(QColor::fromRgb(129,179,234));

    commentFormat.setForeground(Qt::gray);
    functionCallFormat.setForeground(Qt::lightGray);
    quoteFormat.setForeground(Qt::gray);
    quoteFormat.setFontItalic(true);
    todoFormat.setForeground(QColor::fromRgb(250, 50, 50));
    whitespaceEolFormat.setBackground(QColor::fromRgb(250, 50, 50));

    for (QString language : languages) {
        if (filename.endsWith(language)) {
            setCodeRules();
        }
    }

    if (filename.endsWith(".tasks")) {
        for (PluginRule rule : TasksPlugin::getSyntaxRules()) {
            pluginRules.append(rule);
        }
    }

    if (filename.endsWith(".md")) {
        for (PluginRule rule : setMarkdownRules()) {
            pluginRules.append(rule);
        }
    }
}

void SyntaxHighlighter::setCodeRules() {
    SyntaxRule rule;
    QTextCharFormat format;

    // code syntax rules

    format.setForeground(SyntaxHighlighter::getMainColor());
    const QString keywordPatterns[] = {
        "char", "class", "const",
        "double", "enum", "explicit",
        "friend", "inline", "int",
        "long", "namespace", "operator",
        "private", "protected", "public",
        "slots", "static", "struct",
        "if", "else", "const",
        "var", "return", "nil",
        "void", "string", "bool",
        "func", "select", "range",
        "for", "switch", "case", "break",
        "true", "false", "type",
        "while", "delete", "new",
        "def", "end", "until", // ruby
        "package", "import", "#include"
    };

    for (const QString &pattern : keywordPatterns) {
        rule.word = pattern;
        rule.format = format;
        simpleWordEqualityRules.append(rule);
    }
}

void SyntaxHighlighter::processRegexp(const QString& text, QRegularExpression rx, QTextCharFormat format) {
    if (text.size() > 0 && rx.isValid()) {
        QRegularExpressionMatchIterator matchIterator = rx.globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), format);
        }
    }
}

void SyntaxHighlighter::processWord(const QString& word, int wordStart, bool endOfLine, bool inQuote) {
    if (word == " ") {
        return;
    }

    if (word.size() < 2) {
        return;
    }

    for (int i = 0; i < simpleWordEqualityRules.size(); i++) {
        if (word == simpleWordEqualityRules[i].word) {
            setFormat(wordStart, word.size(), simpleWordEqualityRules[i].format);
        }
    }
}

void SyntaxHighlighter::processLine(const QString& line) {
    if (pluginRules.size() > 0) {
        for (PluginRule rule : pluginRules) {
            processRegexp(line, rule.pattern, rule.format);
        }
    }

    if (this->selection.size() > 0) {
        processRegexp(line, this->selectionRx, this->selectionFormat);
    }
    if (this->searchText.size() > 0) {
        processRegexp(line, this->searchTextRx, this->searchTextFormat);
    }

    processRegexp(line, this->whitespaceEolRx, this->whitespaceEolFormat);
}

void SyntaxHighlighter::processQuote(const QString& text, int start) {
    if (text == "") {
        return;
    }

    setFormat(start-1, text.size()+2, quoteFormat);
}

void SyntaxHighlighter::processComment(const QString& comment, int start) {
    if (comment == "") {
        return;
    }

    setFormat(start, comment.size(), commentFormat);
    processRegexp(comment, todoRx, todoFormat);
}

void SyntaxHighlighter::processFunctionCall(const QString& text, int start, bool endOfLine) {
    if (text == "") {
        return;
    }

    int size = text.size();
    if (endOfLine) {
        size += 1;
    }
    setFormat(start, size, functionCallFormat);
}

void SyntaxHighlighter::highlightBlock(const QString &text) {
    QString wordBuffer = "";
    QString quoteBuffer = "";
    QChar charBeforeWord = '0';
    QChar isInQuote = '0';
    QChar pc = '0';
    bool endOfLine = false;
    int quoteStart = 0;
    int wordStart = 0;

    for (int i = 0; i < text.size(); i++) {
        if (i > 0) { pc = text[i-1]; }
        QChar c = text[i];

        // entering comment
        if ((c == '/' && pc == '/') || (c == ' ' && pc == '#')) {
            QString comment = text.right(text.size());
            processComment(comment, i-1);
            wordBuffer.clear();
            break;
        }

        // leaving a quoted text
        if (isInQuote != '0') {
            if (c == isInQuote && pc != '\\') {
                processQuote(quoteBuffer, quoteStart);
                isInQuote = '0';
                wordStart = i+1;
                quoteBuffer.clear();
                wordBuffer.clear();
                continue;
            }

            quoteBuffer += c;
        }

        // we are entering a quoted text, store which char has been used to open it
        if (isInQuote == '0' && pc != '\\' && (c == '\"' || c == '\'' || c == '`')) {
            quoteStart = i+1;
            isInQuote = c;
            quoteBuffer.clear();
            continue;
        }

        // end of word
        if (c.isSpace() || (c.isPunct() && c != '_') || i == text.size()-1) {
            if (wordBuffer.size() > 0) {
                bool endOfLine = (i == text.size()-1);
                if (c == '(' || charBeforeWord == '.') {
                    processFunctionCall(wordBuffer, wordStart, endOfLine);
                } else {
                    processWord(wordBuffer, wordStart, endOfLine, false);
                }
            }
            wordBuffer.clear();
            wordStart = i+1;
            charBeforeWord = c;
            continue;
        }

        wordBuffer.append(c);
    }

    // end of line
    if (wordBuffer.size() > 0) {
        processWord(wordBuffer, wordStart, true, isInQuote != '0');
        wordBuffer.clear();
    }

    processLine(text);

    quoteBuffer = '0';
    wordBuffer.clear();
    quoteBuffer.clear();
    endOfLine = false;
    wordStart = 0;
}

bool SyntaxHighlighter::setSelection(const QString& text) {
    if (this->selection == text) {
        return false;
    }

    if (this->selection != text && text.size() > 0) {
        this->selectionRx = QRegularExpression("(" + text + ")");
    }
    this->selection = text;

    return true;
}

bool SyntaxHighlighter::setSearchText(const QString& text) {
    if (this->searchText == text) {
        return false;
    }
    if (text.size() > 0) {
        this->searchTextRx = QRegularExpression("(" + text + ")");
    }
    this->searchText = text;
    return true;
}

QVector<PluginRule> SyntaxHighlighter::setMarkdownRules() {
    QVector<PluginRule> rv;

    PluginRule rule;
    QTextCharFormat format;
    format.setFontWeight(QFont::Bold);
    format.setForeground(SyntaxHighlighter::getMainColor());
    rule.pattern = QRegularExpression(QStringLiteral("^\\s*#+[^\n]*"));
    rule.format = format;
    rv.append(rule);

    return rv;
}
