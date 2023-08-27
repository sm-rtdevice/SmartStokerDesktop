// Released by Sirkin I.S.: 11.09.14
#ifndef DS1820_H
#define DS1820_H

 #include "CComPort.h"
 #include "DYNAMICDATA.H"

 static CComPort* ComPort;
 static byte SendPacket[64];
 static byte RecvPacket[64];

 static byte SearchROM = 0xF0;         // 00 00 00 00 FF FF FF FF   поиск устройств
 static byte ReadROM = 0x33;           // 33
 static byte MatchROM = 0x55;          // совпадение ПЗУ
 static byte SkipROM = 0xCC;
 static byte ConvertTemperatureFUN = 0x44;  // преобразование температуры
 static byte ReadScratchPadFUN = 0xBE;      // чтение блокнота
 static byte WriteScratchPadFUN = 0x4E;
 static byte CopyScratchPadFUN = 0x48;
 static byte RecallE2FUN = 0xB8;            // Повторная загрузка Recall E2 – загружает данные из EEPROM в ОЗУ. Эта операция выполняется автоматически, как только на датчик подаётся напряжение
 static byte ReadPowerSupplyFUN = 0xB4;
 static byte AlarmSearchROM = 0xEC;

 static bool CRC = true;

 class CTermPoint
 {
  public:
  float Temperature;
  TDateTime Time;
  //float OffsetTemp;  // калибровка датчика (температура +-)

  CTermPoint::CTermPoint(){/*OffsetTemp = 0.0;*/};

  float CTermPoint::GetTemp() // получение температуры  - не нужен метод
  {
   return Temperature;
   //if(Temperature > 125){return -90;}          // первышение верхнего предела температуры
   //else if(Temperature < -55){return -90;}     // первышение нижнего предела температуры
   //else{return Temperature + OffsetTemp;}
  };

 };

 struct sDS1820                 // температурный датчик DS1820
 {
  byte SN[8];                   // серийный номер // {0x10,0xDC,0x8D,0xA3,0x00,0x08,0x00,0x32}
  AnsiString Description;       // описание
  // float Offset;              // калибровка датчика (температура +-)
  //byte Num;                   // номер датчика 1 или 2
 };

// описание датчика:
 class cDS1820                  // температурный датчик DS1820
 {
 public:
  byte SN[8];                   // серийный номер // {0x10,0xDC,0x8D,0xA3,0x00,0x08,0x00,0x32}
  AnsiString Description;       // описание
  float OffsetTemp;             // калибровка датчика (температура +-)
  int Quality;                  // качество опроса датчика %
  //byte Num;                   // номер датчика 1 или 2...

  float TmaxLog;                // Максимальное значение температуры
  float TminLog;                // Минимальное значение температуры

 private:
  TDateTime Time;

 public:

  CTwoLinkList <CTermPoint> TPointsLog;  // список измереных точек температур (загрузка из файла)
  CTwoLinkList <CTermPoint> TPoints;     // оперативный контроль

  void SetTPoint(float Temperauure)      // добавить температурную точку в оперативный контроль
  {
   CTermPoint SetPoint;
   SetPoint.Temperature = Temperauure + OffsetTemp; // температура с учетом поправки
   SetPoint.Time = Time.CurrentDateTime();
   TPoints.SetItem(SetPoint);
   if(TPoints.GetCount() > 60){TPoints.DelIndex(1);} // ограничение 60 точек.
  }

  float GetTemperature()                        // последняя записть, актуальные данные - текущая температура
  {
   if(TPoints.GetCount() == 0){return 0;};
   return (TPoints.Last())->Data.Temperature;
  }

  CTermPoint GetTPoint()                        // последняя записть, актуальные данные
  {
   //if(TPoints.GetCount() == 0){return NULL;};
   return (TPoints.Last())->Data;               // проверить !!!!
  }

  void GetMinMaxT( float &min, float &max)    // оперативный контроль максимальная и минимальная температура
  {
    float Tmax = -90.5;                   // Максимальное значение температуры
    float Tmin = 125.5;                   // Минимальное значение температуры

    TListData2<CTermPoint> *CurrentItem;
    CurrentItem = TPoints.First();
    while(CurrentItem != NULL)     // отрисовка графика температуры датчик 1
    {
     if(CurrentItem->Data.Temperature > -55.5) // -90 (не ошибочная температура)
     {
      if(Tmax < CurrentItem->Data.Temperature){if(CurrentItem->Data.Temperature < 125.5){Tmax = CurrentItem->Data.Temperature;};};  // Максимальное значение температуры DS2 TMax = SetPoint.Temperature
      if(Tmin > CurrentItem->Data.Temperature){if(CurrentItem->Data.Temperature > -55.5){Tmin = CurrentItem->Data.Temperature;};};  // Минимальное значение температуры DS2 != -90 !
     }
     CurrentItem = CurrentItem->Next;
    }

    max = Tmax;
    min = Tmin;
  }

  cDS1820::cDS1820(){OffsetTemp = 0.0; TmaxLog = -90.5; TminLog = 125.5; }; // Quality = 10;
  cDS1820::~cDS1820(){TPointsLog.Free(); TPoints.Free();};
 };
