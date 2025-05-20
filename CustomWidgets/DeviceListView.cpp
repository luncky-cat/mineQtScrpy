#include "DeviceListView.h"
#include "DeviceListModel.h"

#include <QDebug>

DeviceListView::DeviceListView(QWidget *parent)
    : QListView(parent), m_deviceModel(nullptr)
{
    setSelectionMode(QAbstractItemView::SingleSelection); // 单选模式
}

void DeviceListView::setDeviceModel(DeviceListModel *model)
{
    m_deviceModel = model;
    setModel(m_deviceModel); // 设置模型
}

// void DeviceListView::drawRow(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
// {
//     // if (!index.isValid()) return;

//     // // 设置交替行的背景色
//     // if (index.row() % 2 == 0) {
//     //     painter->fillRect(option.rect, QColor(240, 240, 240)); // 背景颜色
//     // } else {
//     //     painter->fillRect(option.rect, QColor(255, 255, 255)); // 背景颜色
//     // }

//     // // 画文本
//     // QStyleOptionViewItem opt = option;
//     // initStyleOption(&opt, index);
//     // style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter);
// }

void DeviceListView::mouseDoubleClickEvent(QMouseEvent *event)
{
    // QModelIndex index = indexAt(event->pos());
    // if (index.isValid()) {
    //     qDebug() << "Double clicked on device: " << index.data(Qt::DisplayRole).toString();
    // }
    // QListView::mouseDoubleClickEvent(event); // 调用基类处理
}

void DeviceListView::contextMenuEvent(QContextMenuEvent *event)
{
    // QMenu menu;
    // menu.addAction("Option 1");
    // menu.addAction("Option 2");
    // menu.exec(event->globalPos());
}


// class DeviceItemDelegate : public QStyledItemDelegate {
// public:
//     void paint(QPainter *painter, const QStyleOptionViewItem &option,
//                const QModelIndex &index) const override {
//         // 在这里写你原本想放在 drawRow() 中的绘制逻辑
//     }
// };
