
#pragma once

namespace xtal{
	
/**
* @brief cast関数、as関数の戻り値の型を決定するためのヘルパーテンプレートクラス
*
* ほとんどの場合、CastResult<T>::typeはT自身を返す。
* 異なる場合としては、例えばCastResult<const int&>::type はintを返す。
*/
template<class T>
struct CastResult{ typedef T type; };

template<class T> 
inline typename CastResult<T>::type as(const AnyPtr& a);
	
template<class T>
inline typename CastResult<T>::type cast(const AnyPtr& a);
	
template<class T>
inline bool can_cast(const AnyPtr& a);

template<class T>
inline typename CastResult<T>::type unchecked_cast(const AnyPtr& a);

template<class T>
const ClassPtr& get_cpp_class();

/////////////////////////////////////////////////////////////////////////////

void cast_failed(const AnyPtr& a, const ClassPtr& cls);

/////////////////////////////////////////////////////////////////////////////

// 変換後の型がSmartPtrの場合
template<class U, class V>
inline const void* unchecked_cast_helper(const AnyPtr& a, const SmartPtr<U>*, const V&){
	return &a;
}

// 変換後の型がBaseを継承した型の場合
template<class U>
inline const void* unchecked_cast_helper(const AnyPtr& a, const Base*, const U*){
	return pvalue(a);
}

// 変換後の型がRefCountingBaseBaseを継承した型の場合
template<class U>
inline const void* unchecked_cast_helper(const AnyPtr& a, const RefCountingBase*, const U*){
	return rcpvalue(a);
}

// 変換後の型がAnyを継承した型の場合
template<class U>
inline const void* unchecked_cast_helper(const AnyPtr& a, const Any*, const U*){
	return &a;
}

// 変換後の型がAnyやBaseを継承していない型の場合
template<class U>
inline const void* unchecked_cast_helper(const AnyPtr& a, const void*, const U*){
	return ((SmartPtr<U>&)a).get();
}

/////////////////////////////////////////////////////////////////////////////

// 変換後の型がSmartPtrの場合
template<class U, class V>
inline const void* get_null_helper(const SmartPtr<U>*, const V&){
	return &null;
}

// 変換後の型がBaseを継承した型の場合
template<class U>
inline const void* get_null_helper(const Base*, const U*){
	return 0;
}

// 変換後の型がRefCountingBaseを継承した型の場合
template<class U>
inline const void* get_null_helper(const RefCountingBase*, const U*){
	return 0;
}

// 変換後の型がAnyを継承した型の場合
template<class U>
inline const void* get_null_helper(const Any*, const U*){
	return &null;
}

// 変換後の型がAnyやBaseを継承していない型の場合
template<class U>
inline const void* get_null_helper(const void*, const U*){
	return U();
}

/////////////////////////////////////////////////////////////////////////////

template<class T>
struct CastHelper{
	// 変換後の型が参照でもポインタでもない場合、ポインタ型としてキャストしたあと実体にする
	
	static bool can_cast(const AnyPtr& a){ 
		return a->is(get_cpp_class<T>());
	}

	static T unchecked_cast(const AnyPtr& a){ 
		return *CastHelper<const T*>::unchecked_cast(a); 
	}
	
	static T get_null(){ 
		return *CastHelper<const T*>::get_null();
	}
};

template<class T>
struct CastHelper<T&>{
	// 変換後の型が参照の場合、ポインタ型としてキャストしたあと参照にする
	
	static bool can_cast(const AnyPtr& a){ 
		return a->is(get_cpp_class<T>());
	}
	
	static T& unchecked_cast(const AnyPtr& a){ 
		return *CastHelper<const T*>::unchecked_cast(a); 
	}

	static T& get_null(){ 
		return *CastHelper<const T*>::get_null();
	}
};

template<class T>
struct CastHelper<T*>{
	// 変換後の型がポインタの場合

	static bool can_cast(const AnyPtr& a){ 
		return a->is(get_cpp_class<T>());
	}

	static T* unchecked_cast(const AnyPtr& a){ 
		return (T*)unchecked_cast_helper(a, (T*)0, (T*)0); 
	}	

