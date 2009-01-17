//Автор: Шестеркин Дмитрий(NW) 2006

#ifndef ReferenceCounters_HPP
#define ReferenceCounters_HPP

///////////////////////////////////////////////////////////////////////////////
// Различные реализации счётчиков ссылок
///////////////////////////////////////////////////////////////////////////////

namespace NWLib {

///////////////////////////////////////////////////////////////////////////////
// Класс позволяет добавлять счётчик ссылок любому классу от которого можно 
// наследоваться и который имеет конструктор по-умолчанию.
// При помощи статического метода Create можно получить интел-ый указатель на 
// на тип переданный как параметр шаблона в CAutoRefCount, которые можно 
// копировать друг в друга. хранить в STL контейнерах и т.п. После того как 
// последний такой указатель будет уничтожен автоматически вызовется метод 
// delete
///////////////////////////////////////////////////////////////////////////////
namespace Private
{
   //Внутренний класс который предоставляет методы по подсчёту ссылок
   template< class ObjectT >
   class CImpl: public ObjectT
   {
      size_t m_RefCount;

      CImpl(): m_RefCount(1) {}
   public:
      static CImpl *Create() { return new CImpl; }

      void AddRef(){ ++m_RefCount; }
      void Release(){ APL_ASSERT( m_RefCount > 0 ); if( --m_RefCount == 0 ) delete this; }
   };
   
   //Реализация интелликтуального указателя
   template<class ResultT, class ObjectT>
   class CPtrBase
   {
      CImpl<ObjectT> *m_pT;

   public:         
      template < class T1, class T2 > friend class CPtrBase;

      CPtrBase( CImpl<ObjectT> *pT = 0 ): m_pT(pT) {}
      //CPtrBase( TUndefinedBoolType pT = 0 ): m_pT(0) {} //Позволяет конструировать только из 0

      CPtrBase( const CPtrBase &Other )
      { 
         m_pT = Other.m_pT;

         if( m_pT != 0 )
            m_pT->AddRef();
      }

      ~CPtrBase()
      {
         if( m_pT != 0 )
            m_pT->Release();
      }

      void Swap( CPtrBase &Other )
      {
         std::swap(m_pT, Other.m_pT);
      }

      void Release()
      {
         if( m_pT != 0 )
         {
            m_pT->Release();
            m_pT = 0;
         }
      }

      CPtrBase &operator=( const CPtrBase &Other )
      {
         CPtrBase(Other).Swap(*this);
         return *this;
      }

      //Позволим проверять указатель на равенство/неравенство нулю и приводить к типу bool
   private:  
      struct OperatorHelper{ int i; }; //см. Александровску(loki)
      typedef int OperatorHelper::*TUndefinedBoolType;
   public:
      //Позволет проверять указатель на равенство/неравенство нулю и приводить к типу bool
      operator TUndefinedBoolType() const { return m_pT != 0? &OperatorHelper::i : 0; }

      ResultT *Get() const { return m_pT; }
      ResultT &operator*() const { APL_ASSERT( Get() != 0 ); return *Get(); }
      ResultT *operator->() const { return &**this; }

      //Если не определять то будет использоваться автоматическое преобразование к TUndefinedBoolType
      //и встроенные операторы
      friend bool operator==( const CPtrBase &P1, const CPtrBase &P2 ){ return P1.m_pT == P2.m_pT; }
      friend bool operator!=( const CPtrBase &P1, const CPtrBase &P2 ){ return P1.m_pT != P2.m_pT; }

      //Если не опредилять по при проверке P1 == 0, 0 == P1, P1 != 0, 0 != P1 возникнет 
      //неоднозначность т.к. конструтор не explicit
      friend bool operator==( const CPtrBase &P1, TUndefinedBoolType P2 ){ return P1.m_pT == 0; }
      friend bool operator==( TUndefinedBoolType P2, const CPtrBase &P1 ){ return P1.m_pT == 0; }
      friend bool operator!=( const CPtrBase &P1, TUndefinedBoolType P2 ){ return P1.m_pT != 0; }
      friend bool operator!=( TUndefinedBoolType P2, const CPtrBase &P1 ){ return P1.m_pT != 0; }


   protected:
      template< class T1, class T2 >
      CPtrBase( const CPtrBase<T1, T2> &Other ) //Шаблоный конструктор не заменяет конструктор по умолчанию
      { 
         m_pT = Other.m_pT;

         if( m_pT != 0 )
            m_pT->AddRef();
      }

      template< class T1, class T2 >
      CPtrBase &operator=( const CPtrBase<T1, T2> &Other ) //Шаблоный оператор присваивания не заменяет оператор присваивания по умолчанию
      {
         CPtrBase(Other).Swap(*this);
         return *this;
      }
   };
} //namespace Private

template< class ObjectT >
class CAutoRefCount
{
public:

   //Инетлликтуальный указатель возращаемый функцией Create
   class SmartPtr: public Private::CPtrBase<ObjectT, ObjectT>
   {
   public:
      SmartPtr( Private::CImpl<ObjectT> * pImpl = 0 ): Private::CPtrBase<ObjectT, ObjectT>(pImpl){}
   };

   //Константный ител-ый указатель (позволяет получить доступ только к const ObjectT)
   //Может быть инициилизован из SmartPtr, но не наоборот
   class ConstSmartPtr: public Private::CPtrBase<const ObjectT, ObjectT>
   {
   public:
      ConstSmartPtr( Private::CImpl<ObjectT> * pImpl = 0 ): Private::CPtrBase<const ObjectT, ObjectT>(pImpl){}
      ConstSmartPtr( const SmartPtr &SM ): Private::CPtrBase<const ObjectT, ObjectT>(SM) {}
      ConstSmartPtr &operator=( const SmartPtr &SM ){ Private::CPtrBase<const ObjectT, ObjectT>::operator=(SM); return *this; }
   };

   //Создаёт новый объект типа ObjectT
   static SmartPtr Create(){ return SmartPtr( Private::CImpl<ObjectT>::Create() ); }
};

} //namespace NWLib 

#endif
