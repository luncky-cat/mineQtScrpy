#ifndef SHOWLAYOUT_H
#define SHOWLAYOUT_H

#include <QWidget>
#include<QGridLayout>
#include<QVector>
#include <QMediaPlayer>
#include <QVideoWidget>
#include<QMap>
#include<QPair>

class showLayout : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qint16 total READ getTotal WRITE updateTotal NOTIFY totalChanged)
    //Q_PROPERTY(qint16 total READ getTotal WRITE updateTotal NOTIFY totalChanged)
public:
    explicit showLayout(QWidget *parent = nullptr);
private:
    void initSignal(); //初始化信号
private:
    qint16 total;   //展示的总数
    QMap<int, QPair<int,int>>ranges; //最大-布局
    QPair<int,int>currentRange; //当前区间
    QGridLayout *showSrc;  //展示布局
    QVector<QVideoWidget*>videoW; //存储页面指针
    QVector<QWidget*>widgets;
private slots:
    void adjustLayout();
private:
    void updateTotal(qint16 newTotal);
    qint16 getTotal();
    void updateLayout(QGridLayout *showSrc, int cols, int rows);
    void initResource();
    void initLayout();
signals:
    void totalChanged();
};

#endif // SHOWLAYOUT_H