//---------------------------------------------------------------------------

int BitsToByte(byte* b)  // биты в байты: 00 FF FF FF FF FF 00 FF -> 0BEh
{
 int result = 0;
 int and = 128;

 for (int i = 7; i >= 0; i--)
 {
  result ^= b[i] == 0xFF ? and : 0x00;
  and /= 2;
 }

 return result;
}
//---------------------------------------------------------------------------

void ByteToBits(byte b, byte* bits)
{
 int and = 128;

 for (int i = 7; i >= 0; i--)
 {
  bits[i] = (b & and) == 0x00 ? 0x00 : 0xFF;
  and /= 2;
 }

}
//---------------------------------------------------------------------------

byte calc_crc(byte buff[], byte num_vals)     // вычисление CRC
{
 byte shift_reg = 0, data_bit, sr_lsb, fb_bit, i, j;

 for (i=0; i<num_vals; i++)                   // for each byte
 {
  for(j=0; j<8; j++)                          // for each bit
  {
   data_bit = (buff[i]>>j)&0x01;
   sr_lsb = shift_reg & 0x01;
   fb_bit = (data_bit ^ sr_lsb) & 0x01;
   shift_reg = shift_reg >> 1;
   if (fb_bit)
   {
    shift_reg = shift_reg ^ 0x8c;
   }
  }
 }
 return(shift_reg);
}
//---------------------------------------------------------------------------

// Данная функция осуществляет сканирование сети 1-wire и записывает найденные
// ID устройств в массив buf, по 8 байт на каждое устройство.
// переменная num ограничивает количество находимых устройств, чтобы не переполнить буфер.
int ScanDevice(byte *buf, int num) // поиск 1-wire устройств
{
 int found = 0;
 byte *lastDevice;
 byte *curDevice = buf;
 int numBit, lastCollision, currentCollision, currentSelection;
 lastCollision = 0; // последняя коллизия из предыдущей итерации, в которой мы пошли направо
 byte OW_R_1 = 255;

 while(found < num)
 {
  numBit = 1;
  currentCollision = 0;

  // посылаем команду на поиск устройств
  ComPort->SetDCB(9600);
  ComPort->Send(&SearchROM,1);                  // SendPacket[0] = 0xF0;  // "Сброс/Присутствие"
  ComPort->Recv(RecvPacket,1);
  if(RecvPacket[0] != 0xE0){return found;}      // нет ответа присутствия

  ComPort->SetDCB(115200);
  ByteToBits(0xF0, SendPacket);                 // 00 00 00 00 FF FF FF FF
  ComPort->Send(SendPacket,8);
  ComPort->Recv(RecvPacket,8);

  for(numBit = 1; numBit <= 64; numBit++)
  {
   // читаем два бита. Основной и комплементарный
   SendPacket[0] = 0xFF; SendPacket[1] = 0xFF;
   //ComPort->SetDCB(115200);    //
   /*ComPort->Send(SendPacket,1);
   ComPort->Recv(RecvPacket,1);  //
   ComPort->Send(SendPacket+1,1);
   ComPort->Recv(RecvPacket+1,1);//
   */
   ComPort->Send(SendPacket,2);  //
   ComPort->Recv(RecvPacket,2);  //

   if (RecvPacket[0] == OW_R_1)                 // прочитали 1
   {
    if(RecvPacket[1] == OW_R_1){return found;}  // прочитали 11 заканчиваем поиск
    else{currentSelection = 1;}                 // 10 - на данном этапе только 1
   }
   else                                         // прочитали 0
   {
    if(RecvPacket[1] == OW_R_1)    // 01 - на данном этапе только 0
    {
     currentSelection = 0;
    }
    else                                        // 00 - коллизия
    {
     if(numBit < lastCollision)                 // идем по дереву, не дошли до развилки
     {
      if(lastDevice[(numBit - 1) >> 3] & 1 << ((numBit - 1) & 0x07))
      {          // (numBit-1)>>3 - номер байта, (numBit-1)&0x07 - номер бита в байте
       currentSelection = 1;
       if(currentCollision < numBit){currentCollision = numBit;} // если пошли по правой ветке, запоминаем номер бита
      }
      else{currentSelection = 0;}
     }
     else
     {
      if (numBit == lastCollision){currentSelection = 0;}
      else // идем по правой ветке
      {
       currentSelection = 1;
       if (currentCollision < numBit){currentCollision = numBit;} // если пошли по правой ветке, запоминаем номер бита
      }
     }
    }
   }

   if (currentSelection == 1)
   {
    curDevice[(numBit - 1) >> 3] |= 1 << ((numBit - 1) & 0x07);
    SendPacket[0] = 0xFF;
   }
   else
   {
    curDevice[(numBit - 1) >> 3] &= ~(1 << ((numBit - 1) & 0x07));
    SendPacket[0] = 0x00;
   }
   //ComPort->SetDCB(115200);        //
   ComPort->Send(SendPacket,1);      // отсеять несовпадающие устройства
   ComPort->Recv(RecvPacket,1);      //
  }

  found++;
  lastDevice = curDevice;
  curDevice += 8;
  if(currentCollision == 0){return found;}
  lastCollision = currentCollision;
 } // while

 return found;
}
//---------------------------------------------------------------------------

