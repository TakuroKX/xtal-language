#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>

#include "../src/xtal/xtal.h"
#include "../src/xtal/xtal_macro.h"

#include "../src/xtal/xtal_lib/xtal_winthread.h"
#include "../src/xtal/xtal_lib/xtal_cstdiostream.h"
#include "../src/xtal/xtal_lib/xtal_winfilesystem.h"
#include "../src/xtal/xtal_lib/xtal_chcode.h"
#include "../src/xtal/xtal_lib/xtal_errormessage.h"

#include "time.h"



#include <iostream>

class TestGetterSetterBind{
public:
    float x, y;
    TestGetterSetterBind(): x(0), y(0) {}

	void foomethod(const char* str){
	
	}
};

int WINAPI getset(TestGetterSetterBind* b){
	return b->x * 10;	
}

XTAL_PREBIND(TestGetterSetterBind){
	it->def_ctor(xtal::ctor<TestGetterSetterBind>());
}

XTAL_BIND(TestGetterSetterBind){
   it->def_getter(Xid(x), &TestGetterSetterBind::x);
   it->def_setter(Xid(set_x), &TestGetterSetterBind::x);

   it->def_var(Xid(y), &TestGetterSetterBind::y);
   it->def_method(Xid(foomethod), &TestGetterSetterBind::foomethod);
 
   it->def_method("getseta", &getset);
   Xdef_method_alias(getset, &getset);
}

struct MyData{
	int a;
};

struct MyDeleter{
	void destroy(MyData* p){
		delete p;
	}
};

XTAL_BIND(MyData){
	it->def_var(Xid(a), &MyData::a);
}


class Vector2D{
public:
    float x, y;
    
    Vector2D(float x = 0, float y = 0)
        :x(x), y(y){
		x = x;
	}
    
    float length() const{
        return sqrt(x*x + y*y);
    }
    
    void normalize(){
        float len = length();
        x /= len;
        y /= len;
    }
};

class AAA{
public:
	xtal::SmartPtr<Vector2D> test;

	AAA(){
		test = xtal::xnew<Vector2D>();
	}
};


// XTAL_PREBINDの中で継承関係の登録、コンストラクタの登録を行う
XTAL_PREBIND(Vector2D){
    // itはClassPtrである。
    // it->でClassクラスのメンバ関数が呼べる
    
    // コンストラクタの登録
    it->def_ctor2<Vector2D, float, float>()->param(1, Xid(x), 0)->param(2, Xid(y), 0);
}

// XTAL_BINDの中でメンバ関数の登録を行う
XTAL_BIND(Vector2D){
    // itはClassPtrである。
    // it->でClassクラスのメンバ関数が呼べる

    it->def_var(Xid(x), &Vector2D::x); // メンバ変数xのセッタ、ゲッタを登録
    it->def_var(Xid(y), &Vector2D::y); // メンバ変数yのセッタ、ゲッタを登録
    it->def_method(Xid(length), &Vector2D::length); // メンバ関数lengthを登録
    it->def_method(Xid(normalize), &Vector2D::normalize); // メンバ関数lengthを登録
}

// XTAL_PREBINDの中で継承関係の登録、コンストラクタの登録を行う
XTAL_PREBIND(AAA){
    // itはClassPtrである。
    // it->でClassクラスのメンバ関数が呼べる
    
    // コンストラクタの登録
    it->def_ctor0<AAA>();
}

// XTAL_BINDの中でメンバ関数の登録を行う
XTAL_BIND(AAA){
    // itはClassPtrである。
    // it->でClassクラスのメンバ関数が呼べる

    it->def_var(Xid(test), &AAA::test); // メンバ変数xのセッタ、ゲッタを登録
}

using namespace xtal;

void foofun(){}

void test(){
    lib()->def(Xid(TestGetterSetterBind), cpp_class<TestGetterSetterBind>());
	lib()->def(Xid(MyData), SmartPtr<MyData>(new MyData, MyDeleter()));

	if(CodePtr code = Xsrc((
		foo: lib::TestGetterSetterBind();
		foo.x = 0.5;
		assert math::abs(foo.x-0.5)<0.001;
		assert math::abs(foo.getset-5)<0.001;

		foo.y = 100.5;
		assert math::abs(foo.y-100.5)<0.001;

		foo.foomethod("test");

		mydata: lib::MyData;
		mydata.a = 10;
		assert mydata.a==10;

		a: cpp::AAA();
		a.test.y = 10;
		a.test.x = a.test.y;
		assert a.test.x==10;
	))){
		code->call();
	}

	XTAL_CATCH_EXCEPT(e){
		stderr_stream()->println(e);
	}
	int n;
	n = 0;
}

class BinaryType;
class BinaryRecordType;
class BinaryRecordData;
class BinaryArrayType;
class BinaryArrayData;
class BinaryOptionalType;

typedef SmartPtr<BinaryType> BinaryTypePtr;
typedef SmartPtr<BinaryRecordType> BinaryRecordTypePtr;
typedef SmartPtr<BinaryRecordData> BinaryRecordDataPtr;
typedef SmartPtr<BinaryArrayType> BinaryArrayTypePtr;
typedef SmartPtr<BinaryArrayData> BinaryArrayDataPtr;
typedef SmartPtr<BinaryOptionalType> BinaryOptionalTypePtr;

class BinaryType : public Base{
public:
	BinaryArrayTypePtr at(const AnyPtr& length);

	BinaryOptionalTypePtr optional(const AnyPtr& checker);

