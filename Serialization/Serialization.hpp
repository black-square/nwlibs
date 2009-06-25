//Автор: Шестеркин Дмитрий(NW) 2006

#ifndef Serialization_HPP
#define Serialization_HPP

#include <sstream>
#include "..\Auxiliary.h"

/*
   Набор классов которые позволяют используя XML строку сохранять и загружать данные набора вложенных
   классов и контейнеров.
   
   Для того чтобы воспользоваться Сохранением/Загрузкой несоставные элементы (такие как int, double, 
   std::string ) должны выполнять [Правило 1] 

   [Правило 1: Начало]
      Некоторый тип данных должен перегружать функции ReadFromStream, WriteToStream. 
      Реализация этих функций по-умолчанию вызвает опрераторы >> и << соответственно, 
      для заранее определённого потока.

      template< class ObjectT, class CharT, class CharTraitsT >
      void WriteToStream( std::basic_ostream<CharT, CharTraitsT> &Ostream, const ObjectT &Object );

      template< class ObjectT, class CharT, class CharTraitsT >
      void ReadFromStream( std::basic_istream<CharT, CharTraitsT> &Istream, ObjectT &Object );

      Если работа операторов >> и << удовлетворительна, то перегрузку делать не обязательно.
   [Правило 1: Конец]


   Для того чтобы воспользоваться Сохранением/Загрузкой каждый класс (или структура) в котором 
   находятся несколько вложенных переменных должен выполнять [Правило 2] 

   [Правило 2: Начало]
      Класс должен определять метод:       
      template< class Archive > void Serialize( Archive &A ) 

      либо перегрузить функцию 

      template< class Archive >void NWLib::Serialize ( Archive &A, T& ob )
      где T класс который неободимо читать/писать.

      Archive - класс который позволяет считывать/записывать как отдельные переменные, 
      так и вложенные классы и контейнеры, имеет след-ий интерфейс:*/
#if 0
      //Описание общих параметров
      //Accessor  - Класс который позволяет получить доступ к члену структуры данных [Правило 3]
      //
      //SettingsT - Опции чтения/записи, пока можно использовать только интерфейс FindErrorReportT [Правило 4]
      //            функция без этого параметра использует поведение по умолчанию CThrowExceptionOnFindError 
      //            (бросает исключение)


      //Сохранить/Загрузить отдельный член ob структуры данных (class или struct) под именем Name
      //при этом T должен выполнять [Правило 1]
      template< class T >
      void Element( const TChar *Name, /*const*/ T &ob )
      {
         ElementEx( Name, ob, CDefaultSettings() );
      }
      
      template< class T, class SettingsT >
      void ElementEx( const TChar *Name, /*const*/ T &ob, const SettingsT &Settings );

      //Сохранить/Загрузить отдельный член структуры данных (class или struct) ob под именем Name
      //при этом T должен выполнять [Правило 1] 
      template< class T, class AccessorT >
      void Element( const TChar *Name, /*const*/ T &ob, const AccessorT &Accessor )
      {
         ElementEx( Name, ob, Accessor, CDefaultSettings() );
      }
      
      template< class T, class AccessorT, class SettingsT  >
      void Element( const TChar *Name, /*const*/ T &ob, const AccessorT &Accessor, const SettingsT &Settings );

      //Сохранить/Загрузить вложенную структуру ob структуры данных (class или struct) под именем Name
      //При этом T должен выполнять [Правило 2]
      template< class T >
      void Group( const TChar *Name, /*const*/ T &ob )
      {
         GroupEx( Name, ob, CDefaultSettings() );
      }

      template< class T, class SettingsT >
      void GroupEx( const TChar *Name, /*const*/ T &ob, const SettingsT &Settings  );

      //Сохранить/Загрузить вложенную структуру ob структуры данных (class или struct) под именем Name
      //При этом T должен выполнять [Правило 2] 
      template< class T, class AccessorT >
      void Group( const TChar *Name, /*const*/ T &ob, const AccessorT &Accessor );
      {
         GroupEx( Name, ob, Accessor, CDefaultSettings() );
      }

      template< class T, class AccessorT, class SettingsT >
      void Group( const TChar *Name, /*const*/ T &ob, const AccessorT &Accessor, const SettingsT &Settings  );

      //Сохранить/Загрузить STL контейнер ob структуры данных (class или struct) под именем Name.
      //ob содержит простые элементы для которых должно выполняться [Правило 1].
      //Каждый отдельный элемент сохраняется под именем ItemName
      template< class T >
      void STLContainerElement( const TChar *Name, const TChar *ItemName, /*const*/ T &ob )
      {
         STLContainerElementEx( Name, ItemName, ob, CDefaultSettings() );
      }
      
      template< class T, class SettingsT >
      void STLContainerElement( const TChar *Name, const TChar *ItemName, /*const*/ T &ob, const SettingsT &Settings  );

      //Сохранить/Загрузить STL контейнер ob структуры данных (class или struct) под именем Name.
      //ob содержит простые элементы для которых должно выполняться [Правило 1].
      //Каждый отдельный элемент сохраняется под именем ItemName
      template< class T, class AccessorT >
      void STLContainerElement( const TChar *Name, const TChar *ItemName, /*const*/ T &ob, const AccessorT &Accessor )
      {
         STLContainerElementEx( Name, ItemName, ob, Accessor, CDefaultSettings() );
      }

      template< class T, class AccessorT, class SettingsT >
      void STLContainerElement( const TChar *Name, const TChar *ItemName, /*const*/ T &ob, const AccessorT &Accessor, const SettingsT &Settings );

      //Сохранить/Загрузить STL контейнер ob структуры данных (class или struct) под именем Name.
      //ob содержит струтуры данных для которых должно выполянться [Правило 2].
      //Каждый отдельный элемент сохраняется под именем ItemName
      template< class T >
      void STLContainerGroup( const TChar *Name, const TChar *ItemName, /*const*/ T &ob )
      {
         STLContainerGroupEx( Name, ItemName, ob, CDefaultSettings() );
      }
      
      template< class T, class SettingsT >
      void STLContainerGroup( const TChar *Name, const TChar *ItemName, /*const*/ T &ob, const SettingsT &Settings  );

      //Сохранить/Загрузить STL контейнер ob структуры данных (class или struct) под именем Name.
      //ob содержит струтуры данных для которых должно выполянться [Правило 2].
      //Каждый отдельный элемент сохраняется под именем ItemName
      template< class T, class AccessorT >
      void STLContainerGroup( const TChar *Name, const TChar *ItemName, /*const*/ T &ob, const AccessorT &Accessor )
      {
         STLContainerGroupEx( Name, ItemName, ob, Accessor, CDefaultSettings() );
      }
      template< class T, class AccessorT, class SettingsT >
      void STLContainerGroup( const TChar *Name, const TChar *ItemName, /*const*/ T &ob, const AccessorT &Accessor, const SettingsT &Settings  );