float GetTemp(byte SN[]) // получение температуры
{
 float ErrTmp = -90;     // значение температуры в случае ошибки
 ComPort->Clean();

try // перехват аппаратного сбоя
{

 ComPort->SetDCB(9600);
 ComPort->Send(&SearchROM,1); // SendPacket[0] = 0xF0;           // "Сброс/Присутствие"
 if(ComPort->Recv(RecvPacket, 1) != 1){return ErrTmp;};

 ComPort->SetDCB(115200);
 ByteToBits(MatchROM, SendPacket);  // "Совпадение ПЗУ" 0x55
 ComPort->Send(SendPacket,8);
 if(ComPort->Recv(RecvPacket, 8) != 8){return ErrTmp;};

 // byte SN[8] = {0x10, 0xDC, 0x8D, 0xA3, 0x00, 0x08, 0x00, 0x32}; // серийник c CRC (пример)
 byte SNb[64]; // байты серийника

 ByteToBits(SN[0], SNb);    // 0x10 FAMILY CODE (0x10) DS-1820
 ByteToBits(SN[1], SNb+8);  // 0xDC
 ByteToBits(SN[2], SNb+16); // 0x8D
 ByteToBits(SN[3], SNb+24); // 0xA3
 ByteToBits(SN[4], SNb+32); // 0x00
 ByteToBits(SN[5], SNb+40); // 0x08
 ByteToBits(SN[6], SNb+48); // 0x00
 ByteToBits(SN[7], SNb+56); // 0x32 = CRC calc_crc(SN,7);

 ComPort->Send(SNb, 64);    // FAMILY CODE & S/N & CRC
 if(ComPort->Recv(RecvPacket, 64) != 64){return ErrTmp;};

 ByteToBits(RecallE2FUN, SendPacket);  // Повторная загрузка Recall E2
 ComPort->Send(SendPacket,8);
 if(ComPort->Recv(RecvPacket, 8) != 8){return ErrTmp;};

 ComPort->SetDCB(9600);
 ComPort->Send(&SearchROM,1); // SendPacket[0] = 0xF0;           // "Сброс/Присутствие"
 if(ComPort->Recv(RecvPacket, 1) != 1){return ErrTmp;};

 ComPort->SetDCB(115200);
 ByteToBits(MatchROM, SendPacket);  // "Совпадение ПЗУ" 0x55
 ComPort->Send(SendPacket,8);
 if(ComPort->Recv(RecvPacket, 8) != 8){return ErrTmp;};

 ByteToBits(SN[0], SNb);    // 0x10 FAMILY CODE (0x10) DS-1820
 ByteToBits(SN[1], SNb+8);  // 0xDC
 ByteToBits(SN[2], SNb+16); // 0x8D
 ByteToBits(SN[3], SNb+24); // 0xA3
 ByteToBits(SN[4], SNb+32); // 0x00
 ByteToBits(SN[5], SNb+40); // 0x08
 ByteToBits(SN[6], SNb+48); // 0x00
 ByteToBits(SN[7], SNb+56); // 0x32 = CRC calc_crc(SN,7);

 ComPort->Send(SNb, 64);    // FAMILY CODE & S/N & CRC
 if(ComPort->Recv(RecvPacket, 64) != 64){return ErrTmp;};

 ByteToBits(ConvertTemperatureFUN, SendPacket);  // 0x44 "Преобразование температуры"
 ComPort->Send(SendPacket,8);
 Sleep(750);
 if(ComPort->Recv(RecvPacket, 8) != 8){return ErrTmp;};

 ByteToBits(0x01, SendPacket);  // 0xFF 0x00 0x00 0x00 0x00 0x00 0x00 0x00
 ComPort->Send(SendPacket,8);
 if(ComPort->Recv(RecvPacket, 8) != 8){return ErrTmp;};

 ComPort->SetDCB(9600);
 ComPort->Send(&SearchROM,1); // SendPacket[0] = 0xF0;           // "Сброс/Присутствие"
 if(ComPort->Recv(RecvPacket, 1) != 1){return ErrTmp;};

 ComPort->SetDCB(115200);
 ByteToBits(MatchROM, SendPacket);  // "Совпадение ПЗУ" 0x55
 ComPort->Send(SendPacket,8);
 if(ComPort->Recv(RecvPacket, 8) != 8){return ErrTmp;};

 ByteToBits(SN[0], SNb);    // 0x10 FAMILY CODE (0x10) DS-1820
 ByteToBits(SN[1], SNb+8);  // 0xDC
 ByteToBits(SN[2], SNb+16); // 0x8D
 ByteToBits(SN[3], SNb+24); // 0xA3
 ByteToBits(SN[4], SNb+32); // 0x00
 ByteToBits(SN[5], SNb+40); // 0x08
 ByteToBits(SN[6], SNb+48); // 0x00
 ByteToBits(SN[7], SNb+56); // 0x32 = CRC calc_crc(SN,7);

 ComPort->Send(SNb, 64);    // FAMILY CODE & S/N & CRC
 if(ComPort->Recv(RecvPacket, 64) != 64){return ErrTmp;};

 ByteToBits(ReadScratchPadFUN, SendPacket);  // "Чтение блокнотной памяти" (команда 0BEh)
 ComPort->Send(SendPacket,8);                // Sleep(50);
 if(ComPort->Recv(RecvPacket, 8) != 8){return ErrTmp;};

// получение данных из блокнотной памяти
 byte ScratchpadBuffer[8];      // полученные данные от датчика

 ByteToBits(0xFF, SendPacket);  // FF FF FF FF FF FF FF FF
 ComPort->Send(SendPacket,8);                           // Sleep(50);
 if(ComPort->Recv(RecvPacket, 8) != 8){return ErrTmp;}; // байт 0
 ScratchpadBuffer[0] = BitsToByte(RecvPacket);
 float LowByte = BitsToByte(RecvPacket);

 ComPort->Send(SendPacket,8);                   // Sleep(50);
 if(ComPort->Recv(RecvPacket, 8) != 8){return ErrTmp;}; // байт 1
 ScratchpadBuffer[1] = BitsToByte(RecvPacket);
 float HighByte  = BitsToByte(RecvPacket);

 ComPort->Send(SendPacket,8);
 if(ComPort->Recv(RecvPacket, 8) != 8){return ErrTmp;}; // байт 2
 ScratchpadBuffer[2] = BitsToByte(RecvPacket);

 ComPort->Send(SendPacket,8);
 if(ComPort->Recv(RecvPacket, 8) != 8){return ErrTmp;}; // байт 3
 ScratchpadBuffer[3] = BitsToByte(RecvPacket);

 ComPort->Send(SendPacket,8);
 if(ComPort->Recv(RecvPacket, 8) != 8){return ErrTmp;}; // байт 4
 ScratchpadBuffer[4] = BitsToByte(RecvPacket);

 ComPort->Send(SendPacket,8);
 if(ComPort->Recv(RecvPacket, 8) != 8){return ErrTmp;}; // байт 5
 ScratchpadBuffer[5] = BitsToByte(RecvPacket);

 ComPort->Send(SendPacket,8);
 if(ComPort->Recv(RecvPacket, 8) != 8){return ErrTmp;}; // байт 6
 ScratchpadBuffer[6] = BitsToByte(RecvPacket);
 int COUNT_REMAIN = BitsToByte(RecvPacket);

 ComPort->Send(SendPacket,8);
 if(ComPort->Recv(RecvPacket, 8) != 8){return ErrTmp;}; // байт 7
 ScratchpadBuffer[7] = BitsToByte(RecvPacket);

 int COUNT_PER_C = BitsToByte(RecvPacket);

 ComPort->Send(SendPacket,8);
 if(ComPort->Recv(RecvPacket, 8) != 8){return ErrTmp;}; // байт 8 CRC
 ScratchpadBuffer[8] = BitsToByte(RecvPacket);

 if(calc_crc(ScratchpadBuffer,8) != ScratchpadBuffer[8]){ CRC = false; return ErrTmp;} else {CRC = true; }; // проверка контрольной суммы полученных данных  // IntToHex( calc_crc(buf,7) ,2);

 float TEMP_READ;
 if(HighByte != 0){TEMP_READ = (256-LowByte)/2 *(-1);}else{TEMP_READ = LowByte/2;}
 TEMP_READ = TEMP_READ - 0.25 + (COUNT_PER_C - COUNT_REMAIN)/COUNT_PER_C;           // высокое разрешение

// отрицательная температура не работает! пример из даташита!
/* float TEMP_READ;
 TEMP_READ = ((int)HighByte << 8) + LowByte;
 TEMP_READ /= 2;
 TEMP_READ = TEMP_READ - 0.25 + (COUNT_PER_C - COUNT_REMAIN)/COUNT_PER_C;           // высокое разрешение
*/

 if(TEMP_READ < -55 || TEMP_READ > 125){return ErrTmp;}  // предел измерений -55..125, -90 - ошибка измерения

 return TEMP_READ; // то что пришло с датчика
} // /try
catch(...) // /перехват аппаратного сбоя
{
 return -90; // error value
}

}
//---------------------------------------------------------------------------

