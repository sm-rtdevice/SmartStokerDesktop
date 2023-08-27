// Released by Sirkin I.S. 18.03.2010
#ifndef DYNAMICDATA_H
#define DYNAMICDATA_H

template <class sT>     // шаблон узла для динамической структуры данных любого типа
struct TData            // структура узла
{
  sT Data;              // данные любого типа
  TData* Next;          // указатель на следующий узел
};

template <class tlT>     // шаблон узла для динамического двусвязного списка любого типа
struct TListData2        // структура узла (двусвязного)
{
  tlT Data;             // данные любого типа
  TListData2* Next;     // указатель на следующий узел
  TListData2* Prev;     // указатель на предыдущий узел
};
//---------------------------------------------------------------------------

template <class qT>       // шаблон типа для класа  qT - тип данных в очереди
class CQueue              // очередь
{
  private:
     TData<qT>* Head;     // указатель на начало очереди "голова"
     TData<qT>* Tail;     // указатель на конец очереди "хвост"
     unsigned int Count;
     unsigned int Limit;  // предельное количество элементов  -1 неограничено

  public:
     bool InsR(qT InpData);            // вставка элемента в очередь (справа)
     qT GetL();                        // вернуть элемент из очереди  (слева)
     void Free();                      // освободить память
     void SetLimit(int SetLimit);      // установка предельного размера очереди
     unsigned int GetCount();          // к-во. эл. в очереди
     CQueue();
     ~CQueue();
};

template <class qT> void CQueue<qT>::Free() // освободить память
{
  TData<qT>* PData;

  while (Head != NULL)
  {
   PData = Head;
   Head = Head->Next;
   delete(PData);
  };

  Head = NULL;
  Tail = NULL;
  Count = 0;
}

template <class qT> CQueue<qT>::CQueue() // конструктор
{
  Head = NULL;
  Tail = NULL;
  Count = 0;

  Limit = 0;
}

template <class qT> CQueue<qT>::~CQueue() // деструктор
{
  this->Free();
}

template <class qT> bool CQueue<qT>::InsR(qT InpData)
{
  if( (Limit != 0) && (Count >= Limit) ){return false;} // невозможно добавить достигнут предел

  TData<qT>* PData;

  if(Tail/*Head*/ == NULL) // вставка первого эл. (в пустую очередь)
  {
    Tail = new(TData<qT>);
    Tail->Data = InpData;
    Tail->Next = NULL;
    Head = Tail;
  }
  else
  {
    PData = new(TData<qT>);
    PData->Data = InpData;
    PData->Next = NULL;
    Tail->Next = PData;
    Tail = PData;
  };

  Count++ ;
  return true;
}

template <class qT> qT CQueue<qT>::GetL()
{
  TData<qT>* PData;
  qT GetL;                       // возвращаемый элемент

  if(Head == NULL){return GetL;} // очередь пустая (возвращается пустой объект)

  GetL = Head->Data;             // возвращаемый объект

  PData = Head;
  Head = Head->Next;
  delete(PData);
  Count-- ;

  if(Count == 0){Head = NULL; Tail = NULL;}
  return GetL;
}

template <class qT> void CQueue<qT>::SetLimit(int SetLimit) // установка предельного размера стэка
{
  Limit = SetLimit;
}

template <class qT> unsigned int CQueue<qT>::GetCount()
{
  return Count;
}
//---------------------------------------------------------------------------

template <class tlT>     // шаблон типа для класа  tlT - тип данных в двусвязном списке
class CTwoLinkList       // двусвязный список (индексация элементов начинается с 1)
{
  private:
     unsigned int Count;      // к-во. эл. в списке
     unsigned int Limit;      // предельное количество элементов  -1 неограничено

     TListData2<tlT>* Head;   // указатель на начало списка "голова"
     TListData2<tlT>* Tail;   // указатель на конец списка "хвост"

  public:
     bool SetItem(tlT InpData);                       // добавление элемента в конец списка
     bool SetItem(tlT InpData, unsigned int ind);     // вставка элемента в позицию ind

     tlT GetItem(unsigned int Ind);                   // получить элемент по индексу

     TListData2<tlT>* GetPointer(unsigned int ind);   // указатель на узел списка по индексу
     tlT* GetDataPointer(unsigned int ind);           // указатель на данные узла списка по индексу

