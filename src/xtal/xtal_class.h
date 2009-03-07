
#pragma once

namespace xtal{

class InstanceVariables{
public:

	struct uninit_t{};

	InstanceVariables(uninit_t){}

	InstanceVariables();
			
	~InstanceVariables();
		
	void init_variables(ClassInfo* class_info);

	const AnyPtr& variable(int_t index, ClassInfo* class_info){
		int_t i = find_class_info(class_info) + index;
		if(i<0){ return undefined; }
		return variables_.at(i);
	}

	void set_variable(int_t index, ClassInfo* class_info, const AnyPtr& value){
		int_t i = find_class_info(class_info) + index;
		if(i<0){ return; }
		variables_.set_at(i, value);
	}

	int_t find_class_info(ClassInfo* class_info){
		VariablesInfo& info = variables_info_.top();
		if(info.class_info == class_info){
			return info.pos;
		}
		return find_class_info_inner(class_info);
	}

	bool is_included(ClassInfo* class_info);

	int_t find_class_info_inner(ClassInfo* class_info);

	bool empty(){
		return variables_.empty();
	}

	void visit_members(Visitor& m){
		for(uint_t i=0; i<variables_.size(); ++i){
			m & variables_.at(i);
		}
	}

protected:
	
	struct VariablesInfo{
		ClassInfo* class_info;
		int_t pos;
	};

	PODStack<VariablesInfo> variables_info_;
	Array variables_;
};

class EmptyInstanceVariables : public InstanceVariables{
public:

	EmptyInstanceVariables();

	~EmptyInstanceVariables();

	static VariablesInfo vi;
};
	
class Class : public Frame{
public:

	Class(const StringPtr& name = empty_string);

	Class(const FramePtr& outer, const CodePtr& code, ClassInfo* info);

	void overwrite(const ClassPtr& p);

public:

	/**
	* @brief 新しいメンバを定義する
	*
	* @param name 新しく定義するメンバの名前
	*/
	virtual void def(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key = null, int_t accessibility = KIND_PUBLIC);

	/**
	* @brief メンバを取り出す
	*
	* @param name 取り出したいメンバの名前
	*/
	virtual const AnyPtr& do_member(const IDPtr& primary_key, const AnyPtr& secondary_key, bool inherited_too, int_t& accessibility, bool& nocache);

	/**
	* @brief メンバを再設定する
	* Xtalレベルでは禁止されている操作だが、C++レベルでは可能にしておく
	*
	* @param name 再設定したいメンバの名前
	*/
	void set_member(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key);

	/**
	* @brief 継承
	*
	* @param cls 継承するクラスオブジェクト
	*/
	void inherit(const ClassPtr& cls);

	/**
	* @brief 継承する
	*
	* @param cls 継承するクラスオブジェクト
	*/
	void inherit_first(const ClassPtr& cls);

	/**
	* @brief 継承する
	*
	* @param cls 継承するクラスオブジェクト
	*/
	void inherit_strict(const ClassPtr& cls);

	/**
	* @brief Mix-inされているか調べる
	*
	* @param md Mix-inされている調べたいクラスオブジェクト
	*/
	bool is_inherited(const AnyPtr& md);

	/**
	* @brief C++のクラスがMix-inされているか調べる
	*
	*/
	bool is_inherited_cpp_class();

	/**
	* @brief Mix-inされているクラスのIteratorを返す
	*
	*/
	AnyPtr inherited_classes();

	/**
	* @brief 近い名前のメンバを検索する
	*/
	IDPtr find_near_member(const IDPtr& primary_key, const AnyPtr& secondary_key = null);
	
	AnyPtr ancestors();

public:
	/**
	* @brief 関数を定義する
	*
	* cls->def_fun("name", &foo); は cls->def("name", xtal::fun(&foo)); と同一
	*/
	template<class TFun>
	const NativeFunPtr& def_fun(const IDPtr& primary_key, const TFun& f, const AnyPtr& secondary_key = null, int_t accessibility = KIND_PUBLIC){
		typedef cfun_holder<TFun> fun_t;
		fun_t fun(f);
		return def_and_return(primary_key, secondary_key, accessibility, fun_param_holder<fun_t>::value, &fun, sizeof(fun));
	}