#endif
/*
   [Правило 2: Конец]

   [Правило 3: Начало]
      Интерфейс AccessorT
      Существует возможность абстрагироваться от конкретного вида получения 
      параметров при помощи аргумента Accessor в функциях класса CWriteArchive. При этом Accessor
      должен поддерживать следий интерфейс.

      interface AccessorT
      {
         typedef ... ValueType;   //Некоторый тип к которому открывается доступ
         typedef ... ParamValue;  //Тип возвращаемого значенияможет бють либо ValueType, либо const ValueType &

         //Взять у структуры данных занчение требуемого члена
         ParamValue Get( Object &Obj ) const;  
         
         //Взять у структуры данных неконстантную ссылку на значение требуемого члена
         //для последующего вызова Serialize. Метод часто приводит к использованию
         //const_cast
         ValueType &GetRefForSerialize( Object &Obj ) const;  

         //Установить у структуры данных занчение требуемого члена
         //TODO: Разрешить передавать сюда ссылку на Val для того, чтобы вместо копирования можно 
         //было сделать swap
         void Set( Object &Obj, ParamValue Val ) const;
      };

   [Правило 3: Конец]

   [Правило 4: Начало]
      Интерфейс FindErrorReportT
      Используется только при чтении данных, опредиляет способ информирования об ошибке при поиске 
      нужного элемента. 
      В основном используется для того чтобы либо бросить исключение либо ничего не делать и таким 
      образом указать что элемент не обязательный при чтении

      interface FindErrorReportT
      {
         //Вызывается в том случае когда не найден элемент с именем Name
         //в классе чтения Reader
         template<class CharT, class TReader>
         void FindErrorReport(const CharT *Name, const TReader &Reader) const;

      }
   [Правило 4: Конец]


   Для записи необходимо инстанцировать класс CWriteArchive. Затем вызвать метод: */
#if 0  
   //Сохранить все элементы структуры и все вложенные в неё структуры в поток
   //Name - Название корня структуры данных
   //ob - ссылка на структуру данных (class или struct)
   //При этом T должен выполнять [Правило 2]
   template< class T > void Save( const TChar *Name,  T &ob );
#endif
/*
   Для чтения необходимо инстанцировать класс CReadArchive. Затем вызвать метод: */
#if 0  
   //Загрузить все элементы структуры и все вложенные в неё структуры в поток
   //Name - Название корня структуры данных
   //ob - ссылка на структуру данных (class или struct)
   //При этом T должен выполнять [Правило 2]
   template< class T > void Load( const TChar *Name,  T &ob );
#endif


#if 0
//Простой пример (более сложный см. Test\Serialization\Main.cpp)
struct Foo
{
   int m_A;
   double m_B;
   std::basic_string<TCHAR> m_C;

