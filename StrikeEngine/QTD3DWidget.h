#pragma once
#include <QtWidgets/QMainWindow>
#include "..//Enging/Game/EngingApp.h"



class QTD3DWidget : public QWidget
{
    Q_OBJECT

public:
    QTD3DWidget(QWidget* parent = Q_NULLPTR);

private:

	virtual void resizeEvent(QResizeEvent* event); //当窗口尺寸改变时响应
	virtual void paintEvent(QPaintEvent* event);   //窗口绘制函数，用于render三维场景
	//键盘和鼠标监听
	virtual void keyPressEvent(QKeyEvent* event);
	virtual void keyReleaseEvent(QKeyEvent* event);
	virtual void mousePressEvent(QMouseEvent* event);
	virtual void mouseReleaseEvent(QMouseEvent* event);
	virtual void mouseMoveEvent(QMouseEvent* event);
	virtual void wheelEvent(QWheelEvent* event);
	virtual QSize sizeHint() const;
private:
    EngingApp* D3DApp;
};