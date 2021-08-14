#include "StrikeEngine.h"

StrikeEngine::StrikeEngine(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    D3Dwidget = new QTD3DWidget(this);
    updateData = UpdateData::GetInstance();

    InitInterfaceData();

    connect(ui.ObjTree->selectionModel(),
        &QItemSelectionModel::currentChanged, 
        this, 
        &StrikeEngine::slotObjCurrentChanged);

    connect(ui.MatList,
        SIGNAL(clicked(QModelIndex)),
        this,
        SLOT(slotMatClick(QModelIndex)));
 
    connect(ui.DiffTex_comboBox,
        SIGNAL(currentIndexChanged(int)),
        this,
        SLOT(slotDiffTexChange(int)));

    connect(ui.NormTex_comboBox,
        SIGNAL(currentIndexChanged(int)),
        this,
        SLOT(slotNormTexChange(int)));

    {
        /*Obj*/
        connect(ui.Transform_X, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d)
            {
                if (obj_id > -1)
                {
                    updateData->mObj[obj_id]->Transform[0] = d;
                    updateData->mObj[obj_id]->MashneedRefresh = updateData->mObj[obj_id]->mMats.size();
                }
            });
        connect(ui.Transform_Y, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d)
            {
                if (obj_id > -1)
                {
                    updateData->mObj[obj_id]->Transform[1] = d;
                    updateData->mObj[obj_id]->MashneedRefresh = updateData->mObj[obj_id]->mMats.size();
                }
            });
        connect(ui.Transform_Z, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d)
            {
                if (obj_id > -1)
                {
                    updateData->mObj[obj_id]->Transform[2] = d;
                    updateData->mObj[obj_id]->MashneedRefresh = updateData->mObj[obj_id]->mMats.size();
                }
            });

        connect(ui.Rotation_X, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d)
            {
                if (obj_id > -1)
                {
                    updateData->mObj[obj_id]->Rotation[0] = d;
                    updateData->mObj[obj_id]->MashneedRefresh = updateData->mObj[obj_id]->mMats.size();
                }
            });
        connect(ui.Rotation_Y, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d)
            {
                if (obj_id > -1)
                {
                    updateData->mObj[obj_id]->Rotation[1] = d;
                    updateData->mObj[obj_id]->MashneedRefresh = updateData->mObj[obj_id]->mMats.size();
                }
            });
        connect(ui.Rotation_Z, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d)
            {
                if (obj_id > -1)
                {
                    updateData->mObj[obj_id]->Rotation[2] = d;
                    updateData->mObj[obj_id]->MashneedRefresh = updateData->mObj[obj_id]->mMats.size();
                }
            });
        
        connect(ui.Scale_X, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d)
            {
                if (obj_id > -1)
                {
                    updateData->mObj[obj_id]->Scale[0] = d;
                    updateData->mObj[obj_id]->MashneedRefresh = updateData->mObj[obj_id]->mMats.size();
                }
            });
        connect(ui.Scale_Y, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d)
            {
                if (obj_id > -1)
                {
                    updateData->mObj[obj_id]->Scale[1] = d;
                    updateData->mObj[obj_id]->MashneedRefresh = updateData->mObj[obj_id]->mMats.size();
                }
            });
        connect(ui.Scale_Z, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d)
            {
                if (obj_id > -1)
                {
                    updateData->mObj[obj_id]->Scale[2] = d;
                    updateData->mObj[obj_id]->MashneedRefresh = updateData->mObj[obj_id]->mMats.size();
                }
            });
        
        /*Mat*/
        connect(ui.Lightcolor_R, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d)
            {
                if (obj_id > -1 && mat_id > -1)
                {
                    updateData->mObj[obj_id]->mMats[mat_id].Lightcolor[0] = d;
                    updateData->mObj[obj_id]->mMats[mat_id].needRefresh = true;
                }
            });
        connect(ui.Lightcolor_G, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d)
            {
                if (obj_id > -1 && mat_id > -1)
                {
                    updateData->mObj[obj_id]->mMats[mat_id].Lightcolor[1] = d;
                    updateData->mObj[obj_id]->mMats[mat_id].needRefresh = true;
                }
            });
        connect(ui.Lightcolor_B, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d)
            {
                if (obj_id > -1 && mat_id > -1)
                {

                    updateData->mObj[obj_id]->mMats[mat_id].Lightcolor[2] = d;
                    updateData->mObj[obj_id]->mMats[mat_id].needRefresh = true;
                }
            });

        connect(ui.Specularcolor_R, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d)
            {
                if (obj_id > -1 && mat_id > -1)
                {
                    updateData->mObj[obj_id]->mMats[mat_id].Specularcolor[0] = d;
                    updateData->mObj[obj_id]->mMats[mat_id].needRefresh = true;
                }
            });
        connect(ui.Specularcolor_G, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d)
            {
                if (obj_id > -1 && mat_id > -1)
                {
                    updateData->mObj[obj_id]->mMats[mat_id].Specularcolor[1] = d;
                    updateData->mObj[obj_id]->mMats[mat_id].needRefresh = true;
                }
            });
        connect(ui.Specularcolor_B, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d)
            {
                if (obj_id > -1 && mat_id > -1)
                {
                    updateData->mObj[obj_id]->mMats[mat_id].Specularcolor[2] = d;
                    updateData->mObj[obj_id]->mMats[mat_id].needRefresh = true;
                }
            });

        connect(ui.Shadowcolor_R, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d)
            {
                if (obj_id > -1 && mat_id > -1)
                {
                    updateData->mObj[obj_id]->mMats[mat_id].Shadowcolor[0] = d;
                    updateData->mObj[obj_id]->mMats[mat_id].needRefresh = true;
                }
            });
        connect(ui.Shadowcolor_G, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d)
            {
                if (obj_id > -1 && mat_id > -1)
                {
                    updateData->mObj[obj_id]->mMats[mat_id].Shadowcolor[1] = d;
                    updateData->mObj[obj_id]->mMats[mat_id].needRefresh = true;
                }
            });
        connect(ui.Shadowcolor_B, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d)
            {
                if (obj_id > -1 && mat_id > -1)
                {
                    updateData->mObj[obj_id]->mMats[mat_id].Shadowcolor[2] = d;
                    updateData->mObj[obj_id]->mMats[mat_id].needRefresh = true;
                }
            });

        connect(ui.Profilecolor_R, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d)
            {
                if (obj_id > -1 && mat_id > -1)
                {
                    updateData->mObj[obj_id]->mMats[mat_id].Profilecolor[0] = d;
                    updateData->mObj[obj_id]->mMats[mat_id].needRefresh = true;
                }
            });
        connect(ui.Profilecolor_G, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d)
            {
                if (obj_id > -1 && mat_id > -1)
                {
                    updateData->mObj[obj_id]->mMats[mat_id].Profilecolor[1] = d;
                    updateData->mObj[obj_id]->mMats[mat_id].needRefresh = true;
                }
            });
        connect(ui.Profilecolor_B, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d)
            {
                if (obj_id > -1 && mat_id > -1)
                {
                    updateData->mObj[obj_id]->mMats[mat_id].Profilecolor[2] = d;
                    updateData->mObj[obj_id]->mMats[mat_id].needRefresh = true;
                }
            });

        connect(ui.Thick, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d)
            {
                if (obj_id > -1 && mat_id > -1)
                {
                    updateData->mObj[obj_id]->mMats[mat_id].Thick = d;
                    updateData->mObj[obj_id]->mMats[mat_id].needRefresh = true;
                }
            });
        connect(ui.Shadowsoft, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d)
            {
                if (obj_id > -1 && mat_id > -1)
                {
                    updateData->mObj[obj_id]->mMats[mat_id].Shadowsoft = d;
                    updateData->mObj[obj_id]->mMats[mat_id].needRefresh = true;
                }
            });
        connect(ui.Shadowsmooth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d)
            {
                if (obj_id > -1 && mat_id > -1)
                {
                    updateData->mObj[obj_id]->mMats[mat_id].Shadowsmooth = d;
                    updateData->mObj[obj_id]->mMats[mat_id].needRefresh = true;
                }
            });
        connect(ui.Depthstrength, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d)
            {
                if (obj_id > -1 && mat_id > -1)
                {
                    updateData->mObj[obj_id]->mMats[mat_id].Depthstrength = d;
                    updateData->mObj[obj_id]->mMats[mat_id].needRefresh = true;
                }
            });
        connect(ui.Normalstrength, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d)
            {
                if (obj_id > -1 && mat_id > -1)
                {
                    updateData->mObj[obj_id]->mMats[mat_id].Normalstrength = d;
                    updateData->mObj[obj_id]->mMats[mat_id].needRefresh = true;
                }
            });
        connect(ui.Specularsmooth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d)
            {
                if (obj_id > -1 && mat_id > -1)
                {
                    updateData->mObj[obj_id]->mMats[mat_id].Specularsmooth = d;
                    updateData->mObj[obj_id]->mMats[mat_id].needRefresh = true;
                }
            });
        connect(ui.Depthdistance, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d)
            {
                if (obj_id > -1 && mat_id > -1)
                {
                    updateData->mObj[obj_id]->mMats[mat_id].Depthdistance = d;
                    updateData->mObj[obj_id]->mMats[mat_id].needRefresh = true;
                }
            });
        connect(ui.Normaldistance, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d)
            {
                if (obj_id > -1 && mat_id > -1)
                {
                    updateData->mObj[obj_id]->mMats[mat_id].Normaldistance = d;
                    updateData->mObj[obj_id]->mMats[mat_id].needRefresh = true;
                }
            });
        connect(ui.Specularsoft, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=](double d)
            {
                if (obj_id > -1 && mat_id > -1)
                {
                    updateData->mObj[obj_id]->mMats[mat_id].Specularsoft = d;
                    updateData->mObj[obj_id]->mMats[mat_id].needRefresh = true;
                }
            });
    }
}