   Foo(): m_A(1), m_B(-12.3e55), m_C(_T("Any String")) {}

   template< class Archive >
   void Serialize( Archive &A ) 
   {
      A.Element( _T("a"), m_A );
      A.Element( _T("b"), m_B );
      A.Element( _T("c"), m_C );
   }
};

struct Bar
{
   Foo F1;
   Foo F2;
   std::basic_string<TCHAR> m_Name;
   
   Bar(): m_Name(_T("Any Name")) {}

   template< class Archive >
   void Serialize( Archive &A ) 
   {
      A.Element( _T("name"), m_Name );
      A.Group( _T("foo1"), F1 );
      A.Group( _T("foo2"), F2 );
   }

   basic_string<TCHAR> GetXML()
   {
      basic_stringstream<TCHAR> Stream;
      Bar B;
      CWriteArchive<basic_stringstream<TCHAR> > Archive(Stream);
      Archive.Save( _T("bar"), B );
      return Stream.str();
   }
};

/* GetXML() возвращает:
<bar>
	<name>Any Name</name>
	<foo1>
		<a>1</a>
		<b>-1.23e+056</b>
		<c>Any String</c>
	</foo1>
	<foo2>
		<a>1</a>
		<b>-1.23e+056</b>
		<c>Any String</c>
	</foo2>
</bar>
*/

#endif

namespace NWLib {

///////////////////////////////////////////////////////////////////////////////
// Функции обеспечивающие действия по-умолчанию, но позволяющие изменить его с 
// помощью перегрузки
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Поведение по умолчанию для классов которые планируется читать-писать.
// Вызов метода Serialize. Можно перегружать для отдельных классов, а не писать
// функцию-член.
///////////////////////////////////////////////////////////////////////////////
template< class Archive, class T >
inline void Serialize( Archive &A, T& ob )
{
   ob.Serialize( A );
}

///////////////////////////////////////////////////////////////////////////////
// Запись в поток по-умолчанию. Вызов оператора <<
///////////////////////////////////////////////////////////////////////////////
template< class ObjectT, class CharT, class CharTraitsT >
inline void WriteToStream( std::basic_ostream<CharT, CharTraitsT> &Ostream, const ObjectT &Object )
{
   Ostream << Object;
}

///////////////////////////////////////////////////////////////////////////////
// Чтение из потока по-умолчанию. Вызов оператора >>
///////////////////////////////////////////////////////////////////////////////
template< class ObjectT, class CharT, class CharTraitsT >
inline void ReadFromStream( std::basic_istream<CharT, CharTraitsT> &Istream, ObjectT &Object )
{
   Istream >> Object;
}

template< class CharT, class CharTraitsT >
inline void ReadFromStream( std::basic_istream<CharT, CharTraitsT> &Istream, std::basic_string<CharT, CharTraitsT> &Object )
{
#if 0
   std::getline( Istream, Object );

   //Если мы прочитали пустую строку, то будет установлен флаг failbit, 
   //а мы должны разрешить чтение пустых строк, поэтому сбрасываем это флаг
   Istream.clear( Istream.rdstate() & ~std::ios::failbit );
#else  
   CharT Buf[100];
   
   Object.clear();

   do 
   {
      Istream.read( Buf, APL_ARRSIZE(Buf) );    
      Object.append( Buf, Buf + Istream.gcount() );
   } 
   while( Istream.gcount() == APL_ARRSIZE(Buf) );

   //Мы прочитали до конца фала и поэтому был установлен флаг failbit, сбрасываем его
   Istream.clear( Istream.rdstate() & ~std::ios::failbit );
#endif
}


///////////////////////////////////////////////////////////////////////////////
// Классы позволяющие абстрагироваться от конкретного формата файла. 
// Необходимо только чтобы каждый формат поддерживал уровни вложенности информации и 
// позволял хранить даные на каждом уровне.
// Уровнем вложенности можно назвать приём аналогичный тому, как одна структура вложена 
// в другую. При этом можно считать то, что уровни не пересекаются, т.е. сначала обязательно 
// должен закончится более глубокий уровень, а потом более поверхностный.
// Каждый класс реализует одни и те же интерфейсы. Затем от них наследуются 
// классы для непосредственной записи и чтения.
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/*
//Интефейс для всех классов форматов файлов для записи
//TOstream - Класс который определяет оператор << для вывода переменных, либо
//           для которого перегружена функция WriteToStream
template < class CharT, class CharTraitsT = std::char_traits<CharT> >
interface IWriter
{
protected:
   typedef std::basic_ostream< CharT, CharTraitsT > TOstream;
   typedef typename CharT TChar; 
   
   explicit IWriter( TOstream &Os );

   //Начать новый увовень вложенности с именем Name
   void BeginLevel( const TChar *Name );
   
   //Закончить уровень вложенности с именем Name
   void EndLevel( const TChar *Name );
   
   //Сохранить Ob с именем Name
   //T должен выполнять [Правило 1]
   template< class T >
   void SaveItem( const TChar *Name, const T &Ob );

   //Начать/Закончить сохранение
   void BeginSave();
   void EndSave();
};
*/
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
/*
//Интефейс для всех классов форматов файлов для чтения
//TIstream - Класс который определяет оператор >> для ввода переменных, либо
//           для которого перегружена функция ReadFromStream
template < class CharT, class CharTraitsT = std::char_traits<CharT> >
interface IReader
{
protected:
   typedef std::basic_istream< CharT, CharTraitsT > TIstream;  
   typedef typename CharT TChar;  
   
   //Курсор, инкапсулирует внутреннюю реализацию поиска
   class TFindCursor{};
 
   explicit IReader( TIstream &Is );

   //Перейти на уровень Name
   //Возв: Найден ли уровень с именем Name
   bool BeginLevel( const TChar *Name );
   
   //Выйти с уровня Name
   void EndLevel( const TChar *Name );
   
   //Загрузить Ob с именем Name
   //T должен выполнять [Правило 1]
   //Возв: Найден ли элемент с именем Name   
   template< class T >
   bool LoadItem( const TChar *Name, T &Ob );

   //Инициализировать поиск всех элементов с именем Name
   void FindInit( const TChar *Name, TFindCursor &FindCursor );
   
   //Найти следующий (в т.ч. первый) элемент и записать его значение в ob.
   //FindCursor - должен быть заранее инициализирован функцией FindInit
   //Возвр - true элемент успешно записан, false - элемент найти не удалось
   //T должен выполнять [Правило 1]
   template< class T >
   bool FindNext( TFindCursor &FindCursor, T &Ob);

   //Найти следующий (в т.ч. первый) элемент.
   //FindCursor - должен быть заранее инициализирован функцией FindInit
   //Возвр - true элемент успешно записан, false - элемент найти не удалось
   template< class T >
   bool FindNext( TFindCursor &FindCursor);

   //Начать/Закончить загрузку
   void BeginLoad();
   void EndLoad();

public:
   //Получить путь к текущему узлу
   std::basic_string<TCHAR> GetCurNodePath() const;
};
*/
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Класс для записи параметров. 
// WriterT - Класс реализующий формат записи
///////////////////////////////////////////////////////////////////////////////
template<class CharT, template<class, template<class> class> class WriterT, template<class> class CharTraits = std::char_traits >
class CWriteArchive: public WriterT<CharT, CharTraits>
{
public:
   typedef WriterT<CharT, CharTraits> TWriter;
   typedef typename TWriter::TChar TChar;
   typedef typename TWriter::TOstream TOstream;

public:
   //Конструктор должен обязательно принимать ссылку на поток
   explicit CWriteArchive( TOstream &Os ): TWriter( Os ) {}
   
