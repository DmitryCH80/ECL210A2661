#include "ECL210A2661.h"
// -------------------------------------------------------------------------------------------------------
ECL210A2661 *pEqControl;
float DriverVer = 1.015;

// Весь цикл работы наладки прибора происходит в данной функции
QDialog* Show(QWidget* pwgt, Qt::WindowFlags f, quint32 objectid, QSqlDatabase *base, QString userlogin)
{
    bool IsOk = false;
    pEqControl = new ECL210A2661(pwgt, f, objectid, base, userlogin, IsOk);
    if(IsOk)
    {
        pEqControl->setModal(true);
        pEqControl->setWindowTitle("Настройка погодного регулятора ECL210 Comfort A266.1");
        pEqControl->setWindowIcon(QIcon(":/icon_1.png"));
        pEqControl->show();
        pEqControl->adjustSize();
        pEqControl->setFixedSize(pEqControl->width(), pEqControl->height());
        return pEqControl;
    }
    return NULL;
}
// 03.06.2017 Получение информации о драйвере
QString GetInfoDll(QString &data)
{
    QString info = "Драйвер настройки погодного регулятора ECL-210 Comfort с ключем A266.1\r\n"
                   "Версия драйвера " + QString::number(DriverVer, 'f', 3) + "\r\n"
                   "Разработчик ООО \"НПК\"СКАРТ\"\r\n"
                   "Дата 26.09.2018\r\n"
                   "Драйвер предназначен для настройки регулятора";
    data = "наладка";
    return info;
}
// -------------------------------------------------------------------------------------------------------
ECL210A2661::ECL210A2661(QWidget* pwgt, Qt::WindowFlags f, quint32 objectid, QSqlDatabase *base, QString userlogin, bool &IsOk) : QDialog(pwgt, f)
{
    IsRead =false;
    InProcessing = false;
    CurrentSystem ="Heat";
    InWork = NULL;
    EqNumber = 0;          // Изначально номер не задан
    ObjectID = objectid;
    FirstClose = true;
    CheckTimeout =false;
    InChange     =false;
    this->setAttribute(Qt::WA_DeleteOnClose, true);
    this->setStyleSheet("QDialog{ border-style: inset; border-width: 4px; border-color: #595959; background-color: #C28282; color: #404040; }"
                        "QDialog:hover { border-style: inset; border-width: 4px; border-color: #595959; background-color: #C28282; color: #000000; }"
                        "QDialog:!hover { border-style: inset; border-width: 4px; border-color: #595959; background-color: #C28282; color: #404040; }"
                        "QDialog:!enabled { border-style: inset; border-width: 4px; border-color: #262626; background-color: #823030; color: #C2B2B2; }");
    db = base;
    mQuery = new QSqlQuery(*db);
    UserString = userlogin;
    CurrentUserCommand = "";
    CurrentWorkMode = "";
    ChangeWorkMode = false;
    CurrentTimer =0;
    connect(this, SIGNAL(sendFullBlock()), this, SLOT(onFullBlock()), Qt::QueuedConnection);
    connect(this, SIGNAL(sendSetColorToEdits(quint32,quint32,quint8)), this, SLOT(onSetColorToEdits(quint32,quint32,quint8)), Qt::QueuedConnection);
    FontSize =10;
    QFont font("Times", FontSize);
    QFont bfont("Times", FontSize+2);
    font.setBold(true);
    bfont.setBold(true);
    // Главный вывод
    lGeneralText = new QLabel("", this);
    lGeneralText->setStyleSheet("QLabel { border: 2px solid black; background-color: #ff7e73; color: black; }"
                                "QLabel:!enabled { border: 2px solid gray; background-color: #dfbfbf; color: #9c9c9c; }");
    lGeneralText->setFont(font);
    lGeneralText->setText("<div align = center><b><font size =\"5\" >Настройка заблокирована!</font></b></div>"
         "<div align = left><b><font size =\"2\" >"
         "1. Для начала работы переведите сервер в режим наладки<br>"
         "2. При наладке оборудования не выключайте компьютер<br>"
         "3. После выполнения процедуры наладки обязательно восстановите режим сервера"
         "</font></b></div>");
    // Лэйлы основных кнопок
    lServerMode = new QLabel("", this);        // Текст над кнопкой смены режима сервера
    lServerMode->setFixedSize(47,20);
    lServerMode->setFont(font);
    lServerMode->setAlignment(Qt::AlignCenter);
    lServerMode->setText("Режим");
    lCurrentSystem = new QLabel("", this);     // Текст над кнопкой выбора контура
    lCurrentSystem->setFixedSize(47,20);
    lCurrentSystem->setFont(font);
    lCurrentSystem->setAlignment(Qt::AlignCenter);
    lCurrentSystem->setText("Контур");
    //lReadAll = new QLabel("", this);   // Текст над кнопкой "прочитать все настройки"
    //lReadAll->setFixedSize(47,20);
    //lReadAll->setFont(font);
    //lReadAll->setAlignment(Qt::AlignCenter);
    //lReadAll->setText("Чит.все");
    // Перечень сетевых номеров
    lcbx = new QComboBox(this);
    lcbx->setFont(font);
    lcbx->setEnabled(true);
    lcbx->setStyleSheet("background-color: #bfdbbf; text-align: center");
    // Основные кнопки
    bServerMode = new QPushButton(QIcon(":/ServerInWork.png"),"", this); // Кнопка смены режима сервера
    bCurrentSystem= new QPushButton(QIcon(":/HeatSystem.png"),"", this); // Кнопка выбора контура
//    bReadAll= new QPushButton(QIcon(":/ReadAllConfig.png"),"", this);    // Кнопка "прочитать все настройки"
    bConnectionSetup = new QPushButton(QIcon(":/ConnectionSetup.png"),
                                       "", this);                        // Кнопка "настройки соединения"
    bServerMode->setFixedSize(47,47);
    bCurrentSystem->setFixedSize(47,47);
//    bReadAll->setFixedSize(47,47);
    bConnectionSetup->setFixedSize(47,47);
    bServerMode->setIconSize(QSize(44,44));
    bCurrentSystem->setIconSize(QSize(44,44));
//    bReadAll->setIconSize(QSize(44,44));
    bConnectionSetup->setIconSize(QSize(44,44));
    bServerMode->setToolTip("Изменить режим работы сервера (работа/наладка");
    bCurrentSystem->setToolTip("Настраиваемый контур");
//    bReadAll->setToolTip("Загрузить все настройки контроллера");
    bConnectionSetup->setToolTip("Настройки соединения с регулятором");
    bCurrentSystem->setEnabled(false);
//    bReadAll->setEnabled(false);
    bServerMode->setEnabled(false);
    bConnectionSetup->setEnabled(false);
    bServerMode->setStyleSheet("QPushButton{ \
              background-color: qradialgradient(cx:0.5, cy:0.5, radius: 0.7, fx:0.5, fy:0.5, \
              stop:0 #DFDFFF, stop:1 #A6A6B8 ); \
              border-style: outset; border-width: 2px; border-color: #5A5A5A; text-align: center; \
              color: #404040; } \
              QPushButton:pressed { \
              background-color: qradialgradient(cx:0.5, cy:0.5, radius: 0.7, fx:0.5, fy:0.5, \
              stop:0 #A6A6B8 , stop:1 #DBDBFF ); \
              border-style: inset; border-width: 2px; border-color: #383838; text-align: center;\
              color: #404040; } \
              QPushButton:hover:!pressed { \
              background-color: qradialgradient(cx:0.5, cy:0.5, radius: 0.7, fx:0.5, fy:0.5, \
              stop:0 #DFDFFF, stop:1 #9090A2 ); \
              border-style: outset; border-width: 2px; border-color: #5A5A5A; text-align: center; \
              color: #404040; }\
              QPushButton:!enabled { \
              background-color: qradialgradient(cx:0.5, cy:0.5, radius: 0.7, fx:0.5, fy:0.5, \
              stop:0 #82829A, stop:1 #686889 ); \
              border-style: outset; border-width: 2px; border-color: #383838; text-align: center;\
              color: #404040; }");
/*   bReadAll->setStyleSheet("QPushButton{ \
              background-color: qradialgradient(cx:0.5, cy:0.5, radius: 0.7, fx:0.5, fy:0.5, \
              stop:0 #DFDFFF, stop:1 #A6A6B8 ); \
              border-style: outset; border-width: 2px; border-color: #5A5A5A; text-align: center; \
              } \
              QPushButton:pressed { \
              background-color: qradialgradient(cx:0.5, cy:0.5, radius: 0.7, fx:0.5, fy:0.5, \
              stop:0 #A6A6B8 , stop:1 #DBDBFF ); \
              border-style: inset; border-width: 2px; border-color: #383838; text-align: center;\
              } \
              QPushButton:hover:!pressed { \
              background-color: qradialgradient(cx:0.5, cy:0.5, radius: 0.7, fx:0.5, fy:0.5, \
              stop:0 #DFDFFF, stop:1 #9090A2 ); \
              border-style: outset; border-width: 2px; border-color: #5A5A5A; text-align: center; \
              }\
              QPushButton:!enabled { \
              background-color: qradialgradient(cx:0.5, cy:0.5, radius: 0.7, fx:0.5, fy:0.5, \
              stop:0 #82829A, stop:1 #686889 ); \
              border-style: outset; border-width: 2px; border-color: #383838; text-align: center;\
              }");*/
   bCurrentSystem->setStyleSheet("QPushButton{ \
              background-color: qradialgradient(cx:0.5, cy:0.5, radius: 0.7, fx:0.5, fy:0.5, \
              stop:0 #DFDFFF, stop:1 #A6A6B8 ); \
              border-style: outset; border-width: 2px; border-color: #5A5A5A; text-align: center; \
              } \
              QPushButton:pressed { \
              background-color: qradialgradient(cx:0.5, cy:0.5, radius: 0.7, fx:0.5, fy:0.5, \
              stop:0 #A6A6B8 , stop:1 #DBDBFF ); \
              border-style: inset; border-width: 2px; border-color: #383838; text-align: center;\
              } \
              QPushButton:hover:!pressed { \
              background-color: qradialgradient(cx:0.5, cy:0.5, radius: 0.7, fx:0.5, fy:0.5, \
              stop:0 #DFDFFF, stop:1 #9090A2 ); \
              border-style: outset; border-width: 2px; border-color: #5A5A5A; text-align: center; \
              }\
              QPushButton:!enabled { \
              background-color: qradialgradient(cx:0.5, cy:0.5, radius: 0.7, fx:0.5, fy:0.5, \
              stop:0 #82829A, stop:1 #686889 ); \
              border-style: outset; border-width: 2px; border-color: #383838; text-align: center;\
              }");
    bConnectionSetup->setStyleSheet("QPushButton{ \
              background-color: qradialgradient(cx:0.5, cy:0.5, radius: 0.7, fx:0.5, fy:0.5, \
              stop:0 #DFDFFF, stop:1 #A6A6B8 ); \
              border-style: outset; border-width: 2px; border-color: #5A5A5A; text-align: center; \
              color: #404040; } \
              QPushButton:pressed { \
              background-color: qradialgradient(cx:0.5, cy:0.5, radius: 0.7, fx:0.5, fy:0.5, \
              stop:0 #A6A6B8 , stop:1 #DBDBFF ); \
              border-style: inset; border-width: 2px; border-color: #383838; text-align: center;\
              color: #404040; } \
              QPushButton:hover:!pressed { \
              background-color: qradialgradient(cx:0.5, cy:0.5, radius: 0.7, fx:0.5, fy:0.5, \
              stop:0 #DFDFFF, stop:1 #9090A2 ); \
              border-style: outset; border-width: 2px; border-color: #5A5A5A; text-align: center; \
              color: #404040; }\
              QPushButton:!enabled { \
              background-color: qradialgradient(cx:0.5, cy:0.5, radius: 0.7, fx:0.5, fy:0.5, \
              stop:0 #82829A, stop:1 #686889 ); \
              border-style: outset; border-width: 2px; border-color: #383838; text-align: center;\
              color: #404040; }");
    QStringList txt;
    txt << "<div align = center>Параметры</div>"
        << "<div align = center>Подробное описание параметра</div>"
        << "<div align = center>Вывод приложения</div>";
    for(int i=0; i <4; i++)
    {
        lMiniHeader[i] = new QLabel("", this);
        if(i==3)
        {
            lMiniHeader[i]->setStyleSheet("QLabel { border-top: 2px solid black; border-bottom: 2px solid black; "
                                          "border-left: 2px solid black; border-right: none; background-color: #a2eea6 }"
                                          "QLabel:!enabled { border-top: 2px solid gray; border-bottom: 2px solid gray; "
                                          "border-left: 2px solid gray; border-right: none; background-color: #bfdbbf }");
            lMiniHeader[i]->setPixmap(QPixmap(":/Heat.png").scaled(18,12,Qt::KeepAspectRatio,Qt::SmoothTransformation));
            lMiniHeader[i]->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
            break;
        }
        lMiniHeader[i]->setStyleSheet("QLabel { border: 2px solid black; background-color: #a2eea6 }"
                                      "QLabel:!enabled { border: 2px solid gray; background-color: #bfdbbf }");
        lMiniHeader[i]->setFont(font);
        if(i==0)
        {
            lMiniHeader[i]->setTextFormat(Qt::RichText);
            //lMiniHeader[i]->setPixmap(QPixmap(":/icon_1.png").scaled(18,18,Qt::KeepAspectRatio,Qt::SmoothTransformation));
        }
        lMiniHeader[i]->setAlignment(Qt::AlignLeft);
        lMiniHeader[i]->setText(txt[i]);
    }
    // Кнопки очистки лога и сохранения в файл
    bSaveLog = new QPushButton(QIcon(":/Save_1.png"),"", this);
    bClearLog = new QPushButton(QIcon(":/broom.png"),"", this);
    bSaveLog->setFixedSize(20,20);
    bClearLog->setFixedSize(20,20);
    bSaveLog->setIconSize(QSize(16,16));
    bClearLog->setIconSize(QSize(16,16));
    bSaveLog->setToolTip("Сохранить вывод в файл");
    bClearLog->setToolTip("Очистить вывод");
    connect(bClearLog, SIGNAL(clicked()), this, SLOT(onClearLog()), Qt::QueuedConnection);
    connect(bSaveLog, SIGNAL(clicked()), this, SLOT(onSaveLog()), Qt::QueuedConnection);
    // Поля вывода лога и информации о параметрах
    teLog = new QTextEdit(this);
    teLog->setReadOnly(true);
 //   teLog->setMinimumHeight(154);
 //   teLog->setMaximumWidth(192);
    teLog->clear();
    teParameterInfo = new QTextEdit(this);
    teParameterInfo->setReadOnly(true);
 //   teParameterInfo->setMinimumHeight(154);
    teParameterInfo->clear();
    teLog->setStyleSheet("QTextEdit { border-style: inset; border-width: 3px; "
                         "border-color: #8A8A8A; background-color: #b2b2f2; color: black; }"
                         "QTextEdit:!enabled { border-style: inset; border-width: 3px; "
                         "border-color: #8A8A8A; background-color: #b8b8b8; color: #606060; }");
    teLog->verticalScrollBar()->setStyleSheet("QScrollBar { background-color: #bEbEcE; }"
                                                        "QScrollBar:!enabled { background-color: #bEbEcE; }");
    teLog->horizontalScrollBar()->setStyleSheet("QScrollBar { background-color: #bEbEcE; }"
                                              "QScrollBar:!enabled { background-color: #bEbEcE; }");
    teParameterInfo->setStyleSheet("QTextEdit { border-style: inset; border-width: 3px; "
                                   "border-color: #8A8A8A; background-color: #b2b2f2; color: black; }"
                         "QTextEdit:!enabled { border-style: inset; border-width: 3px; "
                                   "border-color: #8A8A8A; background-color: #b8b8b8; color: #606060; }");
    teParameterInfo->verticalScrollBar()->setStyleSheet("QScrollBar { background-color: #bEbEcE; }"
                                                        "QScrollBar:!enabled { background-color: #bEbEcE; }");
    teParameterInfo->horizontalScrollBar()->setStyleSheet("QScrollBar { background-color: #bEbEcE; }"
                                              "QScrollBar:!enabled { background-color: #bEbEcE; }");
    myScrollBox = new QScrollArea(this);
    myScrollBox->setObjectName("myScrollBox");
    myScrollBox->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    myScrollBox->setStyleSheet("QScrollArea { border-style: inset; border-width: 3px; "
                               "border-color: #8A8A8A; background-color: #b2b2f2; color: black; }"
                               "QScrollArea:!enabled { border-style: inset; border-width: 3px; "
                               "border-color: #8A8A8A; background-color: #b8b8b8; color: #606060; }");
    myScrollBox->verticalScrollBar()->setStyleSheet("QScrollBar { background-color: #bEbEcE; }"
                                                    "QScrollBar:!enabled { background-color: #bEbEcE; }");
    myScrollBox->horizontalScrollBar()->setStyleSheet("QScrollBar { background-color: #bEbEcE; }"
                                              "QScrollBar:!enabled { background-color: #bEbEcE; }");
    QVBoxLayout* mLOut = new QVBoxLayout();
    /**/mLOut->setSpacing(0);
    /**/mLOut->setMargin(0);
    // Создание элементов выбора колонок отчета
    int currentWidth1, lWidth1,
        currentWidth2, lWidth2,
        currentWidth3, lWidth3;
    currentWidth1= lWidth1= currentWidth2= lWidth2= currentWidth3= lWidth3=0;
    QFontMetrics metrics(font);
    QGridLayout *miniLayout[35];
    for(int i=0; i < 35; i++)
    {
         minicontainer[i] = new QWidget(this);
         miniLayout[i] = new QGridLayout;
         miniLayout[i]->setSpacing(2);
         miniLayout[i]->setMargin(1);
        // Лейблы
        lParamHeader[i] = new QLabel(this);
        lParamHeader[i]->setFont(font);
        lParamHeader[i]->setAlignment(Qt::AlignCenter);
        lParamHeader[i]->setStyleSheet("QLabel { border: 1px solid black; background-color: #b1b1b1 }"
                                       "QLabel:!enabled { border: 1px solid gray; background-color: #b1b1b1 }");
        lParamDBName[i] = new QLabel(this);
        lParamDBName[i]->setFont(font);
        lParamDBName[i]->setAlignment(Qt::AlignCenter);
        lParamDBName[i]->setStyleSheet("QLabel { border: 1px solid black; background-color: #CACACA }"
                                       "QLabel:!enabled { border: 1px solid gray; background-color: #CACACA }");
        lParamRange[i] = new QLabel(this);
        lParamRange[i]->setFont(font);
        lParamRange[i]->setAlignment(Qt::AlignCenter);
        lParamRange[i]->setStyleSheet("QLabel { border: 1px solid black; background-color: #CACACA }"
                                      "QLabel:!enabled { border: 1px solid gray; background-color: #CACACA }");
        // Если параметр температурного графика
        if(i==19)
        {
            int tout[6] ={-30,-15,-5,0,5,15};
            // Заголовок колонки №
            lParamGraphNumberHeader= new QLabel(this);
            lParamGraphNumberHeader->setFont(font);
            lParamGraphNumberHeader->setAlignment(Qt::AlignCenter);
            lParamGraphNumberHeader->setStyleSheet("QLabel { border: 1px solid black; background-color: #CACACA }"
                                                   "QLabel:!enabled { border: 1px solid gray; background-color: #CACACA }");
            lParamGraphNumberHeader->setText("№");
            lParamGraphNumberHeader->setMaximumWidth(metrics.width(lParamGraphNumberHeader->text())+15);
            for(int j=0; j<6; j++)
            {
                lParamGraphNumber[j]= new QLabel(this);
                lParamGraphNumber[j]->setFont(font);
                lParamGraphNumber[j]->setAlignment(Qt::AlignCenter);
                lParamGraphNumber[j]->setStyleSheet("QLabel { border: 1px solid black; background-color: #CACACA }"
                                                    "QLabel:!enabled { border: 1px solid gray; background-color: #CACACA }");
                lParamGraphNumber[j]->setText(QString::number(j+1));
                lParamGraphNumber[j]->setMaximumWidth(metrics.width(lParamGraphNumber[j]->text())+15);
            }
            // Заголовок колонки Тнв
            lParamGraphToutHeader  = new QLabel(this);
            lParamGraphToutHeader->setFont(font);
            lParamGraphToutHeader->setAlignment(Qt::AlignCenter);
            lParamGraphToutHeader->setStyleSheet("QLabel { border: 1px solid black; background-color: #CACACA }"
                                                 "QLabel:!enabled { border: 1px solid gray; background-color: #CACACA }");
            lParamGraphToutHeader->setText("Тнв");
            lParamGraphToutHeader->setMaximumWidth(metrics.width(lParamGraphToutHeader->text())+15);
            for(int j=0; j<6; j++)
            {
                lParamGraphTout[j]= new QLabel(this);
                lParamGraphTout[j]->setFont(font);
                lParamGraphTout[j]->setAlignment(Qt::AlignCenter);
                lParamGraphTout[j]->setStyleSheet("QLabel { border: 1px solid black; background-color: #CACACA }"
                                                  "QLabel:!enabled { border: 1px solid gray; background-color: #CACACA }");
                lParamGraphTout[j]->setText(QString::number(tout[j]));
                lParamGraphTout[j]->setMaximumWidth(metrics.width(lParamGraphToutHeader->text())+15);
            }
            // Поле вывода текущих координат редактируемого параметра
            lParamGraphCurrentPoint= new QLabel(this);
            lParamGraphCurrentPoint->setFont(font);
            lParamGraphCurrentPoint->setAlignment(Qt::AlignCenter);
            lParamGraphCurrentPoint->setStyleSheet("QLabel { border: 1px solid black; background-color: #CACACA }"
                                                   "QLabel:!enabled { border: 1px solid gray; background-color: #CACACA }");
            lParamGraphCurrentPoint->setText("K1_Txпод_xxx(xxxx,xxx)");
            for(int j=0; j<6; j++)
            {
                eParamGraph[j] = new CMEdit(i, this);
                eParamGraph[j]->setText("----");
                eParamGraph[j]->aIndex = j+1;
                eParamGraph[j]->setFont(font);
                eParamGraph[j]->setAlignment(Qt::AlignCenter);
            //    eParamGraph[j]->setStyleSheet("QLineEdit{ border: 1px solid black; background-color: #a2eea6; color: black; }"
            //                                  "QLineEdit:hover { border: 1px solid black; background-color: #a2eea6; color: black; }"
            //                                  "QLineEdit:!hover { border: 1px solid black; background-color: #a2eea6; color: black; }"
            //                                  "QLineEdit:!enabled { border: 1px solid gray; background-color: #CACACA; color: #8d8d8d; }");
                eParamGraph[j]->setMaxLength(3);
                eParamGraph[j]->setEnabled(false);
                connect(eParamGraph[j], SIGNAL(sendADataChange(quint32,quint32)),
                        this, SLOT(onADataTest(quint32, quint32))/*, Qt::QueuedConnection*/);
            }
            wTempGraph = new CTempGraph(this);
            wTempGraph->setEnabled(false);
            for(int j=0; j <6; j++)
            {
                connect(wTempGraph->lParamGraphPoints[j], SIGNAL(PointTemperature(float,quint32)),
                        this, SLOT(onGraphCurrentPointPrint(float,quint32))/*, Qt::QueuedConnection*/);
            }
            connect(this, SIGNAL(sendTemperatureBorder(QString,quint32)),
                    wTempGraph, SLOT(onTempBorderChange(QString, quint32))/*, Qt::QueuedConnection*/);
            connect(this, SIGNAL(sendTemperature(QString,quint32)),
                    wTempGraph, SLOT(onTempChange(QString, quint32))/*, Qt::QueuedConnection*/);
        }
        else
        {
            eParamValue[i] = new CMEdit(i,this);
            eParamValue[i]->setText("----");
            eParamValue[i]->setFont(font);
            eParamValue[i]->setAlignment(Qt::AlignCenter);
        //    eParamValue[i]->setStyleSheet("QLineEdit{ border: 1px solid black; background-color: #a2eea6; color: black; }"
        //                                  "QLineEdit:hover { border: 1px solid black; background-color: #a2eea6; color: black; }"
        //                                  "QLineEdit:!hover { border: 1px solid black; background-color: #a2eea6; color: black; }"
        //                                  "QLineEdit:!enabled { border: 1px solid gray; background-color: #CACACA; color: #8d8d8d; }");
            eParamValue[i]->setMaxLength(4);
            eParamValue[i]->setEnabled(false);
            connect(eParamValue[i], SIGNAL(sendDataChange(quint32)), this, SLOT(onDataTest(quint32)), Qt::QueuedConnection);
        }
        SetLabelText(i);
        currentWidth1 = metrics.width(lParamHeader[i]->text());
        currentWidth2 = metrics.width(lParamDBName[i]->text());
        currentWidth3 = metrics.width(lParamRange[i]->text());
        if(lWidth1 < currentWidth1)
            lWidth1 = currentWidth1;
        if(lWidth2 < currentWidth2)
            lWidth2 = currentWidth2;
        if(lWidth3 < currentWidth3)
            lWidth3 = currentWidth3;
        // Кнопки параметров Прочитать/Записать/Справка
        bParamButton[i][0] = new CMButton(i,0, QIcon(":/Read_1.png"), this);
        bParamButton[i][0]->setIconSize(QSize(16,16));
        bParamButton[i][0]->setFixedSize(18,18);
        bParamButton[i][0]->setToolTip("Считать параметр из регулятора");
        bParamButton[i][0]->setEnabled(false);
        bParamButton[i][0]->setStyleSheet("QPushButton{ \
        border-style: outset; border-width: 2px; border-color: #8A8A8A; text-align: center; \
        } \
        QPushButton:pressed { \
        background-color: qlineargradient(x1 : 0, y1 : 0, x2 : 0, y2 :   1, stop :   0.0 #F5F5F5,\
        stop :   0.5 #D2D2D2, stop :   0.5 #D2D2D2, stop :   1.0 #CFB2B2); \
        border-style: inset; border-width: 2px; border-color: #8A8A8A; text-align: center;\
        } \
        QPushButton:hover:!pressed { \
        background-color: qlineargradient(x1 : 0, y1 : 0, x2 : 0, y2 :   1, stop :   0.0 #F5F5F5, \
        stop :   0.5 #EECDCD, stop :   0.55 #EECDCD, stop :   1.0 #F5F5F5); \
        border-style: outset; border-width: 2px; border-color: #8A8A8A; text-align: center; \
        }");
        connect(bParamButton[i][0], SIGNAL(sendChange(quint32,quint32)), this, SLOT(onReadParameter(quint32,quint32)), Qt::QueuedConnection);
        bParamButton[i][1] = new CMButton(i,1, QIcon(":/Save_1.png"), this);
        bParamButton[i][1]->setIconSize(QSize(16,16));
        bParamButton[i][1]->setToolTip("Записать параметр в регулятор");
        bParamButton[i][1]->setEnabled(false);
        bParamButton[i][1]->setFixedSize(18,18);
        bParamButton[i][1]->setStyleSheet("QPushButton{ \
        border-style: outset; border-width: 2px; border-color: #8A8A8A; text-align: center; \
        } \
        QPushButton:pressed { \
        background-color: qlineargradient(x1 : 0, y1 : 0, x2 : 0, y2 :   1, stop :   0.0 #F5F5F5,\
        stop :   0.5 #D2D2D2, stop :   0.5 #D2D2D2, stop :   1.0 #CFB2B2); \
        border-style: inset; border-width: 2px; border-color: #8A8A8A; text-align: center;\
        } \
        QPushButton:hover:!pressed { \
        background-color: qlineargradient(x1 : 0, y1 : 0, x2 : 0, y2 :   1, stop :   0.0 #F5F5F5, \
        stop :   0.5 #EECDCD, stop :   0.55 #EECDCD, stop :   1.0 #F5F5F5); \
        border-style: outset; border-width: 2px; border-color: #8A8A8A; text-align: center; \
        }");
        connect(bParamButton[i][1], SIGNAL(sendChange(quint32,quint32)), this, SLOT(onSaveParameter(quint32,quint32)), Qt::QueuedConnection);
        bParamButton[i][2] = new CMButton(i,2, QIcon(":/ParamInfo.png"), this);
        bParamButton[i][2]->setIconSize(QSize(16,16));
        bParamButton[i][2]->setToolTip("Вывести описание параметра");
        bParamButton[i][2]->setEnabled(false);
        bParamButton[i][2]->setFixedSize(18,18);
        bParamButton[i][2]->setStyleSheet("QPushButton{ \
        border-style: outset; border-width: 2px; border-color: #8A8A8A; text-align: center; \
        } \
        QPushButton:pressed { \
        background-color: qlineargradient(x1 : 0, y1 : 0, x2 : 0, y2 :   1, stop :   0.0 #F5F5F5,\
        stop :   0.5 #D2D2D2, stop :   0.5 #D2D2D2, stop :   1.0 #CFB2B2); \
        border-style: inset; border-width: 2px; border-color: #8A8A8A; text-align: center;\
        } \
        QPushButton:hover:!pressed { \
        background-color: qlineargradient(x1 : 0, y1 : 0, x2 : 0, y2 :   1, stop :   0.0 #F5F5F5, \
        stop :   0.5 #EECDCD, stop :   0.55 #EECDCD, stop :   1.0 #F5F5F5); \
        border-style: outset; border-width: 2px; border-color: #8A8A8A; text-align: center; \
        }");
        connect(bParamButton[i][2], SIGNAL(sendChange(quint32,quint32)), this, SLOT(onInfoParameter(quint32,quint32)), Qt::QueuedConnection);
    }
    // Длина лэйблов
    lWidth1 += 15;
    lWidth2 += 15;
    lWidth3 += 15;
    // Установка длины и в компановщик  параметров системы отопления
    for(int i=0; i < 35; i++)
    {
        if(i!=19)
        {
            lParamDBName[i]->setMinimumWidth(lWidth2);
            lParamRange[i]->setMinimumWidth(lWidth3);
            eParamValue[i]->setMaximumWidth(metrics.width(lParamGraphCurrentPoint->text())-10);
            miniLayout[i]->addWidget(lParamHeader[i], 0, 0, 1, 9);
            miniLayout[i]->addWidget(lParamDBName[i], 1, 0, 1, 4/*, Qt::AlignLeft*/);
            miniLayout[i]->addWidget(eParamValue[i], 1, 4, 1, 1/*, Qt::AlignCenter*/);
            miniLayout[i]->addWidget(lParamRange[i], 1, 5, 1, 1);
            miniLayout[i]->addWidget(bParamButton[i][0], 1, 6, 1, 1);
            miniLayout[i]->addWidget(bParamButton[i][1], 1, 7, 1, 1);
            miniLayout[i]->addWidget(bParamButton[i][2], 1, 8, 1, 1);
        }
        // График температурный
        else
        {
            lParamDBName[i]->setMaximumWidth(metrics.width(lParamDBName[i]->text()) +15);
            lParamRange[i]->setMinimumWidth(lWidth3);
            miniLayout[i]->addWidget(lParamHeader[i], 0, 0, 1, 9);
            miniLayout[i]->addWidget(lParamGraphNumberHeader, 1, 0, 1, 1);
            miniLayout[i]->addWidget(lParamGraphToutHeader, 1, 1, 1, 1);
            miniLayout[i]->addWidget(lParamDBName[i], 1, 2, 1, 1);
            miniLayout[i]->addWidget(lParamGraphCurrentPoint, 1, 3, 1, 2/*, Qt::AlignLeft*/);
            miniLayout[i]->addWidget(lParamRange[i], 1, 5, 1, 1);
            miniLayout[i]->addWidget(bParamButton[i][0], 1, 6, 1, 1);
            miniLayout[i]->addWidget(bParamButton[i][1], 1, 7, 1, 1);
            miniLayout[i]->addWidget(bParamButton[i][2], 1, 8, 1, 1);
            miniLayout[i]->addWidget(wTempGraph, 2, 3, 6, 6);
            for(int j=0; j<6; j++)
            {
                eParamGraph[j]->setMaximumWidth(lParamDBName[i]->maximumWidth());
                miniLayout[i]->addWidget(lParamGraphNumber[j], 1+1+j, 0, 1, 1);
                miniLayout[i]->addWidget(lParamGraphTout[j], 1+1+j, 1, 1, 1);
                miniLayout[i]->addWidget(eParamGraph[j], 1+1+j, 2, 1, 1);
            }
        }
        minicontainer[i]->setLayout(miniLayout[i]);
        mLOut->addWidget(minicontainer[i]);
    }
    container = new QWidget(this);
    container->setObjectName("container");
    container->setLayout(mLOut);
    myScrollBox->setWidget(container);
    ChangeSystemVisible(1);
    myScrollBox->setMinimumWidth(lWidth1+20);
    container->setStyleSheet("border-style: none;");
//    QVBoxLayout* mConfigLayout = new QVBoxLayout();
//    mConfigLayout->setMargin(4);
//    mConfigLayout->setSpacing(2);
//    mConfigLayout->addWidget(lcbx);
//    mConfigLayout->addWidget(bConnectionSetup);
    // Компановка окна настройки
    QGridLayout* mLayOut = new QGridLayout(this);
    mLayOut->setAlignment(Qt::AlignTop);
    mLayOut->setMargin(8);
    mLayOut->setSpacing(4);
    mLayOut->setColumnStretch(3,9);
    mLayOut->addWidget(lServerMode, 0,0,1,1);
    mLayOut->addWidget(lCurrentSystem,0,1,1,1);
    //mLayOut->addWidget(lReadAll,0,2,1,1);
    mLayOut->addWidget(lcbx,0,2,1,1);
    mLayOut->addWidget(bServerMode, 1,0,1,1);
    mLayOut->addWidget(bCurrentSystem,1,1,1,1);
//    mLayOut->addWidget(bReadAll,1,2,1,1);
    mLayOut->addWidget(bConnectionSetup,1,2,1,1);
    mLayOut->addWidget(lGeneralText,0,3,2,4);
//    mLayOut->addLayout(mConfigLayout,0,6,2,1);
    mLayOut->addWidget(lMiniHeader[0],2,0,1,4);
    mLayOut->addWidget(lMiniHeader[1],2,4,1,3);
    mLayOut->addWidget(lMiniHeader[2],4,4,1,1);
    mLayOut->addWidget(lMiniHeader[3],2,0,1,1);
    mLayOut->addWidget(bSaveLog,4,5,1,1);
    mLayOut->addWidget(bClearLog,4,6,1,1);
    mLayOut->addWidget(teParameterInfo,3,4,1,3);
    mLayOut->addWidget(teLog,5,4,1,3);
    mLayOut->addWidget(myScrollBox, 3, 0, 3, 4);
    // Установка
    this->setLayout(mLayOut);
    mLOut->setMargin(0);
    mLOut->setSpacing(0);
    BlockControls();
    IsOk = true;
    if(!GetUserData())
        IsOk = false;
    if(!GetObjectMode())
        IsOk =false;
    // Соединение сигналов и слотов
    connect(bServerMode, SIGNAL(clicked()), this, SLOT(onChangeServerMode()), Qt::QueuedConnection);
    connect(bCurrentSystem, SIGNAL(clicked()), this, SLOT(onChangeSystem()), Qt::QueuedConnection);
//    connect(bReadAll, SIGNAL(clicked()), this, SLOT(onReadAllParameters()), Qt::QueuedConnection);
    connect(bConnectionSetup, SIGNAL(clicked()), this, SLOT(onConnectionSetup()), Qt::QueuedConnection);
    connect(teLog, SIGNAL(textChanged()), this, SLOT(onChangeLogPosition()));
    connect(this, SIGNAL(sendClose()), this, SLOT(onCloseWindow()), Qt::QueuedConnection);
    connect(this, SIGNAL(sendReadParameter(quint32,quint32)), this, SLOT(onChangeParameter(quint32,quint32)), Qt::QueuedConnection);
    connect(this, SIGNAL(sendChangeParameter(quint32,quint32)), this, SLOT(onChangeParameter(quint32,quint32)), Qt::QueuedConnection);
    connect(this, SIGNAL(sendMessageSlot(QString,quint8)), this, SLOT(onMessageDlg(QString,quint8)), Qt::QueuedConnection);
    if(!IsOk)
        emit sendClose();
}
// ФУНКЦИИ
// Запись в заголовок параметра и его название данных о параметре
void ECL210A2661::SetLabelText(int index)
{
    lParamHeader[index]->setText(ParamTexts.at(index));
    lParamDBName[index]->setText(ParamNames.at(index));
    lParamRange[index]->setText(ParamRanges.at(index));
    // Заводские параметры - установка и подсветка
    if(index!=19)
    {
        eParamValue[index]->setText(FactoryParams.at(index));
        emit sendSetColorToEdits(index, 0, 0);
    }
    else
        for(int i=0; i <6; i++)
        {
            eParamGraph[i]->setText(FactoryTPointsHeat.at(i));
            emit sendSetColorToEdits(index, i, 0);
        }
}
// Изменение видимости элементов систем при смене настраиваемой системы
void ECL210A2661::ChangeSystemVisible(bool t)
{
    for(int i=0; i <21; i++)
        minicontainer[i]->setVisible(t);
    for(int i=21; i <35; i++)
        minicontainer[i]->setVisible(t^1);
    container->adjustSize();
    int width = myScrollBox->width();
    myScrollBox->adjustSize();
 //06.01  myScrollBox->setMinimumWidth(width);
    myScrollBox->setMaximumWidth(width);
}
// Окраска полей ввода в нужные цвета
void ECL210A2661::SetEditToColor(quint32 Index, bool IsAlarm, QColor color)
{
    if((Index >34)|(Index ==19))
        return;
    quint8 attr =0;
    if(IsAlarm)
        attr =5;
    emit sendSetColorToEdits(Index, 0, attr);
    /*QString astr;
    // Цвета блокировки по умолчанию всегда
    if(IsAlarm)
        astr = "#EfCeC3";
    else
        astr = "#CACACA";
    // В цвета по умолчанию
    if(!color.isValid())
    {
        if(IsAlarm == false)
        {
            color = QColor("#a2eea6");
        }
        else
        {
            color = QColor("#ff7e73");
        }
    }
    // Установка в поле ввода
    eParamValue[Index]->setStyleSheet("QLineEdit{ border: 1px solid black; background-color: " + color.name() +"; color: black; }"
                                  "QLineEdit:hover { border: 1px solid black; background-color: " + color.name() +"; color: black; }"
                                  "QLineEdit:!hover { border: 1px solid black; background-color: " + color.name() +"; color: black; }"
                                  "QLineEdit:!enabled { border: 1px solid gray; background-color: " +astr + "; color: #8d8d8d; }");*/
}
// Окраска полей ввода в нужные цвета
void ECL210A2661::SetEditToColor(quint32 Index, quint32 AIndex, bool IsAlarm, QColor color)
{
    if((Index !=19)|(AIndex <1)|(AIndex >6))
        return;
    quint8 attr =0;
    if(IsAlarm)
        attr =5;
    emit sendSetColorToEdits(Index, AIndex-1, attr);
    /*QString astr;
    // Цвета блокировки по умолчанию всегда
    if(IsAlarm)
        astr = "#EfCeC3";
    else
        astr = "#CACACA";
    // В цвета по умолчанию
    if(!color.isValid())
    {
        if(IsAlarm == false)
        {
            color = QColor("#a2eea6");
        }
        else
        {
            color = QColor("#ff7e73");
        }
    }
    // Установка в поле ввода
    eParamGraph[AIndex-1]->setStyleSheet("QLineEdit{ border: 1px solid black; background-color: " + color.name() +"; color: black; }"
                                  "QLineEdit:hover { border: 1px solid black; background-color: " + color.name() +"; color: black; }"
                                  "QLineEdit:!hover { border: 1px solid black; background-color: " + color.name() +"; color: black; }"
                                  "QLineEdit:!enabled { border: 1px solid gray; background-color: " +astr + "; color: #8d8d8d; }");
*/
}
// Получение настроек соединения с регулятором по данным полученным при открытии окна
bool ECL210A2661::GetDefaultConfig()
{
    if(!IsDBConnected())
        return false;
    // Получение настройки первого по счету регулятора из перечня данного объекта
    ECL210A2661Config defaultConfig(ObjectID, DriverVer, QString::number(EqNumber));
    if(!defaultConfig.ConfigStatus)
    {
        emit sendFullBlock();
        return false;
    }
    Config = defaultConfig;
    return true;
}
// 12.04.2017 Получение пользовательских данных
bool ECL210A2661::GetUserData()
{
    if(!IsDBConnected())
        return false;
    mQuery->prepare("SELECT Equipment FROM " + db->databaseName() + ".TObject WHERE ObjectID =:A;");
    mQuery->bindValue(":A", ObjectID);
    if((!mQuery->exec())|(mQuery->size() <=0))
    {
        emit sendFullBlock();
        return false;
    }
    QSqlRecord rec = mQuery->record();
    QByteArray data;
    QStringList leqipment, lMBAddr;
    data.clear();
    leqipment.clear();
    lMBAddr.clear();
    mQuery->next();
    data = mQuery->value(rec.indexOf("Equipment")).toByteArray();
    if(!data.isEmpty())
        leqipment = deserialize(data);
    // Поиск приборов ECL210A2661
    if(!leqipment.isEmpty())
        for(int i=0; i < leqipment.size(); i++)
        {
            if(leqipment.at(i).contains("ECL210A2661/"))
            {
                QString eq = leqipment.at(i);
                QStringList leq = eq.split("/");
                bool IsOk;
                if(leq.size()!=2)
                    continue;
                eq = leq.at(1);
                quint8 addr = eq.toUInt(&IsOk);
                if((addr <1)|(addr > 247)|(!IsOk))
                    continue;
                lMBAddr << eq;
            }
        }
    if(!lMBAddr.isEmpty())
    {
        lModBusAddresses = lMBAddr;
        lcbx->addItems(lModBusAddresses);
        if(lModBusAddresses.size() ==1)
        {
            lcbx->setEnabled(false);
        }
        bConnectionSetup->setEnabled(true);
        connect(lcbx, SIGNAL(currentIndexChanged(QString)), this, SLOT(onSelectedConfig(QString)), Qt::QueuedConnection);
        EqNumber = lModBusAddresses.at(0).toUInt();
        if(!GetDefaultConfig())
            return false;
        return true;
    }
    emit sendFullBlock();
    return false;

    /*mQuery->prepare("SELECT UserFontSize FROM " + db->databaseName() + ".TUsers WHERE UserLogin = :A;");
    mQuery->bindValue(":A", UserString);
    if((!mQuery->exec())|(mQuery->size() <=0))
    {
        emit sendFullBlock();
        return false;
    }
    QSqlRecord rec = mQuery->record();
    mQuery->next();
    bool IsOk = false;
    // Получение размера шрифта
    FontSize = mQuery->value(rec.indexOf("UserFontSize")).toInt(&IsOk);
    if((IsOk == false)|(FontSize < 10)|(FontSize > 40))
        FontSize = 12;
    return true;*/
}
// 12.04.2017 Создание таблицы статистики команд (данная функция также содержится в драйверах, в зависимости от
//            того какая часть ПО обращается к таблице статуса команды
bool ECL210A2661::CreateCommandStatusTable()
{
    if(!IsDBConnected())
        return false;
    QStringList mTableList = db->tables();
    // Поиск таблицы статусов команд
    bool IsTable = false;
    foreach (const QString &mTable, mTableList)
    {
        if(mTable == "tcommandstat"){ IsTable = true; break; }
    }
    // Таблица статусов не найдена - следует создать
    if(IsTable ==false)
    {
        teLog->append(" > Таблица статусов TCommandStat не найдена");
        teLog->append(" > Попытка создания таблицы");
        QString str = "CREATE TABLE IF NOT EXISTS " + db->databaseName() + ".TCommandStat ( "
                      "number INTEGER PRIMARY KEY AUTO_INCREMENT, "
                      "ObjectID INT UNSIGNED DEFAULT NULL, "
                      "LastTime DATETIME DEFAULT NOW(), "
                      "CurrectResultTime DATETIME DEFAULT '1980-12-03 01:00:00', "
                      "CID INT UNSIGNED DEFAULT NULL, "
                      "SESSION INT UNSIGNED DEFAULT NULL, "
                      "Status VARCHAR(100) DEFAULT NULL, "
                      "Answer BLOB, "
                      "ErrorCode VARCHAR(100)"
                                        ") ROW_FORMAT=DYNAMIC;";
        if (!mQuery->exec(str))
        {
            teLog->append(" > Не удалось создать таблицу статусов команд");
            qDebug() << " > Не удалось создать таблицу статусов команд" << mQuery->lastError();
            emit sendFullBlock();
            return false;
        }
        else
        {
            teLog->append(" > Таблица статусов команд создана");
        }
    }
    else
        teLog->append(" > Таблица статусов TCommandStat найдена");
    return true;
}
// 01.01.2018 Проверка состояния соединения
bool ECL210A2661::IsDBConnected()
{
    // Если указатели есть то начинается проверка
    if(mQuery!=NULL)
    {
        if(!mQuery->exec("SELECT 1;"))
        {
            delete mQuery;
            mQuery = NULL;
            teLog->append(" > База данных отключена либо соединение не установлено");
            teLog->append(" > Попытка восстановления соединения");
            if(!db->open())
            {
                teLog->append(" > База данных не открыта");
                emit sendFullBlock();
                return false;
            }
            mQuery = new QSqlQuery(*db);
            teLog->append(" > База данных окрыта, запрос создан");
        }
        return true;
    }
    teLog->append(" > Соединение с БД не установлено либо потеряно");
    teLog->append(" > Работа модуля невозможна");
    emit sendFullBlock();
    return false;
}
// 16.04.2017 Изменение режима
bool ECL210A2661::ChangeObjectMode(QString WorkMode, QString UserCommand)
{
    if(!IsDBConnected())
        return false;
    bool IsOk = true;

    mQuery->prepare("UPDATE " + db->databaseName() + ".TObject SET WorkMode =:A, UserCommand =:B WHERE "
                    "ObjectID =:C and number >0;");
    mQuery->bindValue(":A", WorkMode);
    mQuery->bindValue(":B", UserCommand);
    mQuery->bindValue(":C", ObjectID);
    if(!mQuery->exec())
    {
        teLog->append(" > Ошибка выполнения запроса смены режима");
        IsOk = false;
        emit sendFullBlock();
        return IsOk;
    }
    teLog->append(" > Запрос смены режима выполнен");
    return IsOk;
}
// Получение режима работы сервера и команды пользователя
bool ECL210A2661::GetObjectMode()
{
    if(!IsDBConnected())
        return false;
    // Проверка режима работы объекта и пользовательской команды
    mQuery->prepare("SELECT UserCommand, WorkMode FROM " + db->databaseName() + ".TObject WHERE ObjectID =:A AND number >0;");
    mQuery->bindValue(":A", ObjectID);
    if((!mQuery->exec())|(mQuery->size() <=0))
    {
        teLog->append(" > Ошибка запроса режима работы объекта");
        teLog->append(" > Проверьте подключение базы данных и настройки");
        emit sendFullBlock();
        return false;
    }
    mQuery->next();
    QSqlRecord rec = mQuery->record();
    QString UserCommand, WorkMode;
    UserCommand = mQuery->value(rec.indexOf("UserCommand")).toString();
    WorkMode = mQuery->value(rec.indexOf("WorkMode")).toString();
    // Проверка полученных данных
    if(((WorkMode != "WHILE")&(WorkMode != "TIME")&(WorkMode != "TERMINAL")&(WorkMode != "REQUEST"))|
       ((UserCommand != "EDIT")&(UserCommand != "EDIT+OK")&(UserCommand != "DEL")&(UserCommand != "DEL+OK")&
        (UserCommand != "STOP")&(UserCommand != "STOP+OK")&(UserCommand != "START")&(UserCommand != "START+OK")&
        (UserCommand != "USTOP")&(UserCommand != "USTOP+OK")))
    {
        emit sendMessageSlot("<div align = left><b><font color = red> "
                             "> Системное сообщение: в базе данных объекта<br>"
                             "> имеются некорректные данные.<br>"
                             "> Исправьте режим работы сервера,<br>"
                             "> либо пользовательские команды.<br>"
                             "<font color = black>"
                             "> Данная проблема не является опасной,<br>"
                             "> однако проводить работы до устранения<br>"
                             "> ошибки невозможно</b></div>", 1);
        teLog->append(" > Данные режима работы сервера некорректны");
        teLog->append(" > WorkMode:" + WorkMode + ", UserCommand:" + UserCommand);
        emit sendFullBlock();
        return false;
    }
    // Убрать "+OK" если есть
    if(UserCommand.contains("+"))
        UserCommand = UserCommand.left(UserCommand.indexOf("+"));
    CurrentUserCommand = UserCommand;
    CurrentWorkMode = WorkMode;
    return true;
}
// Разблокировка/блокировка элементов ввода данных
void ECL210A2661::ServerModeChange(bool Mode)
{
    // Временное только для проверки иконок
    if(Mode)
    {
        bServerMode->setIcon(QIcon(":/ServerInControl.png"));
        bServerMode->setIconSize(QSize(44,44));
        bCurrentSystem->setEnabled(true);
//        bReadAll->setEnabled(true);
        lGeneralText->setStyleSheet("QLabel { border: 2px solid black; background-color: #a2eea6; color: black; }"
                                    "QLabel:!enabled { border: 2px solid gray; background-color: #bfdfbf; color: #9c9c9c; }");
        lGeneralText->setText("<div align = center><b><font size =\"5\" > Настройка разблокирована!</font></b></div>"
             "<div align = left><b><font size =\"2\" >"
             "1. Не выключайте компьютер и не разрывайте соединение с базой данных до перевода<br>"
             "сервера в рабочий режим<br>"
             "2. Если компьютер был выключен или потеряно соединение - проверьте таблицу команд объекта"
             "</font></b></div>");
        BlockControls(false);
        teLog->append(" > Смена режима работы сервера на объекте ID:" + QString::number(ObjectID) + " на настройку");
    }
    else
    {
        bServerMode->setIcon(QIcon(":/ServerInWork.png"));
        bServerMode->setIconSize(QSize(44,44));
        bCurrentSystem->setEnabled(false);
//        bReadAll->setEnabled(false);
        lGeneralText->setStyleSheet("QLabel { border: 2px solid black; background-color: #ff7e73; color: black; }"
                                    "QLabel:!enabled { border: 2px solid gray; background-color: #dfbfbf; color: #9c9c9c; }");
        lGeneralText->setText("<div align = center><b><font size =\"5\" >Настройка заблокирована!</font></b></div>"
             "<div align = left><b><font size =\"2\" >"
             "1. Для начала работы переведите сервер в режим наладки<br>"
             "2. При наладке оборудования не выключайте компьютер<br>"
             "3. После выполнения процедуры наладки обязательно восстановите режим сервера"
             "</font></b></div>");
        BlockControls();
        teLog->append(" > Смена режима работы сервера на объекте ID:" + QString::number(ObjectID) + " на рабочий");
    }
}
// 12.04.2017 На запись настроек
void ECL210A2661::Configuration()
{
    // Проверка наличия таблицы, если режим изменен - значит уже проверялось
    if(ChangeWorkMode == false)
    {
        if(!CreateCommandStatusTable())
        {
            teLog->append(" > Команда не выполнена");
            return;
        }
    }
    if(!IsDBConnected())
        return;
    // Проверка подключен ли сервер
    mQuery->prepare("SELECT IsServerSet, ServerDelay FROM " + db->databaseName() + ".TServer;");
    if((!mQuery->exec())|(mQuery->size() <=0))
    {
        // При повторной проверке следует восстановить данные
        if(ChangeWorkMode)
        {
            ChangeObjectMode(CurrentWorkMode, CurrentUserCommand);
            ChangeCommand("", "_");
            ChangeWorkMode = false;
        }
        teLog->append(" > Ошибка проверки работы сервера");
        teLog->append(" > Проверьте подключение базы данных и настройки");
        emit sendFullBlock();
        return;
    }
    mQuery->next();
    QSqlRecord rec = mQuery->record();
    uint IsServerSet =0;
    ServerDelay =0;
    bool IsOk = false;
    IsServerSet = mQuery->value(rec.indexOf("IsServerSet")).toUInt(&IsOk);
    if(IsOk == false)
    {
        // При повторной проверке следует восстановить данные
        if(ChangeWorkMode)
        {
            ChangeObjectMode(CurrentWorkMode, CurrentUserCommand);
            ChangeCommand("", "_");
            ChangeWorkMode = false;
        }
        teLog->append(" > Не удалось проверить работу сервера");
        teLog->append(" > Проверьте подключение базы данных и настройки");
        emit sendFullBlock();
        return;
    }
    else if(IsServerSet != 1)
    {
        // При повторной проверке следует восстановить данные
        if(ChangeWorkMode)
        {
            ChangeObjectMode(CurrentWorkMode, CurrentUserCommand);
            ChangeCommand("", "_");
            ChangeWorkMode = false;
        }
        teLog->append(" > Сервер отключен");
        teLog->append(" > Необходимо запустить сервер для настройки оборудования");
        emit sendFullBlock();
        return;
    }
    teLog->append(" > Сервер запущен");
    ServerDelay = mQuery->value(rec.indexOf("ServerDelay")).toUInt(&IsOk);
    if((ServerDelay >120)|(IsOk == false))
    {
        // При повторной проверке следует восстановить данные
        if(ChangeWorkMode)
        {
            ChangeObjectMode(CurrentWorkMode, CurrentUserCommand);
            ChangeCommand("", "_");
            ChangeWorkMode = false;
        }
        teLog->append(" > Не удалось получить частоту опроса сервера");
        teLog->append(" > Проверьте настройки сервера");
        emit sendFullBlock();
        return;
    }
    teLog->append(" > Частота опроса сервера:" + QString::number(ServerDelay));
    // Если режим изменен, то проверка не требуется т.к. она уже произведена
    if(ChangeWorkMode == false)
    {
        // Проверка режима работы объекта и пользовательской команды
        if(!GetObjectMode())
            return;
        teLog->append(" > Смена режима работы сервера с данным объектом");
        // Смена режима
        if(!ChangeObjectMode("WHILE", "START"))
        {
            return;
        }
        // Сменить в перечне команд тип команды с префиксом _ для отмены обработки
        ChangeCommand("_", "");
        // Команда серверу обновить потоки
        RunObject();
        // Запуск таймера, блокировка кнопок, отмена нажатия кнопок
        bServerMode->setEnabled(false);
        timer = new QTimer(this);
        // Виджет прогресса
        CProgress *progress = new CProgress(this, ServerDelay * 1000 + 60000, ServerDelay *500);
        progress->show();
        connect(this, SIGNAL(sendCloseProgress()), progress, SLOT(onClose()));
        connect(this, SIGNAL(sendStepProgress()), progress, SLOT(onStep()));
        connect(timer, SIGNAL(timeout()), this, SLOT(onServerTimeout()), Qt::QueuedConnection);
        ChangeWorkMode = true;        // Блокировка обработки выхода на время смены режима
        CurrentTimer =0;
        CheckTimeout =false;
        timer->start(ServerDelay *500); // Задержка сервера + 60 сек
        teLog->append(" > .");
        InProcessing = true;
        return;
    }
    ServerModeChange(ChangeWorkMode);
    bServerMode->setEnabled(true);
}
// 16.04.2017 Установка префикса в перечне команд
bool ECL210A2661::ChangeCommand(QString NewPrefix, QString OldPrefix)
{
    if(!IsDBConnected())
        return false;
    bool IsOk = true;
    // Смена команд CURRENT
    mQuery->prepare("UPDATE " + db->databaseName() + ".TCommand SET RequestType = :A "
                    "WHERE RequestType = :B AND ObjectID =:C AND number >0;");
    mQuery->bindValue(":A", NewPrefix + "CURRENT");
    mQuery->bindValue(":B", OldPrefix + "CURRENT");
    mQuery->bindValue(":C", ObjectID);
    if(!mQuery->exec())
    {
        teLog->append(" > Команды типа CURRENT не изменены");
        teLog->append(" > Ошибка запроса");
        IsOk = false;
    }
    else
        teLog->append(" > Команды типа CURRENT успешно изменены");
    // Смена команд STATISTIC
    mQuery->prepare("UPDATE " + db->databaseName() + ".TCommand SET RequestType = :A "
                    "WHERE RequestType = :B AND ObjectID =:C AND number >0;");
    mQuery->bindValue(":A", NewPrefix + "STATISTIC");
    mQuery->bindValue(":B", OldPrefix + "STATISTIC");
    mQuery->bindValue(":C", ObjectID);
    if(!mQuery->exec())
    {
        teLog->append(" > Команды типа STATISTIC не изменены");
        teLog->append(" > Ошибка запроса");
        IsOk = false;
    }
    else
        teLog->append(" > Команды типа STATISTIC успешно изменены");
    // Смена команд CONFIG_C
    mQuery->prepare("UPDATE " + db->databaseName() + ".TCommand SET RequestType = :A "
                    "WHERE RequestType = :B AND ObjectID =:C AND number >0;");
    mQuery->bindValue(":A", NewPrefix + "CONFIG_C");
    mQuery->bindValue(":B", OldPrefix + "CONFIG_C");
    mQuery->bindValue(":C", ObjectID);
    if(!mQuery->exec())
    {
        teLog->append(" > Команды типа CONFIG_C не изменены");
        teLog->append(" > Ошибка запроса");
        IsOk = false;
    }
    else
        teLog->append(" > Команды типа CONFIG_C успешно изменены");
    // Смена команд CONFIG_S
    mQuery->prepare("UPDATE " + db->databaseName() + ".TCommand SET RequestType = :A "
                    "WHERE RequestType = :B AND ObjectID =:C AND number >0;");
    mQuery->bindValue(":A", NewPrefix + "CONFIG_S");
    mQuery->bindValue(":B", OldPrefix + "CONFIG_S");
    mQuery->bindValue(":C", ObjectID);
    if(!mQuery->exec())
    {
        teLog->append(" > Команды типа CONFIG_S не изменены");
        teLog->append(" > Ошибка запроса");
        IsOk = false;
    }
    else
        teLog->append(" > Команды типа CONFIG_S успешно изменены");
    if(!IsOk)emit sendFullBlock();
    return IsOk;
}
// 16.04.2017 Запуск объекта
bool ECL210A2661::RunObject()
{
    if(!IsDBConnected())
        return false;
    bool IsOk = true;
    mQuery->prepare("UPDATE " + db->databaseName() + ".TServer SET ClientCommand =:A WHERE number >0;");
    mQuery->bindValue(":A", "RUN/" + QString::number(ObjectID));
    if(!mQuery->exec())
    {
        teLog->append(" > Ошибка выполнения запроса команды серверу");
        IsOk = false;
    }
    else
        teLog->append(" > Команда на запуск потока записана");
    if(!IsOk)emit sendFullBlock();
    return IsOk;
}
// 16.04.2017 Удаление статусов и команд управления
bool ECL210A2661::DeleteCommands()
{
    CCommandDB command;
    // Удаление всех команд кроме команд графика
    for(int i =0; i < 35; i++)
    {
        if(i ==19)
            continue;
            command = InWork->GetCommand("READ", i, 0);
        if(command.Params.isEmpty())
            continue;
        if(!IsDBConnected())
            return false;
        // Удаление всех команд с аналогичным CID
        mQuery->prepare("DELETE FROM " + db->databaseName() + ".TCommand WHERE ObjectID =:A AND CID =:B AND number >0;");
        mQuery->bindValue(":A", ObjectID);
        mQuery->bindValue(":B", command.CID);
        if(!mQuery->exec())
            teLog->append(" > Ошибка удаления команды CID:" + QString::number(command.CID));
        // Изменение статусов
        SetStatus(command, "EXIT");
    }
    // Удаление команд температурного графика
    for(int i =0; i < 6; i++)
    {
        command = InWork->GetCommand("READ",19,i);
        if(command.Params.isEmpty())
            continue;
        if(!IsDBConnected())
            return false;
        // Удаление всех команд с аналогичным CID
        mQuery->prepare("DELETE FROM " + db->databaseName() + ".TCommand WHERE ObjectID =:A AND CID =:B AND number >0;");
        mQuery->bindValue(":A", ObjectID);
        mQuery->bindValue(":B", command.CID);
        if(!mQuery->exec())
            teLog->append(" > Ошибка удаления команды CID:" + QString::number(command.CID));
        // Изменение статусов
        SetStatus(command, "EXIT");
    }
    return true;
}
// 17.04.2017 Получение статусов из TCommandStat для отслеживания выполнения
QString ECL210A2661::GetStatus(CCommandDB command)
{
    QString Status;
    Status.clear();
    if(!IsDBConnected())
        return Status;
    // Получение статуса команды
    mQuery->prepare("SELECT Status FROM " + db->databaseName() + ".TCommandStat "
                    "WHERE ObjectID =:A AND CID =:B;");
    mQuery->bindValue(":A", ObjectID);
    mQuery->bindValue(":B", command.CID);
    if((!mQuery->exec())|(mQuery->size() <=0))
    {
        teLog->append(" > Статус команды CID:" + QString::number(command.CID) + " не получен");
        teLog->append(" > Ошибка выполнения запроса");
        return Status;
    }
    QSqlRecord rec = mQuery->record();
    mQuery->next();
    Status = mQuery->value(rec.indexOf("Status")).toString();
    if((Status!="READY")&(Status!="PROCESSING")&(Status!="ERROR")&(Status!="TIMEOUT")&(Status!="COMPLETED")&
       (Status !="EXIT"))
    {
        teLog->append(" > Статус команды CID:" + QString::number(command.CID) + " получен но не определен");
        Status.clear();
    }
    return Status;
}
// 16.04.2017 Установка статусов в TCommandStat для отслеживания
bool ECL210A2661::SetStatus(CCommandDB command, QString status)
{
    bool IsOk = true;
    if(!IsDBConnected())
        return false;
    // Обновить строку в БД с указанным CID
    mQuery->prepare("SELECT number FROM " + db->databaseName() + ".TCommandStat "
                     "WHERE ObjectID =:A AND CID =:B;");
    mQuery->bindValue(":A", ObjectID);
    mQuery->bindValue(":B", command.CID);
    // Данных по этой команде нет или ошибка запроса, будет попытка добавить строку с данными
    if((!mQuery->exec())|(mQuery->size() <=0))
    {
        mQuery->prepare("INSERT INTO " + db->databaseName() + ".TCommandStat "
                         "(ObjectID, CID, SESSION, Status, LastTime) VALUES(:A,:B,:C,:D,:E);");
        mQuery->bindValue(":A", ObjectID);
        mQuery->bindValue(":B", command.CID);
        mQuery->bindValue(":C", 0);
        mQuery->bindValue(":D", status);
        mQuery->bindValue(":E", QDateTime::currentDateTime());
        if(!mQuery->exec())
        {
            teLog->append(" > Ошибка записи статуса команды CID:" +
                          QString::number(command.CID));
            IsOk = false;
        }
        else
            teLog->append(" > Запись статуса команды CID:" +
                          QString::number(command.CID) + " произведена");
    }
    else
    {
        mQuery->prepare("UPDATE " + db->databaseName() + ".TCommandStat "
                         "SET LastTime =:A, Status =:B, SESSION =:C WHERE ObjectID =:D AND CID =:E AND number >0;");
        mQuery->bindValue(":A", QDateTime::currentDateTime());
        mQuery->bindValue(":B", status);
        mQuery->bindValue(":C", 0);
        mQuery->bindValue(":D", ObjectID);
        mQuery->bindValue(":E", command.CID);
        if(!mQuery->exec())
        {
            teLog->append(" > Ошибка записи статуса команды CID:" +
                          QString::number(command.CID));
            IsOk = false;
        }
        else
            teLog->append(" > Запись статуса команды CID:" +
                          QString::number(command.CID) + " произведена");
    }
    return IsOk;
}
// 17.04.2017 Получение данных из TCommandStat для обработки
QByteArray ECL210A2661::GetAnswer(CCommandDB command)
{
    QByteArray data;
    data.clear();
    if(!IsDBConnected())
        return data;
    // Получение данных команды
    mQuery->prepare("SELECT Answer FROM " + db->databaseName() + ".TCommandStat "
                     "WHERE ObjectID =:A AND CID =:B;");
    mQuery->bindValue(":A", ObjectID);
    mQuery->bindValue(":B", command.CID);
    if((!mQuery->exec())|(mQuery->size() <=0))
    {
        teLog->append(" > Ответ на команду CID:" + QString::number(command.CID) + " не получен");
        teLog->append(" > Ошибка выполнения запроса");
        return data;
    }
    QSqlRecord rec = mQuery->record();
    mQuery->next();
    data = mQuery->value(rec.indexOf("Answer")).toByteArray();
    if(data.isEmpty())
    {
        teLog->append(" > Ответ на команду CID:" + QString::number(command.CID) + " не содержит данных");
        data.clear();
    }
    return data;
}
// 18.04.2017 Запись команды в базу данных
bool ECL210A2661::SetCommand(CCommandDB command, quint32 index, quint32 gindex)
{
    if(!IsDBConnected())
        return false;
    // Атрибут для окраски поля ввода на случай ошибок
    quint8 attr=0;
    if(command.Params.at(0).VarOffset ==6)
        attr =4; // write
    else
        attr =5; // read
    // Удаление всех команд с аналогичным CID
    mQuery->prepare("DELETE FROM " + db->databaseName() + ".TCommand WHERE ObjectID =:A AND CID =:B AND number >0;");
    mQuery->bindValue(":A", ObjectID);
    mQuery->bindValue(":B", command.CID);
    if(!mQuery->exec())
        teLog->append(" > Ошибка удаления команды CID:" + QString::number(command.CID));
    // Запись новой
    QByteArray arr;
    arr.clear();
    arr = serializec(command.Params);
    mQuery->prepare("INSERT INTO " + db->databaseName() + ".TCommand "
                    "(ObjectID, EqName, EqNumber, EqNumberType, CID, RequestType, TypeCommand, "
                    "Attempts, CountsCommand, Delay, Priority, Funct, PortNum, DataCom, StopBits, "
                    "ParityBits, SpeedCom, Parameters, Command) "
                    "VALUES(:A,:B,:C,:D,:E,:F,:G,:H,:J,:K,:L,:M,:N,:P,:Q,:R,:S,:T,:V);");
    mQuery->bindValue(":A", ObjectID);
    mQuery->bindValue(":B", command.EqName);
    mQuery->bindValue(":C", command.EqNumber);
    mQuery->bindValue(":D", command.EqNumberType);
    mQuery->bindValue(":E", command.CID);
    mQuery->bindValue(":F", command.RequestType);
    mQuery->bindValue(":G", command.TypeCommand);
    mQuery->bindValue(":H", command.Attempts);
    mQuery->bindValue(":J", command.CountCommand);
    mQuery->bindValue(":K", command.Delay);
    mQuery->bindValue(":L", command.Priority);
    mQuery->bindValue(":M", command.Funct);
    mQuery->bindValue(":N", command.PortNum);
    mQuery->bindValue(":P", command.DataCom);
    mQuery->bindValue(":Q", command.StopBits);
    mQuery->bindValue(":R", command.ParityBits);
    mQuery->bindValue(":S", command.SpeedCom);
    mQuery->bindValue(":T", arr, QSql::Binary | QSql::In);
    mQuery->bindValue(":V", command.Command, QSql::Binary | QSql::In);
    if(!mQuery->exec())
    {
        teLog->append(" > Ошибка запроса записи команды CID:" + QString::number(command.CID));
        emit sendSetColorToEdits(index, gindex, attr);
        return false;
    }
    teLog->append(" > Команда записана CID:" + QString::number(command.CID));
    // Установка статуса команды в готовность
    if(SetStatus(command, "READY"))
        return true;
    emit sendSetColorToEdits(index, gindex, attr);
    return false;
}
// Обновление точек графика после блокировки виджета
void ECL210A2661::UpdateGraphPoints()
{
    quint32 Index, AIndex;
    Index =19;
    AIndex =0;
    if(TPointsValues.size()!=6)
    {
        TPointsValues.clear();
        return;
    }
    for(int i=0; i <6; i++)
    {
        AIndex = i+1;
        if((Index!=19)|(AIndex <1)|(AIndex >6)) return;
        QString value =TPointsValues[i];
        bool IsOk;
        float T =value.toFloat(&IsOk);
        if((T < eParamValue[0]->text().toDouble())|(T > eParamValue[1]->text().toDouble())|(!IsOk))
        {
            eParamGraph[AIndex-1]->setText(eParamValue[0]->text());
            emit sendTemperature(eParamValue[0]->text(), AIndex);
            /*if(IsRead)
                emit sendSetColorToEdits(Index, AIndex-1,5);
            else
                emit sendSetColorToEdits(Index, AIndex-1,4);*/
        }
        else
        {
            emit sendTemperature(value, AIndex);
            /*if(eParamGraph[AIndex-1]->text() == FactoryTPointsHeat[AIndex-1])
                emit sendSetColorToEdits(Index,AIndex-1,0);
            else
                if(IsRead)
                    emit sendSetColorToEdits(Index,AIndex-1,1);
                else
                    emit sendSetColorToEdits(Index,AIndex-1,2);*/
        }
    }
    TPointsValues.clear();
}
// 16.04.2017 Установка текста в поле параметра
/*void ECL210A2661::SetTextToEdit(quint32 index, quint32 gindex, QString text)
{
    bool IsOk;
    // Если текст пустой, то запись значения параметра, значит значение было получено
    if(text.isEmpty())
    {
        QString str ="";
        if(index!=19)
        {
            str = Params[index].GetTextFromVal(&IsOk);
            eParamValue[index]->setText(str);
            return;
        }
        str =ParamsTPoints[gindex].GetTextFromVal(&IsOk);
        eParamGraph[gindex]->setText(str);
        return;
    }
    emit sendMessageSlot("<div align = left><b><font color = red> "
                         "> Системное сообщение: " + text  + "<br>"
                         "<font color = black> "
                         "> Проверьте работу компонентов системы<br>"
                         "> Возможна некорректная работа<br>"
                         "> Проверьте настройки соединения и<br>"
                         "> Режим работы сервера</b></div>", 1);
    teLog->append(" > Сообщение системы:" + text);
    return;
}*/
// 16.04.2017 Блокировка контролов на время обработки
void ECL210A2661::BlockControls(bool Block)
{
    if(Block == true)
    {
        teLog->setEnabled(true);
        onChangeLogPosition();
        // Блокировка
        bClearLog->setEnabled(true);
        bSaveLog->setEnabled(true);
        for(int i=0; i < 35; i++)
        {
            lParamHeader[i]->setEnabled(false);
            lParamDBName[i]->setEnabled(false);
            lParamRange[i]->setEnabled(false);
            for(int j=0; j <3; j++)
                bParamButton[i][j]->setEnabled(false);
            // Блокировка полей ввода
            if(i==19)
            {
                lParamGraphNumberHeader->setEnabled(false);
                lParamGraphToutHeader->setEnabled(false);
                lParamGraphCurrentPoint->setEnabled(false);
                wTempGraph->setEnabled(false);
                for(int j=0; j <6; j++)
                {

                    lParamGraphNumber[j]->setEnabled(false);
                    lParamGraphTout[j]->setEnabled(false);
                    eParamGraph[j]->setEnabled(false);
                }
            }
            else
                eParamValue[i]->setEnabled(false);
        }
        return;
    }
    teLog->setEnabled(true);
    onChangeLogPosition();
    bClearLog->setEnabled(true);
    bSaveLog->setEnabled(true);
    for(int i=0; i < 35; i++)
    {
        lParamHeader[i]->setEnabled(true);
        lParamDBName[i]->setEnabled(true);
        lParamRange[i]->setEnabled(true);
        for(int j=0; j <3; j++)
            bParamButton[i][j]->setEnabled(true);
        // Разблокировка полей ввода
        if(i==19)
        {
            lParamGraphNumberHeader->setEnabled(true);
            lParamGraphToutHeader->setEnabled(true);
            lParamGraphCurrentPoint->setEnabled(true);
            wTempGraph->setEnabled(true);
            for(int j=0; j <6; j++)
            {
                lParamGraphNumber[j]->setEnabled(true);
                lParamGraphTout[j]->setEnabled(true);
                eParamGraph[j]->setEnabled(true);
            }
        }
        else
            eParamValue[i]->setEnabled(true);
    }
    return;
}
// СЛОТЫ
// Настройка соединения с прибором
void ECL210A2661::onConnectionSetup()
{
    lGeneralText->setFixedSize(lGeneralText->width(), lGeneralText->height());
    myScrollBox->setFixedSize(myScrollBox->width(), myScrollBox->height());
    teLog->setFixedSize(teLog->width(), teLog->height());
    teParameterInfo->setFixedSize(teParameterInfo->width(), teParameterInfo->height());
    lcbx->setEnabled(false);
    bServerMode->setEnabled(true);
    bConnectionSetup->setEnabled(false);
    // Окно настройки параметров соединения
    ECL210A2661ConfigDlg *pConf = new ECL210A2661ConfigDlg(this, &Config);
    pConf->setModal(true);
    pConf->setWindowTitle("Настройки связи с регулятором");
    pConf->setWindowIcon(QIcon(":/icon_1.png"));
    pConf->show();
    pConf->adjustSize();
    pConf->setFixedSize(pConf->width(), pConf->height());
    pConf->setEnabled(true);
    connect(pConf, SIGNAL(sendConnectionSetupFinished()), this, SLOT(onConnectionSetupFinished()));
}
// После завершения настройки соединения вывод в лог и на создание параметров и команд
void ECL210A2661::onConnectionSetupFinished()
{
    teLog->append(" > Настройки соединения получены:");
    teLog->append(" > Версия драйвера:" + QString::number(Config.DllVer, 'f', 2));
    teLog->append(" > ID объекта:" + QString::number(Config.ObjectID));
    teLog->append(" > Адрес регулятора:" + Config.ModBusAddr);
    teLog->append(" > Номер порта ПЛК:" + QString::number(Config.ComNumber));
    teLog->append(" > Скорость работы интерфейса:" + QString::number(Config.PortSpeed));
    teLog->append(" > Время ожидания ответа:" + QString::number(Config.EqDelay));
    teLog->append(" > Настройка:" + Config.ConfigName + ".bin");
    teLog->append(" > Установка перечня параметров со значениями по умолчанию");
    // Создается один параметр, который будет выполнять все необходимые функции
    InWork = new CParam(&Config,0,0);
    //Params.clear();
    //ParamsTPoints.clear();
    //for(int i=0; i < 35; i++)
    //{
        // Внимание!
        // При создании CParam принимает значение заводских настроек и поэтому стили не применяются повторно
    //    if(i!=19)
    //    {
    //        Params << CParam(&Config, i);
    //        eParamValue[i]->setText(Params[i].GetTextFromVal(&IsOk));
    /*        teLog->append(" > Параметр " + ParamNames.at(i) + "=" + eParamValue[i]->text());
        }
        else
        {
            Params << CParam(&Config, 100); // Создание пустого параметра
            for(int j=0; j <6; j++)
            {
                ParamsTPoints << CParam(&Config, i, j);
                eParamGraph[j]->setText(ParamsTPoints[j].GetTextFromVal(&IsOk));
                teLog->append(" > Параметр " + ParamNames.at(i) + "_" +
                              QString::number(j+1) +"=" + eParamGraph[j]->text());
            }
        }
    }*/
}
// Обработка нажатия кнопки изменения режима
void ECL210A2661::onChangeServerMode()
{
    // Если режим не изменялся, то на начало конфигурирования
    if(!ChangeWorkMode)
        Configuration();
    else
    {
        // Если исполняется таймер, то не следует ничего менять
        if(timer!=NULL)
            return;
    /*
        bServerMode->setEnabled(false);
        timer = new QTimer(this);
        // Виджет прогресса
        CProgress *progress = new CProgress(this, ServerDelay * 1000 + 60000, ServerDelay *500);
        progress->show();
        connect(this, SIGNAL(sendCloseProgress()), progress, SLOT(onClose()));
        connect(this, SIGNAL(sendStepProgress()), progress, SLOT(onStep()));
        connect(timer, SIGNAL(timeout()), this, SLOT(onServerTimeout()), Qt::QueuedConnection);
        ChangeWorkMode = true;        // Блокировка обработки выхода на время смены режима
        CurrentTimer =0;
        CheckTimeout =false;
        timer->start(ServerDelay *500); // Задержка сервера + 60 сек
        teLog->append(" > .");
        InProcessing = true;
*/

        teLog->append(" > Режим изменен, восстановление режима");
        if(ChangeObjectMode(CurrentWorkMode, CurrentUserCommand))
            if(GetObjectMode())
            {
                DeleteCommands();
                ChangeCommand("", "_");
                ChangeWorkMode ^=1;
                ServerModeChange(ChangeWorkMode);
                bServerMode->setEnabled(true);
            }
    }
}
// Чтение всех параметров
void ECL210A2661::onReadAllParameters()
{
    emit sendFullBlock();
}
// Чтение параметра с заданным номером
void ECL210A2661::onReadParameter(quint32 Index, quint32 Type)
{
    if(Type!=0)
        return;
    IsRead =true;
    emit sendReadParameter(Index, 0);    // Сигнал на чтение параметра
}
// Запись параметра с заданным номером по сигналу от кнопки
void ECL210A2661::onSaveParameter(quint32 Index, quint32 Type)
{
    if(Type!=1)
        return;
    IsRead =false;
    TPointsValues.clear();                // Очистка значений
    emit sendChangeParameter(Index, 0);   // Отправка сигнала на изменение параметра
}
// Информация о параметре с заданным номером
void ECL210A2661::onInfoParameter(quint32 Index, quint32 Type)
{
    if((Index >34)|(Type!=2))
        return;
    // Запись данных о параметре в текстовое поле
    teParameterInfo->clear();
    teParameterInfo->append(ParametersInfo.at(Index));
    QTextCursor c =  teParameterInfo->textCursor();
    c.movePosition(QTextCursor::Start);
    teParameterInfo->setTextCursor(c);
}
void ECL210A2661::onDataTest(quint32 Index)
{
    if((Index >35)|(Index ==19))
        return;
    bool IsOk1, IsOk2;
    // TMin/ TMax
    if((Index == 0)|(Index == 1))
    {
        QString valueMin;
        QString valueMax;
        valueMin =eParamValue[0]->text();
        valueMax =eParamValue[1]->text();
        float Min =valueMin.toFloat(&IsOk1);
        float Max =valueMax.toFloat(&IsOk2);
        if((Min < ParamRangesVal.at(0).x())|(Min > ParamRangesVal.at(0).y())|
           (Max < ParamRangesVal.at(1).x())|(Max > ParamRangesVal.at(1).y())|
           (Min >= Max)|(!IsOk1)|(!IsOk2))
        {
            emit sendSetColorToEdits(0,0,5);
            emit sendSetColorToEdits(1,0,5);
            //SetEditToColor(0, true);
            //SetEditToColor(1, true);
            eParamValue[0]->setText(FactoryParams.at(0));
            eParamValue[1]->setText(FactoryParams.at(1));
            teLog->append(" > Значение параметра №" + QString::number(Index) +
                          " некорректно, изменено на " + eParamValue[Index]->text());
            emit sendTemperatureBorder(FactoryParams.at(0), 0);
            emit sendTemperatureBorder(FactoryParams.at(1), 1);
            return;
        }
        else
        {
            if(eParamValue[0]->text() == FactoryParams[0])
                emit sendSetColorToEdits(0,0,0);
            else
                emit sendSetColorToEdits(0,0,3);
            if(eParamValue[1]->text() == FactoryParams[1])
                emit sendSetColorToEdits(1,0,0);
            else
                emit sendSetColorToEdits(1,0,3);
            //SetEditToColor(0, false);
            //SetEditToColor(1, false);
            teLog->append(" > Значение параметра №" + QString::number(Index) +
                          " изменено на " + eParamValue[Index]->text());
            emit sendTemperatureBorder(valueMin, 0);
            emit sendTemperatureBorder(valueMax, 1);
            return;
        }
    }
    // Все остальные параметры
    else
    {
        QString value = eParamValue[Index]->text();
        if(value.contains("ВЫКЛ", Qt::CaseInsensitive))
            value = "0";
        float val =value.toFloat(&IsOk1);
        if((val < ParamRangesVal.at(Index).x())|(val > ParamRangesVal.at(Index).y())|(!IsOk1))
        {
            emit sendSetColorToEdits(Index,0,5);
            //SetEditToColor(Index, true);
            eParamValue[Index]->setText(FactoryParams.at(Index));
            teLog->append(" > Значение параметра №" + QString::number(Index) +
                          " некорректно, изменено на " + eParamValue[Index]->text());
            return;
        }
        else
        {
            if(eParamValue[Index]->text() == FactoryParams[Index])
                emit sendSetColorToEdits(Index,0,0);
            else
                emit sendSetColorToEdits(Index,0,3);
            //SetEditToColor(Index, false);
            teLog->append(" > Значение параметра №" + QString::number(Index) +
                          " изменено на " + eParamValue[Index]->text());
            return;
        }
    }
}
// В полях ввода температур графика изменились данные
void ECL210A2661::onADataTest(quint32 Index, quint32 AIndex)
{
    if((Index!=19)|(AIndex <1)|(AIndex >6)) return;
    QString value =eParamGraph[AIndex-1]->text();
    bool IsOk;
    float T =value.toFloat(&IsOk);
    if((T < eParamValue[0]->text().toDouble())|(T > eParamValue[1]->text().toDouble())|(!IsOk))
    {
        teLog->append(" > Значение параметра точки графика №" + QString::number(AIndex) +
                      " некорректно, изменено на " + eParamValue[0]->text());
        emit sendSetColorToEdits(Index, AIndex-1,5);
        //SetEditToColor(Index, AIndex, true);
        eParamGraph[AIndex-1]->setText(eParamValue[0]->text());
        emit sendTemperature(eParamValue[0]->text(), AIndex);
    }
    else
    {
        teLog->append(" > Значение параметра точки графика №" + QString::number(AIndex) +
                      " изменено на " + value);
        if(eParamGraph[AIndex-1]->text() == FactoryTPointsHeat[AIndex-1])
            emit sendSetColorToEdits(Index,AIndex-1,0);
        else
            emit sendSetColorToEdits(Index,AIndex-1,3);
        //SetEditToColor(Index, AIndex);
        emit sendTemperature(value, AIndex);
    }
}
// Вывод значения точки температурного графика
void ECL210A2661::onGraphCurrentPointPrint(float Temp, quint32 Index)
{
    if(Index > 6)
        return;
    QStringList names;
    QString txt;
    names << "К1_Т1под(-30," << "К1_Т2под(-15," <<"К1_Т3под(-5," <<"К1_Т4под(0," <<"К1_Т5под(5," <<"К1_Т6под(15,";
    QString Temperature = QString::number(Temp, 'f', 1);
    lParamGraphCurrentPoint->setText(names.at(Index -1) + Temperature +")");
    // Запись значений в эдиты
    txt = QString::number(Temp, 'f',0);
    quint8 attr=0;
    if(txt != FactoryTPointsHeat[Index-1])
        attr=3;
    eParamGraph[Index -1]->setText(txt);
    emit sendSetColorToEdits(19,Index-1,attr);
    teLog->append(" > Значение параметра точки графика №" + QString::number(Index) +
                  " изменено на " + eParamGraph[Index -1]->text());
}
// Очистка лога
void ECL210A2661::onClearLog()
{
    teLog->clear();
}
// 16.04.2017 Перемещение скролла в конец текста
void ECL210A2661::onChangeLogPosition()
{
    QTextCursor c =  teLog->textCursor();
    c.movePosition(QTextCursor::End);
    teLog->setTextCursor(c);
}
// Запись лога в файл
void ECL210A2661::onSaveLog()
{
    // Сохранение лога
    QString text = teLog->toPlainText();
    QStringList log = text.split(">");
    for(int i=0; i < log.size(); i++)
        log[i].append("\r\n");
    text = log.join("");
    QString filename = "ECL210A2661_" + QString::number(ObjectID) + "_" + QString::number(EqNumber);
    QFile file;
    // Открытие диалога для возможности записи настроек
    QFileDialog *pFileDlg = new QFileDialog(this, tr("Укажите имя файла"), QCoreApplication::applicationDirPath(), "*.log");
    pFileDlg->setFileMode(QFileDialog::AnyFile);
    pFileDlg->setModal(true);
    filename = pFileDlg->getSaveFileName(this, tr("Укажите имя файла"),
                                         QCoreApplication::applicationDirPath() + "/" + filename, "*.log");//,
                                         //0, QFileDialog::DontUseNativeDialog);
    delete pFileDlg;
    if(filename.isEmpty())
    {
        teLog->append(" > Имя файла не введено");
        return;
    }
    // Если не указано расширение, то добавление его к имени
    if(filename.indexOf(".log") <0)
        filename +=".log";
    file.setFileName(filename);
    // Открытие файла и запись данных
    if(!file.open(QIODevice::WriteOnly))
    {
        emit sendMessageSlot("<div align = left><b><font color = black>"
                             "> Данные протокола не сохранены<br>"
                             "> Не удалось открыть файл данных</b></div>",
                             2);
        teLog->append(" > Данные не сохранены");
        teLog->append(" > Ошибка открытия файла");
        return;
    }
    QByteArray data;
    data.append(text);
    file.resize(0);
    file.write(data);
    file.close();
    teLog->append(" > Данные сохранены");
    return;
}

