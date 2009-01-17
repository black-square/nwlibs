//Автор: Шестеркин Дмитрий(NW) 2006

#ifndef SIMPLEXML_HPP
#define SIMPLEXML_HPP

///////////////////////////////////////////////////////////////////////////////
// Формат файла записи: Упрощённая запись в XML, используется как пример.
// Не заменяет символы ( Например "<" на "&lt;" и д.р. )  
///////////////////////////////////////////////////////////////////////////////
template < class CharT, class CharTraitsT = std::char_traits<CharT> >
class CSimpleXMLWriter
{
protected:
   typedef std::basic_ostream< CharT, CharTraitsT > TOstream;
   typedef typename CharT TChar;

private:
   TOstream &m_Os;

private:
   //Вывести в поток открывающийся тег
   void OpenTag( const TChar *Name )
   {
      //Как для Unicode так и для ANSI такой подход (static_cast<TChar>) будет работать
      m_Os << static_cast<TChar>('<') << Name << static_cast<TChar>('>');
   }
   
   //Вывести в поток закрывающийся тег
   void CloseTag( const TChar *Name )
   {
      //Как для Unicode так и для ANSI такой подход (static_cast<TChar>) будет работать
      m_Os << static_cast<TChar>('<') << static_cast<TChar>('/') << Name << static_cast<TChar>('>');
   }

protected:
   explicit CSimpleXMLWriter( TOstream &Os ): m_Os(Os) {}

   //Начать новый увовень вложенности с именем Name
   void BeginLevel( const TChar *Name )
   {
      OpenTag( Name );   
   }
   
   //Закончить уровень вложенности с именем Name
   void EndLevel( const TChar *Name )
   {
      CloseTag( Name );
   }

   //Сохранить данные Ob с именем Name
   //T должен выполнять [Правило 1]
   template< class T >
   void SaveItem( const TChar *Name, const T &Ob )
   {
      OpenTag( Name );   
      WriteToStream(m_Os, Ob);
      CloseTag( Name );
   }

   //Начать/Закончить загрузку
   void BeginSave() {}
   void EndSave() {}
};

#endif
