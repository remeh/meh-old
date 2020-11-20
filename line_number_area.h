#include <QMouseEvent>
#include <QPaintEvent>
#include <QSize>
#include <QWidget>

class Editor;
class Window;

class LineNumberArea : public QWidget
{
	Q_OBJECT
public:
    LineNumberArea(Window *window, Editor* editor);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent*) override;

private:
    Window* window;
};