// Полная блокировка при ошибках
void ECL210A2661::onFullBlock()
{
    BlockControls();
    bServerMode->setEnabled(false);
    bCurrentSystem->setEnabled(false);
//    bReadAll->setEnabled(false);
    bConnectionSetup->setEnabled(false);
    lGeneralText->setStyleSheet("border: 2px solid black; background-color: #BF0000");
    lGeneralText->setText("<div align = center><b><font size =\"5\" color = white>Модуль заблокирован!</font></b></div>"
                          "<div align = left><b><font size =\"2\" color = white>"
                          "1. Исправьте ошибки и повторите запуск"
                          "</font></b></div>");
}
// Нажата кнопка смены режима контура
void ECL210A2661::onChangeSystem()
{
    if(CurrentSystem =="Heat")
    {
        CurrentSystem = "DHW";
        bCurrentSystem->setIcon(QIcon(":/DHWSystem.png"));
        bCurrentSystem->setIconSize(QSize(44,44));
        lMiniHeader[3]->setPixmap(QPixmap(":/DHW.png").scaled(18,12,Qt::KeepAspectRatio,Qt::SmoothTransformation));
        teLog->append(" > Смена контура с \"отопление\" на \"ГВС\"");
        ChangeSystemVisible(0);
    }
    else
    {
        CurrentSystem = "Heat";
        bCurrentSystem->setIcon(QIcon(":/HeatSystem.png"));
        bCurrentSystem->setIconSize(QSize(44,44));
        lMiniHeader[3]->setPixmap(QPixmap(":/Heat.png").scaled(18,12,Qt::KeepAspectRatio,Qt::SmoothTransformation));
        teLog->append(" > Смена контура с \"ГВС\" на \"отопление\"");
        ChangeSystemVisible(1);
    }
}
// 15.04.2017 Сигнал таймаута
void ECL210A2661::onServerTimeout()
{
    if(CheckTimeout == true)return;
    CheckTimeout = true;
    timer->blockSignals(true);
    CurrentTimer += (ServerDelay * 500);
    teLog->insertPlainText("..");
    // Проверка состояния
    if(!GetObjectMode())
    {
        emit sendCloseProgress();
        timer->stop();
        disconnect(timer, SIGNAL(timeout()), this, SLOT(ServerTimeout()));
        delete timer;
        timer =NULL;
        InProcessing = false; // Разрешение на удаление
        return;
    }
    mQuery->prepare("SELECT UserCommand FROM " + db->databaseName() + ".TObject WHERE ObjectID =:A AND number >0;");
    mQuery->bindValue(":A", ObjectID);
    if((!mQuery->exec())|(mQuery->size() <=0))
    {
        teLog->insertPlainText(" > ошибка запроса");
        teLog->append(" > Внимание произошла ошибка запроса");
        teLog->append(" > Проверьте соединение с базой данных и настройки");
        teLog->append(" > Возможна некорректная работа объекта");
        teLog->append(" > Восстановление режима объекта:");
        if(!ChangeObjectMode(CurrentWorkMode, CurrentUserCommand))
            teLog->append(" > Внимание данные объекта не восстановлены");
        else
            teLog->append(" > Данные восстановлены");
        emit sendCloseProgress();
        timer->stop();
        disconnect(timer, SIGNAL(timeout()), this, SLOT(onServerTimeout()));
        delete timer;
        timer =NULL;
        ChangeWorkMode = false;     // Разрешение кнопок и элементов
        InProcessing = false;       // Разрешение на удаление
        return;
    }
    QSqlRecord rec = mQuery->record();
    mQuery->next();
    QString UserCommand;
    UserCommand = mQuery->value(rec.indexOf("UserCommand")).toString();
    if(UserCommand != "START+OK")
    {
        teLog->insertPlainText("..");
        // Если задержка более задержка сервера + 60 сек, то на выход
        if(CurrentTimer > (ServerDelay * 1000 + 60000))
        {
            teLog->insertPlainText(" превышен таймаут");
            teLog->append(" > За необходимое время сервер не изменил режим работы объекта");
            teLog->append(" > Восстановление режима объекта:");
            if(!ChangeObjectMode(CurrentWorkMode, CurrentUserCommand))
                teLog->append(" > Внимание данные объекта не восстановлены");
            else
                teLog->append(" > Данные восстановлены");
            emit sendCloseProgress();
            timer->stop();
            disconnect(timer, SIGNAL(timeout()), this, SLOT(onServerTimeout()));
            delete timer;
            timer = NULL;
            ChangeWorkMode = false;     // Разрешение кнопок и элементов
            InProcessing = false;       // Разрешение на удаление
            return;
        }
        emit sendStepProgress();
        timer->blockSignals(false);
        CheckTimeout =false;
        return;
    }
    emit sendCloseProgress();
    teLog->insertPlainText(" завершено");
    teLog->append(" > Смена режима объекта произведена");
    timer->stop();
    disconnect(timer, SIGNAL(timeout()), this, SLOT(onServerTimeout()));
    delete timer;
    timer =NULL;
    InProcessing = false; // Разрешение на удаление
    // Повторный запуск обработки
    Configuration();
}
// 11.04.2017 Закрытие окна
void ECL210A2661::onCloseWindow()
{
    QCloseEvent* e = new QCloseEvent;
    QApplication::sendEvent(this, e);
}
// Получение настроек по выбранному адресу прибора
void ECL210A2661::onSelectedConfig(QString MBAddr)
{
    EqNumber = MBAddr.toUInt();
    lcbx->setEnabled(false);
    GetDefaultConfig();
}
// 17.04.2017 Проверка статусов, получение данных и вывод в едиты
void ECL210A2661::onCheckCommand()
{
    emit sendStepProgress();
    if(CheckTimeout == true)return;
    CheckTimeout = true;
    timer->blockSignals(true);
    CurrentTimer += (ServerDelay * 500);
    teLog->insertPlainText("..");
    // Если индекс не активен (код 100500), тогда остановка таймера и выход
    // Аналогично если данные команды отсутствуют
    if((InWork->IsNullIndex())|(InWork->workCommand.Params.isEmpty()))
    {
        // Если открыт то следует закрыть
        emit sendCloseProgress();
        timer->stop();
        disconnect(timer, SIGNAL(timeout()), this, SLOT(onCheckCommand()));
        delete timer;
        timer =NULL;
        InProcessing = false;
        BlockControls(false);
        InWork->setNullIndex();
        teLog->insertPlainText("завершено");
        teLog->append(" > Ожидание ответа от регулятора прервано");
        teLog->append(" > Данные рабочего параметра сброшены");
        teLog->append(" > При частом повторении данной ситуации свяжитесь с разработчиками");
        emit sendMessageSlot("<div align = left><b><font color = red> "
                             "> Системное сообщение:<br>"
                             "> информация о запрашиваемом/настраеваемом параметре отсутствует<br>"
                             "<font color = black>"
                             "> Ожидание ответа от регулятора прервано<br>"
                             "> Данные рабочего параметра сброшены<br>"
                             "> При частом повторении данной ситуации<br>"
                             "> свяжитесь с разработчиками</b></div>", 1);
        InChange =false;
        UpdateGraphPoints();
        return;
    }
    // Вычисление максимальной задержки
    uint sd = ServerDelay *4 *1000;
    uint ad = InWork->workCommand.Delay + 5000;
    uint delay =0;
    if(sd > ad)
        delay = sd;
    else
        delay = ad;
    QString Status = GetStatus(InWork->workCommand);
    quint8 attr =0;
    if(InWork->workCommand.Params.at(0).VarOffset ==6)
        attr =4;
    else if(InWork->workCommand.Params.at(0).VarOffset ==5)
        attr =5;
    // Статус не получен из БД
    if(Status.isEmpty())
    {
        // Если открыт то следует закрыть
        emit sendCloseProgress();
        emit sendSetColorToEdits(InWork->Index, InWork->TPointIndex, attr);
        teLog->insertPlainText("завершено");
        teLog->append(" > Статус выполняющейся команды CID:" + QString::number(InWork->workCommand.CID) +
                      " не получен");
        InWork->setNullIndex();
        timer->stop();
        disconnect(timer, SIGNAL(timeout()), this, SLOT(onCheckCommand()));
        delete timer;
        timer =NULL;
        InProcessing = false;
        BlockControls(false);
        emit sendMessageSlot("<div align = left><b><font color = red> "
                             "> Системное сообщение:<br>"
                             "> статус выполнения команды отсутствует в базе<br>"
                             "<font color = black>"
                             "> Ожидание ответа от регулятора прервано<br>"
                             "> Были уже получены, либо стерты<br>"
                             "> При частом повторении данной ситуации<br>"
                             "> свяжитесь с разработчиками</b></div>", 1);
        InChange =false;
        UpdateGraphPoints();
        return;
    }
    // Проверка статусов
    if(Status == "TIMEOUT")
    {
        teLog->insertPlainText("завершено");
        teLog->append(" > Статус выполняющейся команды CID(" + QString::number(InWork->workCommand.CID) +
                    + "):" + Status);
        teLog->append(" > Оборудование не ответило за отведенное время");
    }
    else if(Status == "ERROR")
    {
        teLog->insertPlainText("завершено");
        teLog->append(" > Статус выполняющейся команды CID(" + QString::number(InWork->workCommand.CID) +
                    + "):" + Status);
        teLog->append(" > Обнаружена ошибка в ответе от регулятора");
    }
    else if((Status!="COMPLETED")&(CurrentTimer > delay))
    {
        teLog->insertPlainText("завершено");
        teLog->append(" > Статус выполняющейся команды CID(" + QString::number(InWork->workCommand.CID) +
                    + "):" + Status);
        teLog->append(" > Ответ получен, время ожидания истекло");
        emit sendMessageSlot("<div align = left><b><font color = red> "
                             "> Системное сообщение:<br>"
                             "> команда не выполнена,<br>"
                             "> возможно отсутствует соединение<br>"
                             "<font color = black>"
                             "> Ожидание ответа от регулятора прервано<br>"
                             "> Указанное время истекло</b></div>", 1);
    }
    else if(Status=="COMPLETED")
    {
        teLog->insertPlainText("завершено");
        teLog->append(" > Статус выполняющейся команды CID(" + QString::number(InWork->workCommand.CID) +
                    + "):" + Status);
        // Получение ответа из БД
        QByteArray data;
        data.clear();
        data = GetAnswer(InWork->workCommand);
        if(!data.isEmpty())
        {
            bool IsOk;
            QString TypeCommand;
            QString txtval;
            quint8 atr =1;
            if(attr==4)
            {
                TypeCommand = "WRITE";
                atr =2;
            }
            else
                TypeCommand ="READ";
            qint16 val = InWork->GetValFromData(data, TypeCommand, InWork->workCommand, &IsOk);
            if(IsOk)
                txtval = InWork->GetTextFromVal(InWork->Index, InWork->TPointIndex, val, &IsOk);
            if(IsOk)
            {
                quint32 Index, GIndex;
                Index= GIndex=0;
                teLog->append(" > Параметр корректен");
                emit sendSetColorToEdits(InWork->Index, InWork->TPointIndex, atr);
                if((InWork->Index!=19)&(InWork->Index <=35))
                    eParamValue[InWork->Index]->setText(txtval);
                else if((InWork->Index==19)&(InWork->TPointIndex < 6))
                {
                    eParamGraph[InWork->TPointIndex]->setText(txtval);
                    TPointsValues << txtval;
                }
                // Если index =19 и не все точки графика отработаны, отправка на обработку следующего параметра
                if((InWork->Index ==19)&(InWork->TPointIndex < 5))
                {
                    teLog->append(" > Продолжение проса точек температурного графика");
                    emit sendMessageSlot("<div align = left><b><font color = black> "
                                         "> Параметр корректен<br>"
                                         "> Точка температурного графика №" + QString::number(InWork->TPointIndex+1) + " опрошена<br>"
                                         "> Идет продолжение опроса</b></div>", 2);
                    // Опрос будет продолжен, контролы не будут разблокированы
                    Index = InWork->Index;
                    GIndex= InWork->TPointIndex;
                }
                else
                {
                    teLog->append(" > Опрос завершен успешно");
                    emit sendMessageSlot("<div align = left><b><font color = black> "
                                         "> Параметр корректен<br>"
                                         "> Значение параметра " + ParamNames.at(InWork->Index) + " =" + txtval +"<br>"
                                         "> Опрос завершен</b></div>", 6);
                    BlockControls(false);
                }
                InWork->setNullIndex();
                emit sendCloseProgress();
                timer->stop();
                disconnect(timer, SIGNAL(timeout()), this, SLOT(onCheckCommand()));
                delete timer;
                timer =NULL;
                InProcessing = false;
                InChange = false;
                if((Index==19)&(GIndex < 5))
                    emit sendChangeParameter(Index, GIndex+1);
                else
                    UpdateGraphPoints();
                return;
            }
            teLog->append(" > Полученное значение параметра вне диапазона");
        }
    }
    else
    {
        // Статус не завершенный - продолжение
        timer->blockSignals(false);
        CheckTimeout = false;
        return;
    }
    // Некорректные значения, и статусы
    emit sendSetColorToEdits(InWork->Index, InWork->TPointIndex, attr);
    BlockControls(false);
    emit sendCloseProgress();
    timer->stop();
    disconnect(timer, SIGNAL(timeout()), this, SLOT(onCheckCommand()));
    delete timer;
    timer =NULL;
    InProcessing = false;
    InChange =false;
    InWork->setNullIndex();
    UpdateGraphPoints();
    return;
}

