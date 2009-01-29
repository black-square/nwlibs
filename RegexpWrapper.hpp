#ifndef RegExpWrapper_HPP
#define RegExpWrapper_HPP

///////////////////////////////////////////////////////////////////////////////
// Обертка для максимально простого использования regexp парсера GRETA 
// http://research.microsoft.com/projects/greta/
///////////////////////////////////////////////////////////////////////////////
#include <greta/regexpr2.h>
#include <ostream>
#include "Auxiliary.h"

namespace NWLib {
namespace RegExp {

///////////////////////////////////////////////////////////////////////////////
// Сохранияет состояния поиска и позволяет искать несколько вхождений шаблона
///////////////////////////////////////////////////////////////////////////////
template<class CharT = TCHAR>
struct TFindState
{
   typedef std::basic_string<CharT> TString;
   typedef typename std::basic_string<CharT>::const_iterator TStringConstIterator;
   typedef regex::basic_match_results<TStringConstIterator> TResult;
   typedef typename TResult::backref_type TBackRef;

   TFindState() { Begin = PrevBegin = End = TStringConstIterator(); }
   TFindState( TStringConstIterator Begin, TStringConstIterator End ) { Init(Begin, End); }      
   explicit TFindState( const TBackRef &BR ){ APL_ASSERT(BR.matched); Init( BR.begin(), BR.end() ); }
   explicit TFindState( const TString &String ) { Init(String.begin(), String.end()); }
  
   void Init( TStringConstIterator B, TStringConstIterator E )
   {
      APL_ASSERT( B <= E );
      
      Begin = B;
      PrevBegin = B;
      End = E;
   }

   TStringConstIterator Begin;      //Место с которого начнётся поиск
   TStringConstIterator PrevBegin;  //Место с которого начинался поиск в прошлый раз
   TStringConstIterator End;        //Последний символ поиска
};

//Не бросать исключения
struct TNoException { enum {NeadThrow = 0}; };

//Бросать исключения при невозможности найти подстроку соотвествующую шаблону
struct TThrowException { enum {NeadThrow = 1}; };

///////////////////////////////////////////////////////////////////////////////
// Обёртка над классами поиска
///////////////////////////////////////////////////////////////////////////////
template< class ExceptionT = TThrowException, 
   class CharT = TCHAR //Если планируется использовать параметр отличный от
                       //TCHAR, то нужно голобально определить REGEX_WIDE_AND_NARROW
>
class TFinder
{
   typedef CharT TChar;
   typedef std::basic_string<TChar> TString;
   typedef TFindState<TChar> TFindState;
   typedef typename std::basic_string<TChar>::const_iterator TStringConstIterator;
   typedef regex::basic_rpattern<TStringConstIterator, regex::perl_syntax<TChar> > TPattern;
   typedef regex::basic_match_results<TStringConstIterator> TResult;
   typedef typename TResult::backref_type TBackRef;

public:
   explicit TFinder( const TChar *szPattern, regex::REGEX_FLAGS Flags = regex::NOFLAGS ): 
      m_Pattern( szPattern, Flags ), m_PatternStr(szPattern) {}

   bool operator()( TStringConstIterator Begin, TStringConstIterator End ) 
   {
      TBackRef BR = m_Pattern.match(Begin, End, m_Results);

      if( ExceptionT::NeadThrow && !BR.matched )
         APL_THROW( _T("Ошибка при поиске шаблона: ") << ConvertToTStr(m_PatternStr) );

      return BR.matched;
   }

   bool operator()( const TString &String ) 
   {
      return (*this)( String.begin(), String.end() );
   }

   bool operator()( const TBackRef &BR ) 
   {
      return (*this)( BR.begin(), BR.end() );
   }

   //Используется для циклического поиска
   bool operator()( TFindState &State ) 
   {
      TBackRef BR = m_Pattern.match(State.Begin, State.End, m_Results);

      if( ExceptionT::NeadThrow && !BR.matched && State.PrevBegin == State.Begin )
         APL_THROW( _T("Ошибка при поиске шаблона: ") << ConvertToTStr(m_PatternStr) );

      State.PrevBegin = State.Begin;
      State.Begin = BR.end();

      return BR.matched;
   }

   //Количество данных о результатах 
   //Если поиск удался то всегда будет больше 0
   size_t Size() const { return m_Results.all_backrefs().size(); }

   //Получить результат поиска номер i
   const TBackRef & operator[]( size_t i ) const { APL_ASSERT( i < Size()); return m_Results.all_backrefs()[i]; }

private:
   TPattern m_Pattern;
   TResult m_Results;
   TString m_PatternStr;            //Сохраняется только для того чтобы отображать значение в исключениях
};


//Вывод в поток значения в секундах
template< class CharT, class CharTraitsT, class ExceptionT >
inline std::basic_ostream<CharT, CharTraitsT> &operator<<( std::basic_ostream<CharT, CharTraitsT> &stream, const TFinder<ExceptionT, CharT> &Finder )
{
   stream << CharT('{');

   for( size_t i = 0; i < Finder.Size(); ++i )
   {
      if( i != 0 )
         stream <<  CharT(' ') << CharT('|') << CharT(' ');
      
      stream << Finder[i].str() ;
   }

   return stream << CharT('}');

}


} //namespace RegExp
} //namespace NWLib

#endif // RegExpWrapper_HPP
