//Автор: Шестеркин Дмитрий(NW) 2007

#ifndef ObjectBinding_HPP
#define ObjectBinding_HPP

///////////////////////////////////////////////////////////////////////////////
// Файлы содержит компоненты позволяющие связать объекты классов друг с другом 
///////////////////////////////////////////////////////////////////////////////

#include "StlAuxiliary.hpp"

namespace NWLib {


///////////////////////////////////////////////////////////////////////////////
// Класс предназначен для связывания объектов по модели один ко многим, причём 
// каждый объект (как родитель так и дети) может уничтожется в любой момент 
// времени и при этом регистрация будет автоматически удалена 
// Для использования класса дочерние объекты должны открыто наследоваться от типа 
// TRelationshipOneToMany::THolder
// Не завист от использования определённого контейнера. Взаимодействия с 
// контейнером инкапсулируются при помощи стратегии ConteinerStrategyT 
// (открыто наследуется классом TRelationshipOneToMany)
// Реализации ConteinerStrategyT должны наследоваться от
// TRelationshipOneToManyBaseStrategy и замещать функции
///////////////////////////////////////////////////////////////////////////////
template < 
   //Указатель на дочерний объект
   //Именно этот тип должен храниться в контейнере (возможно с некоторой 
   //дополнительной информацией)
   class PointerT = NullType *, 
   
   //Итератор по контейнеру
   class IteratorT = PointerT, 
   
   //Тип парматра функции Append, отличается от TPointer в связи с тем что возможно будет необходимо
   //передавать дополнительную информацию
   class AppendParamT = PointerT, 
   
   //Некоторый родительский объект, указатель на который возвращает функция GetParent(). 
   //GetParent() необходимо замещать только, если планируется вызывать 
   //TRelationshipOneToMany::THolder::GetParent()
   class ParentT = NullType
>
struct TRelationshipOneToManyBaseStrategy
{
   typedef PointerT TPointer;
   typedef IteratorT TIterator;
   typedef AppendParamT TAppendParam;
   typedef ParentT TParent;

   //Начало/конец диапозона
   TIterator Begin();
   TIterator End();

   //Добавить элемент в контейнер и вернуть итератор на него
   TIterator Append( TAppendParam Param );

   //Удалить элемент из контейнера
   void Erase( TIterator iterChild );

   //Удалить все элементы из контейнера
   void Clear();

   //Получить по разыменованному итератору указатель на дочерний объект
   TPointer GetPtr( typename std::iterator_traits<TIterator>::reference Ref );

   //Получить по типу TAppendParam указатель на дочерний объект
   TPointer GetPtr( TAppendParam Param );

   //Можно опредилить конструктор принимающий один параметр
   template<class T>
   TRelationshipOneToManyBaseStrategy( const T &t ) {}
   TRelationshipOneToManyBaseStrategy() {}

   //Указатель на некоторый родительский объект необходимо замещать только, если планируется 
   //вызывать TRelationshipOneToMany::THolder::GetParent()
   TParent *GetParent();
   const TParent *GetParent() const;

   //Функция вызывается перед тем как регистрация дочернего объекта iterChild будет удалена
   //и будет произведён обрыв связи
   //(Методы Erase и Clear вызываются уже с разорваной связью дочерний объект -> родитель)
   void BeforeUnregister( TIterator iterChild ) const {}
};
///////////////////////////////////////////////////////////////////////////////

template< class ConteinerStrategyT >
class TRelationshipOneToMany: public ConteinerStrategyT
{
   typedef typename ConteinerStrategyT::TIterator    TIterator;
   typedef typename ConteinerStrategyT::TPointer     TPointer;
   typedef typename ConteinerStrategyT::TAppendParam TAppendParam;

public:
   // Класс от которого должны наследоваться все дочерние объекты
   class THolder;

private:
   //Если использовать просто THolder то возникает неоднозначность если
   //дети связаны с несколькими TRelationshipOneToMany
   typedef typename TRelationshipOneToMany::THolder TThisHolder;

public:  
   TRelationshipOneToMany() {}

   //VC может ругаться на эту строчку если небходимо закрыто наследовать 
   //TRelationshipOneToMany. Это ошибка компилятора, подробнее здесь
   //http://rsdn.ru/Forum/Message.aspx?mid=2493952
   //Для решения проблемы нужно у типа передаваемого в ConteinerStrategyT
   //определить шаблонный конструктор
   template<class T>
   explicit TRelationshipOneToMany( const T &t ): ConteinerStrategyT(t) {}

   //Связать родительский this и дочерний Param объект
   void Register( TAppendParam Param );

   //Удалить связь родительского this и дочернего iterChild объекта
   void Unregister( TIterator iterChild );

   //Удалить все связи
   void UnregisterAll();

   //Удалить существующую связь у дочернего объекта Param и связать его с this
   void MoveFrom( TAppendParam Param );

   ~TRelationshipOneToMany() { UnregisterAll(); }

public: //Функции используемые для отладки, в assert'ах
   //Проверить на валидность итератор
   //const заккоментировать для того чтобы не требовать от пользователя константных Begin() и End()
   bool Check( TIterator iterChild ) /*const*/;  

private:
   //Зарегистрирован ли уже объект
   bool IsContain( TAppendParam Param );

private:                                                                  
   //Ссылка на родительский объект у дочернего
   struct TRegData;
};
///////////////////////////////////////////////////////////////////////////////

template< class ConteinerStrategyT >
class TRelationshipOneToMany<ConteinerStrategyT>::THolder: public NWLib::NonCopyable
{
public:
   ~THolder() { Unregister(); }
   