	virtual void on_write_to_stream(const StreamPtr& stream, const AnyPtr& data, const BinaryRecordDataPtr& parent_data) = 0;
	
	virtual AnyPtr on_read_from_stream(const StreamPtr& stream, const BinaryRecordDataPtr& parent_data) = 0;

	virtual AnyPtr make() = 0;

	virtual AnyPtr on_translate(const AnyPtr& data, const BinaryRecordDataPtr& parent_data) = 0;

	void write_to_stream(const StreamPtr& stream, const AnyPtr& data){
		return on_write_to_stream(stream, data, null);
	}
	
	AnyPtr read_from_stream(const StreamPtr& stream){
		return on_read_from_stream(stream, null);
	}

	AnyPtr translate(const AnyPtr& data){
		return on_translate(data, null);
	}
};

XTAL_PREBIND(BinaryType){
	
}

XTAL_BIND(BinaryType){
	it->def_method(Xid(op_at), &BinaryType::at, cpp_class<Any>());
	it->def_method(Xid(write_to_stream), &BinaryType::write_to_stream);
	it->def_method(Xid(read_from_stream), &BinaryType::read_from_stream);
	it->def_method(Xid(op_call), &BinaryType::make);
	it->def_method(Xid(translate), &BinaryType::translate);
	it->def_method(Xid(optional), &BinaryType::optional);
}

class BinaryArrayData : public Base{
	BinaryTypePtr element_type_;
	ArrayPtr data_;
public:
	
	BinaryArrayData(const BinaryTypePtr& element_type, int_t len = 0)
		:element_type_(element_type), data_(xnew<Array>(len)){}

	void fillup(int_t i){
		if(data_->length()<=i){
			data_->resize(i+1);
		}
	}
	
	AnyPtr at(int_t i){
		fillup(i);
		if(raweq(data_->at(i), undefined)){
			data_->set_at(i, element_type_->make());
		}
		return data_->at(i);
	}
	
	void set_at(int_t i, const AnyPtr& v){
		fillup(i);
		data_->set_at(i, v);
	}
};

XTAL_BIND(BinaryArrayData){
	it->def_method(Xid(op_at), &BinaryArrayData::at, cpp_class<Int>());
	it->def_method(Xid(op_set_at), &BinaryArrayData::set_at, cpp_class<Int>());
}

class BinaryArrayType : public BinaryType{
	BinaryTypePtr element_type_;
	AnyPtr length_;
public:
	
	BinaryArrayType(const BinaryTypePtr& element_type, const AnyPtr& length)
		:element_type_(element_type), length_(length){}
	
	int_t length(const BinaryRecordDataPtr& parent_data);

	void on_write_to_stream(const StreamPtr& stream, const AnyPtr& data, const BinaryRecordDataPtr& parent_data){
		if(BinaryArrayDataPtr d = ptr_cast<BinaryArrayData>(data)){
			int_t len = length(parent_data);
			for(int_t i=0; i<len; ++i){
				element_type_->on_write_to_stream(stream, d->at(i), parent_data);
			}
		}
	}
	
	AnyPtr on_read_from_stream(const StreamPtr& stream, const BinaryRecordDataPtr& parent_data){
		int_t len = length(parent_data);
		BinaryArrayDataPtr data = xnew<BinaryArrayData>(element_type_, len);
		for(int_t i=0; i<len; ++i){
			data->set_at(i, element_type_->on_read_from_stream(stream, parent_data));
		}
		return data;
	}
	
	AnyPtr make(){
		return xnew<BinaryArrayData>(element_type_);
	}

	AnyPtr on_translate(const AnyPtr& data, const BinaryRecordDataPtr& parent_data){
		if(BinaryArrayDataPtr d = ptr_cast<BinaryArrayData>(data)){
			int_t len = length(parent_data);
			ArrayPtr ret = xnew<Array>(len);
			for(int_t i=0; i<len; ++i){
				ret->set_at(i, element_type_->on_translate(d->at(i), parent_data));
			}
			return ret;
		}
		return undefined;
	}

};

XTAL_PREBIND(BinaryArrayType){
	it->inherit(cpp_class<BinaryType>());
}

class BinaryOptionalType : public BinaryType{
	BinaryTypePtr original_type_;
	AnyPtr checker_;
public:
	
	BinaryOptionalType(const BinaryTypePtr& original_type, const AnyPtr& checker)
		:original_type_(original_type), checker_(checker){}
	
	bool check(const BinaryRecordDataPtr& parent_data);

	void on_write_to_stream(const StreamPtr& stream, const AnyPtr& data, const BinaryRecordDataPtr& parent_data){
		if(check(parent_data)){
			original_type_->on_write_to_stream(stream, data, parent_data);
		}
	}
	
	AnyPtr on_read_from_stream(const StreamPtr& stream, const BinaryRecordDataPtr& parent_data){
		if(check(parent_data)){
			return original_type_->on_read_from_stream(stream, parent_data);
		}
		return undefined;
	}
	
	AnyPtr make(){
		return original_type_->make();
	}

	AnyPtr on_translate(const AnyPtr& data, const BinaryRecordDataPtr& parent_data){
		if(check(parent_data)){
			return original_type_->on_translate(data, parent_data);
		}
		return undefined;
	}

};

XTAL_PREBIND(BinaryOptionalType){
	it->inherit(cpp_class<BinaryType>());
}

class BinaryPrimitiveType : public BinaryType{
	int_t type_;
	AnyPtr value_;
public:
	enum{
		I8,
		I16,
		I32,

