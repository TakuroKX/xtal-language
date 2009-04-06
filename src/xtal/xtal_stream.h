#ifndef XTAL_STREAM_H_INCLUDE_GUARD
#define XTAL_STREAM_H_INCLUDE_GUARD

#pragma once

namespace xtal{

/**
* \ingroup Xtal Cpp
* \brief ストリーム
*/
class Stream : public Base{
public:

	/**
	* \brief 文字列strをストリームに流す
	*/
	void put_s(const StringPtr& str){
		write(str->data(), str->data_size()*sizeof(char_t));
	}

	/**
	* \brief 文字列strをストリームに流す
	*/
	void put_s(const char_t* str){
		write(str, string_data_size(str));
	}

	/**
	* \brief 文字列strをストリームに流す
	*/
	void put_s(const StringLiteral& str){
		write(str, str.size());
	}

	/**
	* \brief length文字分ストリームから取り出し、文字列として返す。
	*/
	virtual StringPtr get_s(uint_t length);

	virtual StringPtr get_s_all();

	virtual uint_t read_charactors(AnyPtr* buffer, uint_t max);

	uint_t print(const AnyPtr& value);

	void println(const AnyPtr& value);

	virtual uint_t tell();

	virtual uint_t write(const void* p, uint_t size);

	virtual uint_t read(void* p, uint_t size);

	virtual void seek(uint_t offset);

	virtual void close(){}

	virtual void flush(){}

	virtual uint_t pour(const StreamPtr& in_stream, uint_t size);

	virtual uint_t pour_all(const StreamPtr& in_stream);

	virtual uint_t size();

	virtual bool eos(){ return false; }

	void block_first(const VMachinePtr& vm);

	void block_next(const VMachinePtr& vm);

	void block_break(const VMachinePtr& vm);

	/**
	* \brief オブジェクトを直列化してストリームに書き込む
	*
	* \param obj 直列化して保存したいオブジェクト
	*/
	void serialize(const AnyPtr& obj);

	/**
	* \brief 直列化されたオブジェクトをストリームから読み出す
	*
	* \return 復元されたオブジェクト
	*/	
	AnyPtr deserialize();

	/**
	* \brief 
	*/
	void xtalize(const AnyPtr& obj);

	/**
	* \brief 
	*/	
	AnyPtr dextalize();


public:

	/**
	* \brief 符号付整数8-bitをストリームに書き込む
	*/
	void put_i8(i8 v){
		put_u8(v);
	}

	/**
	* \brief 符号付整数16-bitをストリームに書き込む
	*/
	void put_i16be(i16 v){
		put_u16be(v);
	}

	/**
	* \brief 符号付整数16-bitをストリームに書き込む
	*/
	void put_i16le(i16 v){
		put_u16le(v);
	}

	/**
	* \brief 符号付整数32-bitをストリームに書き込む
	*/
	void put_i32be(i32 v){
		put_u32be(v);
	}

	/**
	* \brief 符号付整数32-bitをストリームに書き込む
	*/
	void put_i32le(i32 v){
		put_u32le(v);
	}

	/**
	* \brief 符号付整数64-bitをストリームに書き込む
	*/
	void put_i64be(i64 v){
		put_u64be(v);
	}

	/**
	* \brief 符号付整数64-bitをストリームに書き込む
	*/
	void put_i64le(i64 v){
		put_u64le(v);
	}

	/**
	* \brief 符号無整数8-bitをストリームに書き込む
	*/
	void put_u8(u8 v){
		write(&v, 1);
	}

	/**
	* \brief 符号無整数16-bitをストリームに書き込む
	*/
	void put_u16be(u16 v){
		struct{ u8 data[2]; } data;
		data.data[0] = (u8)(v>>8);
		data.data[1] = (u8)(v>>0);
		write(data.data, 2);
	}

	/**
	* \brief 符号無整数16-bitをストリームに書き込む
	*/
	void put_u16le(u16 v){
		struct{ u8 data[2]; } data;
		data.data[1] = (u8)(v>>8);
		data.data[0] = (u8)(v>>0);
		write(data.data, 2);
	}

	/**
	* \brief 符号無整数32-bitをストリームに書き込む
	*/
	void put_u32be(u32 v){
		struct{ u8 data[4]; } data;
		data.data[0] = (u8)(v>>24);
		data.data[1] = (u8)(v>>16);
		data.data[2] = (u8)(v>>8);
		data.data[3] = (u8)(v>>0);
		write(data.data, 4);
	}