   //Принудительно удалить регистрацию
   void Unregister() { if(m_RegData.m_pParent) m_RegData.m_pParent->Unregister(m_RegData.m_iterChild); }

   //Вернуть ссылку на, определённый пользователем, класс родителя или 0 если нет регистрации
   typename ConteinerStrategyT::TParent *GetParent() 
   { return m_RegData.m_pParent ? m_RegData.m_pParent->ConteinerStrategyT::GetParent() : 0; }

   typename const ConteinerStrategyT::TParent *GetParent() const 
   { return m_RegData.m_pParent ? m_RegData.m_pParent->ConteinerStrategyT::GetParent() : 0;  }

private:
   friend TRelationshipOneToMany;

   void SetRegData( const TRegData &RegData ) { m_RegData = RegData; }
   const TRegData &GetRegData() const { return m_RegData; }
private:
   TRegData m_RegData;
};
///////////////////////////////////////////////////////////////////////////////

template< class ConteinerStrategyT >
struct TRelationshipOneToMany<ConteinerStrategyT>::TRegData
{      
private:   
   friend TRelationshipOneToMany;
   friend TRelationshipOneToMany::THolder;

   TRegData(): m_pParent(0) {}
   TRegData( TRelationshipOneToMany *pParent, TIterator iterChild ): m_pParent(pParent), m_iterChild(iterChild) {}

   TRelationshipOneToMany *m_pParent;
   TIterator m_iterChild;
};
///////////////////////////////////////////////////////////////////////////////

template< class ConteinerStrategyT >
inline bool TRelationshipOneToMany<ConteinerStrategyT>::IsContain( TAppendParam Param )
{
   for( TIterator I = ConteinerStrategyT::Begin(); I != ConteinerStrategyT::End(); ++I )
      if( ConteinerStrategyT::GetPtr(*I) == ConteinerStrategyT::GetPtr(Param) )
         return true;

   return false;
}
///////////////////////////////////////////////////////////////////////////////

template< class ConteinerStrategyT >
inline bool TRelationshipOneToMany<ConteinerStrategyT>::Check( TIterator iterChild )
{
   return 
      NWLib::CheckInRange(ConteinerStrategyT::Begin(), ConteinerStrategyT::End(), iterChild) &&
      ConteinerStrategyT::GetPtr(*iterChild) != 0 &&
      ConteinerStrategyT::GetPtr(*iterChild)->TThisHolder::GetRegData().m_pParent == this &&
      ConteinerStrategyT::GetPtr(*iterChild)->TThisHolder::GetRegData().m_iterChild == iterChild;
}
///////////////////////////////////////////////////////////////////////////////

template< class ConteinerStrategyT >
inline void TRelationshipOneToMany<ConteinerStrategyT>::Register( TAppendParam Param )
{
   APL_ASSERT_PTR( ConteinerStrategyT::GetPtr(Param) );
   APL_ASSERT( !IsContain(Param) );
   APL_ASSERT( ConteinerStrategyT::GetPtr(Param)->TThisHolder::GetRegData().m_pParent == 0 );

   ConteinerStrategyT::GetPtr(Param)->TThisHolder::SetRegData( TRegData(this, ConteinerStrategyT::Append(Param)) ); 
}
///////////////////////////////////////////////////////////////////////////////

template< class ConteinerStrategyT >
inline void TRelationshipOneToMany<ConteinerStrategyT>::Unregister( TIterator iterChild )
{
   APL_ASSERT( Check(iterChild) );

   ConteinerStrategyT::BeforeUnregister( iterChild );
   ConteinerStrategyT::GetPtr( *iterChild )->TThisHolder::SetRegData( TRegData() );
   ConteinerStrategyT::Erase( iterChild );
}
///////////////////////////////////////////////////////////////////////////////

template< class ConteinerStrategyT >
inline void TRelationshipOneToMany<ConteinerStrategyT>::UnregisterAll()
{
   for( TIterator I = ConteinerStrategyT::Begin(); I != ConteinerStrategyT::End(); ++I )
   {
      APL_ASSERT( Check(I) );
      ConteinerStrategyT::BeforeUnregister( I );
      ConteinerStrategyT::GetPtr(*I)->TThisHolder::SetRegData( TRegData() );
   }

   ConteinerStrategyT::Clear();
}
///////////////////////////////////////////////////////////////////////////////

template< class ConteinerStrategyT >
inline void TRelationshipOneToMany<ConteinerStrategyT>::MoveFrom( TAppendParam Param )
{
   APL_ASSERT_PTR( ConteinerStrategyT::GetPtr(Param) );
   APL_ASSERT_PTR( ConteinerStrategyT::GetPtr(Param)->TThisHolder::GetRegData().m_pParent );  
   APL_ASSERT( ConteinerStrategyT::GetPtr(Param)->TThisHolder::GetRegData().m_pParent != this );  
   
   ConteinerStrategyT::GetPtr(Param)->TThisHolder::Unregister();
   Register(Param);
}
///////////////////////////////////////////////////////////////////////////////

} //namespace NWLib 

#endif