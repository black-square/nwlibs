//Автор: Шестеркин Дмитрий(NW) 2005

#ifndef StlAuxiliary_HPP
#define StlAuxiliary_HPP

/*
   Модуль предоставляет несколько классов и функций которые могут служить расширением stl
*/
#include <iterator>
#include "strings.hpp"

namespace NWLib {

///////////////////////////////////////////////////////////////////////////////
// Пустой тип
///////////////////////////////////////////////////////////////////////////////
class NullType {};

///////////////////////////////////////////////////////////////////////////////
// Запрещает копирование объекта
///////////////////////////////////////////////////////////////////////////////
class NonCopyable
{
protected:
   NonCopyable() {}
   ~NonCopyable() {}
private:  
   NonCopyable( const NonCopyable& );
   NonCopyable& operator=( const NonCopyable& ); 
};

///////////////////////////////////////////////////////////////////////////////
// Запрещает создавать класс любым методом который вызывает конструктор и 
// удалять любым методом который вызывает деструктор.
// Например это можно использовать при запрещении явного создания объектов.
// Для предотвращения предупрежедения (C4624) о том что деструктор не может быть 
// сгенерирован надо явно его объявить (не опредилить) в произодном классе
///////////////////////////////////////////////////////////////////////////////
struct NonCreateable 
{
private:
   NonCreateable();    
   ~NonCreateable();  
};

////////////////////////////////////////////////////////////////////////////////
// class template Int2Type
// Converts each integral constant into a unique type
// Invocation: Int2Type<v> where v is a compile-time constant integral
// Defines 'value', an enum that evaluates to v
// Реализация из loki
////////////////////////////////////////////////////////////////////////////////
template <int v>
struct Int2Type
{
   enum { value = v };
};

////////////////////////////////////////////////////////////////////////////////
// class template Type2Type
// Converts each type into a unique, insipid type
// Invocation Type2Type<T> where T is a type
// Defines the type OriginalType which maps back to T
// Реализация из loki
////////////////////////////////////////////////////////////////////////////////
template <typename T>
struct Type2Type
{   
   typedef T OriginalType;
   Type2Type(){} // VC7
};

////////////////////////////////////////////////////////////////////////////////
// class template Select
// Selects one of two types based upon a boolean constant
// Invocation: Select<flag, T, U>::Result
// where:
// flag is a compile-time boolean constant
// T and U are types
// Result evaluates to T if flag is true, and to U otherwise.
// Реализация из loki
////////////////////////////////////////////////////////////////////////////////
template <bool flag, typename T, typename U>
struct Select
{
private:
   template<bool>
   struct In 
   { typedef T Result; };

   template<>
   struct In<false>
   { typedef U Result; };

public:
   typedef typename In<flag>::Result Result;
};


////////////////////////////////////////////////////////////////////////////////
// class template IsSameType
// Return true if two given types are the same
// Invocation: IsSameType<T, U>::value
// where:
// T and U are types
// Result evaluates to true if U == T (types equal)
// Реализация из loki
////////////////////////////////////////////////////////////////////////////////
template <typename T, typename U>
struct IsSameType
{
private:
   template<typename>
   struct In 
   { enum { value = false }; };

   template<>
   struct In<T>
   { enum { value = true };  };

public:
   enum { value = In<U>::value };
};

///////////////////////////////////////////////////////////////////////////////
// Убирает квалификатор const, если таковой имеется у типа.
// Реализация из loki
///////////////////////////////////////////////////////////////////////////////
template <class U> struct UnConst
{
   typedef U Result;
   enum { isConst = 0 };
};

template <class U> struct UnConst<const U>
{
   typedef U Result;
   enum { isConst = 1 };
};

////////////////////////////////////////////////////////////////////////////////
// class template Conversion
// Figures out the conversion relationships between two types
// Invocations (T and U are types):
// a) Conversion<T, U>::exists
// returns (at compile time) true if there is an implicit conversion from T
// to U (example: Derived to Base)
// b) Conversion<T, U>::exists2Way
// returns (at compile time) true if there are both conversions from T
// to U and from U to T (example: int to char and back)
// c) Conversion<T, U>::sameType
// returns (at compile time) true if T and U represent the same type
//
// Caveat: might not work if T and U are in a private inheritance hierarchy.
// Реализация из loki
////////////////////////////////////////////////////////////////////////////////
template <class T, class U>
struct Conversion
{
   template <class T2, class U2>
   struct ConversionHelper
   {
      typedef char Small;
      struct Big { char dummy[2]; };
      static Big   Test(...);
      static Small Test(U2);
      static T2 MakeT();
   };
  
   typedef ConversionHelper<T, U> H;
#ifndef __MWERKS__
   enum { exists = sizeof(typename H::Small) == sizeof((H::Test(H::MakeT()))) };
#else
   enum { exists = false };
#endif
   enum { exists2Way = exists && Conversion<U, T>::exists };
   enum { sameType = false };
};

template <class T>
struct Conversion<T, T>    
{
   enum { exists = 1, exists2Way = 1, sameType = 1 };
};

template <class T>
struct Conversion<void, T>    
{
   enum { exists = 0, exists2Way = 0, sameType = 0 };
};

template <class T>
struct Conversion<T, void>    
{
   enum { exists = 0, exists2Way = 0, sameType = 0 };
};

template <>
struct Conversion<void, void>    
{
public:
   enum { exists = 1, exists2Way = 1, sameType = 1 };
};

///////////////////////////////////////////////////////////////////////////////
// Создание из одного типа второго, только в том случае если это возможно. 
// Если создание невозможно то первый тип создаётся из конструктора по умолчанию
// Синтаксис:
//    T1 t1;
//    T2 t2(TryConvertOrCreate<T1, T2>::Get(t1));
// Синтаксис 2:
//    T1 t1;
//    T2 t2;
//    t2 = TryConvertOrCreate<T1, T2>::Get(t1);
// Принцип работы:
//    Если возможно преобразование из T1 в T2 то функция Get возвратит ссылку 
//    на t1 и t2 корректно создаться из t1, а если преобразование не определено
//    то Get возвратит T2 созданный при помощи конструктора по умолчанию и t2
//    создаться из конструктора копирования.
///////////////////////////////////////////////////////////////////////////////
namespace Private
{
   template<class FromT, class ToT, bool Flag>
   struct TryConvertOrCreateHelper
   {
      static const FromT &Get( const FromT &From ) { return From; }
   };