   //Сохранить все элементы структуры и все вложенные в неё структуры в поток
   //Name - Название корня структуры данных
   //ob - ссылка на структуру данных (class или struct)
   //При этом T должен выполнять [Правило 2]
   template< class T >
   void Save( const TChar *Name, /*const*/ T &ob )
   {
      TWriter::BeginSave();
      TWriter::BeginLevel(Name);
      Serialize( *this, ob );
      TWriter::EndLevel( Name );
      TWriter::EndSave();
   }

   //Сохранить отдельный член ob структуры данных (class или struct) под именем Name
   //при этом T должен выполнять [Правило 1]
   template< class T, class SettingsT >
   void ElementEx( const TChar *Name, /*const*/ T &ob, const SettingsT &Settings  )
   {
      TWriter::SaveItem( Name, ob );
   }

   template< class T >
   void Element( const TChar *Name, /*const*/ T &ob )
   {
      ElementEx( Name, ob, CDefaultSettings() );
   }


   //Сохранить отдельный член структуры данных (class или struct) ob под именем Name
   //при этом T должен выполнять [Правило 1] 
   template< class T, class AccessorT, class SettingsT >
   void ElementEx( const TChar *Name, /*const*/ T &ob, const AccessorT &Accessor, const SettingsT &Settings  )
   {
      TWriter::SaveItem( Name, Accessor.Get(ob) );
   }

   template< class T, class AccessorT >
   void Element( const TChar *Name, /*const*/ T &ob, const AccessorT &Accessor )
   {
      ElementEx( Name, ob, Accessor, CDefaultSettings() );
   }

   //Сохранить вложенную структуру ob структуры данных (class или struct) под именем Name
   //При этом T должен выполнять [Правило 2]
   template< class T, class SettingsT >
   void GroupEx( const TChar *Name, /*const*/ T &ob, const SettingsT &Settings  )
   {
      TWriter::BeginLevel( Name );
      Serialize( *this, ob );
      TWriter::EndLevel( Name );
   }