	/**
	* \brief 符号無整数32-bitをストリームに書き込む
	*/
	void put_u32le(u32 v){
		struct{ u8 data[4]; } data;
		data.data[3] = (u8)(v>>24);
		data.data[2] = (u8)(v>>16);
		data.data[1] = (u8)(v>>8);
		data.data[0] = (u8)(v>>0);
		write(data.data, 4);
	}

	/**
	* \brief 符号無整数64-bitをストリームに書き込む
	*/
	void put_u64be(u64 v){
		struct{ u8 data[8]; } data;
		data.data[0] = (u8)(v>>56);
		data.data[1] = (u8)(v>>48);
		data.data[2] = (u8)(v>>40);
		data.data[3] = (u8)(v>>32);
		data.data[4] = (u8)(v>>24);
		data.data[5] = (u8)(v>>16);
		data.data[6] = (u8)(v>>8);
		data.data[7] = (u8)(v>>0);
		write(data.data, 8);
	}

	/**
	* \brief 符号無整数64-bitをストリームに書き込む
	*/
	void put_u64le(u64 v){
		struct{ u8 data[8]; } data;
		data.data[7] = (u8)(v>>56);
		data.data[6] = (u8)(v>>48);
		data.data[5] = (u8)(v>>40);
		data.data[4] = (u8)(v>>32);
		data.data[3] = (u8)(v>>24);
		data.data[2] = (u8)(v>>16);
		data.data[1] = (u8)(v>>8);
		data.data[0] = (u8)(v>>0);
		write(data.data, 8);
	}

	i8 get_i8(){
		return (i8)get_u8();
	}

	i16 get_i16be(){
		return (i16)get_u16be();
	}

	i16 get_i16le(){
		return (i16)get_u16le();
	}

	i32 get_i32be(){
		return (i32)get_u32be();
	}

	i32 get_i32le(){
		return (i32)get_u32le();
	}

	i64 get_i64be(){
		return (i64)get_u64be();
	}

	i64 get_i64le(){
		return (i64)get_u64le();
	}

	u8 get_u8(){
		struct{ u8 data[1]; } data;
		read(data.data, 1);
		return (u8)data.data[0];
	}

	u16 get_u16be(){
		struct{ u8 data[2]; } data;
		read(data.data, 2);
		return (u16)((data.data[0]<<8) | data.data[1]);
	}

	u16 get_u16le(){
		struct{ u8 data[2]; } data;
		read(data.data, 2);
		return (u16)((data.data[1]<<8) | data.data[0]);
	}

	u32 get_u32be(){
		struct{ u8 data[4]; } data;
		read(data.data, 4);
		return (u32)((data.data[0]<<24) | (data.data[1]<<16) | (data.data[2]<<8) | data.data[3]);
	}

	u32 get_u32le(){
		struct{ u8 data[4]; } data;
		read(data.data, 4);
		return (u32)((data.data[3]<<24) | (data.data[2]<<16) | (data.data[1]<<8) | data.data[0]);
	}

	u64 get_u64be(){
		struct{ u8 data[8]; } data;
		read(data.data, 8);
		return (u64)(((u64)data.data[0]<<56) | ((u64)data.data[1]<<48) | ((u64)data.data[2]<<40) | ((u64)data.data[3]<<32) | (data.data[4]<<24) | (data.data[5]<<16) | (data.data[6]<<8) | data.data[7]);
	}

	u64 get_u64le(){
		struct{ u8 data[8]; } data;
		read(data.data, 8);
		return (u64)(((u64)data.data[7]<<56) | ((u64)data.data[6]<<48) | ((u64)data.data[5]<<40) | ((u64)data.data[4]<<32) | (data.data[3]<<24) | (data.data[2]<<16) | (data.data[1]<<8) | data.data[0]);
	}

	void put_f32be(f32 v){
		union{ u32 u; f32 f; } u;
		u.f = v;
		put_u32be(u.u);
	}

	void put_f32le(f32 v){
		union{ u32 u; f32 f; } u;
		u.f = v;
		put_u32le(u.u);
	}

	void put_f64be(f64 v){
		union{ u64 u; f64 f; } u;
		u.f = v;
		put_u64be(u.u);
	}

	void put_f64le(f64 v){
		union{ u64 u; f64 f; } u;
		u.f = v;
		put_u64le(u.u);
	}