     bool DelNode(TListData2<tlT>* DEL);     // удаление по указателю на узел
     bool DelIndex(unsigned int ind);        // удаление по индексу узла

     bool Up(TListData2<tlT>* SRC);          // переместить элемент вверх
     bool Down(TListData2<tlT>* SRC);        // переместить элемент ввниз
     bool Up(unsigned int ind);              // по индексу в списке
     bool Down(unsigned int ind);

     TListData2<tlT>* First(){return Head;}  // первый элемент
     TListData2<tlT>* Last(){return Tail;}   // последний элемент

     unsigned int GetCount();                // к-во. эл. в списке
     void SetLimit(unsigned int SetLimit);   // установка предельного количкства элементов в списке

     void Free();                            // освободить память
     CTwoLinkList();
     ~CTwoLinkList();
};

template <class tlT> void CTwoLinkList<tlT>::Free()  // освободить память (очистить список)
{
  TListData2<tlT>* PData;  // временный указатель на узел списка

  while (Head != NULL)     // пока в списке есть элементы
  {
   PData = Head;
   Head = Head->Next;
   delete(PData);
  };

  Head = NULL;
  Tail = NULL;
  Count = 0;
}

template <class tlT> CTwoLinkList<tlT>::~CTwoLinkList()  // деструктор
{
  Free();
}

template <class tlT> CTwoLinkList<tlT>::CTwoLinkList()  // конструктор
{
  Head = NULL;
  Tail = NULL;
  Count = 0;

  Limit = 0; // без ограничения размера
}

template <class tlT> TListData2<tlT>* CTwoLinkList<tlT>::GetPointer(unsigned int ind) // указатель на элемент списка по индексу
{
  if(ind > Count || ind < 1){return NULL;} // неправильно задан индекс

  TListData2<tlT>* pCurrent;  // временный указатель на узел списка
  unsigned int tInd;

  tInd = 1;          // если индексация с 1
  pCurrent = Head;   // текущий указатель на начало

  while(tInd != ind) // do
  {
   pCurrent = pCurrent->Next;
   tInd++ ;
  };

  return pCurrent; // pCurrent - указатель на элемент списка по индексу ind
}

template <class tlT> tlT* CTwoLinkList<tlT>::GetDataPointer(unsigned int ind)   // указатель на данные узла списка по индексу
{
  return &GetPointer(ind)->Data;
}

template <class tlT> bool CTwoLinkList<tlT>::Up( TListData2<tlT>* SRC ) // переместить элемент вверх Src - указатель на перемещаемый узел
{
  if(/*Count == 0 ||*/ SRC == NULL || Head == NULL || SRC->Prev == NULL){return false;} // верхний элемент или нет элементов (или  SRC == NULL - указатель на несуществующий перемещаемый элемент)

  TListData2<tlT>* DST; // место назначения
  TListData2<tlT>* P;   // временный указатель

  DST = SRC->Prev;      // 0

  if(DST->Prev == NULL)// SRC теперь верхний элемент
  {
    Head = SRC;
  }

  if(SRC->Next == NULL)// DST теперь нижний элемент
  {
    Tail = DST;
  }

  SRC->Prev = DST->Prev;// 1
  DST->Next = SRC->Next;// 2

  if(SRC->Next != NULL) // 3
  {
    P = SRC->Next;
    P->Prev = DST;
  }

  if(DST->Prev != NULL) // 4
  {
    P = DST->Prev;
    P->Next = SRC;
  }

  DST->Prev = SRC;      // 5
  SRC->Next = DST;      // 6

  return true;
}

template <class tlT> bool CTwoLinkList<tlT>::Down( TListData2<tlT>* SRC ) // переместить элемент вниз Src - указатель на перемещаемый узел
{
  if(/*Count == 0 ||*/SRC == NULL || Head == NULL || SRC->Next == NULL){return false;} // нижний элемент или нет элементов (или SRC == NULL - указатель на несуществующий перемещаемый элемент)

  TListData2<tlT>* DST; // место назначения
  TListData2<tlT>* P;   // временный указатель

  DST = SRC->Next;      // 0

  if(DST->Next == NULL)// SRC теперь нижний элемент
  {
    Tail = SRC;
  }

  if(SRC->Prev == NULL)// DST теперь верхний элемент
  {
    Head = DST;
  }

  SRC->Next = DST->Next;// 1
  DST->Prev = SRC->Prev;// 2

  if(SRC->Prev != NULL) // 3
  {
    P = SRC->Prev;
    P->Next = DST;
  }

  if(DST->Next != NULL) // 4
  {
    P = DST->Next;
    P->Prev = SRC;
  }

  DST->Next = SRC;      // 5
  SRC->Prev = DST;      // 6

  return true;
}