	static T* get_null(){ 
		return (T*)get_null_helper((T*)0, (T*)0);
	}
};

template<class T>
struct CastHelperHelper{
	static T as(const AnyPtr& a){
		if(can_cast(a)){
			return unchecked_cast(a);
		}
		else{
			return get_null();
		}
	}
		
	static T cast(const AnyPtr& a){ 
		if(can_cast(a)){
			return unchecked_cast(a);
		}
		else{
			cast_failed(a, get_cpp_class<T>());
			return get_null();
		}
	}
		
	static bool can_cast(const AnyPtr& a){ 
		return CastHelper<T>::can_cast(a); 
	}

	static T unchecked_cast(const AnyPtr& a){ 
		return CastHelper<T>::unchecked_cast(a); 
	}
	
	static T get_null(){ 
		return CastHelper<T>::get_null();
	}
};

/////////////////////////////////////////////////////////////////////////////

template<>
struct CastHelper<const AnyPtr*>{
	static bool can_cast(const AnyPtr&){ return true; }
	static const AnyPtr* unchecked_cast(const AnyPtr& a){ return (AnyPtr*)&a; }
	static const AnyPtr* get_null(){ return (AnyPtr*)&null; }
};

template<>
struct CastHelper<const Any*>{
	static bool can_cast(const AnyPtr&){ return true; }
	static const Any* unchecked_cast(const AnyPtr& a){ return (Any*)&a; }
	static const Any* get_null(){ return (Any*)&null; }
};

template<>
struct CastHelper<const char_t*>{
	static bool can_cast(const AnyPtr& a){ return a->is(get_cpp_class<String>()); }
	static const char_t* unchecked_cast(const AnyPtr& a);
	static const Any* get_null(){ return 0; }
};

template<>
struct CastHelper<const IDPtr*>{
	static bool can_cast(const AnyPtr& a);
	static const IDPtr* unchecked_cast(const AnyPtr& a);
	static const IDPtr* get_null(){ return (IDPtr*)&null; }
};


template<>
struct CastHelper<const Int*>{
	static bool can_cast(const AnyPtr& a){ return type(a)==TYPE_INT; }
	static const Int* unchecked_cast(const AnyPtr& a){ return (const Int*)&a; }
	static const Int* get_null(){ return (Int*)&null; }
};

template<>
struct CastHelper<const Float*>{
	static bool can_cast(const AnyPtr& a){ return type(a)==TYPE_FLOAT; }
	static const Float* unchecked_cast(const AnyPtr& a){ return (const Float*)&a; }
	static const Float* get_null(){ return (Float*)&null; }
};

template<>
struct CastHelper<const Bool*>{
	static bool can_cast(const AnyPtr& a){ return type(a)==TYPE_TRUE || type(a)==TYPE_FALSE; }
	static const Bool* unchecked_cast(const AnyPtr& a){ return (const Bool*)&a; }
	static const Bool* get_null(){ return (Bool*)&null; }
};

template<>
struct CastHelper<int_t>{
	static bool can_cast(const AnyPtr& a);
	static int_t unchecked_cast(const AnyPtr& a);
	static int_t get_null(){ return 0; }
};

template<>
struct CastHelper<float_t>{
	static bool can_cast(const AnyPtr& a);
	static float_t unchecked_cast(const AnyPtr& a);
	static float_t get_null(){ return 0; }
};

template<>
struct CastHelper<bool>{
	static bool can_cast(const AnyPtr&){ return true; }
	static bool unchecked_cast(const AnyPtr& a){ return a; }
	static bool get_null(){ return false; }
};

#define XTAL_CAST_HELPER(Type, XType) \
template<> struct CastResult<const Type&>{ typedef Type type; };\
template<>\
struct CastHelper<const Type&>{\
	static bool can_cast(const AnyPtr& a){ return CastHelper<Type>::can_cast(a); }\
	static Type unchecked_cast(const AnyPtr& a){ return CastHelper<Type>::unchecked_cast(a); }\
	static Type get_null(){ return 0; }\
}

XTAL_CAST_HELPER(int_t, Int);
XTAL_CAST_HELPER(float_t, Float);
XTAL_CAST_HELPER(bool, Bool);

template<> struct CppClassSymbol<int_t> : public CppClassSymbol<Int>{};
template<> struct CppClassSymbol<float_t> : public CppClassSymbol<Float>{};
template<> struct CppClassSymbol<bool> : public CppClassSymbol<Bool>{};


#undef XTAL_CAST_HELPER

#define XTAL_CAST_HELPER(Type, Type2, XType) \
template<>\
struct CastHelper<avoid<Type>::type>{\
	static bool can_cast(const AnyPtr& a){ return CastHelper<Type2>::can_cast(a); }\
	static Type unchecked_cast(const AnyPtr& a){ return (Type)CastHelper<Type2>::unchecked_cast(a); }\
	static Type get_null(){ return 0; }\
};\
template<> struct CastResult<avoid<const Type&> >{ typedef Type type; };\
template<>\
struct CastHelper<const avoid<Type>::type&>{\
	static bool can_cast(const AnyPtr& a){ return CastHelper<Type>::can_cast(a); }\
	static Type unchecked_cast(const AnyPtr& a){ return CastHelper<Type>::unchecked_cast(a); }\
	static Type get_null(){ return 0; }\
};\
template<> struct CppClassSymbol<avoid<Type>::type> : public CppClassSymbol<XType>{}

XTAL_CAST_HELPER(int, int_t, Int);
XTAL_CAST_HELPER(unsigned int, int_t, Int);
XTAL_CAST_HELPER(long, int_t, Int);
XTAL_CAST_HELPER(unsigned long, int_t, Int);
XTAL_CAST_HELPER(short, int_t, Int);
XTAL_CAST_HELPER(unsigned short, int_t, Int);
XTAL_CAST_HELPER(char, int_t, Int);
XTAL_CAST_HELPER(signed char, int_t, Int);
XTAL_CAST_HELPER(unsigned char, int_t, Int);
XTAL_CAST_HELPER(float, float_t, Float);
XTAL_CAST_HELPER(double, float_t, Float);
XTAL_CAST_HELPER(long double, float_t, Float);

#undef XTAL_CAST_HELPER

//////////////////////////////////////////////////////////////////////////////

/**
* @brief T型に変換する。
*
* Tに変換できない場合、
* Tがポインタ型ならNULLを返す。
* Tが値か参照で、AnyPtrを継承した型ならxtal::nullを返す。
* それ以外の型の場合は未定義。
*/
template<class T> 
inline typename CastResult<T>::type 
as(const AnyPtr& a){
	return CastHelperHelper<T>::as(a);
}

/**
* @brief T型に変換する。
*
* T型に変換できない場合、builtin()->member("CastError")が投げられる
*/
template<class T>
inline typename CastResult<T>::type 
cast(const AnyPtr& a){
	return CastHelperHelper<T>::cast(a);
}

/**
* @brief T型に変換できるか調べる。
*
*/
template<class T>
inline bool can_cast(const AnyPtr& a){
	return CastHelperHelper<T>::can_cast(a);
}

/**
* @brief T型にチェック無しで変換する。
*
*/
template<class T>
inline typename CastResult<T>::type 
unchecked_cast(const AnyPtr& a){
	return CastHelperHelper<T>::unchecked_cast(a);
}

/**
* @brief SmartPtr<T>型に変換する。
*
* Tに変換できない場合xtal::nullを返す。
*/
template<class T> 
inline const SmartPtr<T>&
ptr_as(const AnyPtr& a){
	return CastHelperHelper<const SmartPtr<T>&>::as(a);
}

/**
* @brief SmartPtr<T>型に変換する。
*
* T型に変換できない場合、builtin()->member("CastError")が投げられる
*/
template<class T>
inline const SmartPtr<T>&
ptr_cast(const AnyPtr& a){
	return CastHelperHelper<const SmartPtr<T>&>::cast(a);
}

/**
* @brief SmartPtr<T>型に、実際の型がどうであるかを無視して強制変換する。
*/
template<class T>
inline const SmartPtr<T>&
unchecked_ptr_cast(const AnyPtr& a){
	return *(const SmartPtr<T>*)&a;
}
	
//////////////////////////////////////////

template<class T>
inline typename CastResult<T>::type 
tricky_cast(const AnyPtr& a, void (*f)(T)){
	return CastHelperHelper<T>::cast(a);
}

template<class T>
inline typename CastResult<T>::type 
tricky_as(const AnyPtr& a, void (*f)(T)){
	return CastHelperHelper<T>::as(a);
}

}
