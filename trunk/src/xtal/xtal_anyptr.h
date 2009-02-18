
#pragma once

namespace xtal{

template<uint_t Size>
struct UserTypeBuffer{
	union{
		u8 buf[Size];
		int_t dummy;
	};

	template<class T>
	void operator()(T* p){
		p->~T();
	}
};

template<class T>
struct UserTypeHolder : public Base{
	UserTypeHolder(){}
	UserTypeHolder(T* p):ptr(p){}
	T* ptr;
};

template<class T, class Deleter = UserTypeBuffer<sizeof(T)> >
struct UserTypeHolderSub : public UserTypeHolder<T>, public Deleter{
	UserTypeHolderSub(){}
	UserTypeHolderSub(T* p, const Deleter& f):UserTypeHolder<T>(p), Deleter(f){}
	virtual ~UserTypeHolderSub(){ Deleter::operator()(this->ptr); }
};

struct undeleter_t{
	template<class T>
	void operator()(T* p){}
};

struct deleter_t{
	template<class T>
	void operator()(T* p){
		delete p;
	}
};

extern undeleter_t undeleter;
extern deleter_t deleter;

enum InheritedEnum{
	INHERITED_BASE,
	INHERITED_RCBASE,
	INHERITED_ANY,
	INHERITED_OTHER
};

template<class T>
struct InheritedN{
	enum{
		value = 
			IsInherited<T, Base>::value ? INHERITED_BASE : 
			IsInherited<T, RefCountingBase>::value ? INHERITED_RCBASE : 
			IsInherited<T, Any>::value ? INHERITED_ANY : INHERITED_OTHER
	};
};

template<class T>
struct SmartPtrCtor1{
	struct type{};
	static int_t call(type){ return 0; };
};

template<class T>
struct SmartPtrCtor2{
	struct type{};
	static int_t call(type){ return 0; };
};

template<class T>
struct SmartPtrCtor3{
	struct type{};
	static int_t call(type){ return 0; };
};

template <int N>
struct SmartPtrSelector{};

/**
* @brief 何の型のオブジェクトでも保持する特殊化されたスマートポインタ
*/
template<>
class SmartPtr<Any> : public Any{
public:
	
	SmartPtr(){}

	template<class T, class Deleter>
	SmartPtr(T* p, const Deleter& deleter){
		UserTypeHolderSub<T, Deleter>* holder = new UserTypeHolderSub<T, Deleter>(p, deleter);
		set_p_with_class(holder, new_cpp_class<T>());
	}

	SmartPtr(const SmartPtr<Any>& p)
		:Any(p){
		inc_ref_count();
	}

	/// nullを受け取るコンストラクタ
	//SmartPtr(const Null&){}

	SmartPtr<Any>& operator =(const Null&);

	SmartPtr<Any>& operator =(const Undefined&);

	SmartPtr<Any>& operator =(const SmartPtr<Any>& p);

	explicit SmartPtr(PrimitiveType type)
		:Any(type){}
	
	explicit SmartPtr(Base* p)
		:Any(p){
		p->inc_ref_count();
	}

	~SmartPtr(){
		dec_ref_count();
		//*(Any*)this = Any();
	}

protected:

	struct special_ctor_t{};

	SmartPtr(const Any& a, special_ctor_t)
		:Any(a){}

	SmartPtr(RefCountingBase* p, int_t type, special_ctor_t);

	SmartPtr(Base* p, const ClassPtr& c, special_ctor_t);

	SmartPtr(Singleton* p, const ClassPtr& c, special_ctor_t);

	SmartPtr(CppSingleton* p, const ClassPtr& c, special_ctor_t);

	void inc_ref_count(){
		inc_ref_count_force(*this);
	}

	void dec_ref_count(){
		dec_ref_count_force(*this);
	}

public:

	/**
	* @brief 整数値から構築するコンストラクタ。
	*
	*/
	SmartPtr(int_t v):Any(v){}
	