float GetTempFast(byte SN[]) // быстрый опрос
{
 float ErrTmp = -90;     // значение температуры в случае ошибки
 ComPort->Clean();

 ComPort->SetDCB(9600);
 ComPort->Send(&SearchROM,1); // SendPacket[0] = 0xF0;           // "Сброс/Присутствие"
 if(ComPort->Recv(RecvPacket, 1) != 1){return ErrTmp;};

 ByteToBits(SkipROM, SendPacket);  // 0xCC  "Пропуск ПЗУ"
 ComPort->SetDCB(115200); //
 ComPort->Send(SendPacket,8);
 if(ComPort->Recv(RecvPacket, 8) != 8){return ErrTmp;};

 ByteToBits(ConvertTemperatureFUN, SendPacket);  // 0x44 "Преобразование температуры"
 ComPort->Send(SendPacket,8);
 Sleep(750);

 if(ComPort->Recv(RecvPacket, 8) != 8){return ErrTmp;};

 ByteToBits(0x01, SendPacket);  // 0xFF 0x00 0x00 0x00 0x00 0x00 0x00 0x00
 ComPort->Send(SendPacket,8);

 if(ComPort->Recv(RecvPacket, 8) != 8){return ErrTmp;};

 ComPort->SetDCB(9600);
 ComPort->Send(&SearchROM,1);       SendPacket[0] = 0xF0;              // "Сброс/Присутствие"
 if(ComPort->Recv(RecvPacket, 1) != 1){return ErrTmp;};

 ComPort->SetDCB(115200);
 ByteToBits(MatchROM, SendPacket);  // "Совпадение ПЗУ" 0x55
 ComPort->Send(SendPacket,8);
 if(ComPort->Recv(RecvPacket, 8) != 8){return ErrTmp;};

// byte SN[8] = {0x10, 0xDC, 0x8D, 0xA3, 0x00, 0x08, 0x00, 0x32}; // c CRC
 byte SNb[64]; // байты серийника

 ByteToBits(SN[0], SNb);    // 0x10 FAMILY CODE (0x10) DS-1820
 ByteToBits(SN[1], SNb+8);  // 0xDC
 ByteToBits(SN[2], SNb+16); // 0x8D
 ByteToBits(SN[3], SNb+24); // 0xA3
 ByteToBits(SN[4], SNb+32); // 0x00
 ByteToBits(SN[5], SNb+40); // 0x08
 ByteToBits(SN[6], SNb+48); // 0x00
 ByteToBits(SN[7], SNb+56); // 0x32 = CRC calc_crc(SN,7);

 ComPort->Send(SNb, 64);    // FAMILY CODE & S/N & CRC
 if(ComPort->Recv(RecvPacket, 64) != 64){return ErrTmp;};

/*
// отправить частями:
 ComPort->Send(SNb,16);
 ComPort->Recv(RecvPacket, 16);

 ComPort->Send(SNb+16,16);
 ComPort->Recv(RecvPacket, 16);

 ComPort->Send(SNb+32,16);
 ComPort->Recv(RecvPacket, 16);

 ComPort->Send(SNb+48,16);
 ComPort->Recv(RecvPacket, 16);
*/

 ByteToBits(ReadScratchPadFUN, SendPacket);  // "Чтение блокнотной памяти" (команда 0BEh)
 ComPort->Send(SendPacket,8);                // Sleep(50);
 if(ComPort->Recv(RecvPacket, 8) != 8){return ErrTmp;};

// получение данных из блокнотной памяти
 byte ScratchpadBuffer[8];      // полученные данные от датчика

 ByteToBits(0xFF, SendPacket);  // FF FF FF FF FF FF FF FF
 ComPort->Send(SendPacket,8);                           // Sleep(50);
 if(ComPort->Recv(RecvPacket, 8) != 8){return ErrTmp;}; // байт 0
 ScratchpadBuffer[0] = BitsToByte(RecvPacket);
 float LowByte = BitsToByte(RecvPacket);

 ComPort->Send(SendPacket,8);                   // Sleep(50);
 if(ComPort->Recv(RecvPacket, 8) != 8){return ErrTmp;}; // байт 1
 ScratchpadBuffer[1] = BitsToByte(RecvPacket);
 float HighByte  = BitsToByte(RecvPacket);

 ComPort->Send(SendPacket,8);
 if(ComPort->Recv(RecvPacket, 8) != 8){return ErrTmp;}; // байт 2
 ScratchpadBuffer[2] = BitsToByte(RecvPacket);

 ComPort->Send(SendPacket,8);
 if(ComPort->Recv(RecvPacket, 8) != 8){return ErrTmp;}; // байт 3
 ScratchpadBuffer[3] = BitsToByte(RecvPacket);

 ComPort->Send(SendPacket,8);
 if(ComPort->Recv(RecvPacket, 8) != 8){return ErrTmp;}; // байт 4
 ScratchpadBuffer[4] = BitsToByte(RecvPacket);

 ComPort->Send(SendPacket,8);
 if(ComPort->Recv(RecvPacket, 8) != 8){return ErrTmp;}; // байт 5
 ScratchpadBuffer[5] = BitsToByte(RecvPacket);

 ComPort->Send(SendPacket,8);
 if(ComPort->Recv(RecvPacket, 8) != 8){return ErrTmp;}; // байт 6
 ScratchpadBuffer[6] = BitsToByte(RecvPacket);
 int COUNT_REMAIN = BitsToByte(RecvPacket);

 ComPort->Send(SendPacket,8);
 if(ComPort->Recv(RecvPacket, 8) != 8){return ErrTmp;}; // байт 7
 ScratchpadBuffer[7] = BitsToByte(RecvPacket);

 int COUNT_PER_C = BitsToByte(RecvPacket);

 ComPort->Send(SendPacket,8);
 if(ComPort->Recv(RecvPacket, 8) != 8){return ErrTmp;}; // байт 8 CRC
 ScratchpadBuffer[8] = BitsToByte(RecvPacket);

 if(calc_crc(ScratchpadBuffer,8) != ScratchpadBuffer[8]){ CRC = false; return ErrTmp;} else {CRC = true; }; // проверка контрольной суммы полученных данных  // IntToHex( calc_crc(buf,7) ,2);

 float TEMP_READ;
 if(HighByte != 0){TEMP_READ = (256-LowByte)/2 *(-1);}else{TEMP_READ = LowByte/2;}
 TEMP_READ = TEMP_READ - 0.25 + (COUNT_PER_C - COUNT_REMAIN)/COUNT_PER_C;           // высокое разрешение

/*
// отрицательная температура не работает!
 TEMP_READ = ((int)HighByte << 8) + LowByte;
 TEMP_READ /= 2;
 TEMP_READ = TEMP_READ - 0.25 + (COUNT_PER_C - COUNT_REMAIN)/COUNT_PER_C;           // высокое разрешение
*/

 return TEMP_READ; // Temperature
}
//---------------------------------------------------------------------------

