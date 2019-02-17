#ifndef CTEMPGRAPH_H
#define CTEMPGRAPH_H
#include <QWidget>
#include <QLabel>
#include <QResizeEvent>
#include <QStyleOption>
#include <QPainter>
#include <QDebug>
#include "CMovedLabel.h"
#include "ECL210A2661Params.h"
#include "CAxis.h"
// Класс виджета температурного графика
 class CTempGraph : public QWidget
 {
     Q_OBJECT
 public:
     explicit CTempGraph(QWidget* pwgt =0) : QWidget(pwgt)
     {
         QFont sfont("Times", 7);
         sfont.setBold(true);
         QFontMetrics metrics(sfont);
         this->setMinimumSize(115,90);        // Минимальный размер виджета для начала работы
         w = this->minimumSize().width();     // Расчет начального расположения точек
         h = this->minimumSize().height();
         dX = dY = 30;
         pCoord.clear();                      // Координаты точек
         pTemp.clear();                       // Температуры точек
         pTextScales.clear();
         lParamGraphScale.clear();
         Side = FactoryPointSide;             // Размер точки по умолчанию
         // Температуры минимума и максимума Tнв(ось X) и Tподачи  (ось Y)
         pTemp << QPointF(FactoryXTextScale.first().toDouble(), // Заводские уставки температуры нв MIN/MAX)
                          FactoryParams.at(1).toDouble());      // Заводские уставки температуры подачи MIN/MAX
         pTemp << QPointF(FactoryXTextScale.last().toDouble(),
                          FactoryParams.at(0).toDouble());
         // Координаты точек по заводским уставкам с учетом стороны точки
         for(int i=0; i < (FactoryTPointsOut.size() + 2); i++)
         {
             // Первая точка Tнв - мин, Tпод - макс
             if(i ==0)
             {
                 pCoord << QPointF(dX, dY);
             }
             // Температура рабочей точки графика и координаты с учетом размера точки
             else if(i < (FactoryTPointsOut.size() + 1))
             {
                 pTemp.insert(i, QPoint(FactoryTPointsOut.at(i-1).toDouble(),
                                          FactoryTPointsHeat.at(i-1).toDouble()));
                 pCoord << QPointF(dX + ((w -2*dX)*(pTemp.at(i).x() - pTemp.first().x()))/
                                  (pTemp.last().x() - pTemp.first().x()) -Side/2,
                                  (dY - (h -dY))*(pTemp.at(i).y() - pTemp.last().y())/
                                  (pTemp.first().y() - pTemp.last().y()) + h -dY -Side/2);
             }
             // Последняя точка Tнв - макс, Тпод - мин
             else
             {
                 pCoord << QPointF(w - dX, h- dY);
                 break;
             }
         }
         // Координаты подписей осей (перечень координат для размещения текстовых обозначений градуировки)
         for(int i =0; i < (FactoryXTextScale.size() + FactoryYTextScale.size()); i++)
         {
             QString lText ="-";
             // Со сдвигом по оси Х вправо
             if(i ==0)
             {
                pTextScales << QPointF(dX +5, (h - dY) + dY/3 +5); // По Y - от начала раб зоны графика (h - dY), оси X dY/3 , 5 отступ от оси Х
                lText = FactoryXTextScale.at(i);
             }
             // По оси Х прям под делением по центру
             else if(i < FactoryXTextScale.size())
             {
                 bool IsOk;
                 qreal data =FactoryXTextScale.at(i).toDouble(&IsOk);
                 // Текстовое значение располагается по центру оси
                 if(!IsOk)
                    pTextScales << QPointF(w/2 - (metrics.width(FactoryXTextScale.at(i))/2),(h - dY) + dY/3 +5);
                 else
                     pTextScales << QPointF(dX + (w -(2*dX))*(data -pTemp.first().x())/
                                           (pTemp.last().x() - pTemp.first().x()) - (metrics.width(FactoryXTextScale.at(i))/2),
                                            (h - dY) + dY/3 +5);
                lText = FactoryXTextScale.at(i);
             }
             // По оси Y сразу от начала виджета по X по центру шрифта
             else if(i >= FactoryXTextScale.size())
             {
                bool IsOk;
                qreal data =FactoryYTextScale.at(i -FactoryXTextScale.size()).toDouble(&IsOk);
                // Текстовое значение располагается по центру оси
                if(!IsOk)
                   pTextScales << QPointF(0, h/2 - (metrics.height()/2));
                else
                {
                    pTextScales << QPointF(0, (dY - (h -dY))*(data - pTemp.last().y())/
                                          (pTemp.first().y() - pTemp.last().y()) + h -dY - metrics.height()/2);
                }
                lText = FactoryYTextScale.at(i -FactoryXTextScale.size());
             }
             lParamGraphScale << new QLabel(this);
             lParamGraphScale.last()->setFont(sfont);
             lParamGraphScale.last()->setAlignment(Qt::AlignCenter);
             lParamGraphScale.last()->setText(lText);
             lParamGraphScale.last()->setFixedSize(18,12);
             lParamGraphScale.last()->setStyleSheet("QLabel{ border: none; background-color: transparent; color: white; }"
                                                "QLabel:!enabled {border: none; background-color: transparent; color: #8d8d8d; }"
                                               );
             lParamGraphScale.last()->move(pTextScales.at(i).x(), pTextScales.at(i).y());
         }
         // Оси координат (X,Y)
         Axis = new CAxis(QPointF(dX, h-dY), QPointF(w-dX, h-dY),
                          QPointF(dX, h-dY), QPointF(dX, dY),
                           2, 1, this);
         connect(this, SIGNAL(sendAxisRepaint(QPointF,QPointF,QPointF,QPointF,quint32)),
                 Axis, SLOT(ReDraw(QPointF,QPointF,QPointF,QPointF,quint32)));
         // Деления координат равны точкам координат + MIN/MAX
         Axis->pTemp  = &pTemp;
         Axis->pCoord = &pCoord;
         Axis->Side = Side;
         // Рабочие точки графика
         for(int j=0; j < (pTemp.size()-2); j++)
         {
             lParamGraphPoints << new CMovedLabel(this, j+1, &pTemp, &pCoord, Side);
             //pTemp << lParamGraphPoints[j]->coord;
             connect(this, SIGNAL(sendMove()), lParamGraphPoints[j], SLOT(onMove())/*, Qt::QueuedConnection*/);
         }
         connect(this, SIGNAL(sendResize()), this, SLOT(onResize())/*, Qt::QueuedConnection*/);
         // устанавливаем цвет фона
         this->setStyleSheet("QWidget{ border: 2pCoord solid black; background-color: #2d2d2d; color: black; }"
                             "QWidget:hover { border: 1pCoord solid black; background: #1d1d1d; color: black; }"
                             "QWidget:!hover { border: 1pCoord solid black; background: #2d2d2d; color: black; }"
                             "QWidget:!enabled { border: 1pCoord solid black; background: #CACACA; color: #3d3d3d; }");
     }
     qreal w, h;
     qreal dX, dY;                // Отступы от начала виджета до начальной метки и от конца до конечной
     qreal Side;                  // Размер точки графика
     QList<QPointF> pTemp;        // Температуры по осям Х,Y параметров 0 - X-min/Y-max(6 точек) последняя X-max/Y-min
     QList<QPointF> pCoord;       // Координаты по осям Х,Y параметров (6 точек)
     QList<QPointF> pTextScales;  // Координаты делений по осям X,Y (9 точек) для отображения текста
     QList<CMovedLabel*>lParamGraphPoints;// Лэйблы точек графика и курсор активации точки (6)
     QList<QLabel*>lParamGraphScale; // Лэйблы температур на шкале графика (9)
     CAxis *Axis;                 // Отрезки осей
 public slots:
     // Получение сигнала об изменении размера виджета
     void onResize()
     {
         QFont sfont("Times", 7);
         sfont.setBold(true);
         QFontMetrics metrics(sfont);
         w = this->width();
         h = this->height();
         pCoord.clear();                       // Координаты точек
         // Координаты точек по заводским уставкам с учетом стороны точки
         for(int i=0; i < pTemp.size(); i++)
         {
             // Первая точка Tнв - мин, Tпод - макс
             if(i ==0)
             {
                 pCoord << QPointF(dX, dY);
             }
             // Температура рабочей точки графика и координаты с учетом размера точки
             else if(i < (pTemp.size()-1))
             {
                 pCoord << QPointF(dX + ((w -2*dX)*(pTemp.at(i).x() - pTemp.first().x()))/
                                  (pTemp.last().x() - pTemp.first().x()) -Side/2,
                                  (dY - (h -dY))*(pTemp.at(i).y() - pTemp.last().y())/
                                  (pTemp.first().y() - pTemp.last().y()) + h -dY -Side/2);
             }
             // Последняя точка Tнв - макс, Тпод - мин
             else
             {
                 pCoord << QPointF(w - dX, h -dY);
                 break;
             }
         }
         // Координаты подписей осей (перечень координат для размещения текстовых обозначений градуировки)
         for(int i =0; i < pTextScales.size(); i++)
         {
             // Со сдвигом по оси Х вправо
             if(i ==0)
             {
                pTextScales[i] = QPointF(dX +5, (h - dY) + dY/3 +5); // По Y - от начала раб зоны графика (h - dY), оси X dY/3 , 5 отступ от оси Х
             }
             else if(i < FactoryXTextScale.size())
             {
                 bool IsOk;
                 qreal data =FactoryXTextScale.at(i).toDouble(&IsOk);
                 // Текстовое значение располагается по центру оси
                 if(!IsOk)
                    pTextScales[i] = QPointF(w/2 - (metrics.width(FactoryXTextScale.at(i))/2),(h - dY) + dY/3 +5);
                 else
                    pTextScales[i] = QPointF(dX + (w -(2*dX))*(data -pTemp.first().x())/
                        (pTemp.last().x() - pTemp.first().x()) - (metrics.width(FactoryXTextScale.at(i))/2),
                        (h - dY) + dY/3 +5);
             }
             // По оси Y сразу от начала виджета по X по центру шрифта
             else if(i >= FactoryXTextScale.size())
             {
                bool IsOk;
                QString dstr = lParamGraphScale[i]->text();
                qreal data =dstr.toDouble(&IsOk);
                if(i == pTextScales.size()-3)
                    data = pTemp.last().y();
                else if(i ==pTextScales.size()-1)
                    data = pTemp.first().y();
                // Текстовое значение располагается по центру оси
                if(!IsOk)
                   pTextScales[i] = QPointF(0, h/2 - (metrics.height()/2));
                else
                {
                   pTextScales[i] = QPointF(0, (dY - (h -dY))*(data - pTemp.last().y())/
                                          (pTemp.first().y() - pTemp.last().y()) + h -dY - metrics.height()/2);
                   lParamGraphScale[i]->setText(QString::number(data, 'f', 0));
                }
             }
             lParamGraphScale[i]->move(pTextScales.at(i).toPoint());
         }
         // Деления координат равны точкам координат + MIN/MAX
         Axis->pTemp  = &pTemp;
         Axis->pCoord = &pCoord;
         // Обновление координат X
         QPointF X1(dX, h-dY); QPointF X2(w-dX, h-dY);
         QPointF Y1(dX, h-dY); QPointF Y2(dX, dY);
         emit sendAxisRepaint(X1, X2, Y1, Y2, 1);
         emit sendMove();
     }
     // Произошло изменение температуры TMin .... TMax
     void onTempBorderChange(QString t, quint32 index)
     {
         if(index >1)return;
         bool IsOk;
         double Temp = t.toDouble(&IsOk);
         // Основной диапазон
         if((!IsOk)|(Temp < ParamRangesVal.at(index).x())|
            (Temp > ParamRangesVal.at(index).y()))
             return;
         // Если значения точек то в диапазоне TMAX/TMIN
         if(index ==0)
            pTemp[pTemp.size()-1].setY(Temp);
         else if(index ==1)
            pTemp[0].setY(Temp);
         // Если у точек значения больше чем TMAX или меньше чем TMIN - подкорректируем
         for(int i=1; i < (pTemp.size()-1); i++)
         {
             if(pTemp[i].y() > pTemp[0].y())
                 pTemp[i].setY(pTemp[0].y());
             else if(pTemp[i].y() < pTemp.last().y())
                 pTemp[i].setY(pTemp.last().y());
         }
         emit sendResize();
     }
     // Произошло изменение температуры T1...Tn
     void onTempChange(QString t, quint32 index)
     {
         if((index <1)|((int)index >= (pTemp.size()-1)))
             return;
         bool IsOk;
         double Temp = t.toDouble(&IsOk);
         // Основной диапазон
         if((!IsOk)|(Temp > pTemp.at(0).y())|(Temp < pTemp.last().y()))
             return;
         // Если значения точек то в диапазоне
         pTemp[index].setY(Temp);
         emit sendResize();
     }
 signals:
     void sendResize();
     void sendMove();
     void sendAxisRepaint(QPointF, QPointF, QPointF, QPointF, quint32);
 private:
     void resizeEvent(QResizeEvent * event )
     {
         QWidget::resizeEvent(event);
         event->accept();
         emit sendResize();
     }
     void paintEvent(QPaintEvent *)
     {
        QStyleOption opt;
        opt.init(this);
        QPainter p(this);
        style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
     }
 };
#endif // CTEMPGRAPH_H

