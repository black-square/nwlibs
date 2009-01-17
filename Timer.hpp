//Автор: Шестеркин Дмитрий(NW) 2007

#ifndef TIMER_H
#define TIMER_H

#include <ostream>
#include <ctime>
#include <limits>


///////////////////////////////////////////////////////////////////////////////
// Классы связанные в вычислением временных промежутков
///////////////////////////////////////////////////////////////////////////////

namespace NWLib {

///////////////////////////////////////////////////////////////////////////////
// Реализация простого и переносимого класса таймера
///////////////////////////////////////////////////////////////////////////////
class TTimer
{
	std::clock_t begin, end;

 public:
   TTimer(){ begin = std::clock(); end = 0; }

   //Запустить остановить таймер
   void Start(){ begin = std::clock(); }
   void Stop(){ end = std::clock(); }

   //Продолжить отсчёт
   void Resume(){ begin = std::clock() - (end - begin); }

   //Сбросить таймер (после этого для начала отсчёта можно 
   //пользоваться Resume, вместо Start)
   void Reset(){ begin = end = 0; }

   operator std::clock_t() const{ return end - begin; }

   //Разница в секундах
   double InSec() const{ return (double)(end - begin) / CLK_TCK; }

   //Разница в единицах времени
   std::clock_t InTCK() const { return end - begin; }

   //Минимальный интервад времени который может опредилить таймер
   double MinInterv() const { return double(1) / CLK_TCK; }
};

//Вывод в поток значения в секундах
template< class CharT, class CharTraitsT >
inline std::basic_ostream<CharT, CharTraitsT> &operator<<( std::basic_ostream<CharT, CharTraitsT> &stream, const TTimer &Ob )
{
   return stream << Ob.InSec();
}

///////////////////////////////////////////////////////////////////////////////
// Класс позволяет получать время с высокой точностью 
// Позволяет добиться точности больше миллисекунды, если такая точность не нужна 
// стоит использовать ::timeGetTime()
// FloatT - тип чисел с плавающей точкой который будет использоваться при 
// конвертации во внутренние единицы измерения и обратно. 
// Используется вместо целых чисел из-за того, что, например, при конвертации из 
// наносекунд при использовании умножения, а затем деления переполнение возникает 
// уже при 4 секундах, а при использовании операций в обратном порядке точность 
// будет всегда порядка секунд. Конечно можно упростить дробь и тогда, на моей 
// машине, при конвертации из наносекунд, максимальный интервал который можно 
// представить будет 10.7 часов. (Вычислял при помощи следующего кода на Maple:
//
//     QPF := 2394470000; #Результат QueryPerformanceFrequency
//     CurUnits := 10^9;  #Единиц измерения в секунде
//     QPF := QPF / gcd(QPF, CurUnits); #Упрощаем дробь
//     S := solve(x * QPF = 2^64 / 2 - 1):
//     conv := (x, u)->evalf(convert( x / CurUnits, 'units', 's', u )):
//     UsingUnits := ['yr', 'mo', 'd', 'h', 'min', 's']:
//     for i in UsingUnits do
//        tmp := conv(S, i);
//        if( tmp > 1 ) then print(sprintf( "Можно представить: %g %s", tmp, i )); break; end if;
//     end do:
// )
// Значение QueryPerformanceFrequency отличается на разных машинах и к нему не хочется привязываться,
// да и, строго говоря, на моей машине 11 часов не идёт не в какое сравнение с тем что в int64 в 
// наносекундах можно представить 292 года.
// В связи с этим я решил представлять время во внешних единицах измерения, как числа с плавающей 
// точкой. 
// Если потребуется можно сделать специализацию на целые
///////////////////////////////////////////////////////////////////////////////
template< class FloatT >
class TPrecisionTime
{
public:
   //Время во внутренних единицах измерения
   //Используется отдельный класс с перегруженными операторами для того чтобы избежать возможных 
   //ошибок с неявным преобразованием типов
   //Любые преобразования осуществляются через функции Convert...
   class TTime;

