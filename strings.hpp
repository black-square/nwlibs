//Автор: Шестеркин Дмитрий(NW) 2007

#ifndef Strings_HPP
#define Strings_HPP

#include <string>
#include <algorithm>

///////////////////////////////////////////////////////////////////////////////
// Модуль содержит ряд функций для работы со строками
///////////////////////////////////////////////////////////////////////////////
namespace NWLib {
namespace Detail {
    ///////////////////////////////////////////////////////////////////////////////
    // Стратегии позволяющие абстрагироваться от конкретной системы счисления
    // Нужны в связи с тем что в ASCII между символом '9' и 'A' существуют ещё 
    // несколько символов
    // В дальнейшем данные стратегии можно использовать для работы с символами как
    // с верхнего так и нижнего регистра
    ///////////////////////////////////////////////////////////////////////////////
    template<bool GreaterThan10, class ValueT, ValueT radixVal> struct TRadixAbstractionImpl;

    template<class ValueT, ValueT radixVal>
    struct TRadixAbstractionImpl<false, ValueT, radixVal>
    {
        ValueT radix() const { APL_ASSERT(radixVal > 1 && radixVal <= 10); return radixVal; }

        template<class CharT>
        bool toVal( CharT ch, ValueT &val )
        {
            if( ch >= '0' && ch < '0' + radix() )
            {
                val = ch - '0';
                return true;
            }

            return false;
        }

        template<class CharT>
        CharT toChar( ValueT val )
        {
            APL_ASSERT( val >= 0 && val < radix() );
            return static_cast<CharT>('0' + val);
        }
    };
    ///////////////////////////////////////////////////////////////////////////////   

    template<class ValueT, ValueT radixVal>
    struct TRadixAbstractionImpl<true, ValueT, radixVal>
    {
        ValueT radix() const { APL_ASSERT(radixVal > 10); return radixVal; }

        template<class CharT>
        bool toVal( CharT ch, ValueT &val )
        {
            if( ch > '9' ) ch -= 'A' - '9' - 1;
             
            if( ch >= '0' && ch < '0' + radix() )
            {
                val = ch - '0';
                return true;
            }

            return false;
        }

        template<class CharT>
        CharT toChar( ValueT val )
        {
            APL_ASSERT( val >= 0 && val < radix() );

            if( val > 9 ) val += 'A' - '9' - 1;
                
            return static_cast<CharT>('0' + val);
        }
    };

