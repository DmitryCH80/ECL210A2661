#ifndef ECL210A2661CONFIGDLG_H
#define ECL210A2661CONFIGDLG_H
#include <QDialog>
#include <QLabel>
#include <QDebug>
#include <QComboBox>
#include <QLineEdit>
#include <QGridLayout>
#include <QPushButton>
#include <QCloseEvent>
#include <QKeyEvent>
#include <QApplication>
#include "ECL210A2661Config.h"

class ECL210A2661ConfigDlg : public QDialog
{
    Q_OBJECT
public:
    explicit ECL210A2661ConfigDlg(QWidget *pwgt, ECL210A2661Config *pconfig) : QDialog(pwgt, Qt::WindowTitleHint)
    {
        this->setAttribute(Qt::WA_DeleteOnClose, true);
        pConfig = pconfig;
        fontsize = 10;
        FirstClose = true;
        QFont font("Times", fontsize);
        font.setBold(true);
        // Виджет родителя
        pParent = this->parentWidget();
        // Лэйблы
        lPort      = new QLabel("Порт №(1..255)",this);
        lPortSpeed = new QLabel("Настройка скорости", this);
        lEqDelay   = new QLabel("Ожидание (100..60000мс)", this);
        lPort->setFont(font);
        lPortSpeed->setFont(font);
        lEqDelay->setFont(font);
        lPort->setAlignment(Qt::AlignCenter);
        lPortSpeed->setAlignment(Qt::AlignCenter);
        lEqDelay->setAlignment(Qt::AlignCenter);
        // Поля ввода
        ePort = new QLineEdit(this);
        ePort->setFont(font);
        ePort->setMaxLength(3);
        ePort->setStyleSheet("QLineEdit{ border: 1px solid black; background-color: #DACAFC; color: black; text-align: center; }"
                             "QLineEdit:hover { border: 1px solid black; background: #DACAFC; color: black; text-align: center; }"
                             "QLineEdit:!hover { border: 1px solid black; background: #DACAFC; color: black; text-align: center; }"
                             "QLineEdit:!enabled { border: 1px solid gray; background: #CACACA; color: #8d8d8d; text-align: center; }");
        ePort->setText(QString::number(pConfig->ComNumber));
        eEqDelay = new QLineEdit(this);
        eEqDelay->setFont(font);
        eEqDelay->setMaxLength(5);
        eEqDelay->setStyleSheet("QLineEdit{ border: 1px solid black; background-color: #DACAFC; color: black; text-align: center; }"
                                "QLineEdit:hover { border: 1px solid black; background: #DACAFC; color: black; text-align: center; }"
                                "QLineEdit:!hover { border: 1px solid black; background: #DACAFC; color: black; text-align: center; }"
                                "QLineEdit:!enabled { border: 1px solid gray; background: #CACACA; color: #8d8d8d; text-align: center; }"
                                );
        eEqDelay->setText(QString::number(pConfig->EqDelay));
        // Перечень настроек скорости
        QStringList lsp = (QStringList() << "9600" << "19200" << "38400");
        lcbxPortSpeed = new QComboBox(this);
        lcbxPortSpeed->setFont(font);
        lcbxPortSpeed->setStyleSheet("QComboBox { background-color: #D6B2F8; text-align: center; }");
        lcbxPortSpeed->addItems(lsp);
        lcbxPortSpeed->setCurrentText(QString::number(pConfig->PortSpeed));
        // Кнопка принять настройки
        bAppent = new QPushButton("Продолжить", this);
        bAppent->setFont(font);
        bAppent->setStyleSheet("QPushButton { background-color: #D6B2F8; text-align: center; }");
        connect(bAppent, SIGNAL(clicked()), this, SLOT(onTestConfig()), Qt::QueuedConnection);
        QGridLayout *mlayout = new QGridLayout(this);
        mlayout->setMargin(4);
        mlayout->setSpacing(2);
        mlayout->addWidget(lPort,0,0,1,1);
        mlayout->addWidget(ePort,0,1,1,1);
        mlayout->addWidget(lPortSpeed,1,0,1,1);
        mlayout->addWidget(lcbxPortSpeed,1,1,1,1);
        mlayout->addWidget(lEqDelay,2,0,1,1);
        mlayout->addWidget(eEqDelay,2,1,1,1);
        mlayout->addWidget(bAppent,3,0,1,2);
        if(pParent!=0)
            pParent->setEnabled(false);
        this->setStyleSheet("QDialog{ background-color: #8282b2; color: black; }"
                            "QDialog:hover { background-color: #8282b2; color: black; }"
                            "QDialog:!hover { background-color: #8282b2; color: black; }"
                            "QDialog:!enabled { background-color: #525282; color: #C2C2D2; }");
    }
    int fontsize;
    QLabel *lPort, *lPortSpeed, *lEqDelay;
    QLineEdit *ePort, *eEqDelay;
    QComboBox *lcbxPortSpeed;
    ECL210A2661Config *pConfig;
    QPushButton *bAppent;
    bool FirstClose;
    QWidget *pParent;
public slots:
    void onTestConfig()
    {
        bool IsOk, IsOkk;
        QString p = ePort->text(), d = eEqDelay->text();
        quint32 ip = p.toUInt(&IsOk);
        quint32 id = d.toUInt(&IsOkk);
        if((ip <1)|(ip >255))
        {
            qDebug() << " > ECL210A2661ConfigDlg::onTestConfig: Ошибка номера порта";
            ePort->setText(QString::number(pConfig->ComNumber));
            ePort->setStyleSheet("QLineEdit{ border: 1px solid black; background-color: #ff7e73; color: black; }"
                                 "QLineEdit:hover { border: 1px solid black; background: #ff7e73; color: black; }"
                                 "QLineEdit:!hover { border: 1px solid black; background: #ff7e73; color: black; }"
                                 "QLineEdit:!enabled { border: 1px solid gray; background: #EfCeC3; color: #8d8d8d; }");
            return;
        }
        else
        {
            ePort->setStyleSheet("QLineEdit{ border: 1px solid black; background-color: #a2eea6; color: black; }"
                                 "QLineEdit:hover { border: 1px solid black; background: #a2eea6; color: black; }"
                                 "QLineEdit:!hover { border: 1px solid black; background: #a2eea6; color: black; }"
                                 "QLineEdit:!enabled { border: 1px solid gray; background: #CACACA; color: #8d8d8d; }");
            pConfig->ComNumber = 0 +ip;
            qDebug() << " > ECL210A2661ConfigDlg::onTestConfig: порт №:" << QString::number(pConfig->ComNumber);
        }
        if((id <100)|(id >60000))
        {
            qDebug() << " > ECL210A2661ConfigDlg::onTestConfig: Ошибка времени ожидания ответа от оборудования";
            eEqDelay->setText(QString::number(pConfig->EqDelay));
            eEqDelay->setStyleSheet("QLineEdit{ border: 1px solid black; background-color: #ff7e73; color: black; }"
                                 "QLineEdit:hover { border: 1px solid black; background: #ff7e73; color: black; }"
                                 "QLineEdit:!hover { border: 1px solid black; background: #ff7e73; color: black; }"
                                 "QLineEdit:!enabled { border: 1px solid gray; background: #EfCeC3; color: #8d8d8d; }");
            return;
        }
        else
        {
            eEqDelay->setStyleSheet("QLineEdit{ border: 1px solid black; background-color: #a2eea6; color: black; }"
                                 "QLineEdit:hover { border: 1px solid black; background: #a2eea6; color: black; }"
                                 "QLineEdit:!hover { border: 1px solid black; background: #a2eea6; color: black; }"
                                 "QLineEdit:!enabled { border: 1px solid gray; background: #CACACA; color: #8d8d8d; }");
            pConfig->EqDelay = id;
            qDebug() << " > ECL210A2661ConfigDlg::onTestConfig: ожидание:" << QString::number(pConfig->EqDelay);
        }
        quint32 ps = lcbxPortSpeed->currentText().toUInt(&IsOk);
        if(((ps!=9600)&(ps!=19200)&(ps!=38400))|(!IsOk))
        {
            qDebug() << " > ECL210A2661ConfigDlg::onTestConfig: Ошибка скорости интерфейса";
            lcbxPortSpeed->setStyleSheet("QComboBox{ border: 1px solid black; background-color: #ff7e73; color: black; }"
                                         "QComboBox:hover { border: 1px solid black; background: #ff7e73; color: black; }"
                                         "QComboBox:!hover { border: 1px solid black; background: #ff7e73; color: black; }"
                                         "QComboBox:!enabled { border: 1px solid gray; background: #EfCeC3; color: #8d8d8d; }");
            return;
        }
        else
        {
            lcbxPortSpeed->setStyleSheet("QComboBox{ border: 1px solid black; background-color: #a2eea6; color: black; }"
                                         "QComboBox:hover { border: 1px solid black; background: #a2eea6; color: black; }"
                                         "QComboBox:!hover { border: 1px solid black; background: #a2eea6; color: black; }"
                                         "QComboBox:!enabled { border: 1px solid gray; background: #CACACA; color: #8d8d8d; }");
            pConfig->PortSpeed = ps;
            qDebug() << " > ECL210A2661ConfigDlg::onTestConfig: скорость:" << QString::number(pConfig->PortSpeed);
        }
        if(!pConfig->SaveConfig())
            qDebug() << " > ECL210A2661ConfigDlg::onTestConfig: Настройка не сохранена. Ошибка.";
        else
            qDebug() << " > ECL210A2661ConfigDlg::onTestConfig: Настройка сохранена";
        qDebug() << " > ECL210A2661ConfigDlg::onTestConfig: Закрытие настройки";
        emit sendConnectionSetupFinished();
        QCloseEvent* e = new QCloseEvent();
        QApplication::sendEvent(this, e);
    }
signals:
    void sendConnectionSetupFinished();
protected:
    virtual void closeEvent(QCloseEvent* event)
    {
        qDebug() << " > ECL210A2661ConfigDlg::closeEvent: Закрытие окна настройки соединения";
        event->accept();
        if(FirstClose)
        {
            FirstClose = false;
            if(pParent!=0)
                pParent->setEnabled(true);
            this->close();
        }
    }
    virtual void keyPressEvent(QKeyEvent* key)
    {
        switch(key->key())
        {
        case Qt::Key_Escape:
        {
            qDebug() << " > ECL210A2661ConfigDlg::keyPressEvent: Игнорирование нажатия клавиши ESC";
            key->ignore();
            return;
        }
        default:
            break;
        }
        key->accept();
    }
};
#endif // ECL210A2661CONFIGDLG_H