   template< class T >
   void Group( const TChar *Name, /*const*/ T &ob )
   {
      GroupEx( Name, ob, CDefaultSettings() );
   }

   //Сохранить вложенную структуру ob структуры данных (class или struct) под именем Name
   //При этом T должен выполнять [Правило 2] 
   template< class T, class AccessorT, class SettingsT >
   void GroupEx( const TChar *Name, /*const*/ T &ob, const AccessorT &Accessor, const SettingsT &Settings  )
   {
      TWriter::BeginLevel( Name );
      Serialize( *this, Accessor.GetRefForSerialize(ob) );
      TWriter::EndLevel( Name );
   }

   template< class T, class AccessorT >
   void Group( const TChar *Name, /*const*/ T &ob, const AccessorT &Accessor )
   {
      GroupEx( Name, ob, Accessor, CDefaultSettings() );
   }

   //Сохранить STL контейнер ob структуры данных (class или struct) под именем Name.
   //ob содержит простые элементы для которых должно выполняться [Правило 1].
   //Каждый отдельный элемент сохраняется под именем ItemName
   template< class T, class SettingsT >
   void STLContainerElementEx( const TChar *Name, const TChar *ItemName, /*const*/ T &ob, const SettingsT &Settings  )
   {
      TWriter::BeginLevel( Name );
      
      for( typename T::const_iterator I = ob.begin(); I != ob.end(); ++I )
         TWriter::SaveItem( ItemName, *I );

      TWriter::EndLevel( Name );
   }

   template< class T >
   void STLContainerElement( const TChar *Name, const TChar *ItemName, /*const*/ T &ob )
   {
      STLContainerElementEx( Name, ItemName, ob, CDefaultSettings() );
   }

   
   //Сохранить STL контейнер ob структуры данных (class или struct) под именем Name.
   //ob содержит простые элементы для которых должно выполняться [Правило 1].
   //Каждый отдельный элемент сохраняется под именем ItemName
   template< class T, class AccessorT, class SettingsT >
   void STLContainerElementEx( const TChar *Name, const TChar *ItemName, /*const*/ T &ob, const AccessorT &Accessor, const SettingsT &Settings  )
   {
      TWriter::BeginLevel( Name );
      
      for( typename AccessorT::ValueType::const_iterator I = Accessor.Get(ob).begin(); I != Accessor.Get(ob).end(); ++I )
         TWriter::SaveItem( ItemName, *I );

      TWriter::EndLevel( Name );
   }

   template< class T, class AccessorT >
   void STLContainerElement( const TChar *Name, const TChar *ItemName, /*const*/ T &ob, const AccessorT &Accessor )
   {
      STLContainerElementEx( Name, ItemName, ob, Accessor, CDefaultSettings() );
   }

   //Сохранить STL контейнер ob структуры данных (class или struct) под именем Name.
   //ob содержит струтуры данных для которых должно выполянться [Правило 2].
   //Каждый отдельный элемент сохраняется под именем ItemName
   template< class T, class SettingsT >
   void STLContainerGroupEx( const TChar *Name, const TChar *ItemName, /*const*/ T &ob, const SettingsT &Settings  )
   {
      TWriter::BeginLevel( Name );

      for( typename T::iterator I = ob.begin(); I != ob.end(); ++I )
      {
         TWriter::BeginLevel( ItemName );
         Serialize( *this, *I );
         TWriter::EndLevel( ItemName );
      }

      TWriter::EndLevel( Name );
   }

   template< class T >
   void STLContainerGroup( const TChar *Name, const TChar *ItemName, /*const*/ T &ob )
   {
      STLContainerGroupEx( Name, ItemName, ob, CDefaultSettings() );
   }
   
   //Сохранить STL контейнер ob структуры данных (class или struct) под именем Name.
   //ob содержит струтуры данных для которых должно выполянться [Правило 2].
   //Каждый отдельный элемент сохраняется под именем ItemName
   template< class T, class AccessorT, class SettingsT >
   void STLContainerGroupEx( const TChar *Name, const TChar *ItemName, /*const*/ T &ob, const AccessorT &Accessor, const SettingsT &Settings  )
   {
      TWriter::BeginLevel( Name );

      for( typename AccessorT::ValueType::iterator I = Accessor.GetRefForSerialize(ob).begin(); I != Accessor.GetRefForSerialize(ob).end(); ++I )
      {
         TWriter::BeginLevel( ItemName );
         Serialize( *this, *I );
         TWriter::EndLevel( ItemName );
      }

      TWriter::EndLevel( Name );
   }