    template<class ValueT, ValueT radixVal>
    struct TRadixAbstraction: TRadixAbstractionImpl<(radixVal > 10), ValueT, radixVal> {};
}

///////////////////////////////////////////////////////////////////////////////
// Преобразовывает строку в формат числа типа int
// Строка символов должна иметь следующий формат:
//    [ws][sn][ddd]
// Где:
//    [ws]  = необязательные пробелы или табуляции;
//    [sn]  = необязательный знак (+ или -);
//    [ddd] = обязательные цифры;
//
// Beg        = Итератор начиная с которого начнётся распознавание;
// End        = Итератор конца строки;
// RadixValue = Основание системы счисления. Допустимые символы из 
//              которых может состоять число ['0'..'9'], ['A'.. 'Z']
//
// При обнаружении первого нераспознаваемого символа или при достижении End
// функция прекращает преобразование, возвращает итератор текущего символа
// и записывает получившееся число в Val (если не встретилось ни одного числа
// то 0). Если не встретилась ни одна цифра то возвращает Beg
// Функция не проверяет число на переполнение
///////////////////////////////////////////////////////////////////////////////
template<int RadixValue, class InputIterator, class T>
InputIterator ConvertStringToIntegerRadix( InputIterator Beg, InputIterator End, T &Val )
{
   Detail::TRadixAbstraction<T, RadixValue> RA;

   //В связи с тем что все символы для проверки которые используются в этой функции в UNICODE представлении
   //имеют тот же код что и в ANSI (только 16 битный) можно использовать одну функцию как для UNICODE так и для 
   //ANSI строк
   bool IsNegate = false;

   InputIterator BegBack(Beg); //Сохраняем позицию начального итератора

   Val = 0;

   //Пропускаем пробелы
   while( Beg != End && (*Beg == '\t' || *Beg == ' ' ) ) ++Beg;

   if( Beg == End ) return BegBack;

   //Разбираемся со знаком
   if( *Beg == '-' )
   {
      IsNegate = true;
      ++Beg;
   }
   else if( *Beg == '+' )
   {
      ++Beg;
   }

   InputIterator FirstDigit(Beg); //Сохраняем позицию предполагаемой первой цифры

   //Начинаем обрабатывать цифры       
   T curDigit;

   while( Beg != End && RA.toVal(*Beg, curDigit) )
   {
      Val = RA.radix() * Val + curDigit;
      ++Beg;
   }

   if( Beg == FirstDigit ) //Мы не преобразовали не одну цифру
      return BegBack;

   #pragma warning( push )
   #pragma warning( disable: 4146 )
   if( IsNegate ) Val = -Val;
   #pragma warning( pop )

   return  Beg;
}
///////////////////////////////////////////////////////////////////////////////

template<class InputIterator, class T>
inline InputIterator ConvertStringToInteger( InputIterator Beg, InputIterator End, T &Val )
{
    return ConvertStringToIntegerRadix<10>( Beg, End, Val );
}

///////////////////////////////////////////////////////////////////////////////
// Преобразовать число типа int в строку, если число отрицательное так же выводится знак '-'
// Val        - преобразуемое число
// [Beg, End) - Строка в которую будет записано литеральное представление переданного числа
// RadixValue - Основание системы счисления. Допустимые символы из 
//              которых может состоять число ['0'..'9'], ['A'.. 'Z']
// Возвр: Итератор стоящий за последним записанным символом. Если интервала [Beg, End) для
//        записи всех символов числа не хватает возвращается Beg
//
// ФУНКЦИЯ НЕ ЗАПИСЫВАЕТ В КОНЦЕ '\0' и поэтому при записи в массив символов
// имеет смысл передавать End = Beg + RG_SIZE - 1, где RG_SIZE - размер массива 
// для того чтобы оставить одну позицию для '\0'
///////////////////////////////////////////////////////////////////////////////
template<int RadixValue, class InputIterator, class T>
InputIterator ConvertIntegerToStringRadix( T Val, InputIterator Beg, InputIterator End )
{
   Detail::TRadixAbstraction<T, RadixValue> RA;

   typedef std::iterator_traits<InputIterator>::value_type TChar;

   InputIterator Cur(Beg);        //Текущий символ
   InputIterator FirstDigit(Beg); //Первая записанная цифра
   
   if( Val < 0 ) //Если число отрицательное выводим знак
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

      *Cur = RA.toChar<TChar>( Val % RA.radix() );   
      ++Cur;

      Val /= RA.radix();
   }
   while( Val > 0 );

   InputIterator RetVal(Cur); //Итератор стоящий за последним записанным символом

   //Мы получили перевёрнутое число. Восстанавливаем нормальный порядок
   --Cur;

   TChar Tmp;
   while( FirstDigit < Cur ) 
   {
      Tmp = *Cur; *Cur = *FirstDigit; *FirstDigit = Tmp;
      ++FirstDigit;
      --Cur;
   } 

   return RetVal;
}

template<class InputIterator, class T>
inline InputIterator ConvertIntegerToString( T Val, InputIterator Beg, InputIterator End )
{
    return ConvertIntegerToStringRadix<10>(Val, Beg, End);
}

///////////////////////////////////////////////////////////////////////////////
// Выполняет тоже что и stl::mismatch (ищет первую позицию, где два интервала 
// не совпадают), но отличается тем что не требует чтобы
// второй диапазон был не меньше чем первый. Т.е. функция останавливается как
// при first1 != last1 так и при first2 != last2
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
// Функция ищет первую позицию где два интервала не совпадают. Причём первый интервал
// представляет собой строку символов (непрерывная последовательность символов 
// оканчивающаяся нулём Null-terminated string), а второй задан параметрами [first2, last2).
// Возвр: в первой компоненте пары результат сравнения (т.е. нашли в первом интервале '\0'), 
// а во второй итератор второго интервала, на котором остановилось сравнение.
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
// Найти в строке Src все подстроки Find и заменить их на ReplaceWith, записать 
// полученную строку в Dst.
// Возвр: Dst
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