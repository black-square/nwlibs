//�����: ��������� �������(NW) 2007

#ifndef Strings_HPP
#define Strings_HPP

#include <string>
#include <algorithm>

///////////////////////////////////////////////////////////////////////////////
// ������ �������� ��� ������� ��� ������ �� ��������
///////////////////////////////////////////////////////////////////////////////
namespace NWLib {

///////////////////////////////////////////////////////////////////////////////
// ���������������� ������ � ������ ����� ���� int
// ������ �������� ������ ����� ��������� ������:
//    [ws][sn][ddd]
// ���:
//    [ws]  = �������������� ������� ��� ���������;
//    [sn]  = �������������� ���� (+ ��� -);
//    [ddd] = ������������ �����;
//
// Beg  = �������� ������� � �������� ������� �������������;
// End  = �������� ����� ������;
//
// ��� ����������� ������� ���������������� ������� ��� ��� ���������� End
// ������� ���������� ��������������, ���������� �������� �������� �������
// � ���������� ������������ ����� � Val (���� �� ����������� �� ������ �����
// �� 0). ���� �� ����������� �� ���� ����� �� ���������� Beg
// ������� �� ��������� ����� �� ������������
///////////////////////////////////////////////////////////////////////////////
template<class InputIterator, class T>
InputIterator ConvertStringToInteger( InputIterator Beg, InputIterator End, T &Val )
{
   //� ����� � ��� ��� ��� ������� ��� �������� ������� ������������ � ���� ������� � UNICODE �������������
   //����� ��� �� ��� ��� � � ANSI (������ 16 ������) ����� ������������ ���� ������� ��� ��� UNICODE ��� � ��� 
   //ANSI �����
   bool IsNegate = false;

   InputIterator BegBack(Beg); //��������� ������� ���������� ���������

   Val = 0;

   //���������� �������
   while( Beg != End && (*Beg == '\t' || *Beg == ' ' ) ) ++Beg;

   if( Beg == End ) return BegBack;

   //����������� �� ������
   if( *Beg == '-' )
   {
      IsNegate = true;
      ++Beg;
   }
   else if( *Beg == '+' )
   {
      ++Beg;
   }

   InputIterator FirstDigit(Beg); //��������� ������� �������������� ������ �����

   //�������� ������������ �����
   while( Beg != End && (*Beg >= '0' && *Beg <= '9') )
   {
      Val = 10 * Val + *Beg - '0';
      ++Beg;
   }

   if( Beg == FirstDigit ) //�� �� ������������� �� ���� �����
      return BegBack;

   #pragma warning( push )
   #pragma warning( disable: 4146 )
   if( IsNegate ) Val = -Val;
   #pragma warning( pop )

   return  Beg;
}

///////////////////////////////////////////////////////////////////////////////
// ������������� ����� ���� int � ������, ���� ����� ������������� ��� �� ��������� ���� '-'
// Val - ������������� �����
// [Beg, End) - ������ � ������� ����� �������� ����������� ������������� ����������� �����
// �����: �������� ������� �� ��������� ���������� ��������. ���� ��������� [Beg, End) ���
//        ������ ���� �������� ����� �� ������� ������������ Beg
//
// ������� �� ���������� � ����� '\0' � ������� ��� ������ � ������ �������
// ����� ������ ���������� End = Beg + RG_SIZE - 1, ��� RG_SIZE - ������ ������� 
// ��� ���� ����� �������� ���� ������� ��� '\0'
///////////////////////////////////////////////////////////////////////////////
template<class InputIterator, class T>
InputIterator ConvertIntegerToString( T Val, InputIterator Beg, InputIterator End )
{
   typedef std::iterator_traits<InputIterator>::value_type TChar;

   InputIterator Cur(Beg);        //������� ������
   InputIterator FirstDigit(Beg); //������ ���������� �����
   T Radix(10);                   //���� ������� ����������
   
   if( Val < 0 ) //���� ����� ������������� ������� ����
   {
      if( Cur == End ) return Beg;

      #pragma warning( push )
      #pragma warning( disable: 4146 )
      Val = -Val;
      #pragma warning( pop )

      *Cur = '-';
      ++Cur;
      ++FirstDigit;
   }

   do
   {
      if( Cur == End ) return Beg;

      *Cur = static_cast<TChar>((Val % Radix) + '0');   
      ++Cur;

      Val /= Radix;
   }
   while( Val > 0 );

   InputIterator RetVal(Cur); //�������� ������� �� ��������� ���������� ��������

   //�� �������� ����������� �����. �������������� ���������� �������
   --Cur;

   while( FirstDigit < Cur ) 
   {
      //���������� Radix, ��� ��������� ����������
      Radix = static_cast<T>(*Cur); *Cur = *FirstDigit; *FirstDigit = static_cast<TChar>(Radix);
      ++FirstDigit;
      --Cur;
   } 

   return RetVal;
}

///////////////////////////////////////////////////////////////////////////////
// ��������� ���� ��� � stl::mismatch (���� ������ �������, ��� ��� ��������� 
// �� ���������), �� ���������� ��� ��� �� ������� �����
// ������ �������� ��� �� ������ ��� ������. �.�. ������� ��������������� ���
// ��� first1 != last1 ��� � ��� first2 != last2
///////////////////////////////////////////////////////////////////////////////
template <class InputIterator1, class InputIterator2>
std::pair<InputIterator1, InputIterator2>
Mismatch(InputIterator1 first1, InputIterator1 last1,
         InputIterator2 first2, InputIterator2 last2 )
{
   while( first1 != last1 && first2 != last2 && *first1 == *first2 )
   {
      ++first1;
      ++first2;
   }

   return std::pair< InputIterator1, InputIterator2 >( first1, first2 );
}

template <class InputIterator1, class InputIterator2, class BinaryPredicate>
std::pair<InputIterator1, InputIterator2>
Mismatch(InputIterator1 first1, InputIterator1 last1,
         InputIterator2 first2, InputIterator2 last2, BinaryPredicate binary_pred )
{
   while( first1 != last1 && first2 != last2 && binary_pred(*first1, *first2) )
   {
      ++first1;
      ++first2;
   }

   return std::pair< InputIterator1, InputIterator2 >( first1, first2 );
}

///////////////////////////////////////////////////////////////////////////////
// ������� ���� ������ ������� ��� ��� ��������� �����������. ������ ������ ��������
// ������������ ����� ������ �������� (����������� ������������������ �������� 
// �������������� ���� Null-terminated string), � ������ ����� ����������� [first2, last2).
// �����: � ������ ���������� ���� ��������� ��������� (�.�. ����� � ������ ��������� '\0'), 
// � �� ������ �������� ������� ���������, �� ������� ������������ ���������.
///////////////////////////////////////////////////////////////////////////////
template <class InputIterator1, class InputIterator2>
std::pair<bool, InputIterator2>
StringMismatch(InputIterator1 str, InputIterator2 first2, InputIterator2 last2)
{
   while( *str != '\0' && first2 != last2 && *str == *first2 )
   {
      ++str;
      ++first2;
   }

   return std::pair< bool, InputIterator2 >( *str == '\0', first2 );
}
///////////////////////////////////////////////////////////////////////////////

template <class InputIterator1, class InputIterator2, class BinaryPredicate>
std::pair<bool, InputIterator2>
StringMismatch(InputIterator1 str, InputIterator2 first2, InputIterator2 last2, BinaryPredicate binary_pred )
{
   while( *str != '\0' && first2 != last2 && binary_pred(*str, *first2) )
   {
      ++str;
      ++first2;
   }

   return std::pair< bool, InputIterator2 >( *str == '\0', first2 );
}

///////////////////////////////////////////////////////////////////////////////
// ����� � ������ Src ��� ��������� Find � �������� �� �� ReplaceWith, �������� 
// ���������� ������ � Dst.
// �����: Dst
///////////////////////////////////////////////////////////////////////////////
template< class CharT >
std::basic_string<CharT> &Replace( 
   const std::basic_string<CharT> &Src, const std::basic_string<CharT> &Find, 
   const std::basic_string<CharT> &ReplaceWith, std::basic_string<CharT> &Dst )
{
   std::basic_string<CharT>::const_iterator Cur = Src.begin();
   std::basic_string<CharT>::const_iterator End = Src.end();
   std::basic_string<CharT>::const_iterator Tmp;

   APL_ASSERT( !Find.empty() );
   Dst.clear();

   for(;;)
   {
      Tmp = std::search( Cur, End, Find.begin(), Find.end() );

      Dst.append(Cur, Tmp);

      if( Tmp == End )
         return Dst;

      Dst.append(ReplaceWith);

      Cur = Tmp + Find.size();
   }
}
///////////////////////////////////////////////////////////////////////////////

} //namespace NWLib

#endif