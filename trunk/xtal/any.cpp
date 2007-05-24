
#include <vector>
#include <iostream>
#include <fstream>

#include "xtal.h"

#include "fwd.h"
#include "any.h"
#include "frame.h"
#include "fun.h"
#include "utilimpl.h"
#include "stringimpl.h"
#include "codeimpl.h"
#include "lexer.h"
#include "visitor.h"
#include "frameimpl.h"
#include "xmacro.h"
#include "stream.h"

namespace xtal{

void InitAny();

Null null;

Any operator +(const Any& a, const Any& b){ return a.send(Xid(op_add), b); }
Any operator -(const Any& a, const Any& b){ return a.send(Xid(op_sub), b); }
Any operator *(const Any& a, const Any& b){ return a.send(Xid(op_mul), b); }
Any operator /(const Any& a, const Any& b){ return a.send(Xid(op_div), b); }
Any operator %(const Any& a, const Any& b){ return a.send(Xid(op_mod), b); }
Any operator |(const Any& a, const Any& b){ return a.send(Xid(op_or), b); }
Any operator &(const Any& a, const Any& b){ return a.send(Xid(op_and), b); }
Any operator ^(const Any& a, const Any& b){ return a.send(Xid(op_xor), b); }
Any operator >>(const Any& a, const Any& b){ return a.send(Xid(op_shr), b); }
Any operator <<(const Any& a, const Any& b){ return a.send(Xid(op_shl), b); }
Any operator ==(const Any& a, const Any& b){ return a.raweq(b) || a.send(Xid(op_eq), b); }
Any operator !=(const Any& a, const Any& b){ return !a.raweq(b) || !a.send(Xid(op_eq), b); }
Any operator <(const Any& a, const Any& b){ return a.send(Xid(op_lt), b); }
Any operator >(const Any& a, const Any& b){ return b.send(Xid(op_lt), a); }
Any operator <=(const Any& a, const Any& b){ return !a.send(Xid(op_lt), b); }
Any operator >=(const Any& a, const Any& b){ return !b.send(Xid(op_lt), a); }

Any& operator +=(Any& a, const Any& b){ a = a.send(Xid(op_add_assign), b); return a; }
Any& operator -=(Any& a, const Any& b){ a = a.send(Xid(op_sub_assign), b); return a; }
Any& operator *=(Any& a, const Any& b){ a = a.send(Xid(op_mul_assign), b); return a; }
Any& operator /=(Any& a, const Any& b){ a = a.send(Xid(op_div_assign), b); return a; }
Any& operator %=(Any& a, const Any& b){ a = a.send(Xid(op_mod_assign), b); return a; }
Any& operator |=(Any& a, const Any& b){ a = a.send(Xid(op_or_assign), b); return a; }
Any& operator &=(Any& a, const Any& b){ a = a.send(Xid(op_and_assign), b); return a; }
Any& operator ^=(Any& a, const Any& b){ a = a.send(Xid(op_xor_assign), b); return a; }
Any& operator >>=(Any& a, const Any& b){ a = a.send(Xid(op_shr_assign), b); return a; }
Any& operator <<=(Any& a, const Any& b){ a = a.send(Xid(op_shl_assign), b); return a; }

void visit_members(Visitor& m, const Any& value){
	if(value.type()==TYPE_BASE){
		XTAL_ASSERT((int)value.impl()->ref_count() >= -m.value());
		value.impl()->add_ref_count(m.value());
	}
}

namespace{

AnyImpl** objects_begin_ = 0;
AnyImpl** objects_current_ = 0;
AnyImpl** objects_end_ = 0;

GCObserverImpl** gcobservers_begin_ = 0;
GCObserverImpl** gcobservers_current_ = 0;
GCObserverImpl** gcobservers_end_ = 0;

Any** llvars_begin_ = 0;
Any** llvars_current_ = 0;
Any** llvars_end_ = 0;

Any** place_begin_ = 0;
Any** place_current_ = 0;
Any** place_end_ = 0;

uint_t cycle_count_ = 0;
uint_t objects_gene_line_ = 0;

ID finalize_id(null);

void print_alive_objects(){
	for(AnyImpl** it = objects_begin_; it!=objects_current_; ++it){
//		StringImpl* str = dynamic_cast<StringImpl*>(*it);
//		fprintf(stderr, " %s rc=%d %s\n", typeid(**it).name(), (*it)->ref_count(), str ? str->c_str() : "");
		fprintf(stderr, " rc=%d\n", (*it)->ref_count());
	}
}

}

void enable_gc(){
	cycle_count_++;
}

void disable_gc(){
	cycle_count_--;
}

bool initialized(){
	return objects_begin_ ? true : false;
}

void initialize(){
	if(objects_begin_){ return; } // initialized

	expand_simple_dynamic_pointer_array((void**&)objects_begin_, (void**&)objects_end_, (void**&)objects_current_);

	disable_gc();

	TClass<Any>::set((ClassImpl*)AnyImpl::operator new(sizeof(ClassImpl)));  
	TClass<Any>::get().impl()->set_ref_count(1);
		
	TClass<Class>::set((ClassImpl*)AnyImpl::operator new(sizeof(ClassImpl)));  
	TClass<Class>::get().impl()->set_ref_count(1);
	
	int_t temp_ref_count;
	
	temp_ref_count = TClass<Any>::get().impl()->ref_count(); 
	new(TClass<Any>::get().impl()) ClassImpl();
	TClass<Any>::get().impl()->add_ref_count(temp_ref_count-1);
		
	temp_ref_count = TClass<Class>::get().impl()->ref_count(); 
	new(TClass<Class>::get().impl()) ClassImpl();
	TClass<Class>::get().impl()->add_ref_count(temp_ref_count-1);
	

	TClass<String>();
	TClass<Null>();
	TClass<Int>();
	TClass<Float>();
	TClass<Array>();
	TClass<Map>();
	TClass<Arguments>();
	TClass<Fiber>();

	finalize_id = Xid(finalize);

	enable_gc();

	InitString();
	InitAny();
	InitDebug();
	InitThread();
	InitInt();
	InitFloat();
	InitArray();
	InitMap();
	InitArguments();
	InitFun();
	InitFiber();
	InitFormat();
	InitClass();
	InitStream();
	InitFileStream();
	InitStringStream();

	initialize_lib();
	/**/
	
	//atexit(&uninitialize); // uninitializeを登録する
}

void uninitialize(){
	//print_alive_objects();

	UninitThread();

	full_gc();

	for(Any** p = llvars_begin_; p!=llvars_current_; ++p){
		**p = null;
	}
	
	for(Any** p = place_begin_; p!=place_current_; ++p){
		**p = null;
	}
		
	finalize_id = null;

	full_gc();
	
	if(objects_current_-objects_begin_ != 0){
		fprintf(stderr, "finished gc\n");
		fprintf(stderr, " alive object = %d\n", objects_current_-objects_begin_);
		//print_alive_objects();
		XTAL_ASSERT(false); // オブジェクトが全て解放されていない。
	}
	
	for(Any** p = place_begin_; p!=place_current_; ++p){
		user_free(*p, sizeof(Any));
	}
	
	llvars_current_ = llvars_begin_;
	place_current_ = place_begin_;

	fit_simple_dynamic_pointer_array((void**&)llvars_begin_, (void**&)llvars_end_, (void**&)llvars_current_);
	fit_simple_dynamic_pointer_array((void**&)gcobservers_begin_, (void**&)gcobservers_end_, (void**&)gcobservers_current_);
	fit_simple_dynamic_pointer_array((void**&)objects_begin_, (void**&)objects_end_, (void**&)objects_current_);
	fit_simple_dynamic_pointer_array((void**&)place_begin_, (void**&)place_end_, (void**&)place_current_);
}

void add_long_life_var(Any* a, int_t n){
	for(Any** p = llvars_begin_; p!=llvars_current_; ++p){
		if(*p==a){
			return;
		}
	}

	for(int_t i = 0; i<n; ++i){
		if(llvars_current_==llvars_end_){
			expand_simple_dynamic_pointer_array((void**&)llvars_begin_, (void**&)llvars_end_, (void**&)llvars_current_);
		}
		*llvars_current_++ = a++;
	}
}

void remove_long_life_var(Any* a, int_t n){
	for(Any** p = llvars_begin_; p!=llvars_current_; ++p){
		if(*p==a){
			std::memmove(p, p+n, sizeof(Any*)*n);
			llvars_current_ -= n;
			break;
		}
	}
}

Any* make_place(){
	if(place_current_==place_end_){
		expand_simple_dynamic_pointer_array((void**&)place_begin_, (void**&)place_end_, (void**&)place_current_);
	}
	*place_current_ = new(user_malloc(sizeof(Any))) Any();
	return *place_current_++;
}

struct CycleCounter{
	uint_t* p;
	CycleCounter(uint_t* p):p(p){ *p+=1; }
	~CycleCounter(){ *p-=1; }
};


void gc(){
	if(cycle_count_!=0){ return; }
	if(stop_the_world()){
		const VMachine& vm = vmachine();

		CycleCounter cc(&cycle_count_);

		for(GCObserverImpl** it = gcobservers_begin_; it!=gcobservers_current_; ++it){
			(*it)->before_gc();
		}

		if((int)objects_gene_line_>objects_current_-objects_begin_){
			objects_gene_line_ = 0;
		}

		AnyImpl** objects_alive = objects_begin_+objects_gene_line_;

		// mallocの中からgcが呼ばれている
		if(calling_malloc()){
			for(AnyImpl** it = objects_alive; it!=objects_current_; ++it){
				if((*it)->ref_count()
					// finalizeメソッドがあるオブジェクトも生き延びさせる
					|| (*it)->get_class().member(finalize_id)){
					std::swap(*it, *objects_alive++);
				}
			}
				
		}else{
			for(AnyImpl** it = objects_alive; it!=objects_current_; ++it){
				if((*it)->ref_count()){
					std::swap(*it, *objects_alive++);
				}
			}
				
			{
				for(GCObserverImpl** it = gcobservers_begin_; it!=gcobservers_current_; ++it){
					(*it)->after_gc();
				}

				int_t objects_alive_n = objects_alive-objects_begin_;
				int_t objects_current_n = objects_current_-objects_begin_;

				// 死ぬ予定のオブジェクトのfinalizerを実行する
				for(int_t i=objects_alive_n; i<objects_current_n; ++i){
					if(const Any& ret = objects_begin_[i]->get_class().member(finalize_id)){
						vm.setup_call(0);
						vm.set_arg_this(objects_begin_[i]);
						ret.call(vm);
						vm.cleanup_call();
					}
				}

				objects_alive = objects_begin_ + objects_alive_n;

				for(GCObserverImpl** it = gcobservers_begin_; it!=gcobservers_current_; ++it){
					(*it)->before_gc();
				}
			}

			AnyImpl** objects_finalized = objects_alive;
			objects_alive = objects_begin_+objects_gene_line_;

			for(AnyImpl** it = objects_alive; it!=objects_current_; ++it){
				if((*it)->ref_count() || it<objects_finalized){
					std::swap(*it, *objects_alive++);
				}
			}

		}

		for(GCObserverImpl** it = gcobservers_begin_; it!=gcobservers_current_; ++it){
			(*it)->after_gc();
		}

		for(AnyImpl** it = objects_alive; it!=objects_current_; ++it){
			delete *it;
		}

		for(AnyImpl** it = objects_alive; it!=objects_current_; ++it){
			user_free(*it, 0);
		}
		objects_current_ = objects_alive;

		objects_gene_line_ = (objects_current_-objects_begin_)*2/3;

		//fprintf(stderr, "finished gc\n");
		//fprintf(stderr, " alive object = %d\n", objects_current_-objects_begin_);

		restart_the_world();
	}
}


void full_gc(){

	if(cycle_count_!=0){ return; }
	if(stop_the_world()){
		const VMachine& vm = vmachine();

		CycleCounter cc(&cycle_count_);
		
		AnyImpl** prev_oc;
		do{
			
			for(GCObserverImpl** it = gcobservers_begin_; it!=gcobservers_current_; ++it){
				(*it)->before_gc();
			}

			prev_oc = objects_current_;

			{
				Visitor m(-1);	
				for(AnyImpl** it = objects_begin_; it!=objects_current_; ++it){
					(*it)->visit_members(m);
				}
			}

			{
				AnyImpl** objects_alive = objects_begin_;

				// mallocの中からgcが呼ばれている
				if(calling_malloc()){
					{
						Visitor m(1);
						bool end = false;
						while(!end){
							end = true;
							for(AnyImpl** it = objects_alive; it!=objects_current_; ++it){
								if((*it)->ref_count() 
									// finalizeメソッドがあるオブジェクトも生き延びさせる
									|| (*it)->get_class().member(finalize_id)){
									end = false;
									(*it)->visit_members(m);
									std::swap(*it, *objects_alive++);
								}
							}
						}
					}
				}else{

					{
						Visitor m(1);
						bool end = false;
						while(!end){
							end = true;
							for(AnyImpl** it = objects_alive; it!=objects_current_; ++it){
								if((*it)->ref_count()){
									end = false;
									(*it)->visit_members(m);
									std::swap(*it, *objects_alive++);
								}
							}
						}
					}
		
					if(finalize_id){
						{
							Visitor m(1);
							for(AnyImpl** it = objects_alive; it!=objects_current_; ++it){
								(*it)->visit_members(m);
							}
						}

						for(GCObserverImpl** it = gcobservers_begin_; it!=gcobservers_current_; ++it){
							(*it)->after_gc();
						}

						int_t objects_alive_n = objects_alive-objects_begin_;
						int_t objects_current_n = objects_current_-objects_begin_;

						// 死ぬ予定のオブジェクトのfinalizerを実行する
						for(int_t i=objects_alive_n; i<objects_current_n; ++i){		
							if(const Any& ret = objects_begin_[i]->get_class().member(finalize_id)){
								vm.setup_call(0);
								vm.set_arg_this(objects_begin_[i]);
								ret.call(vm);
								vm.cleanup_call();
							}
						}
							
						objects_alive = objects_begin_ + objects_alive_n;

						for(GCObserverImpl** it = gcobservers_begin_; it!=gcobservers_current_; ++it){
							(*it)->before_gc();
						}


						{// 参照カウントをまた一旦下げる
							Visitor m(-1);	
							for(AnyImpl** it = objects_begin_; it!=objects_current_; ++it){
								(*it)->visit_members(m);
							}
						}

						AnyImpl** objects_finalized = objects_alive;

						{
							objects_alive = objects_begin_;
					
							Visitor m(1);
							bool end = false;
							while(!end){
								end = true;
								for(AnyImpl** it = objects_alive; it!=objects_current_; ++it){
									if((*it)->ref_count() || it<objects_finalized){
										end = false;
										(*it)->visit_members(m);
										std::swap(*it, *objects_alive++);
									}
								}
								objects_finalized = 0;
							}
						}
					}
				}

				{// 削除されるオブジェクトだが、整合性をとるため参照カウンタを元に戻す
					Visitor m(1);
					for(AnyImpl** it = objects_alive; it!=objects_current_; ++it){
						(*it)->visit_members(m);
					}
				}

				for(GCObserverImpl** it = gcobservers_begin_; it!=gcobservers_current_; ++it){
					(*it)->after_gc();
				}

				for(AnyImpl** it = objects_alive; it!=objects_current_; ++it){
					delete *it;
				}

				for(AnyImpl** it = objects_alive; it!=objects_current_; ++it){
					user_free(*it, 0);
				}
				objects_current_ = objects_alive;
			}

		}while(prev_oc!=objects_current_);
		
		//fprintf(stderr, "finished full_gc\n");
		//fprintf(stderr, " alive object = %d\n", objects_current_-objects_begin_);

		restart_the_world();
	}
}

GCObserverImpl::GCObserverImpl(){
	if(gcobservers_current_==gcobservers_end_){
		expand_simple_dynamic_pointer_array((void**&)gcobservers_begin_, (void**&)gcobservers_end_, (void**&)gcobservers_current_);
	}
	*gcobservers_current_++ = this;
}
	
GCObserverImpl::~GCObserverImpl(){
	for(GCObserverImpl** p = gcobservers_begin_; p!=gcobservers_current_; ++p){
		if(*p==this){
			std::swap(*p, *--gcobservers_current_);
			break;
		}
	}
}

void InitAny(){
	TClass<Any> p("Any");
	p.method("class", &Any::get_class);
	p.method("get_class", &Any::get_class);
	p.method("object_name", &Any::object_name);
	p.method("op_eq", &Any::raweq);
}
	
void* AnyImpl::operator new(size_t size){
	if(objects_current_==objects_end_){
		initialize();
		expand_simple_dynamic_pointer_array((void**&)objects_begin_, (void**&)objects_end_, (void**&)objects_current_);
	}	
	AnyImpl* p = static_cast<AnyImpl*>(user_malloc(size));
	*objects_current_++=p;
	
	p->ref_count_ = 1;
	p->class_ = null;
	
	return p;
}
	
void AnyImpl::operator delete(void* p, size_t size){

}

void* AnyImpl::operator new(size_t size, Any& guard){
	if(objects_current_==objects_end_){
		initialize();
		expand_simple_dynamic_pointer_array((void**&)objects_begin_, (void**&)objects_end_, (void**&)objects_current_);
	}	
	AnyImpl* p = static_cast<AnyImpl*>(user_malloc(size));
	*objects_current_++ = p;
	
	p->ref_count_ = 0;
	p->class_ = TClass<Any>::get();

	guard = p;
	return p;
}
	
void AnyImpl::operator delete(void* p, Any& guard){
	guard = null;
}

void AnyImpl::set_class(const Class& c){
	class_ = c;
}

void AnyImpl::visit_members(Visitor& m){
	//m & class_;
}
	
void AnyImpl::call(const VMachine& vm){

}

const Any& AnyImpl::member(const ID& name){ 
	return null; 
}

void AnyImpl::def(const ID& name, const Any& value){

}

HaveInstanceVariables* AnyImpl::have_instance_variables(){ 
	return 0; 
}

String AnyImpl::object_name(){ 
	return String("instance of ").cat(get_class().object_name());
}

int_t AnyImpl::object_name_force(){ 
	return 0;
}

void AnyImpl::set_object_name(const String& name, int_t force, const Any& parent){

}

uint_t AnyImpl::hashcode(){
	return (uint_t)this;
}

#define XTAL_ANY_CALL0(op, call, args) \
Any Any::op args const{\
	const VMachine& vm = vmachine();\
	vm.setup_call(1);\
	call;\
	return vm.result_and_cleanup_call();\
}

#define XTAL_ANY_CALL1(op, call, args) \
Any Any::op args const{\
	const VMachine& vm = vmachine();\
	vm.setup_call(1);\
	vm.push_arg(a0);\
	call;\
	return vm.result_and_cleanup_call();\
}

#define XTAL_ANY_CALL2(op, call, args) \
Any Any::op args const{\
	const VMachine& vm = vmachine();\
	vm.setup_call(1);\
	vm.push_arg(a0);\
	vm.push_arg(a1);\
	call;\
	return vm.result_and_cleanup_call();\
}

#define XTAL_ANY_CALL3(op, call, args) \
Any Any::op args const{\
	const VMachine& vm = vmachine();\
	vm.setup_call(1);\
	vm.push_arg(a0);\
	vm.push_arg(a1);\
	vm.push_arg(a2);\
	call;\
	return vm.result_and_cleanup_call();\
}

#define XTAL_ANY_CALL4(op, call, args) \
Any Any::op args const{\
	const VMachine& vm = vmachine();\
	vm.setup_call(1);\
	vm.push_arg(a0);\
	vm.push_arg(a1);\
	vm.push_arg(a2);\
	vm.push_arg(a3);\
	call;\
	return vm.result_and_cleanup_call();\
}

#define XTAL_ANY_CALL5(op, call, args) \
Any Any::op args const{\
	const VMachine& vm = vmachine();\
	vm.setup_call(1);\
	vm.push_arg(a0);\
	vm.push_arg(a1);\
	vm.push_arg(a2);\
	vm.push_arg(a3);\
	vm.push_arg(a4);\
	call;\
	return vm.result_and_cleanup_call();\
}

XTAL_ANY_CALL0(operator(), call(vm), ());
XTAL_ANY_CALL1(operator(), call(vm), (const Any& a0));
XTAL_ANY_CALL2(operator(), call(vm), (const Any& a0, const Any& a1));
XTAL_ANY_CALL3(operator(), call(vm), (const Any& a0, const Any& a1, const Any& a2));
XTAL_ANY_CALL4(operator(), call(vm), (const Any& a0, const Any& a1, const Any& a2, const Any& a3));
XTAL_ANY_CALL5(operator(), call(vm), (const Any& a0, const Any& a1, const Any& a2, const Any& a3, const Any& a4));
XTAL_ANY_CALL1(operator(), call(vm), (const Named& a0));
XTAL_ANY_CALL2(operator(), call(vm), (const Any& a0, const Named& a1));
XTAL_ANY_CALL3(operator(), call(vm), (const Any& a0, const Any& a1, const Named& a2));
XTAL_ANY_CALL4(operator(), call(vm), (const Any& a0, const Any& a1, const Any& a2, const Named& a3));
XTAL_ANY_CALL5(operator(), call(vm), (const Any& a0, const Any& a1, const Any& a2, const Any& a3, const Named& a4));
XTAL_ANY_CALL2(operator(), call(vm), (const Named& a0, const Named& a1));
XTAL_ANY_CALL3(operator(), call(vm), (const Any& a0, const Named& a1, const Named& a2));
XTAL_ANY_CALL4(operator(), call(vm), (const Any& a0, const Any& a1, const Named& a2, const Named& a3));
XTAL_ANY_CALL5(operator(), call(vm), (const Any& a0, const Any& a1, const Any& a2, const Named& a3, const Named& a4));
XTAL_ANY_CALL3(operator(), call(vm), (const Named& a0, const Named& a1, const Named& a2));
XTAL_ANY_CALL4(operator(), call(vm), (const Any& a0, const Named& a1, const Named& a2, const Named& a3));
XTAL_ANY_CALL5(operator(), call(vm), (const Any& a0, const Any& a1, const Named& a2, const Named& a3, const Named& a4));
XTAL_ANY_CALL4(operator(), call(vm), (const Named& a0, const Named& a1, const Named& a2, const Named& a3));
XTAL_ANY_CALL5(operator(), call(vm), (const Any& a0, const Named& a1, const Named& a2, const Named& a3, const Named& a4));
XTAL_ANY_CALL5(operator(), call(vm), (const Named& a0, const Named& a1, const Named& a2, const Named& a3, const Named& a4));

XTAL_ANY_CALL0(send, send(name, vm), (const ID& name));
XTAL_ANY_CALL1(send, send(name, vm), (const ID& name, const Any& a0));
XTAL_ANY_CALL2(send, send(name, vm), (const ID& name, const Any& a0, const Any& a1));
XTAL_ANY_CALL3(send, send(name, vm), (const ID& name, const Any& a0, const Any& a1, const Any& a2));
XTAL_ANY_CALL4(send, send(name, vm), (const ID& name, const Any& a0, const Any& a1, const Any& a2, const Any& a3));
XTAL_ANY_CALL5(send, send(name, vm), (const ID& name, const Any& a0, const Any& a1, const Any& a2, const Any& a3, const Any& a4));
XTAL_ANY_CALL1(send, send(name, vm), (const ID& name, const Named& a0));
XTAL_ANY_CALL2(send, send(name, vm), (const ID& name, const Any& a0, const Named& a1));
XTAL_ANY_CALL3(send, send(name, vm), (const ID& name, const Any& a0, const Any& a1, const Named& a2));
XTAL_ANY_CALL4(send, send(name, vm), (const ID& name, const Any& a0, const Any& a1, const Any& a2, const Named& a3));
XTAL_ANY_CALL5(send, send(name, vm), (const ID& name, const Any& a0, const Any& a1, const Any& a2, const Any& a3, const Named& a4));
XTAL_ANY_CALL2(send, send(name, vm), (const ID& name, const Named& a0, const Named& a1));
XTAL_ANY_CALL3(send, send(name, vm), (const ID& name, const Any& a0, const Named& a1, const Named& a2));
XTAL_ANY_CALL4(send, send(name, vm), (const ID& name, const Any& a0, const Any& a1, const Named& a2, const Named& a3));
XTAL_ANY_CALL5(send, send(name, vm), (const ID& name, const Any& a0, const Any& a1, const Any& a2, const Named& a3, const Named& a4));
XTAL_ANY_CALL3(send, send(name, vm), (const ID& name, const Named& a0, const Named& a1, const Named& a2));
XTAL_ANY_CALL4(send, send(name, vm), (const ID& name, const Any& a0, const Named& a1, const Named& a2, const Named& a3));
XTAL_ANY_CALL5(send, send(name, vm), (const ID& name, const Any& a0, const Any& a1, const Named& a2, const Named& a3, const Named& a4));
XTAL_ANY_CALL4(send, send(name, vm), (const ID& name, const Named& a0, const Named& a1, const Named& a2, const Named& a3));
XTAL_ANY_CALL5(send, send(name, vm), (const ID& name, const Any& a0, const Named& a1, const Named& a2, const Named& a3, const Named& a4));
XTAL_ANY_CALL5(send, send(name, vm), (const ID& name, const Named& a0, const Named& a1, const Named& a2, const Named& a3, const Named& a4));

#undef XTAL_ANY_CALL0
#undef XTAL_ANY_CALL1
#undef XTAL_ANY_CALL2
#undef XTAL_ANY_CALL3
#undef XTAL_ANY_CALL4
#undef XTAL_ANY_CALL5

Any::Any(const char* str){
	set_null();
	*this = String(str);
}

Any::~Any(){ 
	dec_ref_count();
	set_null();
}

const Any& Any::member(const ID& name) const{
	if(type()==TYPE_BASE){
		return impl()->member(name);
	}
	return null;
}

void Any::def(const ID& name, const Any& value) const{
	if(type()==TYPE_BASE){
		value.set_object_name(name, object_name_force(), *this);
		return impl()->def(name, value);
	}
}

void Any::send(const ID& name, const VMachine& vm) const{
	vm.set_hint(get_class(), name);
	if(const Any& ret = get_class().member(name)){
		vm.set_arg_this(*this);
		ret.call(vm);
		return;
	}
}

void Any::call(const VMachine& vm) const{
	switch(type()){
		XTAL_DEFAULT;
		XTAL_CASE(TYPE_BASE){ impl()->call(vm); }
	}
}

int_t Any::to_i() const{
	switch(type()){
		XTAL_DEFAULT{ return cast<int_t>((*this).send("to_i")); }
		XTAL_CASE(TYPE_NULL){ return 0; }
		XTAL_CASE(TYPE_INT){ return ivalue(); }
		XTAL_CASE(TYPE_FLOAT){ return (int_t)fvalue(); }
	}
	return 0;
}

float_t Any::to_f() const{
	switch(type()){
		XTAL_DEFAULT{ return cast<float_t>((*this).send("to_f")); }
		XTAL_CASE(TYPE_NULL){ return 0; }
		XTAL_CASE(TYPE_INT){ return (float_t)ivalue(); }
		XTAL_CASE(TYPE_FLOAT){ return fvalue(); }
	}
	return 0;
}

String Any::to_s() const{
	if(const String* ret = as<const String*>(*this)){
		return *ret;
	}
	return cast<String>((*this).send("to_s"));
}

String Any::object_name() const{
	switch(type()){
		XTAL_NODEFAULT;
		XTAL_CASE(TYPE_NULL){ return String("Null"); }
		XTAL_CASE(TYPE_BASE){ return impl()->object_name(); }
		XTAL_CASE(TYPE_INT){ return String("instance of Int"); }
		XTAL_CASE(TYPE_FLOAT){ return String("instance of Float"); }
	}
	return null;	
}

int_t Any::object_name_force() const{
	if(type()==TYPE_BASE){ 
		impl()->object_name_force(); 
	}
	return 0;
}
	
void Any::set_object_name(const String& name, int_t force, const Any& parent) const{
	if(type()==TYPE_BASE){ 
		impl()->set_object_name(name, force, parent); 
	}
}

bool Any::is(const Any& v) const{
	return get_class().is_inherited(v);	
}

const Class& Any::get_class() const{
	switch(type()){
		XTAL_NODEFAULT;
		XTAL_CASE(TYPE_NULL){ return TClass<Null>::get(); }
		XTAL_CASE(TYPE_BASE){ return impl()->get_class(); }
		XTAL_CASE(TYPE_INT){ return TClass<Int>::get(); }
		XTAL_CASE(TYPE_FLOAT){ return TClass<Float>::get(); }
	}
	return TClass<Any>::get();
}

Any Any::cat(const Any& v) const{
	return send(Xid(op_cat), v);
}

const Any Any::at(const Any& index) const{
	return send(Xid(op_at), index);
}

void Any::set_at(const Any& index, const Any& value) const{
	send(Xid(op_set_at), index, value);
}

const Any Any::operator[](const Any& a) const{
	return send(Xid(op_at), a);
}

int_t Any::size() const{
	return send(Xid(size)).to_i();
}

uint_t Any::hashcode() const{
	if(type()==TYPE_BASE){
		return impl()->hashcode();
	}
	return (uint_t)rawvalue();
}

Any Any::p() const{
	VMachine vm = vmachine();
	vm.setup_call(0);
	vm.push_arg(*this);
	println(vm);
	vm.cleanup_call();
	return *this;
}


Any::Any(AnyImpl* v){
	if(v){ set_p(v); impl()->inc_ref_count(); }
	else{ set_null(); }
}

Any::Any(const AnyImpl* v){
	if(v){ set_p(v); impl()->inc_ref_count(); }
	else{ set_null(); }
}

Any::Any(const Any& v){
	UncountedAny::operator =(v);
	inc_ref_count();
}

Any& Any::operator =(const Any& v){
	dec_ref_count();
	UncountedAny::operator =(v);
	inc_ref_count();
	return *this;
}
	
Any& Any::operator =(AnyImpl* v){
	dec_ref_count();
	if(v){ set_p(v); impl()->inc_ref_count(); }
	else{ set_null(); }
	return *this;
}

Any& Any::operator =(const AnyImpl* v){
	dec_ref_count();
	if(v){ set_p(v); impl()->inc_ref_count(); }
	else{ set_null(); }
	return *this;
}

Any& Any::operator =(int_t v){
	dec_ref_count();
	set_i(v);
	return *this;
}
	
Any& Any::operator =(float_t v){
	dec_ref_count();
	set_f(v);
	return *this;
}

Any& Any::operator =(bool v){
	dec_ref_count();
	set_b(v);
	return *this;
}

Any& Any::operator =(const Null&){
	dec_ref_count();
	set_null();
	return *this;
}

}