		U8,
		U16,
		U32,

		F32,
		F64,

		BE = 0,
		LE = 1
	};

	BinaryPrimitiveType(int_t type, int_t endian, const AnyPtr& value){
		type_ = endian | (type<<1);
		value_ = value;
	}

	void on_write_to_stream(const StreamPtr& stream, const AnyPtr& data, const BinaryRecordDataPtr& parent_data){
		switch(type_){
			XTAL_NODEFAULT;
			XTAL_CASE(BE | (I8<<1)){ stream->put_i8(data->to_i()); }
			XTAL_CASE(BE | (I16<<1)){ stream->put_i16be(data->to_i()); }
			XTAL_CASE(BE | (I32<<1)){ stream->put_i32be(data->to_i()); }

			XTAL_CASE(BE | (U8<<1)){ stream->put_u8(data->to_i()); }
			XTAL_CASE(BE | (U16<<1)){ stream->put_u16be(data->to_i()); }
			XTAL_CASE(BE | (U32<<1)){ stream->put_u32be(data->to_i()); }

			XTAL_CASE(BE | (F32<<1)){ stream->put_f32be(data->to_f()); }
			XTAL_CASE(BE | (F64<<1)){ stream->put_f64be(data->to_f()); }

			XTAL_CASE(LE | (I8<<1)){ stream->put_i8(data->to_i()); }
			XTAL_CASE(LE | (I16<<1)){ stream->put_i16le(data->to_i()); }
			XTAL_CASE(LE | (I32<<1)){ stream->put_i32le(data->to_i()); }

			XTAL_CASE(LE | (U8<<1)){ stream->put_u8(data->to_i()); }
			XTAL_CASE(LE | (U16<<1)){ stream->put_u16le(data->to_i()); }
			XTAL_CASE(LE | (U32<<1)){ stream->put_u32le(data->to_i()); }

			XTAL_CASE(LE | (F32<<1)){ stream->put_f32le(data->to_f()); }
			XTAL_CASE(LE | (F64<<1)){ stream->put_f64le(data->to_f()); }
		}
	}
	
	AnyPtr on_read_from_stream(const StreamPtr& stream, const BinaryRecordDataPtr& parent_data){
		switch(type_){
			XTAL_NODEFAULT;
			XTAL_CASE(BE | (I8<<1)){ return stream->get_i8(); }
			XTAL_CASE(BE | (I16<<1)){ return stream->get_i16be(); }
			XTAL_CASE(BE | (I32<<1)){ return stream->get_i32be(); }

			XTAL_CASE(BE | (U8<<1)){ return stream->get_u8(); }
			XTAL_CASE(BE | (U16<<1)){ return stream->get_u16be(); }
			XTAL_CASE(BE | (U32<<1)){ return stream->get_u32be(); }

			XTAL_CASE(BE | (F32<<1)){ return stream->get_f32be(); }
			XTAL_CASE(BE | (F64<<1)){ return stream->get_f64be(); }

			XTAL_CASE(LE | (I8<<1)){ return stream->get_i8(); }
			XTAL_CASE(LE | (I16<<1)){ return stream->get_i16le(); }
			XTAL_CASE(LE | (I32<<1)){ return stream->get_i32le(); }

			XTAL_CASE(LE | (U8<<1)){ return stream->get_u8(); }
			XTAL_CASE(LE | (U16<<1)){ return stream->get_u16le(); }
			XTAL_CASE(LE | (U32<<1)){ return stream->get_u32le(); }

			XTAL_CASE(LE | (F32<<1)){ return stream->get_f32le(); }
			XTAL_CASE(LE | (F64<<1)){ return stream->get_f64le(); }
		}
		return undefined;
	}
	
	AnyPtr make(){
		return value_;
	}

	AnyPtr on_translate(const AnyPtr& data, const BinaryRecordDataPtr& parent_data){
		return data;
	}
};

XTAL_PREBIND(BinaryPrimitiveType){
	it->inherit(cpp_class<BinaryType>());
	//it->def_ctor3<BinaryPrimitiveType>();
}

class BinaryRecordData : public Base{
	BinaryRecordTypePtr type_;
	ArrayPtr data_;
public:
	
	BinaryRecordData(const BinaryRecordTypePtr& type, int_t len = 0)
		:type_(type), data_(xnew<Array>(len)){}

	const BinaryRecordTypePtr& type(){
		return type_;
	}

	void fillup(int_t i){
		if(data_->length()<=i){
			data_->resize(i+1);
		}
	}
	
	AnyPtr at(int_t i, const BinaryTypePtr& type){
		fillup(i);
		if(raweq(data_->at(i), undefined)){
			data_->set_at(i, type->make());
		}
		return data_->at(i);
	}
	
	void set_at(int_t i, const AnyPtr& v){
		fillup(i);
		data_->set_at(i, v);
	}
};

XTAL_BIND(BinaryRecordData){

}

class StructMemberGetterAndSetter : public Base{
public:
	int_t index_;
	BinaryTypePtr type_;

	StructMemberGetterAndSetter(int_t index, const BinaryTypePtr& type)
		:index_(index), type_(type){}

	void on_rawcall(const VMachinePtr& vm){
		if(vm->ordered_arg_count()==0){
			vm->return_result(ptr_cast<BinaryRecordData>(vm->arg_this())->at(index_, type_));
			return;
		}
		else if(vm->ordered_arg_count()==1){
			ptr_cast<BinaryRecordData>(vm->arg_this())->set_at(index_, vm->arg(0));
			vm->return_result();
			return;
		}
		vm->return_result();
	}
};

