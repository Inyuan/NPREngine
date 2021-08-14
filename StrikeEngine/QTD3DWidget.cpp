#include <QKeyEvent>
#include "QTD3DWidget.h"

QTD3DWidget::QTD3DWidget(QWidget* parent)
	: QWidget(parent)
{
	setAttribute(Qt::WA_PaintOnScreen, true);
	setAttribute(Qt::WA_NativeWindow, true);
	setFocusPolicy(Qt::StrongFocus);
	setObjectName(QString::fromUtf8("D3Dwidget"));
	setGeometry(QRect(270, 25, 1280, 720));
    D3DApp = new EngingApp((HWND)winId());
    D3DApp->Initialize(width(), height());
}

void QTD3DWidget::paintEvent(QPaintEvent* event)
{
	D3DApp->Run();
	update();
}

void QTD3DWidget::resizeEvent(QResizeEvent* event)
{
	D3DApp->OnResize(width(),height());
}

QSize QTD3DWidget::sizeHint() const {
	return QSize(1024, 720);
}

//键盘事件可用于场景漫游导航
void QTD3DWidget::keyPressEvent(QKeyEvent* event)
{
	if(event->key() == Qt::Key_W)
		D3DApp->OnKeyboardInput('w');
	if (event->key() == Qt::Key_S)
		D3DApp->OnKeyboardInput('s');
	if (event->key() == Qt::Key_A)
		D3DApp->OnKeyboardInput('a');
	if (event->key() == Qt::Key_D)
		D3DApp->OnKeyboardInput('d');
}

void QTD3DWidget::keyReleaseEvent(QKeyEvent* event)
{

}

//鼠标和滚轮事件可用于场景拖拽和视距缩放
void QTD3DWidget::mousePressEvent(QMouseEvent* event)
{
	int x = event->x();
	int y = event->y();
	D3DApp->OnMouseDown(x, y);
}

void QTD3DWidget::mouseReleaseEvent(QMouseEvent* event)
{
	int x = event->x();
	int y = event->y();
	D3DApp->OnMouseUp(x, y);
}

void QTD3DWidget::mouseMoveEvent(QMouseEvent* event)
{
	int x = event->x();
	int y = event->y();
	D3DApp->OnMouseMove(x, y);
}

void QTD3DWidget::wheelEvent(QWheelEvent* event)
{

}
