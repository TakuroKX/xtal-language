/** \file src/xtal/xtal_environment.h
* \brief src/xtal/xtal_environment.h
*/

#ifndef XTAL_ENVIRONMENT_H_INCLUDE_GUARD
#define XTAL_ENVIRONMENT_H_INCLUDE_GUARD

#pragma once

namespace xtal{

/// \name Xtal���̏������A�j������
//@{

/**
* \brief �A���P�[�^���C�u����
*/
class AllocatorLib{
public:
	virtual ~AllocatorLib(){}
	virtual void* malloc(std::size_t size){ return std::malloc(size); }
	virtual void free(void* p, std::size_t size){ std::free(p); }
	virtual void* out_of_memory(std::size_t size){ return 0; }
};

/**
* \brief �����R�[�h���C�u����
*/
class ChCodeLib{
public:
	virtual ~ChCodeLib(){}

	virtual int_t ch_len(char_t lead){ return 1; }
	virtual int_t ch_len2(const char_t* str){ return ch_len(*str); }
	virtual StringPtr ch_inc(const char_t* data, int_t data_size);
	virtual int_t ch_cmp(const char_t* a, int_t asize, const char_t* b, int_t bsize);
};

/**
* \brief �X���b�h���C�u����
*/
class ThreadLib{
public:
	virtual ~ThreadLib(){}

	virtual void* new_thread(){ return 0; }
	virtual void delete_thread(void* thread_object){}
	virtual void start_thread(void* thread_object, void (*callback)(void*), void* data){}
	virtual void join_thread(void* thread_object){}

	virtual void* new_mutex(){ return 0; }
	virtual void delete_mutex(void* mutex_object){}
	virtual void lock_mutex(void* mutex_object){}
	virtual void unlock_mutex(void* mutex_object){}

	virtual void yield(){}
	virtual void sleep(float_t sec){}
};

/**
* \brief �W�����o�̓��C�u����
*/
class StdStreamLib{
public:
	virtual ~StdStreamLib(){}

	virtual void* new_stdin_stream(){ return 0; }
	virtual void delete_stdin_stream(void* stdin_stream_object){}
	virtual uint_t read_stdin_stream(void* stdin_stream_object, void* dest, uint_t size){ return 0; }

	virtual void* new_stdout_stream(){ return 0; }
	virtual void delete_stdout_stream(void* stdout_stream_object){}
	virtual uint_t write_stdout_stream(void* stdout_stream_object, const void* src, uint_t size){ return 0; }

	virtual void* new_stderr_stream(){ return 0; }
	virtual void delete_stderr_stream(void* stderr_stream_object){}
	virtual uint_t write_stderr_stream(void* stderr_stream_object, const void* src, uint_t size){ return 0; }
};

/**
* \brief �t�@�C���V�X�e�����C�u����
*/
class FilesystemLib{
public:
	virtual ~FilesystemLib(){}

	virtual bool is_directory(const char_t* path){ return false; }

	virtual void* new_file_stream(const char_t* path, const char_t* flags){ return 0; }
	virtual void delete_file_stream(void* file_stream_object){}
	virtual uint_t read_file_stream(void* file_stream_object, void* dest, uint_t size){ return 0; }
	virtual uint_t write_file_stream(void* file_stream_object, const void* src, uint_t size){ return 0; }
	virtual void seek_file_stream(void* file_stream_object, uint_t pos){}
	virtual uint_t tell_file_stream(void* file_stream_object){ return 0; }
	virtual bool end_file_stream(void* file_stream_object){ return true; }
	virtual uint_t size_file_stream(void* file_stream_object){ return 0; }
	virtual void flush_file_stream(void* file_stream_object){}

	virtual void* new_entries(const char_t* path){ return 0; }
	virtual void delete_entries(void* entries_object){}
	virtual const char_t* next_entries(void* entries_object){ return 0; }
	virtual void break_entries(void* entries_object){}

};

/**
* \brief �g�p���C�u�����̎w��̂��߂̍\����
*/
struct Setting{
	AllocatorLib* allocator_lib;
	ChCodeLib* ch_code_lib;
	ThreadLib* thread_lib;
	StdStreamLib* std_stream_lib;
	FilesystemLib* filesystem_lib;

