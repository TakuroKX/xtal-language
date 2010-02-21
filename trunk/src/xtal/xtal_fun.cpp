#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

Arguments::Arguments(const AnyPtr& ordered, const AnyPtr& named){
	if(ordered){ ordered_ = ptr_cast<Array>(ordered); }
	else{ ordered_ = null; }
	if(named){ named_ = ptr_cast<Map>(named); }
	else{ named_ = null; }
}

void Arguments::add_ordered(const AnyPtr& v){
	if(!ordered_){ ordered_ = xnew<Array>(); }
	ordered_->push_back(v);
}

void Arguments::add_named(const AnyPtr& k, const AnyPtr& v){
	if(!named_){ named_ = XNew<Map>(); }
	named_->set_at(k, v);
}
	
void Arguments::add_named(const VMachinePtr& vm){
	if(!named_){ named_ = XNew<Map>(); }
	named_->push_all(vm);
}

uint_t Arguments::ordered_size(){
	if(ordered_){
		return ordered_->size();
	}
	return 0;
}

uint_t Arguments::named_size(){
	if(named_){
		return named_->size();
	}
	return 0;
}

const AnyPtr& Arguments::op_at_int(uint_t index){
	if(ordered_ && index<ordered_->size()){
		return ordered_->at(index);
	}
	return undefined;
}

const AnyPtr& Arguments::op_at_string(const StringPtr& key){
	if(named_){
		return named_->at(key); 
	}
	return undefined;
}

uint_t Arguments::length(){
	if(ordered_){
		return ordered_->length();
	}
	return 0;
}

AnyPtr Arguments::ordered_arguments(){
	if(ordered_){
		return ordered_->each();
	}
	return null;
}

AnyPtr Arguments::named_arguments(){
	if(named_){
		return named_->each();
	}
	return null;
}

AnyPtr Arguments::each(){
	return xnew<ArgumentsIter>(to_smartptr(this));
}

StringPtr Arguments::to_s(){
	MemoryStreamPtr ms = xnew<MemoryStream>();
	ms->put_s(XTAL_STRING("("));
	Xfor2(key, val, each()){
		if(!first_step){
			ms->put_s(XTAL_STRING(", "));
		}
		ms->put_s(key);
		ms->put_s(XTAL_STRING(":"));
		ms->put_s(val);
	}
	ms->put_s(XTAL_STRING(")"));
	return ms->to_s();
}

void Arguments::on_visit_members(Visitor& m){
	Base::on_visit_members(m);
	m & ordered_ & named_;
}

ArgumentsIter::ArgumentsIter(const ArgumentsPtr& a){
	ait_ = unchecked_ptr_cast<ArrayIter>(a->ordered_arguments());
	mit_ = unchecked_ptr_cast<MapIter>(a->named_arguments());
	index_ = 0;
}

void ArgumentsIter::block_next(const VMachinePtr& vm){
	if(ait_){
		AnyPtr ret;
		if(ait_->block_next_direct(ret)){
			vm->return_result(to_smartptr(this), index_, ret);
			index_++;
			return;
		}

		ait_ = null;
	}

	if(mit_){
		AnyPtr key, val;
		if(mit_->block_next_direct(key, val)){
			vm->return_result(to_smartptr(this), key, val);
			return;
		}

		mit_ = null;
	}

	vm->return_result(null, null);
}

void ArgumentsIter::on_visit_members(Visitor& m){
	Base::on_visit_members(m);
	m & ait_ & mit_;
}

InstanceVariableGetter::InstanceVariableGetter(int_t number, ClassInfo* info)
	:number_(number), info_(info){
}

void InstanceVariableGetter::on_rawcall(const VMachinePtr& vm){
	const AnyPtr& self = vm->arg_this();
	vm->return_result(self->instance_variables()->variable(number_, info_));
}

InstanceVariableSetter::InstanceVariableSetter(int_t number, ClassInfo* info)
	:number_(number), info_(info){
}