	/**
	* @brief メソッドを定義する
	*
	* cls->def_method("name", &Klass::foo); は cls->def("name", xtal::method(&Klass::foo)); と同一
	*/
	template<class TFun>
	const NativeFunPtr& def_method(const IDPtr& primary_key, const TFun& f, const AnyPtr& secondary_key = null, int_t accessibility = KIND_PUBLIC){
		typedef cmemfun_holder<TFun> fun_t;
		fun_t fun(f);
		return def_and_return(primary_key, secondary_key, accessibility, fun_param_holder<fun_t>::value, &fun, sizeof(fun));
	}

	/**
	* @brief メンバ変数へのポインタからゲッターを生成し、定義する
	*
	*/
	template<class T, class C>
	const NativeFunPtr& def_getter(const IDPtr& primary_key, T C::* v, const AnyPtr& secondary_key = null, int_t accessibility = KIND_PUBLIC){
		typedef getter_holder<C, T> fun_t;
		fun_t fun(v);
		return def_and_return(primary_key, secondary_key, accessibility, fun_param_holder<fun_t>::value, &fun, sizeof(fun));
	}
	
	/**
	* @brief メンバ変数へのポインタからセッターを生成し、定義する
	*
	* Xtalでは、obj.name = 10; とするにはset_nameとset_を前置したメソッドを定義する必要があるため、
	* 単純なセッターを定義したい場合、set_xxxとすることを忘れないこと。
	*/
	template<class T, class C>
	const NativeFunPtr& def_setter(const IDPtr& primary_key, T C::* v, const AnyPtr& secondary_key = null, int_t accessibility = KIND_PUBLIC){
		typedef setter_holder<C, T> fun_t;
		fun_t fun(v);
		return def_and_return(primary_key, secondary_key, accessibility, fun_param_holder<fun_t>::value, &fun, sizeof(fun));
	}
	
	/**
	* @brief メンバ変数へのポインタからゲッター、セッターを両方生成し、定義する
	*
	* cls->def_getter(primary_key, var, policy);
	* cls->def_setter(StringPtr("set_")->cat(primary_key), v, policy);
	* と等しい	
	*/	
	template<class T, class U>
	void def_var(const IDPtr& primary_key, T U::* v, const AnyPtr& secondary_key = null, int_t accessibility = KIND_PUBLIC){
		def_getter(primary_key, v, secondary_key, accessibility);
		def_setter(String("set_").cat(primary_key), v, secondary_key, accessibility);
	}

	/**
	* @brief 2重ディスパッチメソッドを定義する。
	*/
	void def_double_dispatch_method(const IDPtr& primary_key, int_t accessibility = KIND_PUBLIC);

	/**
	* @brief 2重ディスパッチ関数を定義する。
	*/
	void def_double_dispatch_fun(const IDPtr& primary_key, int_t accessibility = KIND_PUBLIC);


public:

	/**
	* @brief メソッドを定義する
	*
	*/
	template<class TFun>
	const NativeFunPtr& def_singleton_method(const IDPtr& primary_key, const TFun& f, const AnyPtr& secondary_key = null, int_t accessibility = KIND_PUBLIC){
		typedef cmemfun_holder<TFun> fun_t;
		fun_t fun(f);
		return def_and_return_bind_this(primary_key, secondary_key, accessibility, fun_param_holder<fun_t>::value, &fun, sizeof(fun));
	}

	/**
	* @brief メンバ変数へのポインタからゲッターを生成し、定義する
	*
	*/
	template<class T, class C>
	const NativeFunPtr& def_singleton_getter(const IDPtr& primary_key, T C::* v, const AnyPtr& secondary_key = null, int_t accessibility = KIND_PUBLIC){
		typedef getter_holder<C, T> fun_t;
		fun_t fun(v);
		return def_and_return_bind_this(primary_key, secondary_key, accessibility, fun_param_holder<fun_t>::value, &fun, sizeof(fun));
	}
	