// Отправка сообщений в окно вывода или его создание
void ECL210A2661::onMessageDlg(QString txt, quint8 attr)
{
    CInfoDialog* pMB =this->findChild<CInfoDialog*>("ModuleMessage");
    if(pMB!=NULL){
        emit sendMessageDlg(txt,attr);
        return;
    }
    pMB = new CInfoDialog(this,txt,attr);
    pMB->setObjectName("ModuleMessage");
    pMB->show();
    connect(this, SIGNAL(sendMessageDlg(QString,quint8)), pMB, SLOT(onTextInsert(QString,quint8)), Qt::QueuedConnection);
    return;
}
// Окраска полей ввода в соответствующий цвет
// Подсветка параметров:
// 0. Заводские настройки - голубовато-серый
// 1. Прочитанныйе из регулятора - зеленый
// 2. Настройки записанные в регулятор и подтвержденные ответной командой - сине-серый
// 3. Измененные настройки, но не записанные - желтоватосерый
// 4. Настройки в ходе записи которых произошла ошибка - красно-серый
// 5. Настройки в ходе чтения которых произошла ошибка - оранжевый
// Те настройки значение которых введены некорректно исправляются автоматически и цвет восстанавливается через несколько секунд
void ECL210A2661::onSetColorToEdits(quint32 index, quint32 gindex, quint8 attribute)
{
    if(attribute > 5)return;
    if(index!=19)
        eParamValue[index]->setStyleSheet(EditParamStyles.at(attribute));
    else
        eParamGraph[gindex]->setStyleSheet(EditParamStyles.at(attribute));
}
// Отправка команды на запись параметра
void ECL210A2661::onChangeParameter(quint32 index, quint32 gindex)
{
    // Защита от повторного входа в функцию
    if(InChange)
        return;
    InChange = true;
    // Получение и проверка параметра из поля ввода
    QString textval;
    qint16 val;
    bool IsOk;
    CCommandDB command;
    // Если значение некорректно, то в поля будут записаны заводские настройки и попытка записать их в ECL
    if(index!=19)
    {
        // Если на чтение то не проверяется то что находится в полях
        if(!IsRead)
        {
            // Преобразование текстового параметра в формат команды
            textval = eParamValue[index]->text();
            val =InWork->GetValFromText(textval, index, &IsOk);
            // Переданное значение было некорректно и вернулось заводское значение
            if(!IsOk)
            {
                teLog->append(" > Неверное значение для записи параметра №" + QString::number(index));
                teLog->append(" > Передача команды отменена");
                emit sendSetColorToEdits(index, 0, 0);
                eParamValue[index]->setText(InWork->GetTextFromVal(index,gindex, val,&IsOk));
                InChange = false;
                return;
            }
            command = InWork->GetCommand("WRITE", index, gindex, val);
            eParamValue[index]->setText(InWork->GetTextFromVal(index, gindex, val,&IsOk));
        }
        else
            command = InWork->GetCommand("READ", index, gindex, val);
    }
    else
    {
        if(!IsRead)
        {
            textval = eParamGraph[gindex]->text();
            val =InWork->GetValFromText(textval, index, &IsOk);
            // Переданное значение было некорректно и вернулось заводское значение
            if(!IsOk)
            {
                teLog->append(" > Неверное значение для записи параметра №" + QString::number(index));
                teLog->append(" > Передача команды отменена");
                emit sendSetColorToEdits(index, gindex, 0);
                eParamGraph[gindex]->setText(InWork->GetTextFromVal(index,gindex, val,&IsOk));
                InChange = false;
                return;
            }
            command = InWork->GetCommand("WRITE", index, gindex, val);
            eParamGraph[gindex]->setText(InWork->GetTextFromVal(index,gindex, val,&IsOk));
        }
        else
            command = InWork->GetCommand("READ", index, gindex, val);
    }
    // Если команда не сформирована
    if(command.Params.isEmpty())
    {
        emit sendSetColorToEdits(index, gindex, 4);
        teLog->append(" > ModBus-команда для ECL-210 не получена");
        teLog->append(" > Отсутствует логический элемент описания в команде");
        InChange = false;
        return;
    }
    BlockControls(true);
    if(!SetCommand(command, index, gindex))
    {
        BlockControls(false);
        InChange = false;
        return;
    }
    InProcessing =true;  // На время опроса ,блокировка выхода из программы
    teLog->append(" > Ожидание выполнения команды:");
    teLog->append(" >..");
    CurrentTimer = 0;
    InWork->Index       =index;
    InWork->TPointIndex =gindex;
    InWork->workCommand = command;
    CheckTimeout = false;
    timer = new QTimer(this);
    uint sd, ed, d;
    sd = ServerDelay *4*1000;
    ed = command.Delay + 5000;
    if(sd > ed)
        d = sd;
    else
        d = ed;
    CProgress *progress = new CProgress(this, d, ServerDelay *500);
    progress->show();
    connect(this, SIGNAL(sendCloseProgress()), progress, SLOT(onClose()));
    connect(this, SIGNAL(sendStepProgress()), progress, SLOT(onStep()));
    connect(timer, SIGNAL(timeout()), this, SLOT(onCheckCommand()), Qt::QueuedConnection);
    timer->start(ServerDelay * 500);
    return;
}

