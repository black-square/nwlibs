//�����: ��������� �������(NW) 2006

#ifndef SIMPLEXML_HPP
#define SIMPLEXML_HPP

///////////////////////////////////////////////////////////////////////////////
// ������ ����� ������: ���������� ������ � XML, ������������ ��� ������.
// �� �������� ������� ( �������� "<" �� "&lt;" � �.�. )  
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
   //������� � ����� ������������� ���
   void OpenTag( const TChar *Name )
   {
      //��� ��� Unicode ��� � ��� ANSI ����� ������ (static_cast<TChar>) ����� ��������
      m_Os << static_cast<TChar>('<') << Name << static_cast<TChar>('>');
   }
   
   //������� � ����� ������������� ���
   void CloseTag( const TChar *Name )
   {
      //��� ��� Unicode ��� � ��� ANSI ����� ������ (static_cast<TChar>) ����� ��������
      m_Os << static_cast<TChar>('<') << static_cast<TChar>('/') << Name << static_cast<TChar>('>');
   }

protected:
   explicit CSimpleXMLWriter( TOstream &Os ): m_Os(Os) {}

   //������ ����� ������� ����������� � ������ Name
   void BeginLevel( const TChar *Name )
   {
      OpenTag( Name );   
   }
   
   //��������� ������� ����������� � ������ Name
   void EndLevel( const TChar *Name )
   {
      CloseTag( Name );
   }

   //��������� ������ Ob � ������ Name
   //T ������ ��������� [������� 1]
   template< class T >
   void SaveItem( const TChar *Name, const T &Ob )
   {
      OpenTag( Name );   
      WriteToStream(m_Os, Ob);
      CloseTag( Name );
   }

   //������/��������� ��������
   void BeginSave() {}
   void EndSave() {}
};

#endif