   template< class T, class AccessorT >
   void STLContainerGroup( const TChar *Name, const TChar *ItemName, /*const*/ T &ob, const AccessorT &Accessor )
   {
      STLContainerGroupEx( Name, ItemName, ob, Accessor, CDefaultSettings() );
   }
};

///////////////////////////////////////////////////////////////////////////////
// Класс для чтения параметров. 
// ReaderT - Класс реализующий формат чтения
///////////////////////////////////////////////////////////////////////////////
template<class CharT, template<class, template<class> class> class ReaderT, template<class> class CharTraits = std::char_traits >
class CReadArchive: public ReaderT<CharT, CharTraits>
{
public:
   typedef ReaderT<CharT, CharTraits> TReader;
   typedef typename TReader::TChar TChar;
   typedef typename TReader::TIstream TIstream;
   typedef typename TReader::TFindCursor TFindCursor;

public:
   //Конструктор должен обязательно принимать ссылку на поток
   CReadArchive( TIstream &Is ): TReader( Is ) {}
   
   //Загрузить все элементы структуры и все вложенные в неё структуры в поток
   //Name - Название корня структуры данных
   //ob - ссылка на структуру данных (class или struct)
   //При этом T должен выполнять [Правило 2]
   template< class T >
   void Load( const TChar *Name, T &ob )
   {
      TReader::BeginLoad();
      
      if( !TReader::BeginLevel( Name ) )
      {
         CDefaultSettings().FindErrorReport( Name, static_cast<const TReader &>(*this) );
         return;
      }
      
      Serialize( *this, ob );
      TReader::EndLevel( Name );
      TReader::EndLoad();
   }

   //Загрузить отдельный член ob структуры данных (class или struct) под именем Name
   //при этом T должен выполнять [Правило 1]
   template< class T, class SettingsT >
   void ElementEx( const TChar *Name, T &ob, const SettingsT &Settings  )
   {
      if( !TReader::LoadItem(Name, ob) )
         Settings.FindErrorReport( Name, static_cast<const TReader &>(*this) );
   }

   template< class T >
   void Element( const TChar *Name, T &ob )
   {
      ElementEx( Name, ob, CDefaultSettings() );
   }

   //Загрузить отдельный член структуры данных (class или struct) ob под именем Name
   //при этом T должен выполнять [Правило 1] 
   template< class T, class AccessorT, class SettingsT >
   void ElementEx( const TChar *Name, T &ob, const AccessorT &Accessor, const SettingsT &Settings )
   {
      typename AccessorT::ValueType TmpOb;
      
      if( !TReader::LoadItem(Name, TmpOb) )
      {
         Settings.FindErrorReport( Name, static_cast<const TReader &>(*this) );
         return;
      }

      Accessor.Set(ob, TmpOb);
   }

   template< class T, class AccessorT >
   void Element( const TChar *Name, T &ob, const AccessorT &Accessor )
   {
      ElementEx( Name, ob, Accessor, CDefaultSettings() );
   }

   //Загрузить вложенную структуру ob структуры данных (class или struct) под именем Name
   //При этом T должен выполнять [Правило 2]
   template< class T, class SettingsT >
   void GroupEx( const TChar *Name, T &ob, const SettingsT &Settings  )
   {
      if( !TReader::BeginLevel(Name) )
      {
         Settings.FindErrorReport( Name, static_cast<const TReader &>(*this) );
         return;
      }

      Serialize( *this, ob );
      TReader::EndLevel( Name );
   }

   template< class T >
   void Group( const TChar *Name, T &ob )
   {
      GroupEx( Name, ob, CDefaultSettings() );
   }

   //Загрузить вложенную структуру ob структуры данных (class или struct) под именем Name
   //При этом T должен выполнять [Правило 2] 
   template< class T, class AccessorT, class SettingsT >
   void GroupEx( const TChar *Name, T &ob, const AccessorT &Accessor, const SettingsT &Settings )
   {
      if( !TReader::BeginLevel(Name) )
      {
         Settings.FindErrorReport( Name, static_cast<const TReader &>(*this) );
         return;
      }

      Serialize( *this, Accessor.GetRefForSerialize(ob) );
      TReader::EndLevel( Name );
   }

   template< class T, class AccessorT >
   void Group( const TChar *Name, T &ob, const AccessorT &Accessor )
   {
      GroupEx( Name, ob, Accessor, CDefaultSettings() );
   }

   //Загрузить STL контейнер ob структуры данных (class или struct) под именем Name.
   //ob содержит простые элементы для которых должно выполняться [Правило 1].
   //Каждый отдельный элемент сохраняется под именем ItemName
   template< class T, class SettingsT >
   void STLContainerElementEx( const TChar *Name, const TChar *ItemName, T &ob, const SettingsT &Settings )
   {
      typename T::value_type TmpOb;
      TFindCursor FindCursor;

      //TODO: Вместо этого создавать новый контейнер и в конце делать swap,
      //      хотя так медленнее для вектора. Подумать.
      ob.clear();

      if( !TReader::BeginLevel(Name) )
      {
         Settings.FindErrorReport( Name, static_cast<const TReader &>(*this) );
         return;
      }

      TReader::FindInit( ItemName, FindCursor );

      while( TReader::FindNext(FindCursor, TmpOb) )
         ob.push_back( TmpOb );

      TReader::EndLevel( Name );
   }

