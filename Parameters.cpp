#include "Parameters.h"
// Функции единичного параметра
// Объект создается только если есть имя
CParam::CParam(QString name, quint32 objectid)
{
    IsInit = true;
    ParamName = name;
    Value = 0;
    ObjectID = objectid;
    if(ParamName == "HeatingCurveC1")
    {
        RamAddr      = 0xC9;
        EEPRomAddr   = 0x5D;
        CRead        = 0xCE;
        CWriteRAM    = 0xEE;
        CWriteEEPROM = 0xA0;
        TypeRegister = "8L";
    }
    else if(ParamName == "ParallelShiftC1")
    {
        RamAddr      = 0xA8;
        EEPRomAddr   = 0x4D;
        CRead        = 0xCE;
        CWriteRAM    = 0xFE;
        CWriteEEPROM = 0xB0;
        TypeRegister = "8H";
    }
    else if(ParamName == "FlowTemperaturMinC1")
    {
        RamAddr      = 0xCB;
        EEPRomAddr   = 0x5E;
        CRead        = 0xCE;
        CWriteRAM    = 0xEE;
        CWriteEEPROM = 0xA0;
        TypeRegister = "8L";
    }
    else if(ParamName == "FlowTemperaturMaxC1")
    {
        RamAddr      = 0xCC;
        EEPRomAddr   = 0x5F;
        CRead        = 0xCE;
        CWriteRAM    = 0xFE;
        CWriteEEPROM = 0xB0;
        TypeRegister = "8H";
    }
    else if(ParamName == "HotWaterDayTemperature")
    {
        RamAddr      = 0xD0;
        EEPRomAddr   = 0x61;
        CRead        = 0xCE;
        CWriteRAM    = 0xFE;
        CWriteEEPROM = 0xB0;
        TypeRegister = "8H";
    }
    else if(ParamName == "HotWaterNightTemperature")
    {
        RamAddr      = 0xD1;
        EEPRomAddr   = 0x61;
        CRead        = 0xCE;
        CWriteRAM    = 0xEE;
        CWriteEEPROM = 0xA0;
        TypeRegister = "8L";
    }
    else if(ParamName == "SummerCutOutC1")
    {
        RamAddr      = 0xC2;
        EEPRomAddr   = 0x5A;
        CRead        = 0xCE;
        CWriteRAM    = 0xFE;
        CWriteEEPROM = 0xB0;
        TypeRegister = "8H";
    }
    else if(ParamName == "SummerCutOutC2")
    {
        RamAddr      = 0xC3;
        EEPRomAddr   = 0x5A;
        CRead        = 0xCE;
        CWriteRAM    = 0xEE;
        CWriteEEPROM = 0xA0;
        TypeRegister = "8L";
    }
    else if(ParamName == "RoomTemperatureDayC1")
    {
        RamAddr      = 0x9E;
        EEPRomAddr   = 0x48;
        CRead        = 0xCE;
        CWriteRAM    = 0xFE;
        CWriteEEPROM = 0xB0;
        TypeRegister = "8H";
    }
    else if(ParamName == "RoomTemperatureNightC1")
    {
        RamAddr      = 0xA0;
        EEPRomAddr   = 0x49;
        CRead        = 0xCE;
        CWriteRAM    = 0xFE;
        CWriteEEPROM = 0xB0;
        TypeRegister = "8H";
    }
    else if(ParamName == "RoomTemperatureInfluenceMaxC1")
    {
        RamAddr      = 0x76;
        EEPRomAddr   = 0x34;
        CRead        = 0xCE;
        CWriteRAM    = 0xDE;
        CWriteEEPROM = 0x90;
        TypeRegister = "16";
    }
    else if(ParamName == "RoomTemperatureInfluenceMinC1")
    {
        RamAddr      = 0x78;
        EEPRomAddr   = 0x35;
        CRead        = 0xCE;
        CWriteRAM    = 0xDE;
        CWriteEEPROM = 0x90;
        TypeRegister = "16";
    }
    else if(ParamName == "ModeC1")
    {
        RamAddr      = 0x01;
        EEPRomAddr   = 0x00;
        CRead        = 0x11;
        CWriteRAM    = 0x00;
        CWriteEEPROM = 0x00;
        TypeRegister = "8H";      // При получении данные будут в младшем байте
    }
    else if(ParamName == "ModeC2")
    {
        RamAddr      = 0x02;
        EEPRomAddr   = 0x00;
        CRead        = 0x11;
        CWriteRAM    = 0x00;
        CWriteEEPROM = 0x00;
        TypeRegister = "8H";      // При получении данные будут в младшем байте
    }
    else if(ParamName == "PumpC1")
    {
        RamAddr      = 0x00;
        EEPRomAddr   = 0x00;
        CRead        = 0xCE;
        CWriteRAM    = 0x00;
        CWriteEEPROM = 0x00;
        TypeRegister = "8H";
    }
    else if(ParamName == "PumpC2")
    {
        RamAddr      = 0x00;
        EEPRomAddr   = 0x00;
        CRead        = 0xCE;
        CWriteRAM    = 0x00;
        CWriteEEPROM = 0x00;
        TypeRegister = "8H";
    }
    else
    {
        RamAddr      = 0x00;
        EEPRomAddr   = 0x00;
        CRead        = 0x00;
        CWriteRAM    = 0x00;
        CWriteEEPROM = 0x00;
        TypeRegister = "ERROR";
        IsInit = false;
    }
}
// Получение команды
CCommandDB CParam::GetCommand(QString command)
{
    CCommandDB Command;
    // Заполнение данных нулевой команды
    InitNullCommand(Command);
    QByteArray data;
    data.clear();
    if(((command !="READ")&(command !="WRITERAM")&(command !="WRITEROM"))|(IsInit == false))
        return Command;
    if(command == "READ")
    {
        // Подготовка команды
        char c[5] ="";
        c[0] =(char)CRead;    // Команда на чтение
        c[1] =(char)RamAddr;  // Адрес регистра
        c[2] =0x00;           // Не используется
        c[3] =0x00;           // Не используется
        GetMOD2_8bit((unsigned char*)c, (unsigned char*)(c+4), sizeof(c));
        data.append(c,sizeof(c));
        Command.Command = data;
        Command.CID = GetCID();
        Command.Params.append(GetCVarDBRead());
        return Command;
    }
    else if((command == "WRITERAM")|(command == "WRITEROM"))
    {
        // Подготовка команды
        char c[5] ="";
        if(command == "WRITERAM")
        {
            c[0] =(char)CWriteRAM;   // Команда на запись RAM
            c[1] =(char)RamAddr;     // Адрес регистра
        }
        else
        {
            c[0] =(char)CWriteEEPROM;// Команда на запись EEPROM
            c[1] =(char)EEPRomAddr;  // Адрес регистра
        }
        if(TypeRegister == "8H")
        {
            c[2] =Value;      // H-регистр
            c[3] = 0x00;
        }
        else if(TypeRegister == "8L")
        {
            c[2] = 0x00;
            c[3] =Value;      // L-регистр
        }
        else if(TypeRegister == "16")
        {
            char i =0;
            i = Value >> 8;   // Старшая часть регистра
            c[2] = i;
            i = Value << 8;
            i = Value >> 8;   // Младшая часть регистра
            c[3] = i;
        }
        else
            return Command;   // Если регистр не инициализирован то выход с пустой командой
        GetMOD2_8bit((unsigned char*)c, (unsigned char*)(c+4), sizeof(c));
        data.append(c,sizeof(c));
        Command.Command = data;
        Command.CID = GetCID();
        Command.Params.append(GetCVarDBRead());
        return Command;
    }
    return Command;
}
// Генерация контрольной суммы MOD2 (XOR) 8 бит
void CParam::GetMOD2_8bit(unsigned char *ARRAY,unsigned char *KS,unsigned long LENGTH)
{
    unsigned char kc =ARRAY[0];
    for(unsigned long i =1; i < LENGTH-1; i++)
    {
        kc ^= ARRAY[i];
    }
    *KS = kc;
}
// 16.04.2017 Инициализация пустой команды (ошибочной)
void CParam::InitNullCommand(CCommandDB &command)
{
    command.Attempts =1;
    command.CID = std::numeric_limits<quint32>::max();
    command.Command = "";
    command.CountCommand =1;
    command.DataCom =8;
    command.Delay =2500;     // Для ECL210A2661, 300 ожидание 2500
    command.EqName = "ECL210A2661";
    command.EqNumber = QString::number(ObjectID);
    command.EqNumberType = "quint32";
    command.Funct =1;
    command.ObjectID = ObjectID;
    command.ParityBits = 2;
    command.PortNum = 4;
    command.Priority = 2;
    command.RequestType = "CURRENT";
    command.SpeedCom = 1200;
    command.StopBits = 1;
    command.TypeCommand ="PLC";
    command.Params.clear();
}
// 16.04.2017 Получение CID в зависимости от имени параметра
quint32 CParam::GetCID()
{
    quint32 CID = std::numeric_limits<quint32>::max();
    if(ParamName == "HeatingCurveC1")
        return CID;
    else if(ParamName == "ParallelShiftC1")
        return CID-1;
    else if(ParamName == "FlowTemperaturMinC1")
        return CID-2;
    else if(ParamName == "FlowTemperaturMaxC1")
        return CID-3;
    else if(ParamName == "HotWaterDayTemperature")
        return CID-4;
    else if(ParamName == "HotWaterNightTemperature")
        return CID-5;
    else if(ParamName == "SummerCutOutC1")
        return CID-6;
    else if(ParamName == "SummerCutOutC2")
        return CID-7;
   else if(ParamName == "RoomTemperatureDayC1")
        return CID-8;
    else if(ParamName == "RoomTemperatureNightC1")
        return CID-9;
    else if(ParamName == "RoomTemperatureInfluenceMaxC1")
        return CID-10;
    else if(ParamName == "RoomTemperatureInfluenceMinC1")
        return CID-11;
    else if(ParamName == "ModeC1")
        return CID-12;
    else if(ParamName == "ModeC2")
        return CID-13;
    else if(ParamName == "PumpC1")
        return CID-14;
    else if(ParamName == "PumpC2")
        return CID-15;
    return CID;
}
// 16.04.2017 Получение параметра для всех типов при чтении параметра
CVarDB CParam::GetCVarDBRead()
{
    CVarDB Var;
    // Контрольная сумма
    Var.VarIndex =0;            // Индекс переменной
    // Разрешения параметра:
    //            "READ" - только обработка переменной без записи значения в БД, но при этом допускается запись тревог и сообщений
    //            "CREATE" - разрешается запись переменной в таблицу даже если столбец отсутствует, т.е. будет создана заново со столбцами настроек, в случае если переменная существует данные настроек будут считаны из БД
    //            "WRITE" - запись только в уже созданный столбец, данные настроек скопируются из БД
    //            "COMMAND" - команда конфигурирования или управления (аналогична "WRITE" и "CREATE" с перезаписью данных настроек)
    Var.VarPermit ="READ";
    Var.VarName = "MOD2";   // Имя столбца переменной в БД
    Var.VarType = "MOD2";     // Тип данных к которым следует преобразовать байты из входного массива
    Var.VarTypeSave ="MOD2";  // Формат в котором следует записать переменную в БД
    Var.VarOffset =4;           // Сдвиг от начала во входном массиве
    Var.VarData =1;             // Количество считываемых байт из входного массива
    Var.VarInsert ="1";       // Последовательность вставки байтов в переменную VarType
    Var.VarSensor_MIN ="-";     // Минимальное значение датчика типа VarTypeSave
    Var.VarSensor_MAX ="-";    // Максимальное значение датчика типа VarTypeSave
    Var.VarParam_MIN ="-";      // Минимальное значение параметра типа VarTypeSave
    Var.VarParam_MAX ="-";    // Максимальное значение параметра типа VarTypeSave
    Var.VarBorder_MIN ="-";   // Минимальный порог срабатывания типа VarTypeSave разделенный "/" при нескольких значениях
    Var.VarBorder_MAX ="-";     // Максимальный порог срабатывания типа VarTypeSave разделенный "/" при нескольких значениях
    Var.AlarmSet =0;            // Флаг включения сигнализации
    Var.AlarmMin = "Тревога по нижней границе";      // Текст сообщения при тревоги по низкому уровню разделенный "/" при нескольких значениях
    Var.AlarmMax = "Тревога по верхней границе";     // Текст сообщения при тревоги по высокому уровню разделенный "/" при нескольких значениях
    Var.SMSSet =0;              // Включение отправки СМС-при аварийном состоянии параметра

    // Номера телефонов на которые следует отправлять СМС в формате /*/*/12/12... - где первая * - (1 - отправлять, 0 - нет)
    //отправка на номера ответственных, вторая на номера операторов, дальше 12-значные номера дополнительных телефонов
    Var.Telephones ="/0/0";
    Var.StopFlag =1;       // Флаг остановки обработки ответа при ошибке параметра
    return Var;
}
// 17.04.2017 Получить текстовую строку из ответа
qint16 CParam::GetValFromData(QByteArray data, CCommandDB command, bool *IsOk)
{
    Value =0;
    if((command.Params.isEmpty())|(data.size()!=5))
    {
        *IsOk = false;
        return Value;
    }
    if((ParamName == "HeatingCurveC1")|
      (ParamName == "ParallelShiftC1")|
      (ParamName == "FlowTemperaturMinC1")|
      (ParamName == "FlowTemperaturMaxC1")|
      (ParamName == "HotWaterDayTemperature")|
      (ParamName == "HotWaterNightTemperature")|
      (ParamName == "SummerCutOutC1")|
      (ParamName == "SummerCutOutC2")|
      (ParamName == "RoomTemperatureDayC1")|
      (ParamName == "RoomTemperatureNightC1"))
    {
        char d = *(data.data() +2);
        Value ^=d;
        *IsOk = true;
        return Value;
    }
    else if((ParamName == "RoomTemperatureInfluenceMaxC1")|
            (ParamName == "RoomTemperatureInfluenceMinC1"))
    {
        char d = *(data.data() +2);
        Value ^=d;
        Value = Value << 8;
        d = *(data.data() +3);
        Value ^=d;
        *IsOk = true;
        return Value;
    }
    else if((ParamName == "ModeC1")|(ParamName == "ModeC2"))
    {
       char d = *(data.data() +3);
       Value ^=d;
       *IsOk = true;
       return Value;
    }
    else if(ParamName == "PumpC1")
    {
        char d = *(data.data() +2);
        d = d << 7;
        d = d >> 7;
        Value ^=d;
        *IsOk = true;
        return Value;
    }
    else if(ParamName == "PumpC2")
    {
        char d = *(data.data() +2);
        d <<= 5;
        d >>= 7;
        Value ^=d;
        *IsOk = true;
        return Value;
    }
    *IsOk = false;
    return Value;
}
// 18.04.2017 Получение параметра из текстовой строки
bool CParam::GetValFromEdit(QString text)
{
    if(text.isEmpty())
        return false;
    bool IsOk = true;
    // Преобразование данных
    // Наклон графика от 0.2 до 3.4
    if(ParamName == "HeatingCurveC1")
    {
        float f = text.toFloat(&IsOk);
        if(!IsOk)return false;
        f = f*10;
        Value =0;
        Value += f;
        if((Value < 2)|(Value > 34))
        {
            Value = 0;
            return false;
        }
        else
            return true;
    }
    // Паралельный сдвиг от -9 до 9
    else if(ParamName == "ParallelShiftC1")
    {
        Value = text.toInt(&IsOk);
        if((!IsOk)|(Value < -9)|(Value > 9))
        {
            Value =0;
            return false;
        }
        else
            return true;
    }
    // Минимальная температура подачи (10/110)
    else if(ParamName == "FlowTemperaturMinC1")
    {
        Value = text.toInt(&IsOk);
        if((!IsOk)|(Value < 10)|(Value > 110))
        {
            Value =0;
            return false;
        }
        else
            return true;
    }
    // Максимальная температура подачи (10/110)
    else if(ParamName == "FlowTemperaturMaxC1")
    {
        Value = text.toInt(&IsOk);
        if((!IsOk)|(Value < 10)|(Value > 110))
        {
            Value =0;
            return false;
        }
        else
            return true;
    }
    // Температура ГВС днем ED0/061h (10/110)
    else if(ParamName == "HotWaterDayTemperature")
    {
        Value = text.toInt(&IsOk);
        if((!IsOk)|(Value < 10)|(Value > 110))
        {
            Value =0;
            return false;
        }
        else
            return true;
    }
    // Температура ГВС ночью (10/110)
    else if(ParamName == "HotWaterNightTemperature")
    {
        Value = text.toInt(&IsOk);
        if((!IsOk)|(Value < 10)|(Value > 110))
        {
            Value =0;
            return false;
        }
        else
            return true;
    }
    // Отключение летний режим (10/30)
    else if(ParamName == "SummerCutOutC1")
    {
        Value = text.toInt(&IsOk);
        if((!IsOk)|(Value < 10)|(Value > 30))
        {
            Value =0;
            return false;
        }
        else
            return true;
    }
    // Отключение летний режим (10/30)
    else if(ParamName == "SummerCutOutC2")
    {
        Value = text.toInt(&IsOk);
        if((!IsOk)|(Value < 10)|(Value > 30))
        {
            Value =0;
            return false;
        }
        else
            return true;
    }
    // Комнатная температура днем (10/30)
    else if(ParamName == "RoomTemperatureDayC1")
    {
        Value = text.toInt(&IsOk);
        if((!IsOk)|(Value < 10)|(Value > 30))
        {
            Value =0;
            return false;
        }
        else
            return true;
    }
    // Комнатная температура ночью (10/30)
    else if(ParamName == "RoomTemperatureNightC1")
    {
        Value = text.toInt(&IsOk);
        if((!IsOk)|(Value < 10)|(Value > 30))
        {
            Value =0;
            return false;
        }
        else
            return true;
    }
    // Влияние комнатной температуры MAX (-99/0)
    else if(ParamName == "RoomTemperatureInfluenceMaxC1")
    {
        Value = text.toInt(&IsOk);
        if((!IsOk)|(Value < -99)|(Value > 0))
        {
            Value =0;
            return false;
        }
        else
            return true;
    }
    // Влияние комнатной температуры MIN (0/99)
    else if(ParamName == "RoomTemperatureInfluenceMinC1")
    {
        Value = text.toInt(&IsOk);
        if((!IsOk)|(Value < 0)|(Value > 99))
        {
            Value =0;
            return false;
        }
        else
            return true;
    }
    else
    {
        Value =0;
        return false;
    }
}
