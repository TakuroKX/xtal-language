#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

Base::Base(){
	ref_count_ = 0;
	class_ = null;
	instance_variables_ = &empty_instance_variables; 
}

Base::Base(const Base& b){
	ref_count_ = 0;
	if(b.instance_variables_!=&empty_instance_variables){
		instance_variables_ = (InstanceVariables*)user_malloc(sizeof(InstanceVariables));
		new(instance_variables_) InstanceVariables(*b.instance_variables_);		

		class_ = b.class_;
		if(type(class_)==TYPE_BASE){
			pvalue(class_)->inc_ref_count();
		}
	}else{
		instance_variables_ = &empty_instance_variables;
		class_ = b.class_;
	}
}

Base::~Base(){
	if(instance_variables_!=&empty_instance_variables){
		instance_variables_->~InstanceVariables();
		user_free(instance_variables_);

		if(type(class_)==TYPE_BASE){
			pvalue(class_)->dec_ref_count();
		}
	}
}

int_t Base::to_i(){
	return cast<int_t>(send(Xid(to_i)));
}

float_t Base::to_f(){
	return cast<float_t>(send(Xid(to_f)));
}

StringPtr Base::to_s(){
	return ptr_cast<String>(send(Xid(to_s)));
}

ArrayPtr Base::to_a(){
	return ptr_cast<Array>(send(Xid(to_a)));
}

MapPtr Base::to_m(){
	return ptr_cast<Map>(send(Xid(to_m)));
}

AnyPtr Base::p(){
	return send(Xid(p));
}

void Base::set_class(const ClassPtr& c){
	if(instance_variables_==&empty_instance_variables){
		class_ = c;
	}else{
		if(type(class_)==TYPE_BASE)
			pvalue(class_)->dec_ref_count();
		class_ = c;
		if(type(class_)==TYPE_BASE)
			pvalue(class_)->inc_ref_count();
	}
}
	
void Base::call(const VMachinePtr& vm){
	ap(Innocence(this))->rawsend(vm, Xid(op_call));
}

const AnyPtr& Base::do_member(const InternedStringPtr& name, const AnyPtr& ns, const AnyPtr& self, bool inherited_too){ 
	return nop;
}


const AnyPtr& Base::member(const InternedStringPtr& name, const AnyPtr& ns, const AnyPtr& self, bool inherited_too){ 
	return ap(Innocence(this))->member(name, ns, self, inherited_too); 
}

void Base::def(const InternedStringPtr& name, const AnyPtr& value, const AnyPtr& ns, int_t accessibility){

}

SendProxy Base::send(const InternedStringPtr& name, const AnyPtr& ns){
	return SendProxy(ap(Innocence(this)), name, ns);
}

void Base::rawsend(const VMachinePtr& vm, const InternedStringPtr& name, const AnyPtr& ns, const AnyPtr& self, bool inherited_too){
	ap(Innocence(this))->rawsend(vm, name, ns, self, inherited_too);
}

StringPtr Base::object_name(){ 
	return xnew<String>("instance of ")->cat(get_class()->object_name());
}

int_t Base::object_name_force(){ 
	return 0;
}

void Base::set_object_name(const StringPtr& name, int_t force, const AnyPtr& parent){

}

bool Base::is(const AnyPtr& klass){
	return ap(Innocence(this))->is(klass);
}

uint_t Base::hashcode(){
	return (uint_t)this;
}

void Base::make_instance_variables(){
	if(instance_variables_==&empty_instance_variables){
		InstanceVariables* temp = (InstanceVariables*)user_malloc(sizeof(InstanceVariables));
		new(temp) InstanceVariables();
		instance_variables_ = temp;

		if(type(class_)==TYPE_BASE){
			pvalue(class_)->inc_ref_count();
		}
	}
}

void Base::visit_members(Visitor& m){
	if(instance_variables_!=&empty_instance_variables){
		if(type(class_)==TYPE_BASE){
			m & class_;
		}
		instance_variables_->visit_members(m);
	}
}

StringPtr HaveName::object_name(){
	if(!name_){
		return xnew<String>("<instance of ")->cat(get_class()->object_name())->cat(">");
	}

	if(!parent_){
		return name_;
	}

	if(LibPtr lib = ptr_as<Lib>(parent_)){
		lib = lib;
	}
	return parent_->object_name()->cat("::")->cat(name_);
}

int_t HaveName::object_name_force(){
	return force_;
}

void HaveName::set_object_name(const StringPtr& name, int_t force, const AnyPtr& parent){
	if(!name_ || force_<force){
		name_ = name;
		force_ = force;
		parent_ = parent;
	}
}

}