	/**
	* \brief �قƂ�ǉ������Ȃ������ݒ肷��B
	*/
	Setting();
};

/**
* \brief Xtal���s�����쐬���A���������A�J�����g�ɐݒ肷��B
*/
void initialize(const Setting& setting);

/**
* \brief �J�����g��Xtal���s����j������B
*/
void uninitialize();

/**
* \brief ���݂̃J�����g��Xtal���s�����擾����B
*/
Environment* environment();

/**
* \brief �J�����g��Xtal���s����ݒ肷��B
*/
void set_environment(Environment* e);

//@}

ThreadLib* thread_lib();

StdStreamLib* std_stream_lib();

FilesystemLib* filesystem_lib();

/////////////////////////////////////////////////////

/// \name ����������
//@{

/**
* \brief ���[�U�[���o�^�����������A���P�[�g�֐����g���ă������m�ۂ���B
*/
void* xmalloc(size_t size);

/**
* \brief ���[�U�[���o�^�����������f�A���P�[�g�֐����g���ă������������B
*/
void xfree(void* p, size_t size);

/**
* \brief �������m�ۂ��X�R�[�v�ɕ����߂邽�߂̃��[�e�B���e�B�N���X
*/
struct XMallocGuard{
	/**
	* \brief �m�ۂ����ɍ\�z�����R���X�g���N�^
	*/
	XMallocGuard():p(0){}
	
	/**
	* \brief size���̃������m�ۂ���R���X�g���N�^
	*/
	explicit XMallocGuard(uint_t size):p(xmalloc(size)), sz(size){}

	/**
	* \brief ���������������
	*/
	~XMallocGuard(){ xfree(p, sz); }
public:	

	/**
	* \brief ���������m�ۂ���
	* ����܂łɊm�ۂ���Ă����������͉�������
	*/
	void malloc(size_t size){ xfree(p, sz); p = xmalloc(size); sz = size; }

	/**
	* \brief �������̐擪�A�h���X��Ԃ�
	*/
	void* get(){ return p; }

	/**
	* \brief �m�ۂ���Ă��郁�����̃T�C�Y��Ԃ�
	*/
	uint_t size(){ return sz; }

private:
	void* p;
	uint_t sz;

