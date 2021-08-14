#pragma once
#include <QtWidgets/QMainWindow>
#include "..//Enging/Game/EngingApp.h"



class QTD3DWidget : public QWidget
{
    Q_OBJECT

public:
    QTD3DWidget(QWidget* parent = Q_NULLPTR);

private:

	virtual void resizeEvent(QResizeEvent* event); //�����ڳߴ�ı�ʱ��Ӧ
	virtual void paintEvent(QPaintEvent* event);   //���ڻ��ƺ���������render��ά����
	//���̺�������
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