void StrikeEngine::paintEvent(QPaintEvent* event)
{
    UpdateD3DData();
    update();
}

void StrikeEngine::InitInterfaceData()
{
    objtree_model = new QStandardItemModel(ui.ObjTree);
    objtree_model->setHorizontalHeaderLabels(QStringList() << QStringLiteral("Scene Object: "));
    matlist_models.assign(updateData->mObj.size(), nullptr);
    texlist_qslists.assign(updateData->mObj.size(), QStringList());
    for(int i = 0; i < updateData->mObj.size();i++)
    {
        QList<QStandardItem*> obj_items;
        QStandardItem* obj_item = new QStandardItem(QString::fromStdWString(updateData->mObj[i]->name));
        obj_items.append(obj_item);
        objtree_model->appendRow(obj_items);

        matlist_models[i] = new QStandardItemModel(ui.MatList);
        for (int j = 0; j < updateData->mObj[i]->mMats.size(); j++)
        {
            matlist_models[i]->appendRow(
                new QStandardItem(
                    QString::fromStdWString(
                        updateData->mObj[i]->mMats[j].name)));
        }
        for (int j = 0; j < updateData->mObj[i]->mTexs.size(); j++)
        {
            texlist_qslists[i].push_back(
                    QString::fromStdString(
                        updateData->mObj[i]->mTexs[j]));
        }
    }

    

    ui.ObjTree->setModel(objtree_model);
}