	f32 get_f32be(){
		union{ u32 u; f32 f; } u;
		u.u = get_u32be();
		return u.f;
	}

	f32 get_f32le(){
		union{ u32 u; f32 f; } u;
		u.u = get_u32le();
		return u.f;
	}

	f64 get_f64be(){
		union{ u64 u; f64 f; } u;
		u.u = get_u64be();
		return u.f;
	}

	f64 get_f64le(){
		union{ u64 u; f64 f; } u;
		u.u = get_u64le();
		return u.f;
	}

	void put_ch_code_be(char_t ch){
		put_t_be<SelectType<sizeof(char_t)>::uint_t>(ch);
	}

	char_t get_ch_code_be(){
		return get_t_be<SelectType<sizeof(char_t)>::uint_t>();
	}

	void put_ch_code_le(char_t ch){
		put_t_le<SelectType<sizeof(char_t)>::uint_t>(ch);
	}

	char_t get_ch_code_le(){
		return get_t_le<SelectType<sizeof(char_t)>::uint_t>();
	}

public:

	template<class T>
	T get_t_be(){ return get_t_be((T*)0); }

	template<class T>
	void put_t_be(const T& v){ return put_t_be(v, (T*)0); }

	template<class T>
	T get_t_le(){ return get_t_le((T*)0); }

	template<class T>
	void put_t_le(const T& v){ return put_t_le(v, (T*)0); }

private:

	i8 get_t_be(i8*){ return get_i8(); }
	i16 get_t_be(i16*){ return get_i16be(); }
	i32 get_t_be(i32*){ return get_i32be(); }
	i64 get_t_be(i64*){ return get_i64be(); }
	u8 get_t_be(u8*){ return get_u8(); }
	u16 get_t_be(u16*){ return get_u16be(); }
	u32 get_t_be(u32*){ return get_u32be(); }
	u64 get_t_be(u64*){ return get_u64be(); }
	f32 get_t_be(f32*){ return get_f32be(); }
	f64 get_t_be(f64*){ return get_f64be(); }

	void put_t_be(i8 v, i8*){ put_i8(v); }
	void put_t_be(i16 v, i16*){ put_i16be(v); }
	void put_t_be(i32 v, i32*){ put_i32be(v); }
	void put_t_be(i64 v, i64*){ put_i64be(v); }
	void put_t_be(u8 v, u8*){ put_u8(v); }
	void put_t_be(u16 v, u16*){ put_u16be(v); }
	void put_t_be(u32 v, u32*){ put_u32be(v); }
	void put_t_be(u64 v, u64*){ put_u64be(v); }
	void put_t_be(f32 v, f32*){ put_f32be(v); }
	void put_t_be(f64 v, f64*){ put_f64be(v); }

	i8 get_t_le(i8*){ return get_i8(); }
	i16 get_t_le(i16*){ return get_i16le(); }
	i32 get_t_le(i32*){ return get_i32le(); }
	i64 get_t_le(i64*){ return get_i64le(); }
	u8 get_t_le(u8*){ return get_u8(); }
	u16 get_t_le(u16*){ return get_u16le(); }
	u32 get_t_le(u32*){ return get_u32le(); }
	u64 get_t_le(u64*){ return get_u64le(); }
	f32 get_t_le(f32*){ return get_f32le(); }
	f64 get_t_le(f64*){ return get_f64le(); }

	void put_t_le(i8 v, i8*){ put_i8(v); }
	void put_t_le(i16 v, i16*){ put_i16le(v); }
	void put_t_le(i32 v, i32*){ put_i32le(v); }
	void put_t_le(i64 v, i64*){ put_i64le(v); }
	void put_t_le(u8 v, u8*){ put_u8(v); }
	void put_t_le(u16 v, u16*){ put_u16le(v); }
	void put_t_le(u32 v, u32*){ put_u32le(v); }
	void put_t_le(u64 v, u64*){ put_u64le(v); }
	void put_t_le(f32 v, f32*){ put_f32le(v); }
	void put_t_le(f64 v, f64*){ put_f64le(v); }
};

class PointerStream : public Stream{
public:

	PointerStream(const void* data = 0, uint_t size = 0);
		
	virtual uint_t tell();

	virtual uint_t read(void* p, uint_t size);

	virtual void seek(uint_t offset);

	virtual void close(){}

	virtual bool eos();

	virtual StringPtr get_s(uint_t length);

	virtual StringPtr get_s_all();

	virtual uint_t size(){
		return size_;
	}
	
