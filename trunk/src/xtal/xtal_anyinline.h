/** \file src/xtal/xtal_anyinline.h
* \brief src/xtal/xtal_anyinline.h
*/

#ifndef XTAL_ANYINLINE_H_INCLUDE_GUARD
#define XTAL_ANYINLINE_H_INCLUDE_GUARD

#pragma once

namespace xtal{

/*
void xmemmove(int_t* s1, const int_t* s2, size_t n);
void xmemcpy(int_t* s1, const int_t* s2, size_t n);
void xmemset(int_t* s, int_t c, size_t n);

template<class T>
void  xmemmove(T* s1, const T* s2, size_t n){
	xmemmove((int_t*)s1, (const int_t*)s2, n*(sizeof(T)/sizeof(int_t)));
}

template<class T>
void  xmemcpy(T* s1, const T* s2, size_t n){
	xmemcpy((int_t*)s1, (const int_t*)s2, n*(sizeof(T)/sizeof(int_t)));
}

template<class T>
void  xmemset(T* s, int_t c, size_t n){
	xmemset((int_t*)s, c, n*(sizeof(T)/sizeof(int_t)));
}
*/

inline const ClassPtr& Base::get_class(){ 
	return to_smartptr(class_); 
}

inline void add_ref_count_force(const Any& v, int_t i){
	uint_t t = type(v);
	if(t>=TYPE_BASE){
		rcpvalue(v)->add_ref_count(i);
	}
}

inline void inc_ref_count_force(const Any& v){
	uint_t t = type(v);
	if(t>=TYPE_BASE){
		rcpvalue(v)->inc_ref_count();
	}
}

inline void dec_ref_count_force(const Any& v){
	uint_t t = type(v);
	if(t>=TYPE_BASE){
		rcpvalue(v)->dec_ref_count();
	}
}

inline InstanceVariables* Any::instance_variables() const{
	if(type(*this)==TYPE_BASE){
		return pvalue(*this)->instance_variables();
	}
	return &empty_instance_variables;
}

inline void Any::visit_members(Visitor& m) const{
	if(type(*this)>=TYPE_BASE){
		rcpvalue(*this)->visit_members(m); 
	}
}

//{REPEAT{{
/*

/// \brief primary_keyメソッドを呼び出す
template<class A0 #COMMA_REPEAT#class A`i+1`#>
inline AnyPtr Any::send(const IDPtr& primary_key, const A0& a0  #COMMA_REPEAT#const A`i+1`& a`i+1`#) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); #REPEAT#vm->push_arg(a`i+1`); #
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();	
}

/// \brief primary_key#secondary_keyメソッドを呼び出す
template<class A0 #COMMA_REPEAT#class A`i+1`#>
inline AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0  #COMMA_REPEAT#const A`i+1`& a`i+1`#) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); #REPEAT#vm->push_arg(a`i+1`); #
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();	
}

/// \brief 関数を呼び出す
template<class A0 #COMMA_REPEAT#class A`i+1`#>
inline AnyPtr Any::call(const A0& a0  #COMMA_REPEAT#const A`i+1`& a`i+1`#) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); #REPEAT#vm->push_arg(a`i+1`); #
	rawcall(vm);
	return vm->result_and_cleanup_call();	
}

*/


/// \brief primary_keyメソッドを呼び出す
template<class A0 >
inline AnyPtr Any::send(const IDPtr& primary_key, const A0& a0  ) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); 
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();	
}

/// \brief primary_key#secondary_keyメソッドを呼び出す
template<class A0 >
inline AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0  ) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); 
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();	
}

/// \brief 関数を呼び出す
template<class A0 >
inline AnyPtr Any::call(const A0& a0  ) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); 
	rawcall(vm);
	return vm->result_and_cleanup_call();	
}



/// \brief primary_keyメソッドを呼び出す
template<class A0 , class A1>
inline AnyPtr Any::send(const IDPtr& primary_key, const A0& a0  , const A1& a1) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); 
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();	
}

/// \brief primary_key#secondary_keyメソッドを呼び出す
template<class A0 , class A1>
inline AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0  , const A1& a1) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); 
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();	
}

