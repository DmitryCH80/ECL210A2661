#ifndef ECL210A2661PARAMS_H
#define ECL210A2661PARAMS_H
#include <QStringList>
#include <QPoint>
#include <QDebug>
#include "CCommand.h"
#include "ECL210A2661Config.h"
class CRegAddr
{
public:
    CRegAddr(quint8 r8h=0,quint8 r8l=0)
    {
        R8H = r8h;
        R8L = r8l;
    }
    quint8 R8H, R8L;
};
// Подсветка параметров:
// 0. Заводские настройки - голубовато-серый
// 1. Прочитанныйе из регулятора - зеленый
// 2. Настройки записанные в регулятор и подтвержденные ответной командой - сине-серый
// 3. Измененные настройки, но не записанные - желтоватосерый
// 4. Настройки в ходе записи которых произошла ошибка - красно-серый
// 5. Настройки в ходе чтения которых произошла ошибка - оранжевый
extern QStringList ParametersInfo;
extern QStringList ParamNames;
extern QStringList ParamTexts;
extern QList<QPointF> ParamRangesVal;
extern QStringList ParamRanges;
extern QStringList FactoryParams;
extern QStringList FactoryTPointsHeat;
extern QStringList FactoryXTextScale;       // Температура нв от начала шкалы X до ее конца
extern QStringList FactoryYTextScale;       // Температура подачи от начала шкалы Y до ее конца
extern QStringList FactoryTPointsOut;       // Наружные температуры к точкам
extern qreal FactoryPointSide;              // Размер точки на графике
extern QList<CRegAddr> ParamRegisters;      // Адреса ModBus регистров регулятора, параметр [19] - не используется, см. ниже
extern QList<CRegAddr> ParamTPointRegisters;// Адреса ModBus регистров регулятора графика температур
extern QList<qint16> FactoryParamsInt;      // Заводские значения параметров в формате регулятора
extern QList<qint16> FactoryTPointsHeatInt; // Заводские значения параметров графика в формате регулятора
extern QStringList EditParamStyles;         // Стили полей ввода в зависимости от  текущего состояния параметра

// Класс 1 параметра ECL-210 Comfort A266.1
class CParam
{
public:
    // Индекс - по перечням настройки параметра, конфигурация - настройки для формирования команд
    explicit CParam(ECL210A2661Config *pconfig, quint32 pIndex, quint32 pTPointIndex =0);
    CCommandDB GetCommand(QString cType,
                          quint32 index,
                          quint32 gindex,
                          qint16 value=0);                // Получение команды по ее типу (READ, WRITE)
    void InitNullCommand(CCommandDB &command);            // Инициализация пустой команды
    void GetCRC16(unsigned char *ARRAY,
                  unsigned char *KS_H,
                  unsigned char *KS_L,
                  unsigned long LENGHT);                  // Генерация CRC16
    CVarDB GetCVarDBRead();                               // Для всех параметров одинаковый (проверяется тольк КС)
    CVarDB GetCVarDBWrite();                              // Для всех параметров одинаковый (проверяется тольк КС)
    qint16 GetValFromData(QByteArray data,
                          QString TypeCommand,
                          CCommandDB command,
                          bool *IsOk);                    // Получить значение из ответа на команду
    qint16 GetValFromText(QString text,
                          quint32 index,
                            bool *IsOk);                  // Получение параметра из текстовой строки и преобразование его к форму для передачи регулятору
    bool TestValRange(quint32 index, qint16 val);         // Проверка в диапазоне ли параметр
    QString GetTextFromVal(quint32 index,
                           quint32 gindex,
                           qint16 mval,
                           bool *IsOk);                   // Получение текстовой строки из значения параметра, если значение неверное, то заводские
    quint32 GetCID(quint32 index, quint32 gindex);        // Получение свободного CID
    void setNullIndex()                                   // Нулевой индекс установка
    {
        Index =100500;
        TPointIndex =0;
        workCommand = CCommandDB();                       // Сброс данных команды
    }
    bool IsNullIndex()                                    // Проверка нулевого индекса
    {
        if(Index ==100500)
            return true;
        return false;
    }
    CCommandDB workCommand;                               // Объект хранения данных на время опроса
    bool IsInit;                                          // Инициализирован ли параметр
    quint8 AddrH;                                         // Адрес регистра старшая часть
    quint8 AddrL;                                         // Адрес регистра младшая часть
    quint32 Index;                                            // Индекс параметра
    quint32 TPointIndex;                                      // Индекс точки температурного графика (только для графика)
    ECL210A2661Config *pConfig;
};
#endif // ECL210A2661PARAMS_H