	const void* data(){
		return data_;
	}

protected:

	const u8* data_;
	uint_t size_;
	uint_t pos_;
};

/**
* \ingroup Xtal Cpp
* \biref メモリーストリーム
*/
class MemoryStream : public PointerStream{
public:

	MemoryStream();
	
	MemoryStream(const void* data, uint_t data_size);

	virtual ~MemoryStream();
	
	virtual uint_t write(const void* p, uint_t size);

	virtual uint_t pour(const StreamPtr& in_stream, uint_t size);

	virtual uint_t pour_all(const StreamPtr& in_stream);

	void clear();

	void resize(uint_t size);

	StringPtr to_s();

protected:

	void resize_data(uint_t size);

	uint_t capa_;
};

/**
* \ingroup Xtal Cpp
* \biref 文字列ストリーム
*/
class StringStream : public PointerStream{
public:

	StringStream(const StringPtr& str);
		
	StringPtr to_s(){
		return str_;
	}

private:

	virtual void visit_members(Visitor& m){
		PointerStream::visit_members(m);
		m & str_;
	}

	StringPtr str_;
};

/**
* \ingroup Xtal Cpp
* \biref ファイルストリーム
*/
class FileStream : public Stream{
public:

	FileStream(){
		impl_ = 0;
	}

	FileStream(const StringPtr& path, const StringPtr& flags){
		impl_ = 0;
		open(path, flags);
	}

	~FileStream(){
		if(impl_){
			filesystem_lib()->delete_file_stream(impl_);
		}
	}

	void open(const StringPtr& path, const StringPtr& flags);

	virtual void close(){
		if(impl_){
			filesystem_lib()->delete_file_stream(impl_);
			impl_ = 0;
		}
	}

	virtual uint_t read(void* dest, uint_t size){
		if(impl_){
			return filesystem_lib()->read_file_stream(impl_, dest, size);
		}
		return 0;
	}

	virtual uint_t write(const void* src, uint_t size){
		if(impl_){
			return filesystem_lib()->write_file_stream(impl_, src, size);
		}
		return 0;
	}

	virtual void seek(uint_t offset){
		if(impl_){
			filesystem_lib()->seek_file_stream(impl_, offset);
		}
	}

	virtual uint_t tell(){
		if(impl_){
			return filesystem_lib()->tell_file_stream(impl_);
		}
		return 0;
	}

	virtual bool eos(){
		if(impl_){
			return filesystem_lib()->end_file_stream(impl_);
		}
		return true;
	}

	virtual uint_t size(){
		if(impl_){
			return filesystem_lib()->size_file_stream(impl_);
		}
		return 0;
	}

	virtual void flush(){
		if(impl_){
			filesystem_lib()->flush_file_stream(impl_);
		}
	}

private:
	void* impl_;
};


class StdinStream : public Stream{
public:
	StdinStream(){
		impl_ = std_stream_lib()->new_stdin_stream();
	}

	~StdinStream(){
		std_stream_lib()->delete_stdin_stream(impl_);
	}

	virtual uint_t read(void* p, uint_t size){
		return std_stream_lib()->read_stdin_stream(impl_, p, size);
	}
private:
	void* impl_;
};

class StdoutStream : public Stream{
public:
	StdoutStream(){
		impl_ = std_stream_lib()->new_stdout_stream();
	}

	~StdoutStream(){
		std_stream_lib()->delete_stdout_stream(impl_);
	}

	virtual uint_t write(const void* p, uint_t size){
		return std_stream_lib()->write_stdout_stream(impl_, p, size);
	}
private:
	void* impl_;
};

class StderrStream : public Stream{
public:
	StderrStream(){
		impl_ = std_stream_lib()->new_stderr_stream();
	}

	~StderrStream(){
		std_stream_lib()->delete_stderr_stream(impl_);
	}

	virtual uint_t write(const void* p, uint_t size){
		return std_stream_lib()->write_stderr_stream(impl_, p, size);
	}
private:
	void* impl_;
};

class InteractiveStream : public Stream{
public:

	InteractiveStream(){
		line_ = 1;
		continue_stmt_ = false;
	}

	virtual uint_t read(void* p, uint_t size);

	virtual void flush(){
		continue_stmt_ = false;
	}

	uint_t read_line(void* p, uint_t size);

private:
	int_t line_;
	bool continue_stmt_;
};

}

#endif // XTAL_STREAM_H_INCLUDE_GUARD