// EVENTS
// Если нажали ESC
void ECL210A2661::keyPressEvent(QKeyEvent* key)
{
    switch(key->key())
    {
    case Qt::Key_Escape:
    {
        //key->accept();
        key->ignore();
        QCloseEvent* e = new QCloseEvent;
        QApplication::sendEvent(this, e);
        break;
    }
    default:
    {
        key->ignore();
        break;
    }
    }
}
void ECL210A2661::closeEvent(QCloseEvent *event)
{
    CInfoDialog* pMB =this->findChild<CInfoDialog*>("ModuleMessage");
    if(pMB!=NULL){
        disconnect(this, SIGNAL(sendMessageDlg(QString,quint8)), pMB, SLOT(onTextInsert(QString,quint8)));
        delete pMB;
    }
    if(FirstClose)
    {
        // Если идет процесс работы с сервером, то следует дождаться завершения
        if(InProcessing)
        {
            event->ignore();
            return;
        }
        FirstClose = false;
        // Если режим изменен то следует дождаться когда сервер будет восстановлен
        if(ChangeWorkMode == true)
        {
            if(timer!=NULL)
            {
                timer->stop();
                delete timer;
                timer=NULL;
            }
            teLog->append(" > Режим изменен, восстановление режима");
            ChangeObjectMode(CurrentWorkMode, CurrentUserCommand);
            DeleteCommands();
            ChangeCommand("", "_");
        }
        {
            if(mQuery!=NULL)
                delete mQuery;
            mQuery = NULL;
        }
    }
    event->accept();
    this->close();
}
