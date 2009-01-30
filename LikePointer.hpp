//Автор: Шестеркин Дмитрий(NW) 2005

#ifndef LikePointer_HPP
#define LikePointer_HPP

#include "SelectIterator.hpp"
#include "ReferenceCounters.hpp"

#include "AssocVector.hpp"
#include "StlAuxiliary.hpp"

namespace NWLib {

/*
   Модуль содержит ряд классов поведение которых напоминает поведение указателей
   (перегружены опрераторы * и ->), в том числе и итераторы.
   А так же различные классы реализующие парадигму владения объектом. 
*/

///////////////////////////////////////////////////////////////////////////////
// Расширенная реализация auto_ptr, тем что позволяет вместо оператора delete 
// использовать произвольную функцию, определяемую стратегией 
// реализация основана на:
// The C++ Standard Library - A Tutorial and Reference
// by Nicolai M. Josuttis, Addison-Wesley, 1999
// Наследуем стратегию для того чтобы позволить провести компилятору "оптимизацию
// пустого базового класса"
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Стратегии удаления. Требования:
//    Стратегии должны корректно обрабатывать попытку удаления нулевого указателя
//    Стратегии должны конструироваться и присваиваться стратегиям с другим шаблонным типом
///////////////////////////////////////////////////////////////////////////////

//Вспомогательный класс, базовый для всех стратегий
template<class ValueT, class PointerT = ValueT *, class ReferenceT = ValueT &>
struct auto_ptr_ex_strategy_base
{
   typedef ValueT value_type;
   typedef PointerT pointer;
   typedef ReferenceT reference;
};

//Стратегия удаления при помощи оператора delete
template< class T >
struct auto_ptr_ex_delete_strategy: public auto_ptr_ex_strategy_base<T> 
{
   auto_ptr_ex_delete_strategy() {}
   
   template< class Y >
   auto_ptr_ex_delete_strategy( const auto_ptr_ex_delete_strategy<Y> & ) {}
   
   template< class Y >
   auto_ptr_ex_delete_strategy &operator=( const auto_ptr_ex_delete_strategy<Y> & ) { return *this; }

   void operator()( pointer pT ) const { delete pT; }
};
///////////////////////////////////////////////////////////////////////////////

//Стратегия удаления при помощи метода Release
template< class T >
struct auto_ptr_ex_release_strategy: public auto_ptr_ex_strategy_base<T>
{
   auto_ptr_ex_release_strategy() {}
   
   template< class Y >
   auto_ptr_ex_release_strategy( const auto_ptr_ex_release_strategy<Y> & ) {}

   template< class Y >
   auto_ptr_ex_release_strategy &operator=( const auto_ptr_ex_release_strategy<Y> & ) { return *this; }

   void operator()( pointer pT ) const { if(pT != 0) pT->Release(); }
};
///////////////////////////////////////////////////////////////////////////////

//Пустая стратегия, удаления не происходит
template< class T >
struct auto_ptr_ex_empty_strategy: public auto_ptr_ex_strategy_base<T> 
{
   auto_ptr_ex_empty_strategy() {}
   
   template< class Y >
   auto_ptr_ex_empty_strategy( const auto_ptr_ex_empty_strategy<Y> & ) {}

   template< class Y >
   auto_ptr_ex_empty_strategy &operator=( const auto_ptr_ex_empty_strategy<Y> & ) { return *this; }

   void operator()( pointer pT ) const { /*ничего не делаем*/}
};


// auxiliary type to enable copies and assignments (now global)
template<class Y, template <class> class DeleteStrategy>
struct auto_ptr_ref_ex: private DeleteStrategy<Y>
{
   typedef DeleteStrategy<Y> TDeleteStrategy;
   typedef typename DeleteStrategy<Y>::pointer pointer;

   template<class OtherT, template <class> class OtherDeleteStrategy > friend class auto_ptr_ex;

   pointer yp;

