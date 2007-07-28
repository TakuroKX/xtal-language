﻿
#pragma once

#include <algorithm>

#include "xtal_allocator.h"
#include "xtal_stack.h"
#include "xtal_string.h"
#include "xtal_funimpl.h"

namespace xtal{

class CodeImpl : public FunImpl{
public:
	
	CodeImpl();

	void reset_core(){
		FunImpl::code_ = Code(this);
		core_ = &xfun_core_table_[0];
	}

	int_t compliant_line_number(const inst_t* p);
		
	void set_line_number_info(int_t line){
		if(!line_number_table_.empty() && line_number_table_.back().line_number==line)
			return;
		LineNumberTable lnt={(u16)code_.size(), (u16)line};
		line_number_table_.push_back(lnt);
	}

	const inst_t* data(){
		return &code_[0];
	}

	int_t size(){
		return code_.size();
	}

	const ID& symbol(int_t i){
		XTAL_ASSERT(i<(int_t)symbol_table_.size());
		return (const ID&)symbol_table_[i];
	}

	const Any& value(int_t i){
		XTAL_ASSERT(i<(int_t)code_.size());
		return value_table_[i];
	}

	void set_value(int_t i, const Any& v){
		XTAL_ASSERT(i<(int_t)value_table_.size());
		value_table_.set_at(i, v);
	}

	const String& source_file_name(){ 
		return source_file_name_; 
	}

	const Class& toplevel(){ 
		return toplevel_; 
	}

	BlockCore* block_core(int_t i){
		return &block_core_table_[i];
	}

	ClassCore* class_core(int_t i){
		return &class_core_table_[i];
	}

	FunCore* fun_core(int_t i){
		return &xfun_core_table_[i];
	}	

	ExceptCore* except_core(int_t i){
		return &except_core_table_[i];
	}

	String inspect();

public:

	typedef AC<inst_t>::vector code_t;
	code_t code_;
	Array symbol_table_;
	Array value_table_;
	Class toplevel_;
	String source_file_name_;

protected:

	virtual void visit_members(Visitor& m){
		FunImpl::visit_members(m);
		m & symbol_table_ & value_table_ & toplevel_ & source_file_name_;
	}

public:

	AC<FunCore>::vector xfun_core_table_;
	AC<BlockCore>::vector block_core_table_;
	AC<ClassCore>::vector class_core_table_;
	AC<ExceptCore>::vector except_core_table_;

	struct LineNumberTable{
		u16 start_pc;
		u16 line_number;
	};

	struct LineNumberCmp{
		bool operator ()(const LineNumberTable& lnt, int_t pc){
			return lnt.start_pc<pc;
		}
		bool operator ()(int_t pc, const LineNumberTable& lnt){
			return pc<lnt.start_pc;
		}
		bool operator ()(const LineNumberTable& l, const LineNumberTable& r){
			return l.start_pc<r.start_pc;
		}
	};

	AC<LineNumberTable>::vector line_number_table_;
};
	
}