	/**
	* @brief 浮動小数点数値から構築するコンストラクタ。
	*
	*/
	SmartPtr(float_t v):Any(v){}
	
	/**
	* @brief booleanから構築するコンストラクタ。
	*
	*/
	SmartPtr(bool v):Any(v){}

	/**
	* @brief 文字列から構築するコンストラクタ。
	*
	*/
	SmartPtr(const char_t* str);

	/**
	* @brief 文字列から構築するコンストラクタ。
	*
	*/
	SmartPtr(const avoid<char>::type* str);

	// 基本型の整数、浮動小数点数から構築するコンストラクタ
	SmartPtr(avoid<int>::type v):Any(v){}
	SmartPtr(avoid<long>::type v):Any(v){}
	SmartPtr(avoid<short>::type v):Any(v){}
	SmartPtr(avoid<char>::type v):Any(v){}
	SmartPtr(avoid<unsigned int>::type v):Any(v){}
	SmartPtr(avoid<unsigned long>::type v):Any(v){}
	SmartPtr(avoid<unsigned short>::type v):Any(v){}
	SmartPtr(avoid<unsigned char>::type v):Any(v){}
	SmartPtr(avoid<signed char>::type v):Any(v){}
	SmartPtr(avoid<float>::type v):Any(v){}
	SmartPtr(avoid<double>::type v):Any(v){}
	SmartPtr(avoid<long double>::type v):Any(v){}

public:

	/**
	* @brief ->演算子
	* スマートポインタとして扱うためにオーバーロードする。
	*/
	Any* operator ->() const{ return get(); }
	
	/**
	* @brief *演算子
	* スマートポインタとして扱うためにオーバーロードする。
	*/
	Any& operator *() const{ return *get(); }
	
	/**
	* @brief T型へのポインタを取得する。
	*/
	Any* get() const{ return (Any*)this; }

public:

	friend inline const AnyPtr& ap(const Any& v){
		return (const AnyPtr&)v;
	}

	friend inline SmartPtr<Any>& ap_copy(SmartPtr<Any>& a, const SmartPtr<Any>& b){
		a.dec_ref_count();
		*(Any*)&a = b;
		a.inc_ref_count();
		return a;
	}

public:

	struct dummy_bool_tag{ void safe_true(dummy_bool_tag){} };
	typedef void (dummy_bool_tag::*safe_bool)(dummy_bool_tag);

	operator safe_bool() const{
		return type((*this)->self())>TYPE_FALSE ? &dummy_bool_tag::safe_true : (safe_bool)0;
	}

	bool operator !() const{
		return type((*this)->self())<=TYPE_FALSE;
	}

private:

	/**
	* @brief 暗黙の変換を抑えるためのコンストラクタ。
	*
	* 得体の知れないポインタからの構築を拒否するため、このコンストラクタはprivateで実装も存在しない。
	*/
	SmartPtr(void*);

};


class Null : public Any{
public:
	template<class T>
	operator const SmartPtr<T>&() const{
		return *(SmartPtr<T>*)this;
	}

	template<class T>
	operator SmartPtr<T>() const{
		return *(SmartPtr<T>*)this;
	}
};

class Undefined : public AnyPtr{ public: Undefined():AnyPtr(TYPE_UNDEFINED){} };

inline bool operator ==(const AnyPtr& a, const Null&){ return raweq(a, null); }
inline bool operator !=(const AnyPtr& a, const Null&){ return rawne(a, null); }
inline bool operator ==(const Null&, const AnyPtr& a){ return raweq(a, null); }
inline bool operator !=(const Null&, const AnyPtr& a){ return rawne(a, null); }

inline bool operator ==(const AnyPtr& a, const Undefined&){ return raweq(a, undefined); }
inline bool operator !=(const AnyPtr& a, const Undefined&){ return rawne(a, undefined); }
inline bool operator ==(const Undefined&, const AnyPtr& a){ return raweq(a, undefined); }
inline bool operator !=(const Undefined&, const AnyPtr& a){ return rawne(a, undefined); }


}//namespace 