class BinaryRecordType : public BinaryType{
public:
	MapPtr members_;
	MapPtr index_map_;
	ClassPtr cls_;
	
	BinaryRecordType(){
		cls_ = xnew<Class>();
		cls_->inherit(cpp_class<BinaryRecordData>());

		members_ = xnew<Map>();
		index_map_ = xnew<Map>();
	}

	AnyPtr members(){
		return members_;
	}
	
	void def(const IDPtr& key, const BinaryTypePtr& type){
		cls_->def(key, xnew<StructMemberGetterAndSetter>(members_->size(), type));
		cls_->def(Xid(set_)->op_cat(key), xnew<StructMemberGetterAndSetter>(members_->size(), type));
		index_map_->set_at(key, members_->size());
		members_->set_at(key, type);
	}

	AnyPtr access(const BinaryRecordDataPtr& data, const IDPtr& key){
		return data->at(index_map_->at(key)->to_i(), ptr_cast<BinaryType>(members_->at(key)));
	}

	void on_write_to_stream(const StreamPtr& stream, const AnyPtr& data, const BinaryRecordDataPtr& parent_data){
		if(BinaryRecordDataPtr d = ptr_cast<BinaryRecordData>(data)){
			int_t i = 0;
			Xfor2(key, type, members_){
				if(BinaryTypePtr p = ptr_cast<BinaryType>(type)){
					p->on_write_to_stream(stream, d->at(i, p), d);
				}
				++i;
			}
		}
	}
	
	AnyPtr on_read_from_stream(const StreamPtr& stream, const BinaryRecordDataPtr& parent_data){
		BinaryRecordDataPtr data = xnew<BinaryRecordData>(to_smartptr(this));
		data->set_class(cls_);
		int_t i = 0;
		Xfor2(key, type, members_){
			if(BinaryTypePtr p = ptr_cast<BinaryType>(type)){
				data->set_at(i, p->on_read_from_stream(stream, data));
			}
			++i;
		}
		return data;
	}
			
	AnyPtr make(){
		BinaryRecordDataPtr data = xnew<BinaryRecordData>(to_smartptr(this), members_->size());
		data->set_class(cls_);
		int_t i = 0;
		Xfor2(key, type, members_){
			if(BinaryTypePtr p = ptr_cast<BinaryType>(type)){
				data->set_at(i, p->make());
			}
			++i;
		}
		return data;
	}

	AnyPtr on_translate(const AnyPtr& data, const BinaryRecordDataPtr& parent_data){
		if(BinaryRecordDataPtr d = ptr_cast<BinaryRecordData>(data)){
			MapPtr ret = xnew<Map>();
			int_t i=0;
			Xfor2(key, type, members()){
				if(BinaryTypePtr p = ptr_cast<BinaryType>(type)){
					ret->set_at(key, p->on_translate(d->at(i, p), d));
				}
				++i;
			}
			return ret;
		}
		return undefined;
	}
};

BinaryArrayTypePtr BinaryType::at(const AnyPtr& length){
	return xnew<BinaryArrayType>(to_smartptr(this), length);
}

BinaryOptionalTypePtr BinaryType::optional(const AnyPtr& checker){
	return xnew<BinaryOptionalType>(to_smartptr(this), checker);
}

int_t BinaryArrayType::length(const BinaryRecordDataPtr& parent_data){
	if(type(length_)==TYPE_INT){
		return ivalue(length_);
	}
	else if(const StringPtr& key = ptr_cast<String>(length_)){
		return parent_data->type()->access(parent_data, key)->to_i();
	}
	else{
		return length_->call(parent_data)->to_i();
	}
	return 0;
}
	
bool BinaryOptionalType::check(const BinaryRecordDataPtr& parent_data){
	return checker_->call(parent_data);
}

XTAL_PREBIND(BinaryRecordType){
	it->inherit(cpp_class<BinaryType>());
	it->def_ctor0<BinaryRecordType>();
}

XTAL_BIND(BinaryRecordType){
	it->def_method(Xid(def), &BinaryRecordType::def);
	it->def_method(Xid(members), &BinaryRecordType::members);
	it->def_method(Xid(access), &BinaryRecordType::access);

	/*
		fun binary(...args){
			members: args.named_arguments[:];
			ret: cpp::BinaryRecordType();

			members{ |key, val|
				ret.def(key, val);
			}

			return ret;
		}

	*/
}

#include <xmmintrin.h>

struct Vec128{
	__m128 a;
};

struct Spr{
	Vec128 v;
};

XTAL_BIND(Spr){
	it->def_var("v", &Spr::v);
}

struct SLp : public Base{
	virtual void foo(){}
};



void aaa(float v, const ArgumentsPtr& a){
	a->p();
}

int main2(int argc, char** argv){
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | /*_CRTDBG_CHECK_ALWAYS_DF |*/ _CRTDBG_DELAY_FREE_MEM_DF);

	using namespace std;

	VMachine* vm = &*vmachine();

	SmartPtr<Spr> s = xnew<Spr>();