#include <io.h>         //для работы с файлами
#include <fcntl.h>      //для работы с файлами
#include <sys\stat.h>   //для работы с файлами
#include <iostream.h>   // sprintf
#include <fstream.h>    // ofstream

bool SaveSNf(AnsiString Path, CTwoLinkList <sDS1820> &DevList) // Сохранить список датчиков
{
// if(DevList.GetCount() == 0){return true;} // список пуст

 ofstream f_out;   // выходной поток
 //AnsiString Path = ExtractFilePath( ParamStr( 0 ) ) + "SN.dat"; // путь по умолчанию

// ofstream f_out(Path.c_str());   // выходной поток
 f_out.open( Path.c_str(), /*ios::trunc |*/ ios::binary);  // попытаемся открыть файл

 if (!f_out)
 {//ShowMessage( "Невозможно открыть файл!" );
  return false;
 }

 if( FileExists(Path) ) // удалить если существует !!!!
 {
  DeleteFileA(Path);
 }

 TListData2<sDS1820> *CurrentItem;// указатель на узел списка (итератор)
 CurrentItem = DevList.First();   // установить указатель на первый элемент
// int STRLEN;                    // длина строки
 byte count;                      // 0..255
 count = DevList.GetCount();

 f_out.write( reinterpret_cast<char*>(&count), sizeof(byte) ); // КОЛИЧЕСТВО ЗАПИСЕЙ

 while(CurrentItem != NULL) // обход списка
 {
  f_out.write( reinterpret_cast<char*>(&CurrentItem->Data.SN), sizeof(CurrentItem->Data.SN) );

  // for(int j = 0; j<8; j++)
  // {
  //  f_out.write( reinterpret_cast<char*>(&device.SN[j]), sizeof(byte) );
  // }

//    STRLEN = strlen(CurrentItem->Data.Description.c_str());       // длина строки
//  f_out.write( reinterpret_cast<char*>(&STRLEN), sizeof(byte) );  // записать длину строки
//  f_out.write( reinterpret_cast<char*>(CurrentItem->Data.Description.c_str()), STRLEN + 1 );  // строка

  CurrentItem = CurrentItem->Next; // переместиться на следующий элемент
 }

 if (!f_out)
 {//ShowMessage( "Невозможно записать в файл!" );
  return false;
 }

 f_out.close();  // закрываем файл
 return true;
};
//-----------------------------------------------------------------------------

