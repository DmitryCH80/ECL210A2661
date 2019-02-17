#ifndef CPROGRESS_H
#define CPROGRESS_H
#include <QWidget>
#include <QProgressBar>
#include <QLabel>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QDebug>
#include <QApplication>

// ======================================================================
class CProgress : public QWidget
{
    Q_OBJECT
private:
    QProgressBar* m_pprb;
    QLabel      * lText;
    int           m_nStep;
    int           Step;
    int           MaxRange;
    bool          FirstClose;
public:
    explicit CProgress(QWidget* pobj, int mMaxRange, int mStep) : QWidget(pobj,
                                          /*Qt::WindowTitleHint |*/
                                          /*Qt::WindowStaysOnTopHint |*/ Qt::FramelessWindowHint
                                          /*Qt::WindowCloseButtonHint*/), m_nStep(0), Step(mStep), MaxRange(mMaxRange)
    {
        this->setAttribute(Qt::WA_DeleteOnClose, true);
        if(parentWidget()!=0)
            parentWidget()->setEnabled(false);
        QFont font("Times",12);
        font.setBold(true);
        FirstClose = true;
        m_pprb = new QProgressBar;
        lText = new QLabel("Ожидайте завершения", this);
        lText->setFont(font);
        lText->setStyleSheet("QLabel { border: 2px solid black; background-color: #f08080; color: black; }"
                             "QLabel:!enabled { border: 2px solid black; background-color: #f08080; color: black; }");
        m_pprb->setStyleSheet("QProgressBar { border: 2px solid black; background-color: #E0EFE0; color: black; }"
                              "QProgressBar:!enabled { border: 2px solid black; background-color: #E0EFE0; color: black; }");
        lText->setAlignment(Qt::AlignCenter);
        m_pprb->setRange(0, mMaxRange);
        m_pprb->setValue(0);
        //m_pprb->setFixedWidth(parentWidget()->frameGeometry().width()/2);
        //m_pprb->setMinimumHeight(50);
        //lText->setFixedWidth(m_pprb->width());
        m_pprb->setAlignment(Qt::AlignCenter);

        //Layout setup
        QVBoxLayout* pvbxLayout = new QVBoxLayout;
        pvbxLayout->setSpacing(2);
        pvbxLayout->addWidget(lText);
        pvbxLayout->addWidget(m_pprb);
        setLayout(pvbxLayout);
        connect(this, SIGNAL(sendSize()), this, SLOT(onSize()), Qt::QueuedConnection);
    }
    void show()
    {
        this->QWidget::show();
        emit sendSize();
        return;
    }
public slots:
    void onSize()
    {
        this->adjustSize();
        setFixedSize(((float)this->width())*1.5, this->height()/*m_pprb->width(), m_pprb->height() + lText->height()*/);
        move(QPoint((parentWidget()->frameGeometry().width() - this->frameGeometry().width())/2,
             (parentWidget()->frameGeometry().height() - this->frameGeometry().height())/2));
    }
    void onStep ()
    {
        m_nStep += Step;
        m_pprb->setValue(m_nStep);
        if(m_nStep >=MaxRange)
            onClose();
    }
    void onReset()
    {
        m_nStep = 0;
        m_pprb->reset();
    }
    void onClose()
    {
        qDebug() << " > CProgress::onClose: Закрытие по сигналу";
        QCloseEvent* e = new QCloseEvent();
        QApplication::sendEvent(this, e);
    }
protected:
    virtual void closeEvent(QCloseEvent* event)
    {
        qDebug() << " > CProgress::closeEvent: Закрытие окна прогресса";
        event->accept();
        if(FirstClose)
        {
            FirstClose = false;
            if(parentWidget()!=0)
                parentWidget()->setEnabled(true);
            this->close();
        }
    }
    virtual void keyPressEvent(QKeyEvent* key)
    {
        switch(key->key())
        {
        case Qt::Key_Escape:
        {
            qDebug() << " > CProgress::keyPressEvent: Игнорирование нажатия клавиши ESC";
            break;
        }
        default:
            break;
        }
        key->accept();
    }
signals:
    void sendSize();
};

#endif // CPROGRESS_H
