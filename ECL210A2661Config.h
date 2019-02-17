#ifndef ECL210A2661CONFIG_H
#define ECL210A2661CONFIG_H
#include <QtGlobal>
#include <QString>
#include <QStringList>
#include <QDataStream>
#include <QFile>
#include <QCoreApplication>
#include <QDebug>
class ECL210A2661Config
{
public:
    // Настройки по умолчанию (версия драйвера и ID объекта обязательны)
    ECL210A2661Config()
    {
        ConfigName = "ERROR";
        PortSpeed  = 0;
        ComNumber  = 0;
        ModBusAddr = "0";
        ObjectID   = 0;
        DllVer     = 0;
        EqDelay    = 0;
        ConfigStatus = false;
    }
    ECL210A2661Config(quint32 mObjectID, float mDllVer, QString mModBusAddr ="1")
    {
        DllVer = mDllVer;
        ConfigStatus = false;
        bool IsOk;
        PortSpeed = 38400;
        ComNumber = 2;
        EqDelay   = 2000;
        ModBusAddr = mModBusAddr;
        quint8 addr = ModBusAddr.toUInt(&IsOk);
        ObjectID =mObjectID;
        // Если настройки неправильные, то создается объект со статусом false
        if((DllVer <1.01)|(ObjectID <=1)|(addr <1)|(addr > 247)|(!IsOk))
        {
            qDebug() << " > ECL210A2661Config::ECL210A2661Config: настройки некорректны";
            if(DllVer <1.01)
                qDebug() << " > DllVer:" << QString::number(DllVer, 'f', 2);
            if(ObjectID <=1)
                qDebug() << " > ObjectID:" << QString::number(ObjectID);
            if((addr <1)|(addr > 247)|(!IsOk))
                qDebug() << " > ModBusAddr:" << QString::number(addr);
            ConfigName = "ERROR";
            PortSpeed  = 0;
            ComNumber  = 0;
            ModBusAddr = "0";
            ObjectID   = 0;
            DllVer     = 0;
            EqDelay    = 0;
            return;
        }
        ConfigStatus = true;
        ConfigName = "ECL210A2661_" + QString::number(DllVer, 'f', 3) + "_" +
                     QString::number(ObjectID) + "_" + ModBusAddr;
        // Попытка прочитать настройки из файла
        ReadConfig(ConfigName);
    }
    QString ConfigName;                           // Имя конфигурации содержащее ID-объекта, версию драйвера, сетевой
    quint32 PortSpeed;                            // Скорость работы порта 9600, 19200, 38400 (формат БД)
    quint8 ComNumber;                             // Номер порта RS485 для опроса прибора (формат БД)
    QString ModBusAddr;                           // Сетевой адрес прибора (формат БД)
    quint32 ObjectID;                             // Идентификатор объекта (формат БД)
    quint32 EqDelay;                              // Задержка при опросе
    float DllVer;                                 // Версия DLL не менее 1.01
    bool ConfigStatus;                            // Состояние настройки, если false - то ошибка, иначе - норма
    QByteArray serializer(ECL210A2661Config config)
    {
        QByteArray byteArray;
        // Если пустая то на выход
        byteArray.clear();
        byteArray.constData();                    // Для подстраховки от отсутствия символа
        QDataStream out(&byteArray, QIODevice::WriteOnly);
        if(!config.ConfigStatus)                  // Если статус ошибочный, то возвращается пустой массив
            return byteArray;
        out << config.ConfigName << config.PortSpeed << config.ComNumber << config.ModBusAddr
            << config.ObjectID << config.DllVer << config.EqDelay;
        return byteArray;
    }
    ECL210A2661Config deserializer(QByteArray byteArray, QString mConfigName)
    {
        QStringList ldata;
        bool IsOk =true;
        float ver = 0;
        quint32 id = 0;
        quint8 addr = 0;
        ldata.clear();
        ldata = mConfigName.split("_");
        if(ldata.size()!=4)
            IsOk =false;
        if(IsOk)
        {
            bool IsFloat, IsQuint32, IsQuint8;
            ver = ldata.at(1).toFloat(&IsFloat);
            id = ldata.at(2).toUInt(&IsQuint32);
            addr = ldata.at(3).toUInt(&IsQuint8);
            if((!IsFloat)|(!IsQuint32)|(!IsQuint8)|(ver < 1.01)|(id <2)|(addr <1)|(addr > 247)|(ldata.at(0)!="ECL210A2661"))
                IsOk = false;
        }
        if(!IsOk)
            return ECL210A2661Config();
        // Образцовый объект для проверки
        ECL210A2661Config cmpconfig, config;
        cmpconfig.ObjectID =id;
        cmpconfig.DllVer = ver;
        cmpconfig.ModBusAddr =ldata.at(3);
        cmpconfig.ConfigStatus = false;
        cmpconfig.ConfigName = "ECL210A2661_" + QString::number(cmpconfig.DllVer, 'f', 3) + "_" +
                     QString::number(cmpconfig.ObjectID) + "_" + cmpconfig.ModBusAddr;
        // Аналогичный для заполнения из потока
        config = cmpconfig;
        // Если пустая то на выход
        if(byteArray.isEmpty())return cmpconfig;
        byteArray.constData(); // Для подстраховки от отсутствия символа
        QDataStream in(&byteArray, QIODevice::ReadOnly);
        in >> config.ConfigName >> config.PortSpeed >> config.ComNumber >> config.ModBusAddr
            >> config.ObjectID >> config.DllVer >> config.EqDelay;
        // Проверка основных данных
        if((config.ConfigName!=cmpconfig.ConfigName)|                    // Имена должны совпадать
           (config.DllVer!=cmpconfig.DllVer)|                            // Версии также
           (config.ModBusAddr!=cmpconfig.ModBusAddr)|                    // Адреса тоже
           (config.ObjectID!=cmpconfig.ObjectID))                        // Идентификатор объекта
        {
            qDebug() << " > ECL210A2661Config::deserializer: настройки не совпадают";
            if(config.DllVer!=cmpconfig.DllVer){
                qDebug() << " > File DllVer:" << QString::number(config.DllVer, 'f', 2);
                qDebug() << " > Must DllVer:" << QString::number(cmpconfig.DllVer, 'f', 2);
            }
            if(config.ObjectID!=cmpconfig.ObjectID){
                qDebug() << " > File ObjectID:" << QString::number(config.ObjectID);
                qDebug() << " > Must ObjectID:" << QString::number(cmpconfig.ObjectID);
            }
            if(config.ModBusAddr!=cmpconfig.ModBusAddr){
                qDebug() << " > File ModBusAddr:" << config.ModBusAddr;
                qDebug() << " > Must ModBusAddr:" << cmpconfig.ModBusAddr;
            }
            if(config.ConfigName!=cmpconfig.ConfigName){
                qDebug() << " > File ConfigName:" << config.ConfigName;
                qDebug() << " > Must ConfigName:" << cmpconfig.ConfigName;
            }
            return cmpconfig;
        }
        // Проверка данных скорости, задержки и номера порта
        if((config.ComNumber <1)|
           ((config.PortSpeed!=9600)&(config.PortSpeed!=19200)&(config.PortSpeed!=38400))|
           (config.EqDelay <100)|(config.EqDelay >60000))
        {
            if(config.ComNumber <1)
            {
                qDebug() << " > File ComNumber:" <<  QString::number(config.ComNumber);
                qDebug() << " > Must ComNumber range 1...255";
            }
            if((config.PortSpeed!=9600)&(config.PortSpeed!=19200)&(config.PortSpeed!=38400))
            {
                qDebug() << " > File ComSpeed:" <<  QString::number(config.PortSpeed);
                qDebug() << " > Must ComSpeed 9600, 19200 or 38400";
            }
            if((config.EqDelay <100)|(config.EqDelay >60000))
            {
                qDebug() << " > File ECL Delay:" <<  QString::number(config.EqDelay);
                qDebug() << " > Must ECL Delay range 100...60000ms";
            }
            return cmpconfig;
        }
        config.ConfigStatus = true;
        return config;
    }
    // Чтение настройки из файла
    ECL210A2661Config ReadConfig(QString mConfigName)
    {
        QString filename = mConfigName + ".bin";
        QFile file;
        file.setFileName(QCoreApplication::applicationDirPath() + "/config/" + filename);
        // Файл не создан - возврат с ошибочным статусом
        if(!file.exists())
            return ECL210A2661Config();
        // Файл не открывается - возврат с ошибочным статусом
        if(!file.open(QIODevice::ReadOnly))
            return ECL210A2661Config();
        QByteArray data = file.readAll();
        file.close();
        ECL210A2661Config conf = deserializer(data, mConfigName);
        if(conf.ConfigStatus)
        {
            ConfigName = conf.ConfigName;
            PortSpeed  = conf.PortSpeed;
            ComNumber  = conf.ComNumber;
            ModBusAddr = conf.ModBusAddr;
            ObjectID   = conf.ObjectID;
            DllVer     = conf.DllVer;
            EqDelay    = conf.EqDelay;
        }
        return conf;
    }
    // Запись конфигурации
    bool SaveConfig()
    {
        if(!ConfigStatus)return false;
        QString filename = ConfigName + ".bin";
        QFile file;
        file.setFileName(QCoreApplication::applicationDirPath() + "/config/" + filename);
        // Файл не создан - возврат с ошибочным статусом
        //if(!file.exists())
        //    return false;
        // Файл не открывается - возврат с ошибочным статусом
        if(!file.open(QIODevice::WriteOnly))
            return false;

        QByteArray data = serializer(*this);
        if(data.isEmpty())
            return false;
        file.resize(0);
        file.write(data);
        file.close();
        return true;
    }
};
#endif // ECL210A2661CONFIG_H
