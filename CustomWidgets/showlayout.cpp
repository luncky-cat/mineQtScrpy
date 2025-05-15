#include "showlayout.h"
#include<QLabel>

showLayout::showLayout(QWidget *parent)
    : QWidget{parent}
{

    initResource();
    initLayout();
    initSignal(); //初始信号
    //updateTotal(8);
}

void showLayout::initResource(){
    ranges = QMap<int, QPair<int, int>>{
        {1, {1, 1}},
        {4, {2,2}},
        {9, {3,3}},
        {16, {4,4}},
        {32, {4,8}}
    };
    currentRange=qMakePair(0,1);
    total=0;
}

void showLayout::initLayout(){
    showSrc=new QGridLayout(this);
    qDebug()<<"gird init";
    total=9;
    updateLayout(showSrc,3,3);

}



void showLayout::initSignal(){
    connect(this,&showLayout::totalChanged,this,&showLayout::adjustLayout);
}


void showLayout::updateTotal(qint16 total)
{
    if (this->total != total) {
        this->total = total;
        emit totalChanged();
    }
}

qint16 showLayout::getTotal()
{
    return total;
}


void showLayout::adjustLayout()
{
    auto&[min,max]=currentRange;
    qDebug()<<"min:"<<min<<"max"<<max;
    if(min<total&&total<=max){   //无需调整
        return;
    }

    auto it=ranges.find(max);
    bool changed=false;
    if(total<=min){   //减小
        if(it!=ranges.begin()){
            auto preIt=std::prev(it);
            max=min;
            min=preIt.key();
            changed=true;
        }
    }else if(total>max){  //增大
        if(it!=ranges.end()){
            auto nextIt=std::next(it);
            min=max;
            max=nextIt.key();
            changed=true;
        }
    }

    qDebug()<<"adjustLayout:"<<"min:"<<min<<"max:"<<max;

    //调整
    if(changed){
        qDebug()<<"true";
        updateLayout(showSrc,min,max);
    }
}
void showLayout::updateLayout(QGridLayout *layout, int cols, int rows) {
    if (!layout) return;
    qDebug()<<"aa";

    // 1. 移除所有控件但不销毁
    while (QLayoutItem *item = layout->takeAt(0)) {
        if (QWidget *widget = item->widget()) {
            layout->removeWidget(widget);
            widget->setParent(nullptr); // 可选：解除父子关系
        }
        delete item;
    }

    // 2. 清除原有的行列伸缩设置
    // int oldRowCount = layout->rowCount();
    // int oldColCount = layout->columnCount();
    // for (int i = 0; i < oldRowCount; ++i) {
    //     layout->setRowStretch(i, 0);
    // }
    // for (int j = 0; j < oldColCount; ++j) {
    //     layout->setColumnStretch(j, 0);
    // }

    // // 3. 设置新的行列伸缩因子
    // for (int i = 0; i < rows; ++i) {
    //     layout->setRowStretch(i, 1);
    // }
    // for (int j = 0; j < cols; ++j) {
    //     layout->setColumnStretch(j, 1);
    // }

    // 4. 重新添加控件到新的布局

    int index = 0;
    for (int i = 0; i < rows/* && index < widgets.size()*/; ++i) {
        for (int j = 0; j < cols /*&& index < widgets.size()*/; ++j) {
            //layout->addWidget(widgets[index], i, j);
            layout->addWidget(new QLabel("77"),i,j);
            qDebug()<<"i:"<<i<<"j"<<j;
            ++index;
        }
    }
}