bool SaveSN(CTwoLinkList <sDS1820> &DevList) // Сохранить список датчиков
{
 return SaveSNf(ExtractFilePath( ParamStr( 0 ) ) + "SN.dat", DevList); // Сохранить список датчиков (путь по умолчанию)
}
//-----------------------------------------------------------------------------

bool LoadSNf(AnsiString Path, CTwoLinkList <sDS1820> &DevList) // Загрузить список из файла
{
 ifstream f_inp;
  //AnsiString Path = ExtractFilePath( ParamStr( 0 ) ) + "SN.dat";   // путь по умолчанию
  f_inp.open(Path.c_str(), ios::binary); // попытаемся открыть файл

  if (!f_inp)
  {//ShowMessage( "Невозможно открыть файл!" );
   return false;
  }

  sDS1820 device;
//  int STRLEN = 0;                    // длина строки
//  char* STRBUF;
  byte count = 0;

  f_inp.read( reinterpret_cast<char*>(&count), sizeof(byte) ); // КОЛИЧЕСТВО ЗАПИСЕЙ

  for(int i=0; i<count; i++) // прочитать count записей
  {
   f_inp.read( reinterpret_cast<char*>(&device.SN), sizeof(device.SN) );


  // for(int j = 0; j<8; j++)
  // {
  //  f_inp.read( reinterpret_cast<char*>(&device.SN[j]), sizeof(byte) );
  // }

//   f_inp.read( reinterpret_cast<char*>(&STRLEN), sizeof(byte) ); // прочитать длину строки
//     STRBUF = new char[STRLEN+1];                                // созд. буфер для считываемой сроки
//   f_inp.read( reinterpret_cast<char*>(STRBUF), STRLEN + 1);     // прочитать строку в буфер
//     device.Description = STRBUF;                                // уст. из буфера
//     delete(STRBUF);                                             // удалить буфер

     DevList.SetItem(device); // добавить в список
  }


  if (!f_inp)
  {//ShowMessage( "Невозможно загрузить из файла!" );
   return false;
  }

  f_inp.close(); // закрываем файл
  return true;
};
//-----------------------------------------------------------------------------

