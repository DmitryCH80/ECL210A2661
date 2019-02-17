#ifndef CMOVEDLABEL_H
#define CMOVEDLABEL_H
#include <QLabel>
#include <QMouseEvent>
#include <QDebug>
// Класс лэйбла перемещаемого мышкой
class CMovedLabel : public QLabel
{
    Q_OBJECT
public:
    explicit CMovedLabel(QWidget *pwgt, quint32 pointnumber, QList<QPointF> *temp,
                         QList<QPointF> *coord, qreal side) :QLabel(pwgt)
    {
        Side =side;                                                      // Размер стороны
        pointNumber = pointnumber;                                       // Номер точки для определения Tout
        pTemp = temp;
        pCoord = coord;
        setFixedSize(Side,Side);
        setStyleSheet("QLabel{ border: 2px solid white; background-color: #AdAdAd; color: black; }"
                      "QLabel:hover { border: 1px solid #a2eea6; background: #ff7e73; color: black; }"
                      "QLabel:!hover { border: 1px solid white; background: #AdAdAd; color: black; }"
                      "QLabel:!enabled { border: 1px solid #AAAAAA; background: #8A8A8A; color: #3d3d3d; }"
                      );
    }
    QList<QPointF> *pTemp; // Точки графика (градусы)
    QList<QPointF> *pCoord;// Точки графика (координаты)
    bool Activated;        // Флаг фокуса на данной точке
    QPointF offset;        // Сдвиг указателя мышки относительно начала координат данного объекта
    qreal Side;
public slots:
    // Сброс активности точки
    void FromFocus(quint8 number)
    {
        if((number!=pointNumber)&(Activated ==true))
        {
            Activated =false;
        }
    }
    // Передвижение в случае изменения размеров окна родителя
    void onMove()
    {
        this->move(pCoord->at(pointNumber).toPoint());
        emit PointTemperature(pTemp->at(pointNumber).y(), pointNumber);
    }
protected:
    quint32 pointNumber;    // Номер точки графика
    // Нажали на ЛКМ
    void mousePressEvent(QMouseEvent *event)
    {
        offset = event->pos();
        Activated = true;
        emit PointInFocus(pointNumber);
    }
    // Движение с нажатой ЛКМ
    void mouseMoveEvent(QMouseEvent *event)
    {
        qreal t;
        if(!Activated)
            return;
        if(event->buttons() & Qt::LeftButton)
        {
            // Расчет координат
            QPointF newpoint, parentpoint;
            qreal Y;
            newpoint= event->pos() - offset;     // Из координат убираем растояние относительно начала координат
            parentpoint = mapToParent(newpoint.toPoint());
            parentpoint.setX(pCoord->at(pointNumber).x());
            if(parentpoint.y() < (pCoord->first().y() - Side/2))
                parentpoint.setY(pCoord->first().y() - Side/2);
            else if(parentpoint.y() > (pCoord->last().y() - Side/2))
                parentpoint.setY(pCoord->last().y() - Side/2);
            this->move(parentpoint.toPoint());
            Y = parentpoint.y() + Side/2;
            t = (Y - pCoord->last().y())*(pTemp->first().y() - pTemp->last().y())/
                (pCoord->first().y() - pCoord->last().y()) + pTemp->last().y();
            (*pCoord)[pointNumber] = parentpoint;        // Перезаписали точку
            (*pTemp)[pointNumber].setY(t);
            emit PointTemperature(t, pointNumber);
        }
    }
    // Отпустили ЛКМ
    void mouseReleaseEvent(QMouseEvent *event)
    {
        if (event->button() == Qt::LeftButton)
        {
            if(Activated)
            {
                Activated =false;
            }
        }
    }
signals:
    void PointInFocus(quint8);             // Отправка другим точкам и lParamGraphCursor, lParamGraphCurrentPoint информации о том, что фокус на этой теперь точке
    void PointTemperature(float, quint32); // Отправка лэйблам температуры и номера точки для вывода
};
#endif // CMOVEDLABEL_H
