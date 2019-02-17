#ifndef CAXIS_H
#define CAXIS_H
#include <QWidget>
#include <QGraphicsProxyWidget>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QTransform>
#include <QGridLayout>
#include <QPainter>
#include <QPen>
#include <QLabel>
#include <QDebug>
#include <QtCore/qmath.h>
#include "ECL210A2661Params.h"
// Класс стрелки
class CTriangle:public QWidget
{
    Q_OBJECT
public:
    CTriangle(int weight =1, quint32 id=1, QWidget *parent=0):QWidget(parent)
    {
        this->parent = parent;
        ID = id;
        Weight = weight;
        setMaximumSize(this->parent->width(), this->parent->height());
        setMinimumSize(this->parent->width(), this->parent->height());
        setStyleSheet("QWidget{ border: none; background-color: transparent;}");
        Angle =0;
        from = to = QPointF(0,0);
    }
    QWidget *parent;
    quint32 ID;
    int Weight;
    qreal Angle;
    QPointF from, to;
    // Получение угла по 2-м точками
    void GetAngle(QPointF from, QPointF to)
    {
        // Если прямая параллельна оси Х
        if(from.y() == to.y())
        {
            // X - увеличивается (направление ->)
            if(from.x() < to.x())
            {
                Angle =0;
                return;
            }
            else
            {
                Angle =180;
                return;
            }
        }
        // Если прямая параллельна оси Y
        else if(from.x() == to.x())
        {
            // Y - увеличивается (направление вниз т.к. QT отсчет от начала координат экрана)
            if(from.y() < to.y())
            {
                Angle = 270;
                return;
            }
            else
            {
                Angle = 90;
                return;
            }
        }
        // Расчет если направление не параллельно осям
        // Добавочный угол
        qreal sd =0;
        // Направление вниз
        if(((to.y() - from.y())*(-1)) <0)
        {
            // Вправо
            if((to.x() - from.x()) > 0)
                sd = 270;
            // Влево
            else
                sd = 180;
        }
        // Направление вверх
        else
        {
            // Вправо
            if((to.x() - from.x()) > 0)
                sd = 0;
            // Влево
            else
                sd = 90;
        }
        qreal d =0;
        d =((((to.y() - from.y())*(-1))/(to.x() - from.x())));
        d =qAtan(d);
        d =qRadiansToDegrees(d);
        // Если угол отрицательный, нужно сделать положительным т.к. вращаем в одну сторону (против часовой)
        if(d < 0)
            d *= (-1);
        // Добавочный угол (четверть окружности I =0, II =90, III =180, IV =270)
        if((sd ==90)|(sd ==270))
            Angle = 90 -d + sd;
        else
            Angle = d + sd;
        return;
    }
protected:
    void paintEvent(QPaintEvent *e)
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setBrush(QBrush("#AFDFAF"));
        painter.setPen(QPen(QColor("#AFDFAF")));
        QPolygonF poly;
        poly<<QPointF(0-Weight * 2,0 - Weight) // Расположение таким образом, чтобы начало координат было центром вращения
            <<QPointF(0 + Weight*2, 0)
            <<QPointF(0-Weight * 2, 0 +Weight);
        painter.translate(to.x(), to.y());     // Перемещение начала координат в точку 2
        painter.rotate(Angle *(-1));           // Поворот на вычисленный угол относительно оси X
        painter.drawPolygon(poly);             // Прорисовка в повернутой и сдвинутой точке (0,0)
        painter.end();
        QWidget::paintEvent(e);
    }
