#ifndef CINFODIALOG_H
#define CINFODIALOG_H
#include <QDialog>
#include <QLabel>
#include <QDebug>
#include <QComboBox>
#include <QTextEdit>
#include <QGridLayout>
#include <QPushButton>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QApplication>
#include <QScrollBar>

class CInfoDialog : public QDialog
{
    Q_OBJECT
public:
    explicit CInfoDialog(QWidget *pwgt, QString messageText ="", quint8 attribute =0) : QDialog(pwgt, Qt::WindowTitleHint|
                                                                                                Qt::WindowCloseButtonHint)
    {
        this->setAttribute(Qt::WA_DeleteOnClose, true);
        fontsize = 10;
        FirstClose = true;
        QFont font("Times", fontsize);
        QFont fontBig("Times", fontsize+4);
        fontBig.setBold(true);
        QPixmap pix;
        MessageText = messageText;
        Attribute   = attribute;
        lMessageHeader = new QLabel(this);
        lMessageHeader->setFont(fontBig);
        lMessageHeader->setAlignment(Qt::AlignCenter);
        lImage = new QLabel(this);
        lImage->setFont(font);
        lImage->setAlignment(Qt::AlignCenter);
        eMessage = new QTextEdit(this);
        eMessage->setReadOnly(true);
        eMessage->setFont(font);
        eMessage->clear();
        eMessage->append(MessageText);
        eMessage->setAlignment(Qt::AlignJustify);
        eMessage->setStyleSheet("QTextEdit { background-color: #E2E2FA; }"
                                "QTextEdit:!enabled { background-color: #b8b8b8; color: #606060; }");
        eMessage->verticalScrollBar()->setStyleSheet("QScrollBar { background-color: #bEbEcE; }"
                                                     "QScrollBar:!enabled { background-color: #bEbEcE; }");
        eMessage->horizontalScrollBar()->setStyleSheet("QScrollBar { background-color: #bEbEcE; }"
                                                       "QScrollBar:!enabled { background-color: #bEbEcE; }");
        connect(eMessage, SIGNAL(textChanged()), this, SLOT(onChangeLogPosition()));
        //if(parentWidget()!=0)
        //    parentWidget()->setEnabled(false);
        switch (Attribute) {
        // Логотип НПК СКАРТ
        case 0:
        {
            pix.load(":/icon_1.png");
            pix = pix.scaled(80,80,Qt::KeepAspectRatio,Qt::SmoothTransformation);
            this->setStyleSheet("QDialog{ background-color: #a2ddb0; color: black; }"
                                "QDialog:hover { background-color: #a2ddb0; color: black; }"
                                "QDialog:!hover { background-color: #a2ddb0; color: black; }"
                                "QDialog:!enabled { background-color: #647e6a; color: #C2C2D2; }");
            lMessageHeader->setText("Информация о разработчике ООО \"НПК\"СКАРТ\"");
            break;
        }
        // Внимание
        case 1:
        {
            pix.load(":/Attention_1_512.png");
            pix = pix.scaled(80,80,Qt::KeepAspectRatio,Qt::SmoothTransformation);
            this->setStyleSheet("QDialog{ background-color: #e28282; color: black; }"
                                "QDialog:hover { background-color: #e28282; color: black; }"
                                "QDialog:!hover { background-color: #e28282; color: black; }"
                                "QDialog:!enabled { background-color: #a25252; color: #C2C2D2; }");
            lMessageHeader->setText("Внимание!\r\nИмеются ошибки в работе модуля");
            break;
        }
        // Информация
        case 2:
        {
            pix.load(":/Information_1_512.png");
            pix = pix.scaled(80,80,Qt::KeepAspectRatio,Qt::SmoothTransformation);
            this->setStyleSheet("QDialog{ background-color: #b2b2f2; color: black; }"
                                "QDialog:hover { background-color: #b2b2f2; color: black; }"
                                "QDialog:!hover { background-color: #b2b2f2; color: black; }"
                                "QDialog:!enabled { background-color: #525282; color: #C2C2D2; }");
            lMessageHeader->setText("Информация");
            break;
        }
        // Результат расчета
        case 3:
        {
            pix.load(":/Calculation_1_512.png");
            pix = pix.scaled(80,80,Qt::KeepAspectRatio,Qt::SmoothTransformation);
            this->setStyleSheet("QDialog{ background-color: #b2b2b2; color: black; }"
                                "QDialog:hover { background-color: #b2b2b2; color: black; }"
                                "QDialog:!hover { background-color: #b2b2b2; color: black; }"
                                "QDialog:!enabled { background-color: #525252; color: #C2C2D2; }");
            lMessageHeader->setText("Расчетные данные");
            break;
        }
        // Детализация
        case 4:
        {
            pix.load(":/Detals_1_512.png");
            pix = pix.scaled(80,80,Qt::KeepAspectRatio,Qt::SmoothTransformation);
            this->setStyleSheet("QDialog{ background-color: #b5a3a8; color: black; }"
                                "QDialog:hover { background-color: #b5a3a8; color: black; }"
                                "QDialog:!hover { background-color: #b5a3a8; color: black; }"
                                "QDialog:!enabled { background-color: #675b5e; color: #C2C2D2; }");
            lMessageHeader->setText("Детализация информации");
            break;
        }
        // Проектирование
        case 5:
        {
            pix.load(":/Engineering_1_512.png");
            pix = pix.scaled(80,80,Qt::KeepAspectRatio,Qt::SmoothTransformation);
            this->setStyleSheet("QDialog{ background-color: #b9c6cb; color: black; }"
                                "QDialog:hover { background-color: #b9c6cb; color: black; }"
                                "QDialog:!hover { background-color: #b9c6cb; color: black; }"
                                "QDialog:!enabled { background-color: #79877c; color: #C2C2D2; }");
            lMessageHeader->setText("Информация о разработке");
            break;
        }
        // Результат
        case 6:
        {
            pix.load(":/Resullt_1_512.png");
            pix = pix.scaled(80,80,Qt::KeepAspectRatio,Qt::SmoothTransformation);
            this->setStyleSheet("QDialog{ background-color: #baa3a3; color: black; }"
                                "QDialog:hover { background-color: #baa3a3; color: black; }"
                                "QDialog:!hover { background-color: #baa3a3; color: black; }"
                                "QDialog:!enabled { background-color: #776767; color: #C2C2D2; }");
            lMessageHeader->setText("Итоговая информация");
            break;
        }
        // Значение не известно
        default:
        {
            pix.load(":/ParamInfo.png");
            pix = pix.scaled(80,80,Qt::KeepAspectRatio,Qt::SmoothTransformation);
            lMessageHeader->setText("Неопределенная информация");
            break;
        }
        }
        lImage->setPixmap(pix);
        // Кнопка принять настройки
        bAppent = new QPushButton("Продолжить", this);
        bAppent->setFont(font);
        bAppent->setStyleSheet("QPushButton { background-color: #D6B2F8; text-align: center; }");
        connect(bAppent, SIGNAL(clicked()), this, SLOT(onContinue()), Qt::QueuedConnection);
        QGridLayout *mlayout = new QGridLayout(this);
        mlayout->setMargin(4);
        mlayout->setSpacing(2);
        mlayout->setColumnStretch(1, 3);
        mlayout->addWidget(lImage,0,0,1,1,Qt::AlignVCenter | Qt::AlignLeft);
        mlayout->addWidget(lMessageHeader,0,1,1,1, Qt::AlignCenter);
        mlayout->addWidget(eMessage,1,0,1,2);
        mlayout->addWidget(bAppent,2,0,1,2);
    }
    void show()
    {
        setModal(true);
        setWindowTitle("Сообщение");
        setWindowIcon(QIcon(":/icon_1.png"));
        // Размер окна 1/2 от родительского
        if(parentWidget()!=NULL)
            setMinimumSize(parentWidget()->width()/1.7, parentWidget()->height()/ 1.7);
        this->QDialog::show();
        adjustSize();
        setFixedSize(this->width(), this->height());
        //this->setEnabled(true);
    }
    void SetAttribute()
    {
        QPixmap pix;
        switch (Attribute) {
        /*case 0:
        {
            this->setStyleSheet("QDialog{ background-color: #a2ddb0; color: black; }"
                                "QDialog:hover { background-color: #a2ddb0; color: black; }"
                                "QDialog:!hover { background-color: #a2ddb0; color: black; }"
                                "QDialog:!enabled { background-color: #647e6a; color: #C2C2D2; }");
            break;
        }
        // Внимание
        case 1:
        {
            this->setStyleSheet("QDialog{ background-color: #e28282; color: black; }"
                                "QDialog:hover { background-color: #e28282; color: black; }"
                                "QDialog:!hover { background-color: #e28282; color: black; }"
                                "QDialog:!enabled { background-color: #a25252; color: #C2C2D2; }");
            break;
        }
        // Информация
        case 2:
        {
            this->setStyleSheet("QDialog{ background-color: #b2b2f2; color: black; }"
                                "QDialog:hover { background-color: #b2b2f2; color: black; }"
                                "QDialog:!hover { background-color: #b2b2f2; color: black; }"
                                "QDialog:!enabled { background-color: #525282; color: #C2C2D2; }");
            break;
        }
        // Результат расчета
        case 3:
        {
            this->setStyleSheet("QDialog{ background-color: #b2b2b2; color: black; }"
                                "QDialog:hover { background-color: #b2b2b2; color: black; }"
                                "QDialog:!hover { background-color: #b2b2b2; color: black; }"
                                "QDialog:!enabled { background-color: #525252; color: #C2C2D2; }");
            break;
        }
        // Детализация
        case 4:
        {
            this->setStyleSheet("QDialog{ background-color: #b5a3a8; color: black; }"
                                "QDialog:hover { background-color: #b5a3a8; color: black; }"
                                "QDialog:!hover { background-color: #b5a3a8; color: black; }"
                                "QDialog:!enabled { background-color: #675b5e; color: #C2C2D2; }");
            break;
        }
        // Проектирование
        case 5:
        {
            this->setStyleSheet("QDialog{ background-color: #b9c6cb; color: black; }"
                                "QDialog:hover { background-color: #b9c6cb; color: black; }"
                                "QDialog:!hover { background-color: #b9c6cb; color: black; }"
                                "QDialog:!enabled { background-color: #79877c; color: #C2C2D2; }");
            break;
        }
        // Результат
        case 6:
        {
            this->setStyleSheet("QDialog{ background-color: #baa3a3; color: black; }"
                                "QDialog:hover { background-color: #baa3a3; color: black; }"
                                "QDialog:!hover { background-color: #baa3a3; color: black; }"
                                "QDialog:!enabled { background-color: #776767; color: #C2C2D2; }");
            break;
        }
        // Значение не известно
        default:
        {
            break;
        }
        }*/
        // Логотип НПК СКАРТ
        case 0:
        {
            pix.load(":/icon_1.png");
            pix = pix.scaled(80,80,Qt::KeepAspectRatio,Qt::SmoothTransformation);
            this->setStyleSheet("QDialog{ background-color: #a2ddb0; color: black; }"
                                "QDialog:hover { background-color: #a2ddb0; color: black; }"
                                "QDialog:!hover { background-color: #a2ddb0; color: black; }"
                                "QDialog:!enabled { background-color: #647e6a; color: #C2C2D2; }");
            lMessageHeader->setText("Информация о разработчике ООО \"НПК\"СКАРТ\"");
            break;
        }
        // Внимание
        case 1:
        {
            pix.load(":/Attention_1_512.png");
            pix = pix.scaled(80,80,Qt::KeepAspectRatio,Qt::SmoothTransformation);
            this->setStyleSheet("QDialog{ background-color: #e28282; color: black; }"
                                "QDialog:hover { background-color: #e28282; color: black; }"
                                "QDialog:!hover { background-color: #e28282; color: black; }"
                                "QDialog:!enabled { background-color: #a25252; color: #C2C2D2; }");
            lMessageHeader->setText("Внимание!\r\nИмеются ошибки в работе модуля");
            break;
        }
        // Информация
        case 2:
        {
            pix.load(":/Information_1_512.png");
            pix = pix.scaled(80,80,Qt::KeepAspectRatio,Qt::SmoothTransformation);
            this->setStyleSheet("QDialog{ background-color: #b2b2f2; color: black; }"
                                "QDialog:hover { background-color: #b2b2f2; color: black; }"
                                "QDialog:!hover { background-color: #b2b2f2; color: black; }"
                                "QDialog:!enabled { background-color: #525282; color: #C2C2D2; }");
            lMessageHeader->setText("Информация");
            break;
        }
        // Результат расчета
        case 3:
        {
            pix.load(":/Calculation_1_512.png");
            pix = pix.scaled(80,80,Qt::KeepAspectRatio,Qt::SmoothTransformation);
            this->setStyleSheet("QDialog{ background-color: #b2b2b2; color: black; }"
                                "QDialog:hover { background-color: #b2b2b2; color: black; }"
                                "QDialog:!hover { background-color: #b2b2b2; color: black; }"
                                "QDialog:!enabled { background-color: #525252; color: #C2C2D2; }");
            lMessageHeader->setText("Расчетные данные");
            break;
        }
        // Детализация
        case 4:
        {
            pix.load(":/Detals_1_512.png");
            pix = pix.scaled(80,80,Qt::KeepAspectRatio,Qt::SmoothTransformation);
            this->setStyleSheet("QDialog{ background-color: #b5a3a8; color: black; }"
                                "QDialog:hover { background-color: #b5a3a8; color: black; }"
                                "QDialog:!hover { background-color: #b5a3a8; color: black; }"
                                "QDialog:!enabled { background-color: #675b5e; color: #C2C2D2; }");
            lMessageHeader->setText("Детализация информации");
            break;
        }
        // Проектирование
        case 5:
        {
            pix.load(":/Engineering_1_512.png");
            pix = pix.scaled(80,80,Qt::KeepAspectRatio,Qt::SmoothTransformation);
            this->setStyleSheet("QDialog{ background-color: #b9c6cb; color: black; }"
                                "QDialog:hover { background-color: #b9c6cb; color: black; }"
                                "QDialog:!hover { background-color: #b9c6cb; color: black; }"
                                "QDialog:!enabled { background-color: #79877c; color: #C2C2D2; }");
            lMessageHeader->setText("Информация о разработке");
            break;
        }
        // Результат
        case 6:
        {
            pix.load(":/Resullt_1_512.png");
            pix = pix.scaled(80,80,Qt::KeepAspectRatio,Qt::SmoothTransformation);
            this->setStyleSheet("QDialog{ background-color: #baa3a3; color: black; }"
                                "QDialog:hover { background-color: #baa3a3; color: black; }"
                                "QDialog:!hover { background-color: #baa3a3; color: black; }"
                                "QDialog:!enabled { background-color: #776767; color: #C2C2D2; }");
            lMessageHeader->setText("Итоговая информация");
            break;
        }
        // Значение не известно
        default:
        {
            pix.load(":/ParamInfo.png");
            pix = pix.scaled(80,80,Qt::KeepAspectRatio,Qt::SmoothTransformation);
            lMessageHeader->setText("Неопределенная информация");
            break;
        }
        }
        lImage->setPixmap(pix);
    }
    QString MessageText;
    int fontsize;
    quint8 Attribute;
    QLabel *lMessageHeader, *lImage;
    QTextEdit *eMessage;
    QPushButton *bAppent;
    bool FirstClose;
public slots:
    void onTextInsert(QString txt, quint8 attribute=0)
    {
        eMessage->append(txt);
        if(attribute!=0)
            Attribute = attribute;
        SetAttribute();
    }
    void onChangeLogPosition()
    {
        QTextCursor c =  eMessage->textCursor();
        c.movePosition(QTextCursor::End);
        eMessage->setTextCursor(c);
    }
    void onContinue()
    {
        this->hide();
        this->setWindowModality(Qt::NonModal);
        this->QDialog::show();
        //this->setModal(false);
        //if(parentWidget()!=0)
        //    parentWidget()->setEnabled(true);
        /*QCloseEvent* e = new QCloseEvent();
        QApplication::sendEvent(this, e);*/
    }
protected:
    virtual void closeEvent(QCloseEvent* event)
    {
        event->accept();
        if(FirstClose)
        {
            FirstClose = false;
            //if(parentWidget()!=0)
            //    parentWidget()->setEnabled(true);
            this->close();
        }
    }
    virtual void keyPressEvent(QKeyEvent* key)
    {
        switch(key->key())
        {
        case Qt::Key_Escape:
        {
            key->accept();
            QCloseEvent* e = new QCloseEvent();
            QApplication::sendEvent(this, e);
            break;
        }
        default:
            break;
        }
    }
};

#endif // CINFODIALOG_H
