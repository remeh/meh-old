#include <QMouseEvent>
#include <QPaintEvent>
#include <QSize>
#include <QWidget>

class Editor;

class LineNumberArea : public QWidget
{
	Q_OBJECT
public:
    LineNumberArea(Editor* editor);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent*) override;

private:
    Editor* editor;
};