   //Единица измерения времени
   struct Measure
   {
      enum T
      {
         Second,       //Секунда
         Millisecond,  //1e-3 секунды
         Microsecond,  //1e-6 секунды
         Nanosecond,   //1e-9 секунды  

         MAX
      };
   };

   //Время в заранее определённых единицах измерения, указанных в TMeasure 
   typedef FloatT TTimeValue;

   typedef typename Measure::T TMeasure;

public:
   //Инициализация объекта необходимо вызывать до вызова функций конвертации
   //Кидает исключения NWLib::TAplThrowException
   void InitConvert();

   //Получить текущее время 
   TTime GetCurrentTime() const;

   //Преобразовать из времени Time в единицах Measure во время во внутренних единицах
   //Требует вызова InitConvert()
   TTime ConvertToInternalTime( TMeasure Measure, TTimeValue Time ) const;
   
   //Преобразовать из времени во внутренних единицах Time во время в единицах Measure 
   //Требует вызова InitConvert()
   TTimeValue ConvertFromInternalTime( TTime Time, TMeasure Measure ) const;

   //Конструктор вызывает функцию InitConvert() если параметр CallInitConvert == true иначе
   //функцию InitConvert необходимо вызвать самостоятельно. Это нужно для предотвращения генерации 
   //исключения в конструкторе
   explicit TPrecisionTime( bool CallInitConvert = true ) { if(CallInitConvert) InitConvert(); else Clear(); }

private:
   //Очистить коэффициенты
   void Clear() {std::fill(m_ConvertCoefs, m_ConvertCoefs + APL_ARRSIZE(m_ConvertCoefs), TTimeValue(0));}

private:
   //Коэффициенты для перевода значений 
   TTimeValue m_ConvertCoefs[Measure::MAX];
};
///////////////////////////////////////////////////////////////////////////////

template< class FloatT >
class TPrecisionTime<FloatT>::TTime
{
public:    
   //Тип данных внутреннего представления времени
   typedef LONGLONG TValue;

private:  
   //Тип к которому можно присвоить только 0
   struct OnlyNullAssignableHelper{ int i; }; 
   typedef int OnlyNullAssignableHelper::*TOnlyNullAssignable;

   struct TNull {};

public:    
   //Конструктор по-умолчанию инициализирует время нулём
   //Мы позволяем неявно конструировать тип из нуля и не из чего больше
   TTime(TOnlyNullAssignable pNA = 0): m_Value(0) {}

   TTime &operator+=( TTime Ob2 ) { this->m_Value += Ob2.m_Value; return *this; }
   TTime &operator-=( TTime Ob2 ) { this->m_Value -= Ob2.m_Value; return *this; }

   friend bool operator==( TTime Ob1, TTime Ob2 ) { return Ob1.m_Value == Ob2.m_Value; }
   friend bool operator!=( TTime Ob1, TTime Ob2 ) { return Ob1.m_Value != Ob2.m_Value; }
   friend bool operator>( TTime Ob1, TTime Ob2 ) { return Ob1.m_Value > Ob2.m_Value; }
   friend bool operator<( TTime Ob1, TTime Ob2 ) { return Ob1.m_Value < Ob2.m_Value; }
   friend bool operator>=( TTime Ob1, TTime Ob2 ) { return Ob1.m_Value >= Ob2.m_Value; }
   friend bool operator<=( TTime Ob1, TTime Ob2 ) { return Ob1.m_Value <= Ob2.m_Value; }

   friend const TTime operator+( TTime Ob1, TTime Ob2 ) { return TTime(Ob1.m_Value + Ob2.m_Value, TNull()); }
   friend const TTime operator-( TTime Ob1, TTime Ob2 ) { return TTime(Ob1.m_Value - Ob2.m_Value, TNull()); }

   const TTime operator+() { return *this; }
   const TTime operator-() { return TTime(-this->m_Value, TNull()); }

