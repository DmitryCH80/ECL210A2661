#ifndef ECL210A2661_H
#define ECL210A2661_H

#include "ECL210A2661_global.h"
#include <QDialog>
#include "CCommand.h"
#include "General.h"
#include <QPushButton>
#include <QGridLayout>
#include <QKeyEvent>
#include <QCloseEvent>
#include <QApplication>
#include <QTextEdit>
#include <QLineEdit>
#include <QLabel>
#include <QScrollArea>
#include <QScrollBar>
#include <limits>
//#include <QMessageBox>
#include <QTimer>
#include <QColor>
#include <QFileDialog>
#include <QComboBox>
#include <QDebug>
#include <QMutex>
#include "CInfoDialog.h"
#include "CTempGraph.h"
#include "CProgress.h"
#include "ECL210A2661Config.h"
#include "ECL210A2661ConfigDlg.h"
#include "ECL210A2661Params.h"

extern float DriverVer;                               // Версия драйвера для проверки конфигурации

// Кнопка объектов
class CMButton : public QPushButton
{
    Q_OBJECT
public:
    explicit CMButton(quint32 index, quint32 type, QIcon ico, QWidget *parent) : QPushButton(ico, "", parent)
    {
        Index = index;
        Type  = type;
        connect(this, SIGNAL(clicked()), this, SLOT(onChange()));
    }
    quint32 Index;             // Номер при вызове
    quint32 Type;              // Тип чтение/запись/справка
public slots:
    void onChange(){emit sendChange(Index, Type);}
signals:
    void sendChange(quint32, quint32);  // Отправка сигнала нажатия с индексом кнопки
};
// Эдит передающий при завершении сигнал со своим индексом
class CMEdit : public QLineEdit
{
    Q_OBJECT
public:
    explicit CMEdit(quint32 index, QWidget *parent) : QLineEdit(parent)
    {
        Index = index;
        aIndex =0;
        connect(this, SIGNAL(editingFinished()), this, SLOT(onDataChange()));
    }
    quint32 aIndex;            // Доп номер для темп. графика
    quint32 Index;             // Номер при вызове
public slots:
    void onDataChange()
    {
        if(aIndex ==0)
            emit sendDataChange(Index);
        else
            emit sendADataChange(Index, aIndex);
    }
signals:
    void sendDataChange(quint32);  // Отправка сигнала с индексом
    void sendADataChange(quint32, quint32);
};


class ECL210A2661_EXPORT ECL210A2661  : public QDialog
{
    Q_OBJECT
public:
    QStringList TPointsValues;                     // Значения точек температуры для обновления
    QString CurrentSystem;
    ECL210A2661Config Config;                      // Объект настроек
    QStringList lModBusAddresses;                  // Адреса аналогичных устройств данного объекта
    bool FirstClose;
    bool ChangeWorkMode;                           // Изменен ли режим сервера
    bool InProcessing;                             // Модуль в процессе работы ВНИМАНИЕ не закрывать во время работы
    bool IsRead;                                   // Флаг чтения, если нет, то запись
    bool CheckTimeout;
    bool InChange;                                 // Идет процесс записи параметра
    CParam *InWork;                                // Указатель на параметр, который в работе
    quint32 ObjectID;
    quint32 EqNumber;
    uint ServerDelay;
    QString UserString;
    QString CurrentUserCommand;
    QString CurrentWorkMode;
    int FontSize;
    QSqlDatabase *db;
    QSqlQuery *mQuery;

    QTimer *timer;
    uint CurrentTimer;                           // Счетчик времени
    QLabel *lGeneralText;                        // Общий текст
    // Выбор сетевого номера прибора
    QComboBox *lcbx;                             // Выбор объекта, если не один
    // Лэйблы основных кнопок
    QLabel *lServerMode;                         // Текст над кнопкой смены режима сервера
    QLabel *lCurrentSystem;                      // Текст над кнопкой выбора контура
    //QLabel *lReadAll;                            // Текст над кнопкой "прочитать все настройки"
    // Основные кнопки
    QPushButton *bServerMode;                    // Кнопка смены режима сервера
    QPushButton *bCurrentSystem;                 // Кнопка выбора контура
    //QPushButton *bReadAll;                       // Кнопка "прочитать все настройки"
    QPushButton *bConnectionSetup;               // Кнопка настройки соединения
    // Заголовки в окнах
    QLabel *lMiniHeader[4];                      // 1-й Параметры, 2-й -описание подробное, 3-й лог
    QTextEdit *teLog, *teParameterInfo;          // Поля вывода лога и информации о параметрах
    // Кнопки лога
    QPushButton *bSaveLog, *bClearLog;           // Кнопки очистки лога и сохранения в файл
    // Заголовки параметров
    QLabel *lParamHeader[35];                    // 34 одинаковых и 6 температурного графика
    QLabel *lParamDBName[35];                    // Имена параметров в БД
    QLabel *lParamRange[35];                     // Лэйбл диапазона параметров мин/макс
    // Кнопки параметров Прочитать/Записать/Справка
    CMButton *bParamButton[35][3];               // Кнопки параметров
    // Виджеты отопления и ГВС
    QWidget *container, *minicontainer[35];
    // Общий виджет параметров
    QScrollArea *myScrollBox;
    // Поля ввода параметров
    CMEdit *eParamValue[35];                     // 35 параметров без температурного графика (19 параметр пропускается)
    // Дополнительные поля температурного графика
    QLabel *lParamGraphNumberHeader, *lParamGraphToutHeader, *lParamGraphCurrentPoint;
    QLabel *lParamGraphNumber[6], *lParamGraphTout[6];
    CMEdit *eParamGraph[6];                      // Эдиты ввода температур подачи графика
    CTempGraph *wTempGraph;