   auto_ptr_ref_ex (pointer rhs, const TDeleteStrategy &DS)
   : yp(rhs), TDeleteStrategy(DS) {
   }
};

template<class T, template <class> class DeleteStrategy = auto_ptr_ex_delete_strategy >
class auto_ptr_ex: private DeleteStrategy<T> {
public:
   typedef DeleteStrategy<T> TDeleteStrategy;
   typedef typename DeleteStrategy<T>::pointer pointer;
   typedef typename DeleteStrategy<T>::reference reference;
   typedef typename DeleteStrategy<T>::value_type value_type;
   typedef value_type element_type; //Синоним value_type, для совместимости
  
private:
   pointer ap;    // refers to the actual owned object (if any)

   template<class OtherT, template <class> class OtherDeleteStrategy > friend class auto_ptr_ex;

private:  
   struct OperatorHelper{ int i; }; 
   typedef int OperatorHelper::*TUndefinedBoolType;

public:

   // constructor
   
   //Нельзя писать конструктор который принимает TDeleteStrategy, т.к. это позволит 
   //конструировать auto_ptr_ex из константного auto_ptr_ex минуя auto_ptr_ref_ex
   //Если наследовать DeleteStrategy закрыто то ситуация не изменится
   //explicit auto_ptr_ex( const TDeleteStrategy &DS ) throw()
   //   : ap(0), TDeleteStrategy(DS) {}

   explicit auto_ptr_ex (pointer ptr = 0, const TDeleteStrategy &DS = TDeleteStrategy()) throw()
      : ap(ptr), TDeleteStrategy(DS) {}

   // copy constructors (with implicit conversion)
   // - note: nonconstant parameter
   auto_ptr_ex (auto_ptr_ex& rhs) throw()
   : ap(rhs.release()), TDeleteStrategy(rhs) {
   }
   template<class Y>
   auto_ptr_ex (auto_ptr_ex<Y, DeleteStrategy>& rhs) throw()
   : ap(rhs.release()), TDeleteStrategy(rhs) {
   }
   
   //Конструируем из std::auto_ptr
   template<class Y>
   auto_ptr_ex (std::auto_ptr<Y>& rhs) throw()
   : ap(rhs.release()), TDeleteStrategy(auto_ptr_ex_delete_strategy<T>()) {
   }

   // assignments (with implicit conversion)
   // - note: nonconstant parameter
   auto_ptr_ex& operator= (auto_ptr_ex& rhs) throw() {
      reset(rhs.release());
      static_cast<TDeleteStrategy&>(*this) = rhs;
      return *this;
   }
   
   template<class Y>
   auto_ptr_ex& operator= (auto_ptr_ex<Y, DeleteStrategy>& rhs) throw() {
      reset(rhs.release());
      static_cast<TDeleteStrategy&>(*this) = rhs;
      return *this;
   }

   //Присваиваем std::auto_ptr
   template<class Y>
   auto_ptr_ex& operator= (std::auto_ptr<Y>& rhs) throw() {
      reset(rhs.release());
      static_cast<TDeleteStrategy&>(*this) = auto_ptr_ex_delete_strategy<T>();
      return *this;
   }
   
   // destructor
   ~auto_ptr_ex() throw() {
      static_cast<TDeleteStrategy&>(*this)( ap );
   }

   // value access
   pointer get() const throw() {
      return ap;
   }
   reference operator*() const throw() {
      APL_ASSERT_PTR(ap);
      return *ap;
   }
   pointer operator->() const throw() {
      APL_ASSERT_PTR(ap);
      return ap;
   }

   // release ownership
   pointer release() throw() {
      pointer tmp(ap);
      ap = 0;
      return tmp;
   }

   // reset value
   void reset ( pointer ptr = 0 ) throw() {
      if (ap != ptr) {
         static_cast<TDeleteStrategy&>(*this)( ap );
         ap = ptr;
      }
   }

