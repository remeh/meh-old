#include <QMap>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QSize>
#include <QWidget>

class Editor;

#define GIT_FLAG_UNKNOWN	0
#define GIT_FLAG_ADDED	1
#define GIT_FLAG_REMOVED	2
#define GIT_FLAG_BOTH		3

class LineNumberArea : public QWidget
{
	Q_OBJECT
public:
    LineNumberArea(Editor* editor);
    QSize sizeHint() const override;

    void clearGitFlags();
    void setGitFlag(int line, int flag);
    QMap<int, int> gitFlags;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent*) override;

private:
    Editor* editor;
};