   //Вполне возможна ситуация когда нам захочется время в N раз меньше или больше текущего,
   //поэтому добавим операторы * /
   template<class T> TTime &operator*=( const T &V ) { this->m_Value *= V; return *this; }
   template<class T> TTime &operator/=( const T &V ) { this->m_Value /= V; return *this; }

   template<class T> friend const TTime operator*( TTime Ob, const T &V ) { return TTime( TValue(Ob.m_Value * V), TNull() ); }
   template<class T> friend const TTime operator*( const T &V, TTime Ob ) { return TTime( TValue(V * Ob.m_Value), TNull() ); }
   template<class T> friend const TTime operator/( TTime Ob, const T &V ) { return TTime( TValue(Ob.m_Value / V), TNull() ); }
   //Не нужно template<class T> friend const TTime operator/( const T &V, TTime Ob ) { return TTime( TValue(V * Ob.m_Value), TNull() ); }

   //Установить получить время во внутренних единицах измерения
   //Пользоваться можно только если необходимо совершить некоторые нестандартное действие
   void SetRawValue( TValue Value ) { m_Value = Value; }
   TValue GetRawValue() const { return m_Value; }

private:
   //Мы передаём тип TNull для того чтобы избежать неоднозначности при явном вызове 
   //конструктора как TTime(0)
   explicit TTime( TValue Value, TNull ): m_Value(Value) {}

private:
   friend class TPrecisionTime;
   TValue m_Value;
};
///////////////////////////////////////////////////////////////////////////////

template< class FloatT >
inline void TPrecisionTime<FloatT>::InitConvert()
{
   APL_ASSERT( !std::numeric_limits<TTimeValue>::is_exact );
   
   Clear(); 

   LARGE_INTEGER Tmp;

   if( !QueryPerformanceFrequency(&Tmp) )
      APL_THROW( _T("QueryPerformanceFrequency not supported: ") << GetDWErrorInfo(GetLastError()) );

   TTimeValue TimeInSec(static_cast<TTimeValue>(Tmp.QuadPart) );

   m_ConvertCoefs[Measure::Second]      = TimeInSec;
   m_ConvertCoefs[Measure::Millisecond] = TimeInSec / 1000;
   m_ConvertCoefs[Measure::Microsecond] = TimeInSec / 1000000;
   m_ConvertCoefs[Measure::Nanosecond]  = TimeInSec / 1000000000;
}
///////////////////////////////////////////////////////////////////////////////

template< class FloatT >
inline typename TPrecisionTime<FloatT>::TTime TPrecisionTime<FloatT>::GetCurrentTime() const
{
   LARGE_INTEGER Tmp;

   APL_CHECK( QueryPerformanceCounter(&Tmp) );
   return TTime(Tmp.QuadPart, TTime::TNull());
}
///////////////////////////////////////////////////////////////////////////////

template< class FloatT >
inline typename TPrecisionTime<FloatT>::TTime TPrecisionTime<FloatT>::ConvertToInternalTime( TMeasure Measure, TTimeValue Time ) const
{
   APL_ASSERT( Measure >= 0 && Measure < APL_ARRSIZE(m_ConvertCoefs) );
   APL_ASSERT( m_ConvertCoefs[Measure] > 0 );

   return TTime( TTime::TValue(Time * m_ConvertCoefs[Measure]), TTime::TNull() );
}
///////////////////////////////////////////////////////////////////////////////

template< class FloatT >
inline typename TPrecisionTime<FloatT>::TTimeValue TPrecisionTime<FloatT>::ConvertFromInternalTime( TTime Time, TMeasure Measure ) const
{
   APL_ASSERT( Measure >= 0 && Measure < APL_ARRSIZE(m_ConvertCoefs) );
   APL_ASSERT( m_ConvertCoefs[Measure] > 0 );

   return Time.m_Value / m_ConvertCoefs[Measure];
}

} //namespace NWLib 
#endif