/// \brief 関数を呼び出す
template<class A0 , class A1>
inline AnyPtr Any::call(const A0& a0  , const A1& a1) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); 
	rawcall(vm);
	return vm->result_and_cleanup_call();	
}



/// \brief primary_keyメソッドを呼び出す
template<class A0 , class A1, class A2>
inline AnyPtr Any::send(const IDPtr& primary_key, const A0& a0  , const A1& a1, const A2& a2) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); 
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();	
}

/// \brief primary_key#secondary_keyメソッドを呼び出す
template<class A0 , class A1, class A2>
inline AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0  , const A1& a1, const A2& a2) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); 
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();	
}

/// \brief 関数を呼び出す
template<class A0 , class A1, class A2>
inline AnyPtr Any::call(const A0& a0  , const A1& a1, const A2& a2) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); 
	rawcall(vm);
	return vm->result_and_cleanup_call();	
}



/// \brief primary_keyメソッドを呼び出す
template<class A0 , class A1, class A2, class A3>
inline AnyPtr Any::send(const IDPtr& primary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); 
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();	
}

/// \brief primary_key#secondary_keyメソッドを呼び出す
template<class A0 , class A1, class A2, class A3>
inline AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); 
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();	
}

/// \brief 関数を呼び出す
template<class A0 , class A1, class A2, class A3>
inline AnyPtr Any::call(const A0& a0  , const A1& a1, const A2& a2, const A3& a3) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); 
	rawcall(vm);
	return vm->result_and_cleanup_call();	
}



/// \brief primary_keyメソッドを呼び出す
template<class A0 , class A1, class A2, class A3, class A4>
inline AnyPtr Any::send(const IDPtr& primary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); 
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();	
}

/// \brief primary_key#secondary_keyメソッドを呼び出す
template<class A0 , class A1, class A2, class A3, class A4>
inline AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); 
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();	
}

/// \brief 関数を呼び出す
template<class A0 , class A1, class A2, class A3, class A4>
inline AnyPtr Any::call(const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); 
	rawcall(vm);
	return vm->result_and_cleanup_call();	
}



/// \brief primary_keyメソッドを呼び出す
template<class A0 , class A1, class A2, class A3, class A4, class A5>
inline AnyPtr Any::send(const IDPtr& primary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); 
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();	
}

/// \brief primary_key#secondary_keyメソッドを呼び出す
template<class A0 , class A1, class A2, class A3, class A4, class A5>
inline AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); 
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();	
}

/// \brief 関数を呼び出す
template<class A0 , class A1, class A2, class A3, class A4, class A5>
inline AnyPtr Any::call(const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); 
	rawcall(vm);
	return vm->result_and_cleanup_call();	
}



/// \brief primary_keyメソッドを呼び出す
template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6>
inline AnyPtr Any::send(const IDPtr& primary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); 
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();	
}

/// \brief primary_key#secondary_keyメソッドを呼び出す
template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6>
inline AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); 
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();	
}

/// \brief 関数を呼び出す
template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6>
inline AnyPtr Any::call(const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); 
	rawcall(vm);
	return vm->result_and_cleanup_call();	
}



/// \brief primary_keyメソッドを呼び出す
template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7>
inline AnyPtr Any::send(const IDPtr& primary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); 
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();	
}

/// \brief primary_key#secondary_keyメソッドを呼び出す
template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7>
inline AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); 
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();	
}

/// \brief 関数を呼び出す
template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7>
inline AnyPtr Any::call(const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); 
	rawcall(vm);
	return vm->result_and_cleanup_call();	
}



/// \brief primary_keyメソッドを呼び出す
template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
inline AnyPtr Any::send(const IDPtr& primary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); 
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();	
}

/// \brief primary_key#secondary_keyメソッドを呼び出す
template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
inline AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); 
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();	
}

/// \brief 関数を呼び出す
template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8>
inline AnyPtr Any::call(const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); 
	rawcall(vm);
	return vm->result_and_cleanup_call();	
}



/// \brief primary_keyメソッドを呼び出す
template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
inline AnyPtr Any::send(const IDPtr& primary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); vm->push_arg(a9); 
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();	
}

/// \brief primary_key#secondary_keyメソッドを呼び出す
template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
inline AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); vm->push_arg(a9); 
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();	
}

