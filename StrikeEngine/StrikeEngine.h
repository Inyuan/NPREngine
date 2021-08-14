#pragma once

#include <QtWidgets/QMainWindow>
#include "QTD3DWidget.h"
#include "ui_StrikeEngine.h"
#include "../Enging/Game/UpdateData.h"
#include <QStandardItemModel>
#include <vector>

class StrikeEngine : public QMainWindow
{
    Q_OBJECT

public:
    StrikeEngine(QWidget *parent = Q_NULLPTR);

private:
    virtual void paintEvent(QPaintEvent* event);

private:
    void UpdateD3DData();
    void InitInterfaceData();
    void showObj();
    void showMat();
    void slotObjCurrentChanged(const QModelIndex& current, const QModelIndex& previous);
private slots:
    void slotMatClick(QModelIndex index);
    void slotDiffTexChange(int index);
    void slotNormTexChange(int index);
private:
    int mat_id = -1;
    int obj_id = -1;
    bool combox_refresh = false;
    QStandardItemModel* objtree_model;
    std::vector<QStandardItemModel*> matlist_models;
    std::vector<QStringList> texlist_qslists;
    Ui::StrikeEngineClass ui;
    QTD3DWidget* D3Dwidget;
    UpdateData* updateData;
};
