
#pragma once

#include <map>

namespace xtal{

/**
* @brief ���I�ȃ|�C���^�̔z����쐬�A�g������֐��B
*
* ��ԍŏ��̍쐬���́A�����S�Ă�null�ł���K�v������B
* @param begin �������̐擪
* @param end �������̍Ō�̈��
* @param current �g�p���̗v�f�̈��
*/
void expand_simple_dynamic_pointer_array(void**& begin, void**& end, void**& current, int addsize=1024);

/**
* @brief expand_simple_dynamic_pointer_array�Ő��������z����A�g���Ă�����ۂ̃T�C�Y�܂ŏk������B
*
* �v�f����������Ȃ�(current==begin�Ȃ�)���S�ɉ�������B
* @param begin �������̐擪
* @param end �������̍Ō�̈��
* @param current �g�p���̗v�f�̈��
*/
void fit_simple_dynamic_pointer_array(void**& begin, void**& end, void**& current);

/*
* �Œ�T�C�Y�������A���P�[�^
*/
class FixedAllocator{
public:

	typedef void* data_t;	

	struct Chunk{
		data_t* head;
		data_t* tail;
		uint_t count;
		Chunk* next;

		data_t* buf(){
			return reinterpret_cast<data_t*>(this+1);
		}
	};

	size_t calc_size(size_t block_size){
		return 128/block_size + 4;
	}

private:

	data_t* free_data_;
	Chunk* chunk_;
	uint_t all_count_;
	uint_t used_count_;

	bool cant_fit_;

public:

	FixedAllocator();

	void* malloc(size_t block_size);

	void free(void* p, size_t block_size);

	void release(size_t block_size);

	void fit(size_t block_size);

private:

	void add_chunk(size_t block_size);

	XTAL_DISALLOW_COPY_AND_ASSIGN(FixedAllocator);
};

/*
* �������T�C�Y�̃������A���P�[�^
*/
class SmallObjectAllocator{	

	typedef FixedAllocator::data_t data_t;

public:

	enum{
		POOL_SIZE = 16,
		HANDLE_MAX_SIZE = POOL_SIZE*sizeof(data_t)
	};

public:

	SmallObjectAllocator(){}
	
	void* malloc(size_t size);

	void free(void* p, size_t size);

	void release();

	void fit();

private:

	FixedAllocator pool_[POOL_SIZE];

	XTAL_DISALLOW_COPY_AND_ASSIGN(SmallObjectAllocator);
};

}//namespace 