	XTAL_DISALLOW_COPY_AND_ASSIGN(XMallocGuard);
};

#define XTAL_PROTECT if(const ::xtal::Protect& xtalprotect = setjmp(::xtal::protect().buf)!=0)

#define XTAL_OUT_OF_MEMORY else 

//@}

/**
* \internal
* \brief jmp_buf������߂�\����
*/
struct JmpBuf{
	jmp_buf buf;
};

JmpBuf& protect();

void reset_protect();

bool ignore_memory_assert();

struct Protect{
	bool pass;
	Protect(bool pass):pass(pass){}
	~Protect(){ reset_protect(); }
	operator bool() const{ return !pass; }
};

/////////////////////////////////////////////////////

/**
* \xbind lib::builtin
* \brief �K�[�x�W�R���N�V���������s����
*
* ���قǎ��Ԃ͂�����Ȃ����A���S�ɃS�~������ł��Ȃ��K�[�x�W�R���N�g�֐�
* �Ⴆ�Ώz�Q�Ƃ͂���ł͌��m�ł��Ȃ��B
*/
void gc();

/**
* \xbind lib::builtin
* \brief �z�I�u�W�F�N�g���������t���K�[�x�W�R���N�V���������s����
*
* ���Ԃ͂����邪�A���݃S�~�ƂȂ��Ă�����̂͂Ȃ�ׂ��S�ĉ������K�[�x�W�R���N�g�֐�
* �z�Q�Ƃ����m�ł���B
*/
void full_gc();

/**
* \xbind lib::builtin
* \brief �K�[�x�W�R���N�V�����𖳌�������
*
* gc��full_gc�̌Ăяo���𖳌�������֐��B
* �����ł��ꂪ����Ăяo���ꂽ���L������Ă���A�Ăяo������enable_gc���ĂтȂ��ƃK�[�x�W�R���N�V�����͗L���ɂȂ�Ȃ�
*/
void disable_gc();

/**
* \xbind lib::builtin
* \brief �K�[�x�W�R���N�V������L��������

* disable_gc���Ă΂ꂽ�񐔂Ɠ��������Ăяo���ƃK�[�x�W�R���N�V�������L���ɂȂ�
*/
void enable_gc();

uint_t alive_object_count();

AnyPtr alive_object(uint_t i);

/////////////////////////////////////////////////////

/**
* \internal
* \brief key�ɑΉ�����C++�̃N���X�̃N���X�I�u�W�F�N�g��Ԃ��B
*/
const ClassPtr& cpp_class(CppClassSymbolData* key);

/*
* \internal
* \brief key�ɑΉ�����C++�̃N���X�̃N���X�I�u�W�F�N�g��ݒ肷��B
*/
void set_cpp_class(const ClassPtr& cls, CppClassSymbolData* key);

/**
* \brief �N���XT�ɑΉ�����C++�̃N���X�̃N���X�I�u�W�F�N�g��Ԃ��B
*/
template<class T>
inline const ClassPtr& cpp_class(){
	return cpp_class(&CppClassSymbol<T>::value);
}

/**
* \brief �N���XT�ɑΉ�����C++�̃N���X�̃N���X�I�u�W�F�N�g��ݒ肷��B
*/
template<class T>
inline void set_cpp_class(const ClassPtr& cls){
	return set_cpp_class(cls, &CppClassSymbol<T>::value);
}

/**
* \internal
* \brief T�^�̃I�u�W�F�N�g����������o��
*/
void* cpp_var(CppVarSymbolData* key);

/**
* \internal
* \brief T�^�̃I�u�W�F�N�g�����ɐݒ肷��
*/
void set_cpp_var(void* p, void (*deleter)(void*), CppVarSymbolData* key);

template<class T>
struct CppVarDeleter{
	static void deleter(void* p){
		((T*)p)->~T();
		xfree(p, sizeof(T));
	}
};

/**
* \brief T�^�̃I�u�W�F�N�g����������o��
*/
template<class T>
T& cpp_var(){
	if(void* p=cpp_var(&CppVarSymbol<T>::value)){
		return *(T*)p;
	}
	void* p=xmalloc(sizeof(T));
	new(p) T;
	set_cpp_var(p, &CppVarDeleter<T>::deleter, &CppVarSymbol<T>::value);
	return *(T*)p;
}

/////////////////////////////////////////////////////

/**
* \internal
* \brief �N���X�̃����o�����o���B
*/
const AnyPtr& cache_member(const AnyPtr& target_class, const IDPtr& primary_key, const AnyPtr& secondary_key, int_t& accessibility);

/**
* \internal
* \brief �N���X�̌p���֌W�𒲂ׂ�B
*/
bool cache_is(const AnyPtr& target_class, const AnyPtr& klass);

/**
* \internal
* \brief �N���X�̃R���X�g���N�^���L���b�V������Ă��邩�璲�ׂ�B
*/
bool cache_ctor(const AnyPtr& target_class, int_t kind);

/**
* \internal
* \brief �L���b�V���������B
*/
void clear_cache();

/**
* \internal
* \brief �����o�[�̃L���b�V���e�[�u���ɓo�^����Ă���f�[�^�𖳌��ɂ���B
*/
void invalidate_cache_member();

/**
* \internal
* \brief �p���֌W�̃L���b�V���e�[�u���ɓo�^����Ă���f�[�^�𖳌��ɂ���B
*/
void invalidate_cache_is();

/**
* \internal
* \brief �N���X�̃R���X�g���N�^�̃L���b�V���e�[�u���ɓo�^����Ă���f�[�^�𖳌��ɂ���B
*/
void invalidate_cache_ctor();

/////////////////////////////////////////////////////

/**
* \internal
* \brief VMachine�C���X�^���X�������^������B
*/
VMachinePtr vmachine_take_over();

/**
* \internal
* \brief VMachine�C���X�^���X��ԋp����B
*/
void vmachine_take_back(const VMachinePtr& vm);

/////////////////////////////////////////////////////

/// \name ���C�u�����I�u�W�F�N�g�擾�֐�
// @{

/**
* \brief builtin�I�u�W�F�N�g��Ԃ�
*/
const ClassPtr& builtin();

/**
* \brief lib�I�u�W�F�N�g��Ԃ�
*/
const LibPtr& lib();

/**
* \brief stdin�X�g���[���I�u�W�F�N�g��Ԃ�
*/
const StreamPtr& stdin_stream();

/**
* \brief stdout�X�g���[���I�u�W�F�N�g��Ԃ�
*/
const StreamPtr& stdout_stream();

/**
* \brief stderr�X�g���[���I�u�W�F�N�g��Ԃ�
*/
const StreamPtr& stderr_stream();

/**
* \brief VMachinePtr�I�u�W�F�N�g��Ԃ�
*
* �O���[�o����VMachinePtr�I�u�W�F�N�g��Ԃ��B
* �X���b�h���ɂ��̃O���[�o��VMachinePtr�I�u�W�F�N�g�͑��݂���B
*/
const VMachinePtr& vmachine();

// @}

/**
* \internal
* \brief ��������C���^�[���ςݕ�����ɕϊ�����
*/
const IDPtr& intern(const char_t* str, uint_t data_size);

/**
* \internal
* \brief ��������C���^�[���ςݕ�����ɕϊ�����
*/
const IDPtr& intern(const char_t* str);

/**
* \internal
* \brief �����񃊃e�������C���^�[���ςݕ�����ɕϊ�����
*/
const IDPtr& intern(const char_t* str, IdentifierData* iddata);

/**
* \internal
* \brief �����񃊃e�������C���^�[���ςݕ�����ɕϊ�����
*/
inline const IDPtr& intern(const StringLiteral& str){
	return intern(str, str.size());
}

/**
* \brief �C���^�[���ςݕ������񋓂���
*/
AnyPtr interned_strings();

/**
* \brief �������b�N����
*/
void xlock();

/**
* \brief �����A�����b�N����
*/
void xunlock();

struct XUnlock{
	XUnlock(int){ xunlock(); }
	~XUnlock(){ xlock(); }
	operator bool() const{ return true; }
private:
	XTAL_DISALLOW_COPY_AND_ASSIGN(XUnlock);
};

void register_thread(Environment*);

void unregister_thread(Environment*);

/**
* \brief �e�L�X�g�}�b�v��Ԃ�
*/
const MapPtr& text_map();

/**
* \internal
* \brief �擪�o�C�g�����āA���̃}���`�o�C�g���������������𒲂ׂ�B
*
* �}�C�i�X�̒l���Ԃ��ꂽ�ꍇ�A�Œᕶ������Ԃ��B
* -2�̏ꍇ�A�Œ�2�����ȏ�̕����ŁA�{���̒�����2�����ڂ�ǂ܂Ȃ��Ɣ��f�ł��Ȃ��A�Ƃ����Ӗ��ƂȂ�B
*/
int_t ch_len(char_t lead);

/**
* \internal
* \brief �}���`�o�C�g���������������𒲂ׂ�B
*
* int_t ch_len(char_t lead)���Ă΂ꂽ��A�}�C�i�X�̒l��Ԃ����ꍇ�ɑ����ČĂԁB
* ch_len��-2�̒l��Ԃ�����́Astr�̐�ɂ͍Œ�2�o�C�g���̃f�[�^���i�[���邱�ƁB
*/
int_t ch_len2(const char_t* str);

/**
* \internal
* \brief ���̕�����Ԃ�
*
* �Ⴆ�� a ��n�����ꍇ�Ab ���Ԃ�
*/
StringPtr ch_inc(const char_t* data, int_t data_size);

/**
* \internal
* \brief �����̑召����
*
* ���̒l a �̕����̕���b�̕�����菬����
* 0�̒l ������
* ���̒l b�̕����̕���a�̕�����菬����
*/
int_t ch_cmp(const char_t* a, uint_t asize, const char_t* b, uint_t bsize);

/**
* \internal
* \brief ���Z�q�̖��O������킷ID�̔z���Ԃ�
*/
const IDPtr* id_op_list();

StreamPtr open(const StringPtr& file_name, const StringPtr& mode);

#ifndef XTAL_NO_PARSER

/// \name �R���p�C���n�֐�
//@{

/**
* \xbind lib::builtin
* \brief file_name�t�@�C�����R���p�C������B
* ���̖߂�l��serialize����ƁA�o�C�g�R�[�h�`���ŕۑ������B
* \param file_name Xtal�X�N���v�g���L�q���ꂽ�t�@�C���̖��O
* \return ���s�ł���֐��I�u�W�F�N�g
*/
CodePtr compile_file(const StringPtr& file_name);

/**
* \xbind lib::builtin
* \brief source��������R���p�C������B
* ���̖߂�l��serialize����ƁA�o�C�g�R�[�h�`���ŕۑ������B
* \param source Xtal�X�N���v�g���L�q���ꂽ������
* \return ���s�ł���֐��I�u�W�F�N�g
*/
CodePtr compile(const StringPtr& source);

/**
* \xbind lib::builtin
* \brief file_name�t�@�C�����R���p�C�����Ď��s����B
* \param file_name Xtal�X�N���v�g���L�q���ꂽ�t�@�C���̖��O
* \return �X�N���v�g����return���ꂽ�l
*/
AnyPtr load(const StringPtr& file_name);

/**
* \xbind lib::builtin
* \brief file_name�t�@�C�����R���p�C�����ăR���p�C���ς݃\�[�X��ۑ����A���s����B
* \param file_name Xtal�X�N���v�g���L�q���ꂽ�t�@�C���̖��O
* \return �X�N���v�g����return���ꂽ�l
*/
AnyPtr load_and_save(const StringPtr& file_name);

//@}

CodePtr source(const char_t* src, int_t size);

#endif

CodePtr compiled_source(const void* src, int_t size);

}

#endif // XTAL_ENVIRONMENT_H_INCLUDE_GUARD