   template< class T >
   void STLContainerElement( const TChar *Name, const TChar *ItemName, T &ob )
   {
      STLContainerElementEx( Name, ItemName, ob, CDefaultSettings() );
   }
   
   //Загрузить STL контейнер ob структуры данных (class или struct) под именем Name.
   //ob содержит простые элементы для которых должно выполняться [Правило 1].
   //Каждый отдельный элемент сохраняется под именем ItemName
   template< class T, class AccessorT, class SettingsT >
   void STLContainerElementEx( const TChar *Name, const TChar *ItemName, T &ob, const AccessorT &Accessor, const SettingsT &Settings  )
   {
      typename AccessorT::ValueType TmpContainer;
      typename AccessorT::ValueType::value_type TmpValue;

      TFindCursor FindCursor;

      if( !TReader::BeginLevel(Name) )
      {
         Settings.FindErrorReport( Name, static_cast<const TReader &>(*this) );
         return;
      }

      TReader::FindInit( ItemName, FindCursor );

      while( TReader::FindNext(FindCursor, TmpValue) )
      {
         TmpContainer.push_back(TmpValue);
      }

      TReader::EndLevel( Name );

      Accessor.Set(ob, TmpContainer);
   }

   template< class T, class AccessorT >
   void STLContainerElement( const TChar *Name, const TChar *ItemName, T &ob, const AccessorT &Accessor )
   {
      STLContainerElementEx( Name, ItemName, ob, Accessor, CDefaultSettings() );
   }

   //Загрузить STL контейнер ob структуры данных (class или struct) под именем Name.
   //ob содержит струтуры данных для которых должно выполянться [Правило 2].
   //Каждый отдельный элемент сохраняется под именем ItemName
   template< class T, class SettingsT >
   void STLContainerGroupEx( const TChar *Name, const TChar *ItemName, T &ob, const SettingsT &Settings )
   {
      typename T::value_type TmpOb;
      TFindCursor FindCursor;

      ob.clear();

      if( !TReader::BeginLevel(Name) )
      {
         Settings.FindErrorReport( Name, static_cast<const TReader &>(*this) );
         return;
      }

      TReader::FindInit( ItemName, FindCursor );

      while( TReader::FindNext(FindCursor) )
      {
         Serialize( *this, TmpOb );
         ob.push_back( TmpOb );
      }

      TReader::EndLevel( Name );
   }

   template< class T >
   void STLContainerGroup( const TChar *Name, const TChar *ItemName, T &ob )
   {
      STLContainerGroupEx( Name, ItemName, ob, CDefaultSettings() );
   }
  
   //Загрузить STL контейнер ob структуры данных (class или struct) под именем Name.
   //ob содержит струтуры данных для которых должно выполянться [Правило 2].
   //Каждый отдельный элемент сохраняется под именем ItemName
   template< class T, class AccessorT, class SettingsT >
   void STLContainerGroupEx( const TChar *Name, const TChar *ItemName,  T &ob, const AccessorT &Accessor, const SettingsT &Settings  )
   {
      typename AccessorT::ValueType TmpContainer;
      typename AccessorT::ValueType::value_type TmpValue;

      TFindCursor FindCursor;

      if( !TReader::BeginLevel(Name) )
      {
         Settings.FindErrorReport( Name, static_cast<const TReader &>(*this) );
         return;
      }

      TReader::FindInit( ItemName, FindCursor );

      while( TReader::FindNext(FindCursor) )
      {
         Serialize( *this, TmpValue );
         TmpContainer.push_back(TmpValue);
      }

      TReader::EndLevel( Name );

      Accessor.Set(ob, TmpContainer);
   }

   template< class T, class AccessorT >
   void STLContainerGroup( const TChar *Name, const TChar *ItemName, T &ob, const AccessorT &Accessor )
   {
      STLContainerGroupEx( Name, ItemName, ob, Accessor, CDefaultSettings() );
   }
};

///////////////////////////////////////////////////////////////////////////////
// Реализация интерфейса AccessorT [Правило 3] при помощи указателей на функции Set/Get
// Для использования необходимо вызвать функцию 
//    SetGetBind</*Тип структуры данных*/, /*Тип получаемого устанавлемого члена*/>(/*Указатель на функцию член Set*/, /*Указатель на функцию член Get*/)
//    Оба указатели на функции члены могут быть 0;
///////////////////////////////////////////////////////////////////////////////
template< class Object, class T >
struct CSetGetBindImpl
{
   typedef T ValueType;
   typedef const ValueType &ParamValue;
   typedef void (Object::*SetFunc)(ParamValue);
   typedef ParamValue (Object::*GetFunc)() const;
   