	/**
	* @brief メンバ変数へのポインタからセッターを生成し、定義する
	*
	* Xtalでは、obj.name = 10; とするにはset_nameとset_を前置したメソッドを定義する必要があるため、
	* 単純なセッターを定義したい場合、set_xxxとすることを忘れないこと。
	*/
	template<class T, class C>
	const NativeFunPtr& def_singleton_setter(const IDPtr& primary_key, T C::* v, const AnyPtr& secondary_key = null, int_t accessibility = KIND_PUBLIC){
		typedef setter_holder<C, T> fun_t;
		fun_t fun(v);
		return def_and_return_bind_this(primary_key, secondary_key, accessibility, fun_param_holder<fun_t>::value, &fun, sizeof(fun));
	}
	
	/**
	* @brief メンバ変数へのポインタからゲッター、セッターを両方生成し、定義する
	*
	* cls->def_getter(primary_key, var, policy);
	* cls->def_setter(StringPtr("set_")->cat(primary_key), v, policy);
	* と等しい	
	*/	
	template<class T, class U>
	void def_singleton_var(const IDPtr& primary_key, T U::* v, const AnyPtr& secondary_key = null, int_t accessibility = KIND_PUBLIC){
		def_singleton_getter(primary_key, v, secondary_key, accessibility);
		def_singleton_setter(String("set_").cat(primary_key), v, secondary_key, accessibility);
	}

public:
	struct cpp_class_t{};

	Class(cpp_class_t, const StringPtr& name = empty_string);

	virtual void rawcall(const VMachinePtr& vm);
	
	virtual void s_new(const VMachinePtr& vm);

	void init_instance(const AnyPtr& self, const VMachinePtr& vm);
	
	void set_member_direct(int_t i, const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility);

	const AnyPtr& any_member(const IDPtr& primary_key, const AnyPtr& secondary_key);
	
	const AnyPtr& bases_member(const IDPtr& primary_key);

	const AnyPtr& find_member(const IDPtr& primary_key, const AnyPtr& secondary_key, bool inherited_too, int_t& accessibility, bool& nocache);

	MultiValuePtr child_object_name(const AnyPtr& a);

	void set_object_name(const StringPtr& name);

	StringPtr object_name();

	void set_object_parent(const ClassPtr& parent);

	uint_t object_force(){
		return object_force_;
	}

	void set_object_force(uint_t force){
		object_force_ = (u16)force;
	}

	ClassInfo* info(){
		return (ClassInfo*)scope_info_;
	}
	
	bool is_native(){
		return is_native_!=0;
	}

	bool is_final(){
		return is_final_!=0;
	}

	void set_final(){
		is_final_ = true;
	}

protected:

	const NativeFunPtr& def_and_return(const IDPtr& primary_key, const AnyPtr& secondary_key, int_t accessibility, const param_types_holder_n& pth, const void* val, int_t val_size);

	const NativeFunPtr& def_and_return_bind_this(const IDPtr& primary_key, const AnyPtr& secondary_key, int_t accessibility, const param_types_holder_n& pth, const void* val, int_t val_size);
	
	const AnyPtr& def2(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key = null, int_t accessibility = KIND_PUBLIC);

	StringPtr name_;
	ArrayPtr mixins_;
	u16 object_force_;
	u8 is_native_;
	u8 is_final_;

	virtual void visit_members(Visitor& m){
		Frame::visit_members(m);
		m & mixins_;
	}

};

class CppClass : public Class{
public:
		
	CppClass(const StringPtr& name = empty_string);

public:

	virtual void rawcall(const VMachinePtr& vm);

	virtual void s_new(const VMachinePtr& vm);
};

class Singleton : public Class{
public:

	Singleton(const StringPtr& name = empty_string);

	Singleton(const FramePtr& outer, const CodePtr& code, ClassInfo* info);
	
public:

	void init_singleton(const VMachinePtr& vm);

	// クラスの設定はスルーする
	void set_class(const ClassPtr&){}

	virtual void rawcall(const VMachinePtr& vm);
	
	virtual void s_new(const VMachinePtr& vm);
};

class CppSingleton : public Class{
public:
		
	CppSingleton(const StringPtr& name = empty_string);

public:

	// クラスの設定はスルーする
	void set_class(const ClassPtr&){}

};

}