//	_mm_add_ps(s->v.a, s->v.a);

	//debug::enable();
	//debug::set_breakpoint_hook(fun(&linehook));

	//DebugConnector dc;
	//dc.connect("12321");

	/*{
		CodePtr code=require_source("test");

		while(true){
			dc.update();
			code->call();
			
			XTAL_CATCH_EXCEPT(e){
				e->p();
			}

			Sleep(15);
		}
	}*/

	//test2();
   // lib()->def(Xid(Vector2D), cpp_class<Vector2D>());

	//AnyPtr a = cast<bool>(false);

	//static_cmemfun_holder<StringPtr (String::*)() const, &String::clone>::call(vmachine(), 0, 0);

	{
		//XTAL_RETURN_TO_VM(StringPtr, vm, str());
		//foo(str(), str());
		//aaa(&n);
		//vm->return_result(str());
		//vm->return_result((
		//	unchecked_cast<String*>(empty_string)->*(&String::to_s))());
		//XTAL_RETURN_TO_VM(vmachine(), (unchecked_cast<String*>(empty_string)->*(&String::clone))(	
		//));
	}

	if(CodePtr code = Xsrc((
		arg[0](9);

	))){
		code->inspect()->p();
		//AnyPtr ret = code->call(xnew<Spr>());
		code->call(fun(&aaa));
	}

	XTAL_CATCH_EXCEPT(e){
		StringPtr str = e->to_s();
		const char_t* cstr = str->data();
		stderr_stream()->println(e);
		return 1;
	}

	full_gc();

	//compile_file("../bench/ao.xtal")->inspect()->p();
	
	if(0){
		int c = clock();
		load("../bench/ao.xtal");
		printf("ao %g\n\n", (clock()-c)/1000.0f);		
	}

	XTAL_CATCH_EXCEPT(e){
		stderr_stream()->println(e);
		return 1;
	}
//*/

#if 1

	int c;

	/*
	benchmark("../fannkuch.xtal", 12);
	benchmark("../n-body.xtal", 12);
	benchmark("../special-norm.xtal", 12);
	benchmark("../binary-trees.xtal", 3);
	benchmark("../mandelbrot.xtal", 10);
	//*/

	/*		
	c = clock();
	load("../bench/sum_fiber.xtal");
	printf("sum_fiber %g\n\n", (clock()-c)/1000.0f);	

	c = clock();
	load("../bench/sum_array.xtal");
	printf("sum_array %g\n\n", (clock()-c)/1000.0f);		

	c = clock();
	load("../bench/vec.xtal");
	printf("vec %g\n\n", (clock()-c)/1000.0f);		
	
	c = clock();
	load("../bench/inst.xtal");
	printf("inst %g\n\n", (clock()-c)/1000.0f);

	c = clock();
	load("../bench/gc.xtal");
	printf("gc %g\n\n", (clock()-c)/1000.0f);

	c = clock();
	load("../bench/loop.xtal");
	printf("loop %g\n\n", (clock()-c)/1000.0f);

	c = clock();
	load("../bench/nested_loops.xtal");
	printf("nested_loops %g\n\n", (clock()-c)/1000.0f);

	c = clock();
	load("../bench/fib.xtal");
	printf("fib %g\n\n", (clock()-c)/1000.0f);

	c = clock();
	load("../bench/loop_iter.xtal");
	printf("loop_iter %g\n\n", (clock()-c)/1000.0f);

	c = clock();
	load("../bench/array_for.xtal");
	printf("array_for %g\n\n", (clock()-c)/1000.0f);

	c = clock();
	load("../bench/array_each.xtal");
	printf("array_each %g\n\n", (clock()-c)/1000.0f);

	//*/

	//*

	//set_gc_stress(true);

#ifdef XTAL_USE_WCHAR
	CodePtr code = compile_file("../utf16le-test/test.xtal_");
#else
	CodePtr code = compile_file("../test/test.xtal_");
#endif

	XTAL_CATCH_EXCEPT(e){
		stderr_stream()->println(e);
		return 1;
	}


	code->call();

	XTAL_CATCH_EXCEPT(e){
		stderr_stream()->println(e);
		return 1;
	}

#ifdef XTAL_USE_WCHAR
	lib()->member("test")->send("run_dir", "../utf16le-test");
#else
	lib()->member("test")->send("run_dir", "../test");
#endif

	//lib()->member("test")->send("run_file", "../test/test_xpeg.xtal");
	//lib()->member("test")->send("print_result");
	//*/
#endif

	debug::enable();
	test();

	full_gc();

	XTAL_CATCH_EXCEPT(e){
		stderr_stream()->println(e);
		return 1;
	}

	return 0;
}


namespace xxx{

class MemoryManager{
	enum{
		USED = 1<<0,
		RED = 1<<1,
		FLAGS_MASK = RED | USED,
		
		MIN_ALIGNMENT = 8,

		GUARD_MAX = 32,
	};

	struct Chunk;

	// デバッグ情報はここに埋め込む
	struct DebugInfo{
		int line;
		const char* file;
		int size;
	};

	union PtrWithFlags{
		uint_t u;
		Chunk* p;
	};

	struct ChunkHeader{
#ifdef XTAL_DEBUG
		DebugInfo debug;
#endif
		Chunk* next;
		PtrWithFlags prev; // prevを指すポインタに、色と使用中かのフラグを埋め込む
	};

	struct ChunkBody{
		Chunk* left;
		Chunk* right;
	};

	struct Chunk{
		ChunkHeader h;
		ChunkBody b;

		void init(){ 
			h.next = h.prev.p = b.left = b.right = 0; 
		}

		int size(){ return (unsigned char*)h.next - (unsigned char*)buf(); }
		void* buf(){ return (unsigned char*)(&h+1); }

		Chunk* next(){
			return h.next; 
		}

		Chunk* prev(){ 
			PtrWithFlags u = h.prev;
			u.u &= ~FLAGS_MASK;
			return u.p; 
		}