   CSetGetBindImpl( SetFunc SF, GetFunc GF ): m_SetFunc(SF), m_GetFunc(GF) {}
   ParamValue Get( Object &Obj ) const { APL_ASSERT_PTR(m_GetFunc); return (Obj.*m_GetFunc)(); }
   T &GetRefForSerialize( Object &Obj ) const { return const_cast<T &>( Get(Obj) ); }
   void Set( Object &Obj, ParamValue Val ) const { APL_ASSERT_PTR(m_SetFunc); (Obj.*m_SetFunc)(Val); }

private:
   SetFunc m_SetFunc;
   GetFunc m_GetFunc;
};

template< class Object, class T >
struct CSetGetBindImplNoRef
{
   typedef T ValueType;
   typedef ValueType ParamValue;
   typedef void (Object::*SetFunc)(ParamValue);
   typedef ValueType (Object::*GetFunc)() const;

   CSetGetBindImplNoRef( SetFunc SF, GetFunc GF ): m_SetFunc(SF), m_GetFunc(GF) {}
   ValueType Get( Object &Obj ) const { APL_ASSERT_PTR(m_GetFunc); return (Obj.*m_GetFunc)(); }
   T &GetRefForSerialize( Object &Obj ) const 
   { 
      //!!!Не по стандарту!!! 
      //Нельзя взять неконстантную ссылку на rvalue
      return const_cast<T &>( Get(Obj) ); 
   }
   void Set( Object &Obj, ValueType Val ) const { APL_ASSERT_PTR(m_SetFunc); (Obj.*m_SetFunc)(Val); }

private:
   SetFunc m_SetFunc;
   GetFunc m_GetFunc;
};


template< class Object, class T >
CSetGetBindImpl<Object, T> SetGetBind(void (Object::*SetFunc)(const T &), const T &(Object::*GetFunc)() const )
{
   return CSetGetBindImpl<Object, T>( SetFunc, GetFunc);
}

template< class Object, class T >
CSetGetBindImplNoRef<Object, T> SetGetBind(void (Object::*SetFunc)(T), T (Object::*GetFunc)() const )
{
   return CSetGetBindImplNoRef<Object, T>(SetFunc, GetFunc);
}


///////////////////////////////////////////////////////////////////////////////
// Реализация интерфейса FindErrorReportT [Правило 4]
///////////////////////////////////////////////////////////////////////////////
//Бросить исключение при неудачной попытке поиска
//ИСПОЛЬЗУЕТСЯ ПО УМОЛЧАНИЮ
struct CThrowExceptionOnFindError
{
   //Вызывается в том случае когда не найден элемент с именем Name
   template<class CharT, class TReader>
   void FindErrorReport(const CharT *Name, const TReader &Reader) const
   {
      APL_THROW( _T("Не найден узел '") << Reader.GetCurNodePath() << _T("/") << ConvertToTStr(Name) << _T("'") );
   }
};

//Ничего не делать если элемент не найден. Т.е. элемент не обязательный
struct CNotMandatory
{
   //Вызывается в том случае когда не найден элемент с именем Name
   template<class CharT, class TReader>
   void FindErrorReport(const CharT *Name, const TReader &Reader) const
   {
      // Ничего не делаем
   }
};

///////////////////////////////////////////////////////////////////////////////
// Класс для настроек по умолчанию
///////////////////////////////////////////////////////////////////////////////
struct CDefaultSettings: public CThrowExceptionOnFindError {};

///////////////////////////////////////////////////////////////////////////////
// Дополнительные функции облегчающие работу
///////////////////////////////////////////////////////////////////////////////

//Получить по классу Object в котором определен метод Serialize XML строку OutXML с 
//названием корневого тега RootName
template< template<class, template<class> class > class WriterT, class Object, class CharInT, class CharOutT >
void GetSerializeXMLString( Object &Object, const CharInT *RootName, std::basic_string<CharOutT> &OutXML, std::ios_base::fmtflags FmtFlags = 0 )
{
   std::basic_stringstream<CharOutT> Stream;

   Stream.setf( FmtFlags );

   CWriteArchive<CharInT, WriterT> Archive(Stream);
   Archive.Save( RootName, Object );
   OutXML = Stream.str();
}

//Прочитать из XML строки, с корневым тегом RootName, объект Object в котором определен метод Serialize
template< template<class, template<class> class> class ReaderT, class Object, class CharT >
void SetSerializeXMLString( Object &Object, const CharT *RootName, const std::basic_string<CharT> &InXML, std::ios_base::fmtflags FmtFlags = 0 )
{
   std::basic_stringstream<CharT> Stream(InXML);

   Stream.setf( FmtFlags );

   CReadArchive<CharT, ReaderT> Archive(Stream);
   Archive.Load( RootName, Object );
}

} //namespace NWLib 

#endif