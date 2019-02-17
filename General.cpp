#include "General.h"
QDataStream &operator<<(QDataStream &out, const CVarDB &myObj)
{
    out   << myObj.VarIndex      // Индекс переменной
          << myObj.VarPermit
          << myObj.VarName       // Имя столбца переменной в БД
          << myObj.VarType       // Тип данных к которым следует преобразовать байты из входного массива
          << myObj.VarTypeSave   // Формат в котором следует записать переменную в БД
          << myObj.VarOffset     // Сдвиг от начала во входном массиве
          << myObj.VarData       // Количество считываемых байт из входного массива
          << myObj.VarInsert     // Последовательность вставки байтов в переменную VarType
          << myObj.VarSensor_MIN // Минимальное значение датчика типа VarTypeSave
          << myObj.VarSensor_MAX // Максимальное значение датчика типа VarTypeSave
          << myObj.VarParam_MIN  // Минимальное значение параметра типа VarTypeSave
          << myObj.VarParam_MAX  // Максимальное значение параметра типа VarTypeSave
          << myObj.VarBorder_MIN // Минимальный порог срабатывания типа VarTypeSave разделенный "/" при нескольких значениях
          << myObj.VarBorder_MAX // Максимальный порог срабатывания типа VarTypeSave разделенный "/" при нескольких значениях
          << myObj.AlarmSet      // Флаг включения сигнализации
          << myObj.AlarmMin      // Текст сообщения при тревоги по низкому уровню разделенный "/" при нескольких значениях
          << myObj.AlarmMax      // Текст сообщения при тревоги по высокому уровню разделенный "/" при нескольких значениях
          << myObj.SMSSet        // Включение отправки СМС-при аварийном состоянии параметра
          << myObj.Telephones
          << myObj.StopFlag;     // Флаг остановки обработки ответа при ошибке параметра
    return out;
}
QDataStream &operator>>(QDataStream &in, CVarDB &myObj)
{
    in >> myObj.VarIndex      // Индекс переменной
       >> myObj.VarPermit
       >> myObj.VarName       // Имя столбца переменной в БД
       >> myObj.VarType       // Тип данных к которым следует преобразовать байты из входного массива
       >> myObj.VarTypeSave   // Формат в котором следует записать переменную в БД
       >> myObj.VarOffset     // Сдвиг от начала во входном массиве
       >> myObj.VarData       // Количество считываемых байт из входного массива
       >> myObj.VarInsert     // Последовательность вставки байтов в переменную VarType
       >> myObj.VarSensor_MIN // Минимальное значение датчика типа VarTypeSave
       >> myObj.VarSensor_MAX // Максимальное значение датчика типа VarTypeSave
       >> myObj.VarParam_MIN  // Минимальное значение параметра типа VarTypeSave
       >> myObj.VarParam_MAX  // Максимальное значение параметра типа VarTypeSave
       >> myObj.VarBorder_MIN // Минимальный порог срабатывания типа VarTypeSave разделенный "/" при нескольких значениях
       >> myObj.VarBorder_MAX // Максимальный порог срабатывания типа VarTypeSave разделенный "/" при нескольких значениях
       >> myObj.AlarmSet      // Флаг включения сигнализации
       >> myObj.AlarmMin      // Текст сообщения при тревоги по низкому уровню разделенный "/" при нескольких значениях
       >> myObj.AlarmMax      // Текст сообщения при тревоги по высокому уровню разделенный "/" при нескольких значениях
       >> myObj.SMSSet        // Включение отправки СМС-при аварийном состоянии параметра
       >> myObj.Telephones
       >> myObj.StopFlag;     // Флаг остановки обработки ответа при ошибке параметра
    return in;
}
QDataStream& operator<<(QDataStream& s, const QList<CVarDB>& l)
{
    s << quint32(l.size());
    for (int i = 0; i < l.size(); ++i)
        s << l.at(i);
    return s;
}

QDataStream& operator>>(QDataStream& s, QList<CVarDB>& l)
{
    l.clear();
    quint32 c;
    s >> c;
    l.reserve(c);
    for(quint32 i = 0; i < c; ++i)
    {
        CVarDB t;
        s >> t;
        l.append(t);
        if (s.atEnd())
            break;
    }
    return s;
}
// Разбор на массив байт
QByteArray serialize(QStringList stringList)
{
  QByteArray byteArray;
  // Если пустая то на выход
  byteArray.clear();
  if(stringList.isEmpty())return byteArray;
  byteArray.constData(); // Для подстраховки от отсутствия символа
  QDataStream out(&byteArray, QIODevice::WriteOnly);
  out << stringList;
  return byteArray;
}
QByteArray serializec(QList<CVarDB> List)
{
  QByteArray byteArray;
  // Если пустая то на выход
  byteArray.clear();
  if(List.isEmpty())return byteArray;
  byteArray.constData(); // Для подстраховки от отсутствия символа
  QDataStream out(&byteArray, QIODevice::WriteOnly);
  out << List;
  return byteArray;
}
QByteArray serializev(CVarDB Parameter)
{
  QByteArray byteArray;
  // Если пустая то на выход
  byteArray.clear();
  if(Parameter.VarName.isEmpty())return byteArray;
  byteArray.constData(); // Для подстраховки от отсутствия символа
  QDataStream out(&byteArray, QIODevice::WriteOnly);
  out << Parameter;
  return byteArray;
}
// Разбор на стандартные строки
QStringList deserialize(QByteArray byteArray)
{
  QStringList result;
  // Если пустая то на выход
  result.clear();
  if(byteArray.isEmpty())return result;
  byteArray.constData(); // Для подстраховки от отсутствия символа
  QDataStream in(&byteArray, QIODevice::ReadOnly);
  in >> result;
  return result;
}
QList<CVarDB> deserializec(QByteArray byteArray)
{
  QList<CVarDB> result;
  // Если пустая то на выход
  result.clear();
  if(byteArray.isEmpty())return result;
  byteArray.constData(); // Для подстраховки от отсутствия символа
  QDataStream in(&byteArray, QIODevice::ReadOnly);
  in >> result;
  return result;
}
CVarDB deserializev(QByteArray byteArray)
{
  CVarDB result;
  if(byteArray.isEmpty())return result;
  byteArray.constData(); // Для подстраховки от отсутствия символа
  QDataStream in(&byteArray, QIODevice::ReadOnly);
  in >> result;
  return result;
}