		void set_next(Chunk* p){ 
			h.next = p; 
		}

		void set_prev(Chunk* p){
			PtrWithFlags u;
			u.p = p;
			u.u |= h.prev.u & FLAGS_MASK;
			h.prev = u; 
		}

		void ref(){			
			h.prev.u |= USED;
		}

		void unref(){
			h.prev.u &= ~USED;
		}

		int is_used(){			
			return h.prev.u & USED;
		}

		int is_red(){ 
			return h.prev.u & RED;
		}

		void set_red(){ 
			h.prev.u |= RED;
		}

		void set_black(){
			h.prev.u &= ~RED;
		}

		void flip_color(){ 
			h.prev.u ^= RED;
		}

		void set_same_color(Chunk* a){ 
			set_black(); 
			h.prev.u |= a->h.prev.u&RED; 
		}
	};

public:


	MemoryManager(){ 
		head_ = begin_ = end_ = 0; 
	}
	
	MemoryManager(void* buffer, size_t size){
		init(buffer, size);
	}

	void init(void* buffer, size_t size);

	/**
	* \brief メモリ確保
	*/
	void* malloc(size_t size, int alignment = sizeof(int_t), const char* file = "", int line = 0);

	/**
	* \brief メモリ解放
	*/
	void free(void* p);

private:

	Chunk* chunk_align(Chunk* it, int alignment);

	void* malloc_inner(size_t size, int alignment = MIN_ALIGNMENT, const char* file = "", int line = 0);

	void free_inner(void* p);

	Chunk* begin(){ return begin_; }

	Chunk* end(){ return end_; }

	Chunk* to_chunk(void* p){ 
		return (Chunk*)((ChunkHeader*)p - 1);
	}
	
private:

	int compare(Chunk* a, Chunk* b){
		if(int ret = a->size() - b->size()){
			return ret;
		}

		return a - b;
	}

	Chunk* find(Chunk* l, int key);

	Chunk* find(int key){
		return find(root_, key);
	}

	void flip_colors(Chunk* n);

	Chunk* rotate_left(Chunk* n);

	Chunk* rotate_right(Chunk* n);

	Chunk* fixup(Chunk* n);

	Chunk* insert(Chunk* n, Chunk* key);

	void insert(Chunk* key);

	Chunk* minv(Chunk* n);

	Chunk* move_red_left(Chunk* n);

	Chunk* move_red_right(Chunk* n);

	Chunk* remove_min(Chunk* n);

	Chunk* remove(Chunk* n, Chunk* key);

	void remove(Chunk* key){
		root_ = remove(root_, key);
		root_->set_black();
	}

public:

	enum{
		COLOR_FREE,
		COLOR_USED
	};

	void dump(unsigned char* dest, size_t size, unsigned char* marks);

	void dump(unsigned char* dest, size_t size);

private:
	Chunk* head_;
	Chunk* begin_;
	Chunk* end_;
	Chunk* root_;