bool LoadSN(CTwoLinkList <sDS1820> &DevList) // Загрузить список (путь по умолчанию)
{
 return LoadSNf(ExtractFilePath( ParamStr( 0 ) ) + "SN.dat", DevList);
};
//-----------------------------------------------------------------------------

void ParceSNtoByte(AnsiString StrSN, byte* device)  // StrSN --> device (byte device[8])  // string --> byte[]
{
 //device.SN[1] = StrToInt(StrSN.SubString(1,2));
 AnsiString OX;
 for(int i = 0; i<8; i++)
 {
  OX = "0x";
  OX += StrSN.SubString(i*2+1+i,2);
  //Form1->StringGrid1->Cells[0][i] = OX;
  device[i] = StrToInt(OX);
 }
}
//-----------------------------------------------------------------------------

void ParceSNtoStr(AnsiString &StrSN, byte* device)//  byte device[8] --> StrSN        // byte[] --> string
{
 StrSN = "";
 StrSN += IntToHex(device[0] ,2); StrSN += " ";
 StrSN += IntToHex(device[1] ,2); StrSN += " ";
 StrSN += IntToHex(device[2] ,2); StrSN += " ";
 StrSN += IntToHex(device[3] ,2); StrSN += " ";
 StrSN += IntToHex(device[4] ,2); StrSN += " ";
 StrSN += IntToHex(device[5] ,2); StrSN += " ";
 StrSN += IntToHex(device[6] ,2); StrSN += " ";
 StrSN += IntToHex(device[7] ,2);
}
//-----------------------------------------------------------------------------

bool SaveTermPoint(CTermPoint TermPoint1, CTermPoint TermPoint2) // Сохранить 2 температурные точки, формат: [№][dt][dt][dt][dt][dt][dt][dt][dt][T][T][T][T]...
{
// if(MeasureTemp.GetCount() == 0){return true;} // список пуст
 ofstream f_out;   // выходной поток ofstream
 AnsiString Path = ExtractFilePath( ParamStr( 0 ) ) + "\\Logs\\" + Now().DateString() + ".sit"; // "23.07.2013"

// ofstream f_out(Path.c_str());   // выходной поток
 f_out.open( Path.c_str(), /*ios::trunc |*/ ios::binary /*| ios::in | ios::out*/ | ios::app);  // попытаемся открыть файл

 if (!f_out)
 {//ShowMessage( "Невозможно открыть файл!" );
  return false;
 }

 if( FileExists(Path) ) // удалить если существует!
 {
  //DeleteFileA(Path);
  //дописать в файл
 }

 byte DS1 = 1;
 byte DS2 = 2;
// DS1
 f_out.write( reinterpret_cast<char*>(&DS1), sizeof(byte) );                                                    // 1 байт - номер датчика
 f_out.write( reinterpret_cast<char*>(&TermPoint1.Time.Val), sizeof(double) );                                  // 8 байт - время измерения
 f_out.write( reinterpret_cast<char*>(&TermPoint1.Temperature), sizeof(TermPoint1.Temperature) );               // 4 байта - температура
// DS2
 f_out.write( reinterpret_cast<char*>(&DS2), sizeof(byte) );                                                    // 1 байт - номер датчика
 f_out.write( reinterpret_cast<char*>(&TermPoint2.Time.Val), sizeof(double) );                                  // 8 байт - время измерения
 f_out.write( reinterpret_cast<char*>(&TermPoint2.Temperature), sizeof(TermPoint2.Temperature) );               // 4 байта - температура

 //f_out.seekp(ios::end); // переместиться в конец
 //f_out.seekp(0); // переместиться в начало (0 - номер байта, 1 - хз)  ios::beg;

 if (!f_out)
 {//ShowMessage( "Невозможно записать в файл!" );
  return false;
 }

 f_out.close();  // закрываем файл
 return true;
};
//-----------------------------------------------------------------------------