   /* special conversions with auxiliary type to enable copies and assignments
   */
   auto_ptr_ex(auto_ptr_ref_ex<T, DeleteStrategy> rhs) throw()
   : ap(rhs.yp), TDeleteStrategy(rhs) {
   }
   auto_ptr_ex& operator= (auto_ptr_ref_ex<T, DeleteStrategy> rhs) throw() {  // new
         reset(rhs.yp);
         static_cast<TDeleteStrategy&>(*this) = rhs;
         return *this;
   }

   template<class Y>
   operator auto_ptr_ref_ex<Y, DeleteStrategy>() throw() {
      return auto_ptr_ref_ex<Y, DeleteStrategy>(release(), static_cast<TDeleteStrategy&>(*this));
   }
   
   //Этот метод нужен для того, что бы работал следующий код:
   //    struct base{}; struct derived: base{};
   //    void f( const std::auto_ptr<base> & );
   //    void m(){ f( std::auto_ptr<derived>() ); }
   //http://www.rsdn.ru/forum/message/3270892.all.aspx
   template<class Y>
   operator auto_ptr_ex<Y, DeleteStrategy>() throw() {
      return auto_ptr_ex<Y, DeleteStrategy>(release(), static_cast<TDeleteStrategy&>(*this));
   }

   void SetDeleteStrategy(const TDeleteStrategy &DS) {
      static_cast<TDeleteStrategy&>(*this) = DS;
   }

   const TDeleteStrategy &GetDeleteStrategy() const {
      return static_cast<const TDeleteStrategy&>(*this);
   }

public:
   //Позволет проверять указатель на равенство/неравенство нулю и приводить к типу bool
   operator TUndefinedBoolType() const{ return ap != 0? &OperatorHelper::i : 0; }
   
   //Нельзя определить auto_ptr_ex( TUndefinedBoolType ), т.к. возникает неоднозначность 
   //с auto_ptr_ex(auto_ptr_ref_ex<T, DeleteStrategy> rhs)

   //Если не определять то будет использоваться автоматическое преобразование к TUndefinedBoolType
   //и встроенные операторы
   friend bool operator==( const auto_ptr_ex &P1, const auto_ptr_ex &P2 ){ return P1.ap == P2.ap; }
   friend bool operator!=( const auto_ptr_ex &P1, const auto_ptr_ex &P2 ){ return P1.ap != P2.ap; }

   //Если не опредилять по при проверке P1 == 0, 0 == P1, P1 != 0, 0 != P1 возникнет 
   //неоднозначность если конструктор будет не explicit
   friend bool operator==( const auto_ptr_ex &P1, TUndefinedBoolType P2 ){ return P1.ap == 0; }
   friend bool operator==( TUndefinedBoolType P2, const auto_ptr_ex &P1 ){ return P1.ap == 0; }
   friend bool operator!=( const auto_ptr_ex &P1, TUndefinedBoolType P2 ){ return P1.ap != 0; }
   friend bool operator!=( TUndefinedBoolType P2, const auto_ptr_ex &P1 ){ return P1.ap != 0; }
};

///////////////////////////////////////////////////////////////////////////////
// Интеллектуальный указатель обладающей семантикой клонирования. Т.е. при 
// присвоении и конструировании создаётся дубликат объетка и указатель на него
// переходит во владение класса
// Благодаря этому такие указатели можно хранить в контейнерах STL
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Стратегии клонирования. Требования:
//    Стратегии должны корректно обрабатывать попытку клонирования нулевого указателя (возвращать 0)
//    Стратегии должны конструироваться и присваиваться стратегиям с другим шаблонным типом
///////////////////////////////////////////////////////////////////////////////

//Стратегия конирования при помощи метода Clone, который возвращает укаатель на клонируемый объект
template< class T >
struct clone_ptr_ex_strategy_clone_member: public auto_ptr_ex_strategy_base<T>
{
   clone_ptr_ex_strategy_clone_member() {}

   template< class Y >
      clone_ptr_ex_strategy_clone_member( const clone_ptr_ex_strategy_clone_member<Y> & ) {}