/// \brief 関数を呼び出す
template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9>
inline AnyPtr Any::call(const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); vm->push_arg(a9); 
	rawcall(vm);
	return vm->result_and_cleanup_call();	
}



/// \brief primary_keyメソッドを呼び出す
template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
inline AnyPtr Any::send(const IDPtr& primary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); vm->push_arg(a9); vm->push_arg(a10); 
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();	
}

/// \brief primary_key#secondary_keyメソッドを呼び出す
template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
inline AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); vm->push_arg(a9); vm->push_arg(a10); 
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();	
}

/// \brief 関数を呼び出す
template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10>
inline AnyPtr Any::call(const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); vm->push_arg(a9); vm->push_arg(a10); 
	rawcall(vm);
	return vm->result_and_cleanup_call();	
}



/// \brief primary_keyメソッドを呼び出す
template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
inline AnyPtr Any::send(const IDPtr& primary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); vm->push_arg(a9); vm->push_arg(a10); vm->push_arg(a11); 
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();	
}

/// \brief primary_key#secondary_keyメソッドを呼び出す
template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
inline AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); vm->push_arg(a9); vm->push_arg(a10); vm->push_arg(a11); 
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();	
}

/// \brief 関数を呼び出す
template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11>
inline AnyPtr Any::call(const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); vm->push_arg(a9); vm->push_arg(a10); vm->push_arg(a11); 
	rawcall(vm);
	return vm->result_and_cleanup_call();	
}



/// \brief primary_keyメソッドを呼び出す
template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
inline AnyPtr Any::send(const IDPtr& primary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); vm->push_arg(a9); vm->push_arg(a10); vm->push_arg(a11); vm->push_arg(a12); 
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();	
}

/// \brief primary_key#secondary_keyメソッドを呼び出す
template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
inline AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); vm->push_arg(a9); vm->push_arg(a10); vm->push_arg(a11); vm->push_arg(a12); 
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();	
}

/// \brief 関数を呼び出す
template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12>
inline AnyPtr Any::call(const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); vm->push_arg(a9); vm->push_arg(a10); vm->push_arg(a11); vm->push_arg(a12); 
	rawcall(vm);
	return vm->result_and_cleanup_call();	
}



/// \brief primary_keyメソッドを呼び出す
template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
inline AnyPtr Any::send(const IDPtr& primary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12, const A13& a13) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); vm->push_arg(a9); vm->push_arg(a10); vm->push_arg(a11); vm->push_arg(a12); vm->push_arg(a13); 
	rawsend(vm, primary_key);
	return vm->result_and_cleanup_call();	
}

/// \brief primary_key#secondary_keyメソッドを呼び出す
template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
inline AnyPtr Any::send2(const IDPtr& primary_key, const AnyPtr& secondary_key, const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12, const A13& a13) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); vm->push_arg(a9); vm->push_arg(a10); vm->push_arg(a11); vm->push_arg(a12); vm->push_arg(a13); 
	rawsend(vm, primary_key, secondary_key);
	return vm->result_and_cleanup_call();	
}

/// \brief 関数を呼び出す
template<class A0 , class A1, class A2, class A3, class A4, class A5, class A6, class A7, class A8, class A9, class A10, class A11, class A12, class A13>
inline AnyPtr Any::call(const A0& a0  , const A1& a1, const A2& a2, const A3& a3, const A4& a4, const A5& a5, const A6& a6, const A7& a7, const A8& a8, const A9& a9, const A10& a10, const A11& a11, const A12& a12, const A13& a13) const{
	const VMachinePtr& vm = setup_call();
	vm->push_arg(a0); vm->push_arg(a1); vm->push_arg(a2); vm->push_arg(a3); vm->push_arg(a4); vm->push_arg(a5); vm->push_arg(a6); vm->push_arg(a7); vm->push_arg(a8); vm->push_arg(a9); vm->push_arg(a10); vm->push_arg(a11); vm->push_arg(a12); vm->push_arg(a13); 
	rawcall(vm);
	return vm->result_and_cleanup_call();	
}


//}}REPEAT}

}

#endif // XTAL_ANYINLINE_H_INCLUDE_GUARD
