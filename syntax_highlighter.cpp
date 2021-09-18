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
    QList<QString> languages{
        ".go", ".java", ".py", ".rs", ".rb", ".zig", ".c", ".cpp", ".h", ".hpp",
        ".scala"
    };

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
    setCodeRules();
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

void SyntaxHighlighter::processWord(const QString& word, int wordStart, bool startOfLine, bool endOfLine, bool inQuote) {
    if (word == " ") {
        // TODO(remy): we want to display it red if endOfLine == true
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
    if (this->selection.size() > 0 && this->selectionRx.isValid()) {
        QRegularExpressionMatchIterator matchIterator = selectionRx.globalMatch(line);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), selectionFormat);
        }
    }

    if (this->searchText.size() > 0 && this->searchTextRx.isValid()) {
        QRegularExpressionMatchIterator matchIterator = searchTextRx.globalMatch(line);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), match.capturedLength(), searchTextFormat);
        }
    }
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

    // TODO(remy): FIXME/TODO/NOTE/XXX support

    setFormat(start, comment.size(), commentFormat);
}

void SyntaxHighlighter::processFunctionCall(const QString& text, int start) {
    if (text == "") {
        return;
    }

    setFormat(start, text.size(), functionCallFormat);
}

void SyntaxHighlighter::highlightBlock(const QString &text) {
    // FIXME(remy): highlightStateReset();
    QString wordBuffer = "";
    QString quoteBuffer = "";
    QChar isInQuote = '0'; // FIXME(remy): we want to ignore quote chars if previous char is '\'
    QChar pc = '0';
    bool startOfLine = true;
    bool endOfLine = false;
    int quoteStart = 0;
    int wordStart = 0;

    for (int i = 0; i < text.size(); i++) {
        QChar c = text[i];

        if ((c == '/' && pc == '/') || (c == ' ' && pc == '#')) {
            // entering comment
            QString comment = text.right(text.size() - wordStart);
            processComment(comment, wordStart);
            break;
        }

        if (isInQuote != '0') {
            if (c == isInQuote) {
                processQuote(quoteBuffer, quoteStart);
                isInQuote = '0';
                wordStart = i+1;
                quoteBuffer.clear();
                wordBuffer.clear();
                continue;
            }

            quoteBuffer += c;
        }

        // we are entering a quote, store which char has been used to open it
        if (isInQuote == '0' && (c == '\"' || c == '\'' || c == '`')) {
            quoteStart = i+1;
            isInQuote = c;
            quoteBuffer.clear();
            continue;
        }

        // end of word
        if (c.isSpace() || c.isPunct()) {
            if (wordBuffer.size() > 0) {
                if (c == '(') {
                    processFunctionCall(wordBuffer, wordStart);
                } else {
                    processWord(wordBuffer, wordStart, startOfLine, false, false);
                } 
                wordStart = i+1;
                wordBuffer.clear();
                startOfLine = false;
                continue;
            }
        }
        wordBuffer.append(c);
        pc = c;
    }

    // end of line
    if (wordBuffer.size() > 0) {
        processWord(wordBuffer, wordStart, false, true, isInQuote != '0');
        wordBuffer.clear();
    }

    processLine(text);

    // FIXME(remy): highlightStateReset();
    quoteBuffer = '0';
    wordBuffer.clear();
    quoteBuffer.clear();
    startOfLine = true;
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