   template< class Y >
      clone_ptr_ex_strategy_clone_member &operator=( const clone_ptr_ex_strategy_clone_member<Y> & ) { return *this; }

   pointer operator()( pointer pT ) const { return pT ? pT->Clone() : static_cast<pointer>(0); }
};
///////////////////////////////////////////////////////////////////////////////

//Стратегия конирования при помощи метода Clone, который возвращает auto_ptr
template< class T >
struct clone_ptr_ex_strategy_clone_member_auto_ptr: public auto_ptr_ex_strategy_base<T> 
{
   clone_ptr_ex_strategy_clone_member_auto_ptr() {}

   template< class Y >
      clone_ptr_ex_strategy_clone_member_auto_ptr( const clone_ptr_ex_strategy_clone_member<Y> & ) {}

   template< class Y >
      clone_ptr_ex_strategy_clone_member_auto_ptr &operator=( const clone_ptr_ex_strategy_clone_member<Y> & ) { return *this; }

   pointer operator()( pointer pT ) const { return pT ? pT->Clone().release() : static_cast<pointer>(0); }
};
///////////////////////////////////////////////////////////////////////////////

template<
   class T, 
   template <class> class CloneStrategy = clone_ptr_ex_strategy_clone_member, 
   template <class> class DeleteStrategy = auto_ptr_ex_delete_strategy
>
class clone_ptr_ex: private CloneStrategy<T>, private DeleteStrategy<T> {
public:
   typedef DeleteStrategy<T> TDeleteStrategy;
   typedef CloneStrategy<T> TCloneStrategy;
   typedef typename DeleteStrategy<T>::pointer pointer;
   typedef typename DeleteStrategy<T>::reference reference;
   typedef typename DeleteStrategy<T>::value_type value_type;
   typedef value_type element_type; //Синоним value_type, для совместимости

private:
   pointer ap;    // refers to the actual owned object (if any)

   template<class OtherT, template <class> class OtherCloneStrategy, template <class> class OtherDeleteStrategy > friend class clone_ptr_ex;

private:  
   struct OperatorHelper{ int i; }; 
   typedef int OperatorHelper::*TUndefinedBoolType;

public:
   // constructor
   explicit clone_ptr_ex ( pointer ptr = 0, const TCloneStrategy &CS = TCloneStrategy(), const TDeleteStrategy &DS = TDeleteStrategy() ) throw()
      : ap(ptr), TCloneStrategy(CS), TDeleteStrategy(DS) {}

   // copy constructors (with implicit conversion)
   clone_ptr_ex (const clone_ptr_ex& rhs)
   : ap(TCloneStrategy::operator()(rhs.ap)), TCloneStrategy(rhs), TDeleteStrategy(rhs) {
   }
   template<class Y>
   clone_ptr_ex (const clone_ptr_ex<Y, CloneStrategy, DeleteStrategy>& rhs)
   : ap(TCloneStrategy::operator()(rhs.ap)), TCloneStrategy(rhs), TDeleteStrategy(rhs) {
   }
   
   //Конструируем из std::auto_ptr
   template<class Y>
   clone_ptr_ex (const std::auto_ptr<Y>& rhs) throw()
   : ap(TCloneStrategy::operator()(rhs.get())), TDeleteStrategy(auto_ptr_ex_delete_strategy<T>()) {
   }

   //Конструируем из std::auto_ptr_ex
   template<class Y>
   clone_ptr_ex (const auto_ptr_ex<Y, DeleteStrategy>& rhs) throw()
   : ap(TCloneStrategy::operator()(rhs.get())), TDeleteStrategy(rhs.GetDeleteStrategy()) {
   }

   // assignments (with implicit conversion)
   clone_ptr_ex& operator=(const clone_ptr_ex& rhs) 
   {   
      reset(static_cast<TCloneStrategy&>(*this)(rhs.ap));
      static_cast<TCloneStrategy&>(*this) = rhs;
      static_cast<TDeleteStrategy&>(*this) = rhs;
      return *this;
   }
   
