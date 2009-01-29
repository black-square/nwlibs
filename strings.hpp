//Автор: Шестеркин Дмитрий(NW) 2007

#ifndef Strings_HPP
#define Strings_HPP

#include <string>
#include <algorithm>

///////////////////////////////////////////////////////////////////////////////
// Модуль содержит ряд функций для работы со строками
///////////////////////////////////////////////////////////////////////////////
namespace NWLib {

///////////////////////////////////////////////////////////////////////////////
// Преобразовыввает строку в формат числа типа int
// Строка символов должна иметь следующий формат:
//    [ws][sn][ddd]
// Где:
//    [ws]  = необязательные пробелы или табуляции;
//    [sn]  = необязательный знак (+ или -);
//    [ddd] = обязательные цифры;
//
// Beg  = Итератор начиная с которого начнётся распознавание;
// End  = Итератор конца строки;
//
// При обнаружении первого нерспознаваемого символа или при достижении End
// функция прекращает преобразование, возвращает итератор текущего символа
// и записывает получившеяся число в Val (если не встретилось ни одного числа
// то 0). Если не встретилась ни одна цифра то возвращает Beg
// Функция не проверяет число на переполнение
///////////////////////////////////////////////////////////////////////////////
template<class InputIterator, class T>
InputIterator ConvertStringToInteger( InputIterator Beg, InputIterator End, T &Val )
{
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
   while( Beg != End && (*Beg >= '0' && *Beg <= '9') )
   {
      Val = 10 * Val + *Beg - '0';
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
// Преобразовать число типа int в строку, если число отрицательное так же выводится знак '-'
// Val - преобразуемое число
// [Beg, End) - Строка в которую будет записано литеральное представление переданного числа
// Возвр: Итератор стоящий за последним записанным символом. Если интервала [Beg, End) для
//        записи всех символов числа не хватает возвращается Beg
//
// ФУНКЦИЯ НЕ ЗАПИСЫВАЕТ В КОНЦЕ '\0' и поэтому при записи в массив смволов
// имеет смысел передавать End = Beg + RG_SIZE - 1, где RG_SIZE - размер массива 
// для того чтобы оставить одну позицию для '\0'
///////////////////////////////////////////////////////////////////////////////
template<class InputIterator, class T>
InputIterator ConvertIntegerToString( T Val, InputIterator Beg, InputIterator End )
{
   typedef std::iterator_traits<InputIterator>::value_type TChar;

   InputIterator Cur(Beg);        //Текущий символ
   InputIterator FirstDigit(Beg); //Первая записанная цифра
   T Radix(10);                   //База системы исчисления
   
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

      *Cur = static_cast<TChar>((Val % Radix) + '0');   
      ++Cur;

      Val /= Radix;
   }
   while( Val > 0 );

   InputIterator RetVal(Cur); //Итератор стоящий за последним записанным символом

   //Мы получили перевёрнутое число. Востанавливаем нормальный порядок
   --Cur;

   while( FirstDigit < Cur ) 
   {
      //используем Radix, как временную переменную
      Radix = static_cast<T>(*Cur); *Cur = *FirstDigit; *FirstDigit = static_cast<TChar>(Radix);
      ++FirstDigit;
      --Cur;
   } 

   return RetVal;
}

///////////////////////////////////////////////////////////////////////////////
// Выполняет тоже что и stl::mismatch (ищет первую позицию, где два интревала 
// не совпадают), но отличается тем что не требует чтобы
// второй диапозон был не меньше чем первый. Т.е. функция останавливается как
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
// Функция ищет первую позицию где два интервала несовпадают. Причём первый интервал
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