public slots:
    void ReDraw(QPointF from, QPointF to, quint32 id)
    {
        if(id!=ID)return;
        setMaximumSize(parent->width(), parent->height());
        setMinimumSize(parent->width(), parent->height());
        this->from = from;
        this->to = to;
        GetAngle(from, to);
        this->update();
    }
};
// Класс оси
class CAxis : public QWidget
{
    Q_OBJECT
public:
    explicit CAxis(QPointF fromX, QPointF toX, QPointF fromY, QPointF toY,
                   int weight, quint32 id, QWidget* pwgt) : QWidget(pwgt)
    {
        this->pwgt = pwgt;
        ID = id;
        FromX = fromX;
        ToX = toX;
        FromY = fromY;
        ToY = toY;
        Weight = weight;
        setStyleSheet("QWidget{ border: none; background-color: transparent; color: white; }"
                      "QWidget:!enabled {border: none; background-color: transparent; color: #8d8d8d; }"
                     );
        setMaximumSize(this->pwgt->width(), this->pwgt->height());
        setMinimumSize(this->pwgt->width(), this->pwgt->height());
        arrowX = new CTriangle(Weight, ID, this);
        arrowY = new CTriangle(Weight, ID+1, this);
    }
    QList<QPointF> *pTemp;          // Перечень координат точек графика для рисования (в градусах)
    QList<QPointF> *pCoord;         // Перечень координат точек графика для рисования (в координатах)
    QWidget *pwgt;
    CTriangle *arrowX, *arrowY;
    quint32 ID;
    QPointF FromX, ToX, FromY, ToY;
    int Weight;
    quint8 Side;
protected:
    void paintEvent(QPaintEvent* e)
    {
        // Рисование линии
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setBrush(QBrush("#AFDFAF"));
        QPen pen(QColor("#AFDFAF"));
        pen.setWidth(Weight);
        painter.setPen(pen);
        painter.drawLine(QPointF(FromX.x()-10,FromX.y() +10), QPointF(ToX.x() + 10,ToX.y() +10));
        painter.drawLine(QPointF(FromY.x()-10,FromY.y() +10), QPointF(ToY.x() - 10,ToY.y() -10));
        QPointF Scale;
        // Отображение делений по оси X (не меняются)
        for(int i =0; i < pTemp->size(); i++)
        {
            Scale.setX(FromX.x() + ((ToX.x() -FromX.x())*(pTemp->at(i).x() - pTemp->first().x())/
                                  (pTemp->last().x() - pTemp->first().x())) - Weight/2); // С учетом толщины
            Scale.setY(ToX.y()-Weight + ToY.y()/3);                                    // От оси на толщину вверх
            painter.drawLine(Scale, QPointF(Scale.x(), Scale.y() + Weight*2));
        }
        // Отображение делений по оси Y (меняются)
        for(int i =0; i < pTemp->size(); i++)
        {
            Scale.setX(FromY.x() - Weight - FromY.x()/3);                                        // С учетом толщины
            Scale.setY(FromY.y() + ((ToY.y() - FromY.y())*(pTemp->at(i).y() - pTemp->last().y())/
                                  (pTemp->first().y() - pTemp->last().y()))  - Weight/2);                                           // От оси на толщину вверх
            painter.drawLine(Scale, QPointF(Scale.x() + Weight*2, Scale.y()));
        }
        // Соединение точек графика
        pen = QPen(QColor("#6F8F6F"));
        pen.setWidth(Weight);
        painter.setPen(pen);
        for(int i=1; i < pCoord->size(); i++)
        {
            QPointF p1, p2;
            p1 = pCoord->at(i-1);
            p2 = pCoord->at(i);
            if((i -1) !=0)
            {
                p1.setX(p1.x() + Side/2);
                p1.setY(p1.y() + Side/2);
            }
            if(i !=(pCoord->size()-1))
            {
                p2.setX(p2.x() + Side/2);
                p2.setY(p2.y() + Side/2);
            }
            painter.drawLine(p1 , p2);
        }
        // Соединение проекции на ось Y и X
        pen = QPen(QColor("#3F5F3F"));
        pen.setWidth(Weight/2);
        painter.setPen(pen);
        for(int i=1; i < (pCoord->size()-1); i++)
        {
            QPointF p1;
            p1 = pCoord->at(i);
            p1.setX(p1.x() + Side/2);
            p1.setY(p1.y() + Side/2);
            painter.drawLine(p1 , QPointF(p1.x(), FromX.y() +10)); // Линия к оси X
            painter.drawLine(p1 , QPointF(FromX.x() -10, p1.y())); // Линия к оси Y
        }
        painter.end();
        arrowX->ReDraw(QPointF(FromX.x()-10,FromX.y() +10), QPointF(ToX.x() + 10,ToX.y() +10), ID);
        arrowY->ReDraw(QPointF(FromY.x()-10,FromY.y() +10), QPointF(ToY.x() - 10,ToY.y() -10), ID+1);
        QWidget::paintEvent(e);
    }
public slots:
    void ReDraw(QPointF fromX, QPointF toX, QPointF fromY, QPointF toY, quint32 id)
    {
        if(id!=ID)return;
        FromX = fromX;
        ToX = toX;
        FromY = fromY;
        ToY = toY;
        setMaximumSize(pwgt->width(), pwgt->height());
        setMinimumSize(pwgt->width(), pwgt->height());
        this->update();
    }
};
#endif // CAXIS_H