void InstanceVariableSetter::on_rawcall(const VMachinePtr& vm){
	const AnyPtr& self = vm->arg_this();
	self->instance_variables()->set_variable(number_, info_, vm->arg(0));
	vm->return_result();
}


Method::Method(const FramePtr& outer, const CodePtr& code, FunInfo* info)
	:outer_(outer), code_(code), info_(info){
}

const inst_t* Method::source(){ 
	return code_->data()+info_->pc; 
}

const IDPtr& Method::param_name_at(size_t i){ 
	return code_->identifier(i+info_->variable_identifier_offset); 
}

int_t Method::param_size(){ 
	return info_->variable_size-(int)extendable_param(); 
}	

bool Method::extendable_param(){ 
	return (info_->flags&FunInfo::FLAG_EXTENDABLE_PARAM)!=0; 
}

bool Method::check_arg(const VMachinePtr& vm){
	int_t n = vm->ordered_arg_count();
	if(n<info_->min_param_count || (!(info_->flags&FunInfo::FLAG_EXTENDABLE_PARAM) && n>info_->max_param_count)){
		set_argument_num_error(object_name(), n, info_->min_param_count, info_->max_param_count, vm);
		return false;
	}
	return true;
}

const IDPtr& Method::object_temporary_name(){
	if(code_ && info_){
		return code_->identifier(info_->name_number);
	}
	return empty_id;
}

void Method::on_rawcall(const VMachinePtr& vm){
	if(vm->ordered_arg_count()!=info_->max_param_count){
		if(!check_arg(vm)){
			return;
		}
	}

	vm->carry_over(this);
}

Fun::Fun(const FramePtr& outer, const AnyPtr& athis, const CodePtr& code, FunInfo* info)
	:Method(outer, code, info), this_(athis){
}

void Fun::on_rawcall(const VMachinePtr& vm){
	if(vm->ordered_arg_count()!=info_->max_param_count){
		if(!check_arg(vm)){
			return;
		}
	}

	vm->set_arg_this(this_);
	vm->carry_over(this);
}

Lambda::Lambda(const FramePtr& outer, const AnyPtr& th, const CodePtr& code, FunInfo* info)
	:Fun(outer, th, code, info){
}

void Lambda::on_rawcall(const VMachinePtr& vm){
	vm->set_arg_this(this_);
	vm->mv_carry_over(this);
}

Fiber::Fiber(const FramePtr& outer, const AnyPtr& th, const CodePtr& code, FunInfo* info)
	:Fun(outer, th, code, info), resume_pc_(0), alive_(true){
}

void Fiber::on_finalize(){
	halt();
}

void Fiber::halt(){
	if(resume_pc_!=0){
		vm_->exit_fiber();
		resume_pc_ = 0;
		vmachine_take_back(vm_);
		vm_ = null;
		alive_ = false;
		unset_finalizer_flag();
	}
}

void Fiber::call_helper(const VMachinePtr& vm, bool add_succ_or_fail_result){
	if(alive_){
		vm->set_arg_this(this_);
		if(resume_pc_==0){
			if(!vm_){ 
				set_finalizer_flag();
				vm_ = vmachine_take_over(); 
			}
			resume_pc_ = vm_->start_fiber(this, vm.get(), add_succ_or_fail_result);
		}
		else{ 
			resume_pc_ = vm_->resume_fiber(this, resume_pc_, vm.get(), add_succ_or_fail_result);
		}

		if(resume_pc_==0){
			vmachine_take_back(vm_);
			vm_ = null;
			alive_ = false;
			unset_finalizer_flag();
		}
	}
	else{
		vm->return_result();
	}
}

const FiberPtr& Fiber::reset(){
	halt();
	alive_ = true;
	return to_smartptr(this);
}

void BindedThis::on_rawcall(const VMachinePtr& vm){
	vm->set_arg_this(this_);
	fun_->rawcall(vm);
}

}