void StrikeEngine::UpdateD3DData()
{

}


void StrikeEngine::slotObjCurrentChanged(
    const QModelIndex& current,
    const QModelIndex& previous)
{
    QStandardItem* item = objtree_model->itemFromIndex(current);
    if (item)
    {
        obj_id = item->row();
        showObj();
        mat_id = 0;
        showMat();
    }
}

void StrikeEngine::showObj()
{
    if (obj_id > -1)
    {
        ui.Transform_X->setValue(updateData->mObj[obj_id]->Transform[0]);
        ui.Transform_Y->setValue(updateData->mObj[obj_id]->Transform[1]);
        ui.Transform_Z->setValue(updateData->mObj[obj_id]->Transform[2]);

        ui.Rotation_X->setValue(updateData->mObj[obj_id]->Rotation[0]);
        ui.Rotation_Y->setValue(updateData->mObj[obj_id]->Rotation[1]);
        ui.Rotation_Z->setValue(updateData->mObj[obj_id]->Rotation[2]);

        ui.Scale_X->setValue(updateData->mObj[obj_id]->Scale[0]);
        ui.Scale_Y->setValue(updateData->mObj[obj_id]->Scale[1]);
        ui.Scale_Z->setValue(updateData->mObj[obj_id]->Scale[2]);

        ui.MatList->setModel(matlist_models[obj_id]);
    }
}

void StrikeEngine::slotMatClick(QModelIndex index)
{
    if (obj_id > -1)
    {
            mat_id = index.row();
            showMat();

    }
}