bool LoadTermPoints(cDS1820 &DS1, cDS1820 &DS2, AnsiString Path) // Загрузить  CTwoLinkList <CTermPoint> &MeasureTemp1,CTwoLinkList <CTermPoint> &MeasureTemp2, AnsiString Path
{
 ifstream f_inp;
 //AnsiString Path = ExtractFilePath( ParamStr( 0 ) ) + "\\Logs\\" + Now().DateString() + ".SIT";

  f_inp.open(Path.c_str(), ios::binary); // попытаемся открыть файл

  if (!f_inp)
  {//ShowMessage( "Невозможно открыть файл!" );
   return false;
  }

  byte nDS = 1;
  CTermPoint CurrentPoint;

  int PointDS1 = 0;    // удачные опросы
  int PointDS2 = 0;
  int TotalPoint;      // общее количество опросов

  DS1.TPointsLog.Free();  // очистка списков
  DS2.TPointsLog.Free();

  DS1.TmaxLog = -90.5;    // Максимальное значение температуры
  DS1.TminLog = 125.5;    // Минимальное значение температуры

  DS2.TmaxLog = -90.5;
  DS2.TminLog = 125.5;

  while(/*!f_inp.eof()*/ f_inp.read( reinterpret_cast<char*>(&nDS), sizeof(nDS) ) )                     // 1 байт - номер датчика
  {
   f_inp.read( reinterpret_cast<char*>(&CurrentPoint.Time.Val), sizeof(CurrentPoint.Time.Val) );        // 8 байт - время измерения
   f_inp.read( reinterpret_cast<char*>(&CurrentPoint.Temperature), sizeof(CurrentPoint.Temperature) );  // 4 байта - температура

   if(nDS == 1)
   {
    DS1.TPointsLog.SetItem(CurrentPoint);                // добавить в список
    if(CurrentPoint.Temperature > -55.5)                 // != -90
    {
     ++PointDS1;  // удачные измерения
     if(DS1.TmaxLog < CurrentPoint.Temperature){if(CurrentPoint.Temperature < 125.5){DS1.TmaxLog = CurrentPoint.Temperature;};};  // Максимальное значение температуры
     if(DS1.TminLog > CurrentPoint.Temperature){if(CurrentPoint.Temperature > -55.5){DS1.TminLog = CurrentPoint.Temperature;};};  // Минимальное значение температуры
    }    
   }
   else
   {
    DS2.TPointsLog.SetItem(CurrentPoint);                // добавить в список
    if(CurrentPoint.Temperature > -55.5)                 // != -90
    {
     ++PointDS2;                                         // удачные измерения
     if(DS2.TmaxLog < CurrentPoint.Temperature){if(CurrentPoint.Temperature < 125.5){DS2.TmaxLog = CurrentPoint.Temperature;};};  // Максимальное значение температуры
     if(DS2.TminLog > CurrentPoint.Temperature){if(CurrentPoint.Temperature > -55.5){DS2.TminLog = CurrentPoint.Temperature;};};  // Минимальное значение температуры
    }
   }

  }

  // качество опроса датчиков
  TotalPoint = DS1.TPointsLog.GetCount(); if( TotalPoint == 0 ){ TotalPoint = 1; }
  DS1.Quality = PointDS1 * 100 / TotalPoint;

  TotalPoint = DS2.TPointsLog.GetCount(); if( TotalPoint == 0 ){ TotalPoint = 1; }
  DS2.Quality = PointDS2 * 100 / TotalPoint;

  //Form5->Chart1->Series[0]->Title = "Датчик 1 (на улице): " + IntToStr(DS1.Quality) + "%";
  //Form5->Chart1->Series[1]->Title = "Датчик 2 (в помещении): " + IntToStr(DS2.Quality) + "%";

  if (!f_inp)
  {//ShowMessage( "Невозможно загрузить из файла!" );
   return false;
  }

  f_inp.close(); // закрываем файл
  return true;
}

#endif
