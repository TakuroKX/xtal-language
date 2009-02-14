
#pragma once

namespace xtal{

class Code : public Base{
public:
	
	Code();

	/**
	* @brief �R�[�h�ɑΉ������\�[�X�s����Ԃ��B
	*/
	int_t compliant_lineno(const inst_t* p);
		
	void set_lineno_info(uint_t line);

	const inst_t* data(){
		return &code_[0];
	}

	int_t size(){
		return (int_t)code_.size();
	}

	/**
	* @brief �V���{���e�[�u������i�Ԗڂ̃V���{�������o���B
	*/
	const IDPtr& identifier(int_t i){
		XTAL_ASSERT(i<(int_t)identifier_table_->size());
		return unchecked_ptr_cast<ID>(identifier_table_->at(i));
	}

	/**
	* @brief �l�e�[�u������i�Ԗڂ̒l�����o���B
	*/
	const AnyPtr& value(int_t i){
		XTAL_ASSERT(i<(int_t)value_table_->size());
		return value_table_->at(i);
	}

	/**
	* @brief once�e�[�u������i�Ԗڂ̒l�����o���B
	*/
	const AnyPtr& once_value(int_t i){
		XTAL_ASSERT(i<(int_t)once_table_->size());
		return once_table_->at(i);
	}

	/**
	* @brief once�e�[�u����i�Ԗڂɒl��ݒ肷��B
	*/
	void set_once_value(int_t i, const AnyPtr& v){
		XTAL_ASSERT(i<(int_t)once_table_->size());
		once_table_->set_at(i, v);
	}

	const StringPtr& source_file_name(){ 
		return source_file_name_; 
	}

	const ClassPtr& filelocal(){ 
		return filelocal_; 
	}

	ScopeInfo* scope_info(int_t i){
		return &scope_info_table_[i];
	}

	ClassInfo* class_info(int_t i){
		return &class_info_table_[i];
	}

	FunInfo* fun_info(int_t i){
		return &xfun_info_table_[i];
	}	

	ExceptInfo* except_info(int_t i){
		return &except_info_table_[i];
	}

	FunPtr first_fun(){
		return first_fun_;
	}

	StringPtr inspect();

	StringPtr inspect_range(int_t start, int_t end);

	virtual void rawcall(const VMachinePtr& vm);

	void insert_code(inst_t* p, inst_t* code, int_t size);

	void erase_code(inst_t* p, int_t size);

	bool add_break_point(uint_t lineno);

	void remove_break_point(uint_t lineno);

protected:

	void insert_erase_common(inst_t* p, int_t size);

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & identifier_table_ & value_table_ & filelocal_ & source_file_name_ & first_fun_ & once_table_;
	}

private:

	friend class CodeBuilder;
	friend class VMachine;
	friend class Serializer;

	typedef PODArrayList<inst_t> code_t;
	code_t code_;

	ArrayPtr identifier_table_;
	ArrayPtr value_table_;
	ArrayPtr once_table_;
	ClassPtr filelocal_;
	StringPtr source_file_name_;
	FunPtr first_fun_;


private:

	PODArrayList<FunInfo> xfun_info_table_;
	PODArrayList<ScopeInfo> scope_info_table_;
	PODArrayList<ClassInfo> class_info_table_;
	PODArrayList<ExceptInfo> except_info_table_;

	struct LineNumberTable{
		u32 start_pc;
		u32 lineno;
	};

	struct LineNumberCmp{
		bool operator ()(const LineNumberTable& lnt, uint_t pc){
			return lnt.start_pc<pc;
		}
		bool operator ()(uint_t pc, const LineNumberTable& lnt){
			return pc<lnt.start_pc;
		}
		bool operator ()(const LineNumberTable& l, const LineNumberTable& r){
			return l.start_pc<r.start_pc;
		}
	};

	PODArrayList<LineNumberTable> lineno_table_;

	struct AddressJump{
		u32 pos;
	};

	PODArrayList<AddressJump> address_jump_table_;
};

}