	size_t buffer_size_;
};

void MemoryManager::init(void* buffer, size_t size){
	buffer_size_ = size;

	head_ = (Chunk*)align_p(buffer, MIN_ALIGNMENT);
	begin_ = head_+1;
	end_ = (Chunk*)align_p((Chunk*)((char*)buffer+size)-2, MIN_ALIGNMENT);
	
	head_->init();
	head_->set_next(begin_);
	head_->set_prev(head_);
	head_->set_black();
	head_->ref();
	
	begin_->init();
	begin_->set_next(end_);
	begin_->set_prev(head_);
	begin_->set_red();
	
	end_->init();
	end_->set_next(end_);
	end_->set_prev(begin_);
	end_->set_black();
	end_->ref();
	end_->b.left = end();
	end_->b.right = end();

	begin_ = chunk_align(begin_, MIN_ALIGNMENT);
	
	root_ = begin_;
	root_->b.left = end();
	root_->b.right = end();
	root_->set_red();
}

void* MemoryManager::malloc(size_t size, int alignment, const char* file, int line){
#ifdef XTAL_DEBUG
	if(void* p = malloc_inner(size+GUARD_MAX, alignment, file, line)){
		Chunk* cp = to_chunk(p);
		cp->h.debug.size = size;
		cp->h.debug.file = file;
		cp->h.debug.line = line;
		memset((unsigned char*)p+size, 0xcc, GUARD_MAX);
		memset(p, 0xdd, size);
		return p;
	}
	return 0;
#else
	return malloc_inner(size, alignment, file, line);
#endif
}

void MemoryManager::free(void* p){
#ifdef XTAL_DEBUG
	Chunk* cp = to_chunk(p);
	unsigned char* ucp = (unsigned char*)p+cp->h.debug.size;
	for(int i=0; i<GUARD_MAX; ++i){
		int cc = *((unsigned char*)p+cp->h.debug.size+i);
		XTAL_ASSERT(*((unsigned char*)p+cp->h.debug.size+i)==0xcc);
	}

	free_inner(p);
#else
	free_inner(p);
#endif
}

MemoryManager::Chunk* MemoryManager::chunk_align(Chunk* it, int alignment){
	// ユーザーへ返すメモリの先頭アドレスを計算
	void* p = align_p(it->buf(), alignment);

	// アライメント調整のため、ノードの再設定
	if(it!=to_chunk(p)){
		Chunk temp = *it;
		it = to_chunk(p);
		*it = temp;
		it->prev()->set_next(it);
		it->next()->set_prev(it);
	}

	return it;
}

/**
* \brief メモリ確保
*/
void* MemoryManager::malloc_inner(size_t size, int alignment, const char* file, int line){
	if(alignment>MIN_ALIGNMENT){
		alignment = align_2(alignment);
	}
	else{
		alignment = MIN_ALIGNMENT;
	}

	if(size<alignment){
		size = alignment;
	}
	else{
		size = align(size, alignment);
	}

	size_t find_size = size + (alignment-MIN_ALIGNMENT);

	// もっとも要求サイズに近いノードを探す
	Chunk* it = find(find_size);
	if(it!=end()){
		// 赤黒木から外す
		remove(it);
		it->ref();

		// it->buf()のアドレスがアラインしているように調整
		it = chunk_align(it, alignment);

		Chunk* newchunk = (Chunk*)((unsigned char*)it->buf()+size);
		newchunk = to_chunk(align_p(newchunk->buf(), MIN_ALIGNMENT));

		if(it->next()-1>=newchunk){
			newchunk->init();
			it->next()->set_prev(newchunk);
			newchunk->set_next(it->next());
			it->set_next(newchunk);
			newchunk->set_prev(it);

			insert(newchunk);
		}

		return it->buf();
	}
		
	return 0;
}

void MemoryManager::free_inner(void* p){
	if(p){
		Chunk* it = to_chunk(p);
		it->unref();

		if(!it->prev()->is_used()){
			remove(it->prev());
			it->prev()->set_next(it->next());
			it->next()->set_prev(it->prev());
			it = it->prev();
		}
	
		if(!it->next()->is_used()){
			remove(it->next());
			it->next()->next()->set_prev(it);
			it->set_next(it->next()->next());
		}

		insert(it);
	}
}

MemoryManager::Chunk* MemoryManager::find(Chunk* l, int key){
	Chunk* ret = end();
	while(l!=end()){
		int cmp = key - l->size();

		// 探している大きさより同じか大きいノードを発見
		if(cmp<=0){
			// とりあえず当てはまる大きさのノードは見つけたので保存する
			ret = l; 

			// 基本的に一番左のノードを優先する
			// 左にいくほど、サイズが小さく、小さいアドレスのノードであるため
			l = l->b.left;
		}
		// 探している大きさより小さいノードだった
		else{
			// 右のノードを検索しなくてももう既に当てはまる大きさのノードは見つかっている
			if(ret!=end()){
				return ret;
			}

			l = l->b.right;
		}
	}

	return ret;
}

void MemoryManager::flip_colors(Chunk* n){
	n->flip_color();
	n->b.left->flip_color();
	n->b.right->flip_color();
}

MemoryManager::Chunk* MemoryManager::rotate_left(Chunk* n){
	Chunk* x = n->b.right;
	n->b.right = x->b.left;
	x->b.left = n;
	x->set_same_color(n);
	n->set_red();
	return x;
}

MemoryManager::Chunk* MemoryManager::rotate_right(Chunk* n){
	Chunk* x = n->b.left;
	n->b.left = x->b.right;
	x->b.right = n;
	x->set_same_color(n);
	n->set_red();
	return x;
}

MemoryManager::Chunk* MemoryManager::fixup(Chunk* n){
	if(n->b.right->is_red()){
		n = rotate_left(n);
	}
	
	if(n->b.left->is_red() && n->b.left->b.left->is_red()){
		n = rotate_right(n);
	}
	
	if(n->b.left->is_red() && n->b.right->is_red()){
		flip_colors(n);
	}

	return n;
}

MemoryManager::Chunk* MemoryManager::insert(Chunk* n, Chunk* key){
	if(n==end()){
		return key;
	}

	int cmp = compare(key, n);
	
	if(cmp<0){
		n->b.left = insert(n->b.left, key);
	}
	else{
		n->b.right = insert(n->b.right, key);
	}
	
	if(n->b.right->is_red() && !n->b.left->is_red()){
		n = rotate_left(n);
	}
	
	if(n->b.left->is_red() && n->b.left->b.left->is_red()){
		n = rotate_right(n);
	}
	
	if(n->b.left->is_red() && n->b.right->is_red()){
		flip_colors(n);
	}
	
	return n;
}

void MemoryManager::insert(Chunk* key){
	key->b.right = key->b.left = end();
	key->set_red();

	root_ = insert(root_, key);
	root_->set_black();
}

MemoryManager::Chunk* MemoryManager::minv(Chunk* n){
	while(n->b.left!=end()){
		n = n->b.left;
	}
	return n;
}

MemoryManager::Chunk* MemoryManager::move_red_left(Chunk* n){
	flip_colors(n);
	if(n->b.right->b.left->is_red()){
		n->b.right = rotate_right(n->b.right);
		n = rotate_left(n);
		flip_colors(n);
	}
	return n;
}

MemoryManager::Chunk* MemoryManager::move_red_right(Chunk* n){
	flip_colors(n);
	
	if(n->b.left->b.left->is_red()){
		n = rotate_right(n);
		flip_colors(n);
	}
	return n;
}

MemoryManager::Chunk* MemoryManager::remove_min(Chunk* n){
	if(n->b.left==end()){
		return end();
	}
	
	if(!n->b.left->is_red() && !n->b.left->b.left->is_red()){
		n = move_red_left(n);
	}

	n->b.left = remove_min(n->b.left);
	return fixup(n);
}

MemoryManager::Chunk* MemoryManager::remove(Chunk* n, Chunk* key){
	if(compare(key, n) < 0){
		if(!n->b.left->is_red() && !n->b.left->b.left->is_red()){
			n = move_red_left(n);
		}
		
		n->b.left = remove(n->b.left, key);
	}
	else{
		if(n->b.left->is_red()){
			n = rotate_right(n);
		}
		
		if(n==key && n->b.right==end()){
			return end();
		}
		
		if(!n->b.right->is_red() && !n->b.right->b.left->is_red()){
			n = move_red_right(n);
		}
		
		if(n==key){
			Chunk* x = minv(n->b.right);
			x->b.right = remove_min(n->b.right);
			x->b.left = n->b.left;
			x->set_same_color(n);
			n = x;
		}
		else{
			n->b.right = remove(n->b.right, key);
		}
	}

	return fixup(n);
}

void MemoryManager::dump(unsigned char* dest, size_t size, unsigned char* marks){
	Chunk* p = head_;
	int_t* it = (int_t*)head_;

	size -= 1;

	size_t n = 0;
	while(p!=end()){
		double sz = p->size()*size/(double)buffer_size_;
		size_t szm = (size_t)sz+1;
		if(szm!=0){
			memset(dest+(size_t)n, marks[!p->is_used()], szm);
		}

		n += sz;
		p = p->next();
	}

	size_t m = (size_t)n;
	memset(dest+m, 0, (size+1)-m);
}

void MemoryManager::dump(unsigned char* dest, size_t size){
	unsigned char marks[] = {'O', 'X'};
	dump(dest, size, marks);
}

}