   template<class FromT, class ToT>
   struct TryConvertOrCreateHelper<FromT, ToT, false>
   {
      static ToT Get( const FromT &From ) { return ToT(); }
   };
} //namespace Private

template<class FromT, class ToT>
struct TryConvertOrCreate: Private::TryConvertOrCreateHelper<FromT, ToT, Conversion<FromT, ToT>::exists>{};

///////////////////////////////////////////////////////////////////////////////
// Позволяет считать количество инстанцированных в данный момент объектов
///////////////////////////////////////////////////////////////////////////////
template< class T >
class CObjectCount
{
   static size_t m_N;

private:
   static void Increment(){ APL_ASSERT(m_N < std::numeric_limits<size_t>::max() ); ++m_N; }
   static void Decrement(){ APL_ASSERT( m_N > 0 ); --m_N; }

public:
   CObjectCount() { Increment(); }
   CObjectCount( const CObjectCount& ) { Increment(); }
   ~CObjectCount() { Decrement(); }

   static size_t Count(){ return m_N; }
};

template< class T >
size_t CObjectCount<T>::m_N = 0;

///////////////////////////////////////////////////////////////////////////////
// Делает тоже, что std::advance, но не меняет переданный итератор и возвращает
// полученный итератор
///////////////////////////////////////////////////////////////////////////////
template <class InputIterator, class Distance>
inline InputIterator AdvanceBuf(InputIterator i, Distance n)
{
   std::advance( i, n );
   return i;
}

///////////////////////////////////////////////////////////////////////////////
// Получить следующий / предыдущий итератор. 
// Нужно в связи с тем, что --set.end() работает, а --vector.end() уже может 
// и нет, т.к. итератор вектора может быть простым указателем, а изменять rvalue
// встроенных типов нельзя
///////////////////////////////////////////////////////////////////////////////
template <class InputIterator> inline InputIterator Next(InputIterator i) { return ++i; }
template <class BidirectionalIterator> inline BidirectionalIterator Prev(BidirectionalIterator i) { return --i; }
   
///////////////////////////////////////////////////////////////////////////////
// Проверить то, что итератор Iter находится в диапозоне [First, Last)
// Используется в assert'ах
// Два разных типа InputIterator1 и InputIterator2 вместо одного используются 
// для того чтобы можно было вывести сравнение iterator с const_iterator
///////////////////////////////////////////////////////////////////////////////
template <class InputIterator1, class InputIterator2>
bool CheckInRange( InputIterator1 First, InputIterator1 Last, InputIterator2 Iter )
{
   while( First != Last ) 
   {
      if( First == Iter ) return true;

      ++First;
   }

   return false;
}

///////////////////////////////////////////////////////////////////////////////
// Проверить то, что итератор Iter находится в диапозоне [Cont.begin(), Cont.end())
// Используется в assert'ах
///////////////////////////////////////////////////////////////////////////////
template <class Container>
inline bool CheckInRange( const Container &Cont, typename Container::const_iterator Iter )
{
   return CheckInRange( Cont.begin(), Cont.end(), Iter );
}

///////////////////////////////////////////////////////////////////////////////
// Функция позволяет эффективно вставить в std::map M значение (K, V) если 
// значения по ключу K нет, или в противном случае оновить значение по ключу K.
// Эквивалентно M[K] = V, но более эффективно т.к. исключает дополнительное 
// копирование
// Возвр: Итератор значения по ключу K
// Подробнее см. Мейерс - Эффективное использование STL, совет 24
///////////////////////////////////////////////////////////////////////////////
template< class MapType > 
typename MapType::iterator MapEfficientAddOrUpdate(
   MapType &M, 
   const typename MapType::key_type &K, 
   const typename MapType::mapped_type &V
)
{
   typename MapType::iterator lb = M.lower_bound(K);						

   if ( lb != M.end() && !M.key_comp()(K, lb->first) ) 
   {	
      lb->second = V;							
      return lb;								
   }											
   else 
   {
      typedef typename MapType::value_type MVT;
      return M.insert( lb, MVT(K, V) );			
   }											
}

///////////////////////////////////////////////////////////////////////////////
// Функция позволяет найти значение в std::map M по ключу K и если 
// такого значения нет, то добавить новое значение по ключу 
// K = MapType::data_type()
// Эквивалентно M.insert( make_pair(K, MapType::data_type()) ).first, но более 
// эффективно т.к. исключает дополнительное копирование
// Возвр: Итератор значения по ключу K
///////////////////////////////////////////////////////////////////////////////
template< class MapType > 
typename MapType::iterator MapEfficientFindOrAdd(
   MapType &M, 
   const typename MapType::key_type &K
)
{   
   typedef typename MapType::value_type MVT;
   typedef typename MapType::mapped_type MMT;

   typename MapType::iterator lb = M.lower_bound(K);

   return ( lb != M.end() && !M.key_comp()(K, lb->first) ) ? lb : M.insert( lb, MVT(K, MMT()) );																
}

} //namespace NWLib 

#endif