template <class tlT> bool CTwoLinkList<tlT>::Up( unsigned int ind ) // переместить элемент вверх ind - индекс перемещаемого узела
{
  return Up(GetPointer(ind)); // перемещение узла вверх по индексу ind
}

template <class tlT> bool CTwoLinkList<tlT>::Down( unsigned int ind ) // переместить элемент вниз ind - индекс перемещаемого узела
{
  return Down(GetPointer(ind)); // перемещение узла вниз по индексу ind
}

template <class tlT> bool CTwoLinkList<tlT>::DelNode(TListData2<tlT>* DEL) // удаление по указателю на узел
{
  if( Head == NULL || DEL == NULL){return false;} // нет элементов (нечего удалять), удаление нулевого элемента (не правильно задан индекс)

  TListData2<tlT>* P;   // временный указатель

  if(DEL->Prev != NULL)  // 1
  {
    P = DEL->Prev;
    P->Next = DEL->Next;
  }

  if(DEL->Next != NULL)  // 2
  {
    P = DEL->Next;
    P->Prev = DEL->Prev;
  }

  if(DEL == Head){Head = DEL->Next;} // удаляемый элемент - голова
  if(DEL == Tail){Tail = DEL->Prev;} // удаляемый элемент - хвост

  delete(DEL); // 5 удаление элемента
  Count-- ;
  return true;
}

template <class tlT> bool CTwoLinkList<tlT>::DelIndex(unsigned int ind) // удаление по указателю на узел
{
  return DelNode(GetPointer(ind));
}

template <class tlT> unsigned int CTwoLinkList<tlT>::GetCount()
{
  return Count;
}

template <class tlT> void CTwoLinkList<tlT>::SetLimit(unsigned int SetLimit) // установка предельного размера стэка
{
  Limit = SetLimit;
}

template <class tlT> tlT CTwoLinkList<tlT>::GetItem(unsigned int Ind) // вернуть элемент по индексу
{
  TListData2<tlT>* pCurrent;  // временный указатель на узел списка
  unsigned int tInd;

  tlT Get;                // возвращаемый объект
  if(Ind > Count || Ind < 1){return Get;} // возвращает обЪект по умолчанию, недопустимый индекс (если индексация с 1) Count-1(если индексация с 0)

  tInd = 1;          // если индексация с 1
  pCurrent = Head;

  while(tInd != Ind) // do
  {
   pCurrent = pCurrent->Next;
   tInd++ ;
  };

  return pCurrent->Data; // *pD11 - Элемент, pD11 - указатель на элемент
}

template <class tlT> bool CTwoLinkList<tlT>::SetItem(tlT InpData) // добавление элемента в конец списка
{
  if( (Limit != 0) && (Count >= Limit) ){return false;} // невозможно добавить достигнут предел

  TListData2<tlT>* PData;  // временный указатель на узел списка

  if(Tail/*||Head*/ == NULL)         // вставка эл. в конец (в пустой список)
  {
    Tail = new(TListData2<tlT>);     // создание узла
    Tail->Data = InpData;            // загрузка данных
    Tail->Next = NULL;               // следующий
    Tail->Prev = NULL;               // предыдущий
    Head = Tail;
  }
  else                               // вставка эл. в конец (список не пустой)
  {
    PData = new(TListData2<tlT>);    // создание узла
    PData->Data = InpData;           // загрузка данных
    PData->Next = NULL;              // следующий
    PData->Prev = Tail;              // предыдущий

    Tail->Next = PData;              // теперь хвост не последний элемент
    Tail = PData;                    // уст. хвост на вновь созд. эл.
  };

  Count++ ;
  return true;
}

#endif