   template<class Y>
   clone_ptr_ex& operator=( const clone_ptr_ex<Y, CloneStrategy, DeleteStrategy>& rhs)
   {
      reset(static_cast<TCloneStrategy&>(*this)(rhs.ap));
      static_cast<TCloneStrategy&>(*this) = rhs;
      static_cast<TDeleteStrategy&>(*this) = rhs;
      return *this;
   }

   //Присваиваем std::auto_ptr
   template<class Y>
   clone_ptr_ex& operator= (const std::auto_ptr<Y>& rhs) throw() {
      reset(static_cast<TCloneStrategy&>(*this)(rhs.get()));
      static_cast<TDeleteStrategy&>(*this) = auto_ptr_ex_delete_strategy<T>();
      return *this;
   }
   
   //Присваиваем std::auto_ptr_ex
   template<class Y>
   clone_ptr_ex& operator= (const auto_ptr_ex<Y, DeleteStrategy>& rhs) throw() {
      reset(static_cast<TCloneStrategy&>(*this)(rhs.get()));
      static_cast<TDeleteStrategy&>(*this) = rhs.GetDeleteStrategy();
      return *this;
   }

  
   // destructor
   ~clone_ptr_ex() throw() {
      static_cast<TDeleteStrategy&>(*this)( ap );
   }

   // value access
   pointer get() const throw() {
      return ap;
   }
   reference operator*() const throw() {
      APL_ASSERT_PTR(ap);
      return *ap;
   }
   pointer operator->() const throw() {
      APL_ASSERT_PTR(ap);
      return ap;
   }

   // release ownership
   pointer release() throw() {
      pointer tmp(ap);
      ap = 0;
      return tmp;
   }

   // reset value
   void reset (pointer ptr=0) throw() {
      if (ap != ptr) 
      {
         static_cast<TDeleteStrategy&>(*this)( ap );
         ap = ptr;
      }
   }

   void SetDeleteStrategy(const TDeleteStrategy &DS) {
      static_cast<TDeleteStrategy&>(*this) = DS;
   }

   void SetCloneStrategy(const TCloneStrategy &DS) {
      static_cast<TCloneStrategy&>(*this) = DS;
   }

   const TDeleteStrategy &GetDeleteStrategy() const {
      return static_cast<const TDeleteStrategy&>(*this);
   }
   
   const TCloneStrategy &GetCloneStrategy() const {
      return static_cast<const TCloneStrategy&>(*this);
   }

public:
   //Позволет проверять указатель на равенство/неравенство нулю и приводить к типу bool
   operator TUndefinedBoolType() const{ return ap != 0? &OperatorHelper::i : 0; }

   //Если не определять то будет использоваться автоматическое преобразование к TUndefinedBoolType
   //и встроенные операторы
   friend bool operator==( const clone_ptr_ex &P1, const clone_ptr_ex &P2 ){ return P1.ap == P2.ap; }
   friend bool operator!=( const clone_ptr_ex &P1, const clone_ptr_ex &P2 ){ return P1.ap != P2.ap; }

   //Если не опредилять по при проверке P1 == 0, 0 == P1, P1 != 0, 0 != P1 возникнет 
   //неоднозначность если конструктор будет не explicit
   friend bool operator==( const clone_ptr_ex &P1, TUndefinedBoolType P2 ){ return P1.ap == 0; }
   friend bool operator==( TUndefinedBoolType P2, const clone_ptr_ex &P1 ){ return P1.ap == 0; }
   friend bool operator!=( const clone_ptr_ex &P1, TUndefinedBoolType P2 ){ return P1.ap != 0; }
   friend bool operator!=( TUndefinedBoolType P2, const clone_ptr_ex &P1 ){ return P1.ap != 0; }
};

} //namespace NWLib 

#include "ObjectSelector.hpp"

#endif