    ECL210A2661(QWidget* pwgt, Qt::WindowFlags f, quint32 objectid, QSqlDatabase *base, QString userlogin, bool &IsOk);
    void SetLabelText(int index);                // Установка текстовых значений по умолчанию (заводские)
    void ChangeSystemVisible(bool t);            // Изменение видимости элементов систем при смене настраиваемой системы
    void SetEditToColor(quint32 Index,
                        bool IsAlarm =false,
                        QColor color =QColor()); // Окраска полей ввода в нужные цвета
    void SetEditToColor(quint32 Index,
                        quint32 AIndex,
                        bool IsAlarm =false,
                        QColor color = QColor());
    bool GetUserData();
    bool GetDefaultConfig();                     // Получение настроек соединения с регулятором по данным полученным при открытии окна
    bool CreateCommandStatusTable();
    bool IsDBConnected();                        // Проверка состояния соединения
    void Configuration();                        // Начало установки
    bool ChangeCommand(QString NewPrefix,
                       QString OldPrefix);       // Изменение перечня команд установкой префикса
    bool GetObjectMode();                        // Получение режима работы сервера и команды пользователя
    bool ChangeObjectMode(QString WorkMode,
                          QString UserCommand);  // Запись нового режима работы объекта
    void ServerModeChange(bool Mode);            // Разблокировка/блокировка элементов ввода данных
    bool RunObject();                            // Запуск потока
 //   bool GetAllParameters(QSqlQuery *mQuery);  // Получение всех параметров
    bool SetCommand(CCommandDB command,
                    quint32 index,
                    quint32 gindex =0);          // Запись команды в БД
    void SetTextToEdit(quint32 index,
                       quint32 gindex =0,
                       QString text ="");        // Установка текста параметра
    bool SetStatus(CCommandDB command,
                        QString status);         // Установка статусов READY в TCommandStat для отслеживания
    QString GetStatus(CCommandDB command);       // Получение статусов из TCommandStat для отслеживания выполнения
    QByteArray GetAnswer(CCommandDB command);    // Получение данных из TCommandStat
    bool DeleteCommands();                       // Удаление команд управления и очистка статусов
    void BlockControls(bool Block=true);         // Блокировка едитов на время обработки
    void UpdateGraphPoints();                    // Обновление точек графика после блокировки виджета
 //   QLineEdit* GetLineEdit(quint32 index);     // Получение едита по индексу
public slots:
    void onSetColorToEdits(quint32 index,
                           quint32 gindex,
                           quint8 attribute);    // Окраска полей ввода в соответствующий цвет
    void onMessageDlg(QString txt, quint8 attr); // Открытие окна с сообщениями
    void onSelectedConfig(QString MBAddr);       // Получение настроек по выбранному адресу прибора
    void onConnectionSetup();                    // Настройка соединения с прибором
    void onConnectionSetupFinished();            // После завершения настройки соединения вывод в лог и на создание параметров и команд
    void onChangeServerMode();                   // Обработка нажатия кнопки изменения режима
    void onChangeSystem();                       // Смена контура настройки
    void onReadAllParameters();                  // Чтение всех параметров
    void onReadParameter(quint32 Index,
                         quint32 Type);          // Чтение параметра с заданным номером
    void onSaveParameter(quint32 Index,
                         quint32 Type);          // Запись параметра с заданным номером
    void onInfoParameter(quint32 Index,
                         quint32 Type);          // Информация о параметре с заданным номером
    void onDataTest(quint32 Index);              // Проверка введенных данных
    void onADataTest(quint32 Index,
                     quint32 AIndex);
    void onGraphCurrentPointPrint(float Temp,
                                quint32 Index);  // Вывод значения точки температурного графика
    void onClearLog();                           // Очистка лога
    void onSaveLog();                            // Запись лога в файл
    void onChangeLogPosition();                  // Перемещение скролла в конец вывода
    void onCloseWindow();                        // Слот закрытия приложения
    void onServerTimeout();                      // Проверка отработки смены режима
    void onFullBlock();                          // Полная блокировка при ошибках
    void onCheckCommand();                       // Проверка статуса текущего параметра, проверка данных команды, вывод в едиты
    void onChangeParameter(quint32 index,
                           quint32 gindex);      // Изменение 1 параметра запись
signals:
    void sendSetColorToEdits(quint32,
                             quint32,
                             quint8);            // Окраска полей ввода в соответствующий цвет
    void sendChangeParameter(quint32,quint32);   // Отправка сигнала на изменение параметра
    void sendReadParameter(quint32,quint32);     // Сигнал на чтение параметра
    void sendCloseProgress();                    // Закрытие прогресс виджета
    void sendClose();                            // Отправка сигнала на закрытие окна
    void sendStepProgress();                     // Сигнал добавить в прогресс виджет
    void sendTemperature(QString, quint32);
    void sendTemperatureBorder(QString, quint32);
    void sendFullBlock();
    void sendMessageDlg(QString, quint8);        // Передача текста в окно с сообщениями
    void sendMessageSlot(QString, quint8);       // Передача на обработчик работы с окном
protected:
    virtual void closeEvent(QCloseEvent* event);
    virtual void keyPressEvent(QKeyEvent* key);
};

extern "C" ECL210A2661_EXPORT QDialog* Show(QWidget* pwgt, Qt::WindowFlags f,
                                      quint32 objectid, QSqlDatabase *base, QString userlogin);

extern "C" ECL210A2661_EXPORT QString GetInfoDll(QString &data);
#endif // ECL210A2661_H