char memory[1024*1024*200];
xxx::MemoryManager smm(memory, 1024*1024*200);

class AAllocatorLib : public AllocatorLib{
public:
	virtual ~AAllocatorLib(){}
	virtual void* malloc(std::size_t size){ return smm.malloc(size); }
	virtual void free(void* p, std::size_t size){ smm.free(p); }
	virtual void* out_of_memory(std::size_t size){ return 0; }
};

#include <dbghelp.h>
#include <tlhelp32.h>
#pragma comment(lib, "Dbghelp.lib") 

// 例外発生時に関数の呼び出し履歴を表示する、例外フィルタ関数
LONG CALLBACK print_stacktrace(EXCEPTION_POINTERS *ExInfo){
	STACKFRAME sf;
	BOOL bResult;
	PIMAGEHLP_SYMBOL pSym;
	DWORD Disp;
	std::string buffer;

	//シンボル情報格納用バッファの初期化
	pSym = (PIMAGEHLP_SYMBOL)GlobalAlloc(GMEM_FIXED, 10000);
	pSym->SizeOfStruct = 10000;
	pSym->MaxNameLength = 10000 - sizeof(IMAGEHLP_SYMBOL);

	//スタックフレームの初期化
	ZeroMemory(&sf, sizeof(sf));
	sf.AddrPC.Offset = ExInfo->ContextRecord->Eip;
	sf.AddrStack.Offset = ExInfo->ContextRecord->Esp;
	sf.AddrFrame.Offset = ExInfo->ContextRecord->Ebp;
	sf.AddrPC.Mode = AddrModeFlat;
	sf.AddrStack.Mode = AddrModeFlat;
	sf.AddrFrame.Mode = AddrModeFlat;

	//シンボルハンドラの初期化
	SymInitialize(GetCurrentProcess(), NULL, TRUE);

	//スタックフレームを順に表示していく
	for(;;) {
		//次のスタックフレームの取得
		bResult = StackWalk(
			IMAGE_FILE_MACHINE_I386,
			GetCurrentProcess(),
			GetCurrentThread(),
			&sf,
			NULL, 
			NULL,
			SymFunctionTableAccess,
			SymGetModuleBase,
			NULL);

		//失敗ならば、ループを抜ける
		if(!bResult || sf.AddrFrame.Offset == 0) break;

		//プログラムカウンタから関数名を取得
		bResult = SymGetSymFromAddr(GetCurrentProcess(), sf.AddrPC.Offset, &Disp, pSym);
		
		if(bResult) printf("0x%08x %s() + 0x%x\n", sf.AddrPC.Offset, pSym->Name, Disp);
		else printf("%08x, ---", sf.AddrPC.Offset);
	}

	//後処理 
	SymCleanup(GetCurrentProcess());
	GlobalFree(pSym);

	return(EXCEPTION_EXECUTE_HANDLER);
}

struct TestA{
	int n;
};

struct TestB : TestA{
	int nn;
	virtual void a(){}
};

int main(int argc, char** argv){
	TestB aa;
	TestA* pb = &aa;


	SetUnhandledExceptionFilter(&print_stacktrace);

	CStdioStdStreamLib cstd_std_stream_lib;
	WinThreadLib win_thread_lib;
	WinFilesystemLib win_filesystem_lib;
	SJISChCodeLib sjis_ch_code_lib;
	AAllocatorLib alloc_lib;

	Setting setting;
	setting.thread_lib = &win_thread_lib;
	setting.std_stream_lib = &cstd_std_stream_lib;
	setting.filesystem_lib = &win_filesystem_lib;
	setting.ch_code_lib = &sjis_ch_code_lib;
	//setting.allocator_lib = &alloc_lib;

	initialize(setting);

	int ret = 1;
	XTAL_PROTECT{
		bind_error_message();

		ret = main2(argc, argv);

		vmachine()->print_info();
	}
	XTAL_OUT_OF_MEMORY{
		puts("out of memory");
	}
		
	uninitialize();

	return ret;
}
