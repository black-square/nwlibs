//�����: ��������� �������(NW) 2007

#ifndef TIMER_H
#define TIMER_H

#include <ostream>
#include <ctime>
#include <limits>


///////////////////////////////////////////////////////////////////////////////
// ������ ��������� � ����������� ��������� �����������
///////////////////////////////////////////////////////////////////////////////

namespace NWLib {

///////////////////////////////////////////////////////////////////////////////
// ���������� �������� � ������������ ������ �������
///////////////////////////////////////////////////////////////////////////////
class TTimer
{
	std::clock_t begin, end;

 public:
   TTimer(){ begin = std::clock(); end = 0; }

   //��������� ���������� ������
   void Start(){ begin = std::clock(); }
   void Stop(){ end = std::clock(); }

   //���������� ������
   void Resume(){ begin = std::clock() - (end - begin); }

   //�������� ������ (����� ����� ��� ������ ������� ����� 
   //������������ Resume, ������ Start)
   void Reset(){ begin = end = 0; }

   operator std::clock_t() const{ return end - begin; }

   //������� � ��������
   double InSec() const{ return (double)(end - begin) / CLK_TCK; }

   //������� � �������� �������
   std::clock_t InTCK() const { return end - begin; }

   //����������� �������� ������� ������� ����� ���������� ������
   double MinInterv() const { return double(1) / CLK_TCK; }
};

//����� � ����� �������� � ��������
template< class CharT, class CharTraitsT >
inline std::basic_ostream<CharT, CharTraitsT> &operator<<( std::basic_ostream<CharT, CharTraitsT> &stream, const TTimer &Ob )
{
   return stream << Ob.InSec();
}

///////////////////////////////////////////////////////////////////////////////
// ����� ��������� �������� ����� � ������� ��������� 
// ��������� �������� �������� ������ ������������, ���� ����� �������� �� ����� 
// ����� ������������ ::timeGetTime()
// FloatT - ��� ����� � ��������� ������ ������� ����� �������������� ��� 
// ����������� �� ���������� ������� ��������� � �������. 
// ������������ ������ ����� ����� ��-�� ����, ���, ��������, ��� ����������� �� 
// ���������� ��� ������������� ���������, � ����� ������� ������������ ��������� 
// ��� ��� 4 ��������, � ��� ������������� �������� � �������� ������� �������� 
// ����� ������ ������� ������. ������� ����� ��������� ����� � �����, �� ���� 
// ������, ��� ����������� �� ����������, ������������ �������� ������� ����� 
// ����������� ����� 10.7 �����. (�������� ��� ������ ���������� ���� �� Maple:
//
//     QPF := 2394470000; #��������� QueryPerformanceFrequency
//     CurUnits := 10^9;  #������ ��������� � �������
//     QPF := QPF / gcd(QPF, CurUnits); #�������� �����
//     S := solve(x * QPF = 2^64 / 2 - 1):
//     conv := (x, u)->evalf(convert( x / CurUnits, 'units', 's', u )):
//     UsingUnits := ['yr', 'mo', 'd', 'h', 'min', 's']:
//     for i in UsingUnits do
//        tmp := conv(S, i);
//        if( tmp > 1 ) then print(sprintf( "����� �����������: %g %s", tmp, i )); break; end if;
//     end do:
// )
// �������� QueryPerformanceFrequency ���������� �� ������ ������� � � ���� �� ������� �������������,
// �� �, ������ ������, �� ���� ������ 11 ����� �� ��� �� � ����� ��������� � ��� ��� � int64 � 
// ������������ ����� ����������� 292 ����.
// � ����� � ���� � ����� ������������ ����� �� ������� �������� ���������, ��� ����� � ��������� 
// ������. 
// ���� ����������� ����� ������� ������������� �� �����
///////////////////////////////////////////////////////////////////////////////
template< class FloatT >
class TPrecisionTime
{
public:
   //����� �� ���������� �������� ���������
   //������������ ��������� ����� � �������������� ����������� ��� ���� ����� �������� ��������� 
   //������ � ������� ��������������� �����
   //����� �������������� �������������� ����� ������� Convert...
   class TTime;