void StrikeEngine::showMat()
{
    if (obj_id > -1 && mat_id > -1)
    {
        ui.Lightcolor_R->setValue(updateData->mObj[obj_id]->mMats[mat_id].Lightcolor[0]);
        ui.Lightcolor_G->setValue(updateData->mObj[obj_id]->mMats[mat_id].Lightcolor[1]);
        ui.Lightcolor_B->setValue(updateData->mObj[obj_id]->mMats[mat_id].Lightcolor[2]);
        
        ui.Specularcolor_R->setValue(updateData->mObj[obj_id]->mMats[mat_id].Specularcolor[0]);
        ui.Specularcolor_G->setValue(updateData->mObj[obj_id]->mMats[mat_id].Specularcolor[1]);
        ui.Specularcolor_B->setValue(updateData->mObj[obj_id]->mMats[mat_id].Specularcolor[2]);

        ui.Shadowcolor_R->setValue(updateData->mObj[obj_id]->mMats[mat_id].Shadowcolor[0]);
        ui.Shadowcolor_G->setValue(updateData->mObj[obj_id]->mMats[mat_id].Shadowcolor[1]);
        ui.Shadowcolor_B->setValue(updateData->mObj[obj_id]->mMats[mat_id].Shadowcolor[2]);

        ui.Profilecolor_R->setValue(updateData->mObj[obj_id]->mMats[mat_id].Profilecolor[0]);
        ui.Profilecolor_G->setValue(updateData->mObj[obj_id]->mMats[mat_id].Profilecolor[1]);
        ui.Profilecolor_B->setValue(updateData->mObj[obj_id]->mMats[mat_id].Profilecolor[2]);
        
        ui.Thick->setValue(updateData->mObj[obj_id]->mMats[mat_id].Thick);
        ui.Shadowsmooth->setValue(updateData->mObj[obj_id]->mMats[mat_id].Shadowsmooth);
        ui.Shadowsoft->setValue(updateData->mObj[obj_id]->mMats[mat_id].Shadowsoft);
        ui.Specularsmooth->setValue(updateData->mObj[obj_id]->mMats[mat_id].Specularsmooth);
        ui.Specularsoft->setValue(updateData->mObj[obj_id]->mMats[mat_id].Specularsoft);
        ui.Depthstrength->setValue(updateData->mObj[obj_id]->mMats[mat_id].Depthstrength);
        ui.Depthdistance->setValue(updateData->mObj[obj_id]->mMats[mat_id].Depthdistance);
        ui.Normalstrength->setValue(updateData->mObj[obj_id]->mMats[mat_id].Normalstrength);
        ui.Normaldistance->setValue(updateData->mObj[obj_id]->mMats[mat_id].Normaldistance);
        
        combox_refresh = true;
        ui.DiffTex_comboBox->clear();
        ui.DiffTex_comboBox->addItems(texlist_qslists[obj_id]);
        ui.DiffTex_comboBox->setCurrentIndex(updateData->mObj[obj_id]->mMats[mat_id].DiffuseSrvHeapIndex - updateData->mObj[obj_id]->mTexs_srv_offset);
        ui.NormTex_comboBox->clear();
        ui.NormTex_comboBox->addItems(texlist_qslists[obj_id]);
        ui.NormTex_comboBox->setCurrentIndex(updateData->mObj[obj_id]->mMats[mat_id].NormalSrvHeapIndex - updateData->mObj[obj_id]->mTexs_srv_offset);
        combox_refresh = false;
    }
}

void StrikeEngine::slotDiffTexChange(int index)
{
    if (obj_id > -1 && mat_id > -1 && !combox_refresh)
    {
        updateData->mObj[obj_id]->mMats[mat_id].DiffuseSrvHeapIndex = updateData->mObj[obj_id]->mTexs_srv_offset + index;
        updateData->mObj[obj_id]->mMats[mat_id].needRefresh = true;
        ui.DiffTex_comboBox->setCurrentIndex(index);

    }
}

void StrikeEngine::slotNormTexChange(int index)
{
    if (obj_id > -1 && mat_id > -1 && !combox_refresh)
    {
        updateData->mObj[obj_id]->mMats[mat_id].NormalSrvHeapIndex = updateData->mObj[obj_id]->mTexs_srv_offset + index;
        updateData->mObj[obj_id]->mMats[mat_id].needRefresh = true;
        ui.NormTex_comboBox->setCurrentIndex(index);
    }
}