   //������� ��������� �������
   struct Measure
   {
      enum T
      {
         Second,       //�������
         Millisecond,  //1e-3 �������
         Microsecond,  //1e-6 �������
         Nanosecond,   //1e-9 �������  

         MAX
      };
   };

   //����� � ������� ����������� �������� ���������, ��������� � TMeasure 
   typedef FloatT TTimeValue;

   typedef typename Measure::T TMeasure;

public:
   //������������� ������� ���������� �������� �� ������ ������� �����������
   //������ ���������� NWLib::TAplThrowException
   void InitConvert();

   //�������� ������� ����� 
   TTime GetCurrentTime() const;

   //������������� �� ������� Time � �������� Measure �� ����� �� ���������� ��������
   //������� ������ InitConvert()
   TTime ConvertToInternalTime( TMeasure Measure, TTimeValue Time ) const;
   
   //������������� �� ������� �� ���������� �������� Time �� ����� � �������� Measure 
   //������� ������ InitConvert()
   TTimeValue ConvertFromInternalTime( TTime Time, TMeasure Measure ) const;

   //����������� �������� ������� InitConvert() ���� �������� CallInitConvert == true �����
   //������� InitConvert ���������� ������� ��������������. ��� ����� ��� �������������� ��������� 
   //���������� � ������������
   explicit TPrecisionTime( bool CallInitConvert = true ) { if(CallInitConvert) InitConvert(); else Clear(); }

private:
   //�������� ������������
   void Clear() {std::fill(m_ConvertCoefs, m_ConvertCoefs + APL_ARRSIZE(m_ConvertCoefs), TTimeValue(0));}

private:
   //������������ ��� �������� �������� 
   TTimeValue m_ConvertCoefs[Measure::MAX];
};
///////////////////////////////////////////////////////////////////////////////

template< class FloatT >
class TPrecisionTime<FloatT>::TTime
{
public:    
   //��� ������ ����������� ������������� �������
   typedef LONGLONG TValue;

private:  
   //��� � �������� ����� ��������� ������ 0
   struct OnlyNullAssignableHelper{ int i; }; 
   typedef int OnlyNullAssignableHelper::*TOnlyNullAssignable;

   struct TNull {};

public:    
   //����������� ��-��������� �������������� ����� ����
   //�� ��������� ������ �������������� ��� �� ���� � �� �� ���� ������
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

   //������ �������� �������� ����� ��� ��������� ����� � N ��� ������ ��� ������ ��������,
   //������� ������� ��������� * /
   template<class T> TTime &operator*=( const T &V ) { this->m_Value *= V; return *this; }
   template<class T> TTime &operator/=( const T &V ) { this->m_Value /= V; return *this; }

   template<class T> friend const TTime operator*( TTime Ob, const T &V ) { return TTime( TValue(Ob.m_Value * V), TNull() ); }
   template<class T> friend const TTime operator*( const T &V, TTime Ob ) { return TTime( TValue(V * Ob.m_Value), TNull() ); }
   template<class T> friend const TTime operator/( TTime Ob, const T &V ) { return TTime( TValue(Ob.m_Value / V), TNull() ); }
   //�� ����� template<class T> friend const TTime operator/( const T &V, TTime Ob ) { return TTime( TValue(V * Ob.m_Value), TNull() ); }

   //���������� �������� ����� �� ���������� �������� ���������
   //������������ ����� ������ ���� ���������� ��������� ��������� ������������� ��������
   void SetRawValue( TValue Value ) { m_Value = Value; }
   TValue GetRawValue() const { return m_Value; }

private:
   //�� ������� ��� TNull ��� ���� ����� �������� ��������������� ��� ����� ������ 
   //������������ ��� TTime(0)
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

