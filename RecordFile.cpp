#include <iostream>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <functional>


#define FALSE 0
#define TRUE 1

using namespace std;

/*
class IOBuffer
class VariableLengthBuffer:public IOBuffer
class DelimFieldBuffer:public VariableLengthBuffer
class LengthFieldBuffer: public VariableLengthBuffer
class FixedLengthBuffer: public IOBuffer
class FixedFieldBuffer: public FixedLengthBuffer
*/

class IOBuffer
{
public:
	IOBuffer(int maxBytes = 1000);
	IOBuffer & operator=(const IOBuffer &);
	virtual void Clear();
	virtual int Pack(const void * field, int size = -1) = 0;
	virtual int Unpack(void * field, int maxbytes = -1) = 0;
	virtual void Print(ostream &) const;
	int Init(int maxBytes);
	virtual int Read(istream &) = 0;
	virtual int Write(ostream &) const = 0;
	virtual int DRead(istream &, int recref);
	virtual int DWrite(ostream &, int recref) const;
	virtual int ReadHeader(istream &);
	virtual int WriteHeader(ostream &) const;
protected:
	int Initialized;//TURE if buffer is initialized
	char * Buffer;//character array to hold field values
	int BufferSize;//sum of the sizes of packed fields
	int MaxBytes;//maximum number of characters in the buffer
	int NextByte;//index of next byte to be packed/unpacked
	int Packing;//TRUE if in packing mode, FALSE, if unpacking
};

int PackFixed(char * buffer, void * field, int size = -1);
int PackDelimeted(char * buffer, void * field, int size = -1);
int PackLength(char * buffer, void * field, int size = -1);

IOBuffer::IOBuffer(int maxBytes)
{
	Init(maxBytes);
}

IOBuffer & IOBuffer::operator=(const IOBuffer & buffer)
{
	if (MaxBytes<buffer.BufferSize) return *this; //fails
	Initialized = buffer.Initialized;
	BufferSize = buffer.BufferSize;
	memcpy(Buffer, buffer.Buffer, buffer.BufferSize);
	NextByte = buffer.NextByte;
	Packing = Packing;
	return *this;
}

void IOBuffer::Clear()
{
	NextByte = 0;
	Packing = TRUE;
}

void IOBuffer::Print(ostream & stream) const
{
	stream << "MaxBytes " << MaxBytes << " BufferSize " << BufferSize;
}

int IOBuffer::Init(int maxBytes)
{
	Initialized = FALSE;
	if (maxBytes<0) maxBytes = 0;
	MaxBytes = maxBytes;
	Buffer = new char[MaxBytes];
	BufferSize = 0;
	Clear();
	return 1;
}

int IOBuffer::DRead(istream & stream, int recref)
{
	stream.seekg(recref, ios::beg);
	if ((int)stream.tellg() != recref) return -1;
	return Read(stream);
}

int IOBuffer::DWrite(ostream & stream, int recref) const
{
	stream.seekp(recref, ios::beg);
	if ((int)stream.tellp() != recref) return -1;
	return Write(stream);
}

static const char* headerStr1 = "IOBuffer";
static const int headerSize1 = 9;

int IOBuffer::ReadHeader(istream & stream)
{
	char str[headerSize1 + 1];
	stream.seekg(0, ios::beg);
	stream.read(str, headerSize1);
	if (!stream.good()) return -1;
	if (strncmp(str, headerStr1, headerSize1) == 0)
		return headerSize1;
	else return -1;
}

int IOBuffer::WriteHeader(ostream & stream) const
{
	stream.seekp(0, ios::beg);
	stream.write(headerStr1, headerSize1);
	if (!stream.good()) return -1;
	return headerSize1;
}



class VariableLengthBuffer :public IOBuffer
{
public:
	VariableLengthBuffer(int MaxBytes = 1000);
	VariableLengthBuffer(const VariableLengthBuffer & buffer)
		:IOBuffer(buffer) {}
	void Clear();
	int Read(istream &);
	int Write(ostream &) const;
	int ReadHeader(istream &);
	int WriteHeader(ostream &) const;
	int PackFixLen(void *, int);
	int PackDelimeted(void *, int);
	int PackLength(void *, int);
	void Print(ostream &) const;
	int SizeOfBuffer() const;
	int Init();
};

VariableLengthBuffer::VariableLengthBuffer(int maxBytes)
	:IOBuffer(maxBytes)
{
	Init();
}

void VariableLengthBuffer::Clear()
{
	IOBuffer::Clear();
}

int VariableLengthBuffer::Read(istream & stream)
{
	if (stream.eof()) return -1;
	int recaddr = stream.tellg();
	Clear();
	unsigned short bufferSize;
	stream.read((char*)&bufferSize, sizeof(bufferSize));
	if (!stream.good())
	{
		stream.clear();
		return -1;
	}
	BufferSize = bufferSize;
	if (BufferSize>MaxBytes) return -1;//buffer overflow
	stream.read(Buffer, BufferSize);
	if (!stream.good())
	{
		stream.clear();
		return -1;
	}
	return recaddr;
}

int VariableLengthBuffer::Write(ostream & stream) const
{
	int recaddr = stream.tellp();
	unsigned short bufferSize;
	bufferSize = BufferSize;
	stream.write((char *)&bufferSize, sizeof(bufferSize));
	if (!stream) return -1;
	stream.write(Buffer, BufferSize);
	if (!stream.good()) return -1;
	return recaddr;
}

const char* headerStr2 = "Variable";
const int headerSize2 = 9;

int VariableLengthBuffer::ReadHeader(istream & stream)
{
	char str[headerSize2 + 1];
	int result;
	//read the IOBuffer header
	result = IOBuffer::ReadHeader(stream);
	if (!result) return FALSE;
	//read the header string
	stream.read(str, headerSize2);
	if (!stream.good()) return FALSE;
	if (strncmp(str, headerStr2, headerSize2) != 0) return FALSE;
	return stream.tellg();
}

int VariableLengthBuffer::WriteHeader(ostream & stream) const
{
	int result;
	//write the parent(IOBuffer) header
	result = IOBuffer::WriteHeader(stream);
	if (!result) return FALSE;
	//write the header string
	stream.write(headerStr2, headerSize2);
	if (!stream.good()) return FALSE;
	//write the record description
	return stream.tellp();
}

void VariableLengthBuffer::Print(ostream & stream) const
{
	IOBuffer::Print(stream);
}

int VariableLengthBuffer::Init()
{
	Clear();
	return TRUE;
}


class DelimFieldBuffer :public VariableLengthBuffer
{
public:
	DelimFieldBuffer(char Delim = -1, int maxBytes = 1000);
	DelimFieldBuffer(const DelimFieldBuffer & buffer);
	void Clear();
	int Pack(const void*, int size = -1);
	int Unpack(void * field, int maxBytes = -1);
	int ReadHeader(istream & stream);
	int WriteHeader(ostream & stream) const;
	void Print(ostream &) const;
	int Init(char delim = 0);
	static void SetDefaultDelim(char delim);
protected:
	char Delim;
	static char DefaultDelim;
};

inline DelimFieldBuffer::DelimFieldBuffer(const DelimFieldBuffer & buffer)
	:VariableLengthBuffer(buffer)
{
	Init(buffer.Delim);
}

DelimFieldBuffer::DelimFieldBuffer(char delim, int maxBytes)
	: VariableLengthBuffer(maxBytes)
{
	Init(delim);
}

void DelimFieldBuffer::Clear()
{
	VariableLengthBuffer::Clear();
}

int DelimFieldBuffer::Pack(const void* field, int size)
{
	short len;
	if (size >= 0) len = size;
	else len = strlen((char*)field);
	if (len>(short)strlen((char*)field)) return -1;
	int start = NextByte;
	NextByte += len + 1;
	if (NextByte>MaxBytes) return -1;
	memcpy(&Buffer[start], field, len);
	Buffer[start + len] = Delim;
	BufferSize = NextByte;
	return len;
}

int DelimFieldBuffer::Unpack(void * field, int maxBytes)
{
	int len = -1;
	int start = NextByte;
	for (int i = start;i<BufferSize;i++)
	{
		if (Buffer[i] == Delim)
		{
			len = i - start;
			break;
		}
	}
	if (len == -1) return -1;
	NextByte += len + 1;
	if (NextByte>BufferSize) return -1;
	//check for maxBytes
	memcpy(field, &Buffer[start], len);
	if (maxBytes>len || maxBytes == -1)
		((char*)field)[len] = 0;
	//zero termination for string
	return len;
}

int DelimFieldBuffer::ReadHeader(istream & stream)
{
	char ch;
	int result;
	result = VariableLengthBuffer::ReadHeader(stream);
	if (!result) return FALSE;
	stream.get(ch);
	if (!Initialized)
	{
		SetDefaultDelim(ch);
		return TRUE;
	}
	if (ch != Delim) return FALSE;
	return stream.tellg();
}

int DelimFieldBuffer::WriteHeader(ostream & stream) const
{
	if (!Initialized) return FALSE;
	int result;
	result = VariableLengthBuffer::WriteHeader(stream);
	if (!result) return FALSE;
	stream.put(Delim);
	return stream.tellp();
}

void DelimFieldBuffer::Print(ostream & stream) const
{
	VariableLengthBuffer::Print(stream);
	stream << " Delimeter '" << Delim << "'" << endl;
}

int DelimFieldBuffer::Init(char delim)
{
	Initialized = TRUE;
	Clear();
	if (delim == -1) Delim = DefaultDelim;
	else Delim = delim;
	return TRUE;
}

void DelimFieldBuffer::SetDefaultDelim(char delim)
{
	DefaultDelim = delim;
}

char DelimFieldBuffer::DefaultDelim = 0;



class LengthFieldBuffer : public VariableLengthBuffer
	// a buffer which holds a length plus value fields.
	// Record variables can be packed into and extracted from a buffer.
	// 
	// To use this class, create a LengthFieldBuffer variable and associate definitions with the fields.
	// operations are provided to allow values to be associated with the fields (Pack)
	//	and to fetch the values of fields (Unpack)
{
public:
	LengthFieldBuffer(int maxBytes = 1000); // construct with a maximum of maxFields
											// construct with fields of specific size
	LengthFieldBuffer(const LengthFieldBuffer & buffer) // copy constructor
		: VariableLengthBuffer(buffer) {}
	void Clear(); // clear fields from buffer
	int Pack(const void * field, int size = -1); // set the value of the next field of the buffer;
	int Unpack(void *field, int maxBytes = -1); // extract the value of the next field of the buffer
	void Print(ostream &) const;
	int Init();
protected:
};


LengthFieldBuffer::LengthFieldBuffer(int maxBytes)
// construct with a maximum of maxFields
	: VariableLengthBuffer(maxBytes)
{
	Init();
}

void LengthFieldBuffer::Clear()
// clear fields from buffer
{
	VariableLengthBuffer::Clear();
}

int LengthFieldBuffer::Pack(const void* field, int size)
// set the value of the next field of the buffer;
// if size = -1 (default) use strlen(str) as length of field
// return number of bytes packed, -1 if error
{
	short len; // length of string to be packed
	if (size >= 0) len = size;
	else len = strlen((char *)field);
	int start = NextByte; // first character to be packed
	NextByte += (len + sizeof(len));
	if (NextByte > MaxBytes) return -1;
	memcpy(&Buffer[start], &len, sizeof(len));
	memcpy(&Buffer[start + sizeof(len)], field, len);
	BufferSize = NextByte;
	return len;
}

int LengthFieldBuffer::Unpack(void* field, int maxBytes)
// extract the value of the next field of the buffer
// return the number of bytes extracted, -1 if error
{
	short len; // length of packed string
	if (NextByte >= BufferSize) return -1; // no more fields
	int start = NextByte; // first character to be unpacked
	memcpy(&len, &Buffer[start], sizeof(len));
	if (maxBytes != -1 && len > maxBytes) return -1; // field too long
	NextByte += len + sizeof(len);
	if (NextByte > BufferSize) return -1;
	memcpy(field, &Buffer[start + sizeof(len)], len);
	if (maxBytes > len || maxBytes == -1)
		((char *)field)[len] = 0; // zero termination for string 
	return len;
}

void LengthFieldBuffer::Print(ostream & stream) const
{
	stream << "Buffer has characters " << MaxBytes << " and Buffer Size " << BufferSize << endl;
}

// Protected members

int LengthFieldBuffer::Init()
// construct with a maximum of maxFields
{
	Initialized = TRUE;
	Clear();
	return TRUE;
}



class FixedLengthBuffer : public IOBuffer
	// Abstract class designed to support fixed length records
{
public:
	FixedLengthBuffer(int recordSize = 1000);
	FixedLengthBuffer(const FixedLengthBuffer & buffer); // copy constructor

	void Clear(); // clear values from buffer
	int Read(istream &);
	int Write(ostream &) const;
	int ReadHeader(istream &); // read header from stream
	int WriteHeader(ostream &) const; // write a header to the stream
	void Print(ostream &) const;
	int SizeOfBuffer() const; // return size of buffer
protected:
	int Init(int recordSize);
	int ChangeRecordSize(int recordSize);
};

inline FixedLengthBuffer::FixedLengthBuffer(const FixedLengthBuffer & buffer)
	: IOBuffer(buffer)
{
	Init(buffer.BufferSize);
}


FixedLengthBuffer::FixedLengthBuffer(int recordSize)
// construct with a maximum of maxFields
	: IOBuffer(recordSize)
{
	Init(recordSize);
}

void FixedLengthBuffer::Clear()
// clear fields from buffer
{
	IOBuffer::Clear();
	Buffer[0] = 0;
	Packing = TRUE;
}

int FixedLengthBuffer::Read(istream & stream)
// write the number of bytes in the buffer field definitions
{
	int recaddr = stream.tellg(); stream.clear();
	Clear();
	Packing = FALSE;
	stream.read(Buffer, BufferSize);
	if (!stream.good()) { stream.clear(); return recaddr; }
	return recaddr;
}


int FixedLengthBuffer::Write(ostream & stream) const
// read the number of bytes in the buffer field definitions
// return the location of the record in the file
{
	int recaddr = stream.tellp();
	stream.write(Buffer, BufferSize);
	if (!stream.good()) return -1;
	return recaddr;
}

static const char * headerStr3 = "Fixed";
static const int headerStrSize3 = 6;

int FixedLengthBuffer::ReadHeader(istream & stream)
// read the header and check for consistency
// see WriteHeader for header record structure
{
	char str[headerStrSize3 + 1];
	int recordSize;
	int result;
	// read the IOBuffer header
	result = IOBuffer::ReadHeader(stream);
	if (result < 0) return -1;
	// read the string "Fixed"
	stream.read(str, headerStrSize3);
	if (!stream.good()) return -1;
	if (strncmp(str, headerStr3, headerStrSize3) != 0) return -1;
	stream.read((char*)&recordSize, sizeof(recordSize));
	if (Initialized) // check header for consistency
	{
		if (recordSize != BufferSize) return -1;
	}
	// else initialize the buffer from the header
	ChangeRecordSize(recordSize);
	return stream.tellg();
}

int FixedLengthBuffer::WriteHeader(ostream & stream) const
// write a buffer header to the beginning of the stream
// A header consists of the 
//	IOBUFFER header	
//	FIXED			5 bytes
//	record size		2 bytes
{
	int result;
	if (!Initialized) return -1; // cannot write unitialized buffer
								 // write the parent (IOBuffer) header
	result = IOBuffer::WriteHeader(stream);
	if (!result) return -1;
	// write the string "Fixed"
	stream.write(headerStr3, headerStrSize3);
	if (!stream.good()) return -1;
	// write the record size
	stream.write((char *)&BufferSize, sizeof(BufferSize));
	if (!stream.good()) return -1;
	return stream.tellp();
}

void FixedLengthBuffer::Print(ostream & stream) const
{
	IOBuffer::Print(stream);
	stream << "Fixed ";
}

int FixedLengthBuffer::Init(int recordSize)
// construct with a maximum of maxFields
{
	Clear();
	BufferSize = recordSize;
	return 1;
}

int FixedLengthBuffer::ChangeRecordSize(int recordSize)
// construct with a maximum of maxFields
{
	BufferSize = recordSize;
	return 1;
}


class FixedFieldBuffer : public FixedLengthBuffer
	// Abstract class designed to support fixed length records
	// Use of this class requires that all fields be defined before
	//    reading and writing can take place
{
public:
	FixedFieldBuffer(int maxFields, int RecordSize = 1000);
	FixedFieldBuffer(int maxFields, int * fieldSize);
	// initialize all fields at once
	FixedFieldBuffer(const FixedFieldBuffer &); //copy constructor
	FixedFieldBuffer & operator = (const FixedFieldBuffer &);
	void Clear(); // clear values from buffer
	int AddField(int fieldSize); // define the next field
	int ReadHeader(istream &); // write a buffer to the stream
	int WriteHeader(ostream &) const; // write a buffer to the stream
	int Pack(const void * field, int size = -1); // set the value of the next field of the buffer;
	int Unpack(void * field, int maxBytes = -1); // extract the value of the next field of the buffer
	void Print(ostream &) const;
	int NumberOfFields() const; // return number of defined fields
	int Init(int maxFields);
	int Init(int numFields, int * fieldSize);
protected:
	int * FieldSize; // array to hold field sizes
	int MaxFields; // maximum number of fields
	int NumFields; // actual number of defined fields
	int NextField; // index of next field to be packed/unpacked
};

inline FixedFieldBuffer::FixedFieldBuffer(const FixedFieldBuffer & buffer)
	: FixedLengthBuffer(buffer)
{
	Init(buffer.NumFields, buffer.FieldSize);
}


FixedFieldBuffer::FixedFieldBuffer(int maxFields, int maxBytes)
// construct with a maximum of maxFields
	: FixedLengthBuffer(maxBytes)
{
	Init(maxFields);
}

// private function to calculate the record size from the field sizes
static int SumFieldSizes(int numFields, int * fieldSize)
{
	int sum = 0;
	for (int i = 0; i < numFields; i++)
		sum += fieldSize[i];
	return sum;
}

FixedFieldBuffer & FixedFieldBuffer :: operator =
(const FixedFieldBuffer & buffer)
{
	// disallow copy unless fields are identical
	if (NumFields != buffer.NumFields) return *this;
	for (int i = 0; i < NumFields; i++)
		if (FieldSize[i] != buffer.FieldSize[i]) return *this;
	NextField = buffer.NextField;
	FixedLengthBuffer :: operator = (buffer);
	return *this;
}

FixedFieldBuffer::FixedFieldBuffer(int numFields, int * fieldSize)
// construct with fields of specific size
	: FixedLengthBuffer(SumFieldSizes(numFields, fieldSize))
{
	Init(numFields, fieldSize);
}

int FixedFieldBuffer::NumberOfFields() const
// return number of fields 
{
	return NumFields;
}

void FixedFieldBuffer::Clear()
// clear fields from buffer
{
	FixedLengthBuffer::Clear();
	NextField = 0;
	Buffer[0] = 0;
	Packing = TRUE;
}

int FixedFieldBuffer::AddField(int fieldSize)
{
	Initialized = TRUE;
	if (NumFields == MaxFields) return FALSE;
	if (BufferSize + fieldSize > MaxBytes) return FALSE;
	FieldSize[NumFields] = fieldSize;
	NumFields++;
	BufferSize += fieldSize;
	return TRUE;
}


static const char * headerStr4 = "Field";
static const int headerStrSize4 = strlen(headerStr4);

int FixedFieldBuffer::ReadHeader(istream & stream)
// read the header and check for consistency
// see WriteHeader for header record structure
{
	char * str = new char[headerStrSize4 + 1];
	int numFields, *fieldSize;
	int result;
	// read the FixedLengthBufferheader
	result = FixedLengthBuffer::ReadHeader(stream);
	if (result < 0) return -1;
	// read the header string 
	stream.read(str, headerStrSize4);
	if (!stream.good()) return -1;
	if (strncmp(str, headerStr4, headerStrSize4) != 0) return -1;
	// read the record description
	stream.read((char*)&numFields, sizeof(numFields));
	if (!stream) return -1; // failed to read numFields
	fieldSize = new int[numFields];
	for (int i = 0; i < numFields; i++)
	{
		stream.read((char*)&fieldSize[i], sizeof(fieldSize[i]));
	}

	if (Initialized) // check header for consistency
	{
		if (numFields != NumFields) return -1;
		for (int j = 0; j < numFields; j++)
			if (fieldSize[j] != FieldSize[j]) return -1;
		return stream.tellg(); // everything matches
	}
	// else initialize the buffer from the header
	Init(numFields, fieldSize);
	return stream.tellg();
}

int FixedFieldBuffer::WriteHeader(ostream & stream) const
// write a buffer header to the beginning of the stream
// A header consists of the 
//	FixedLengthBufferheader	
//	FIXED			5 bytes
//	Variable sized record of length fields
//	that describes the file records
//	Header record size	2 bytes
//	number of fields		4 bytes
//	field sizes			4 bytes per field
{
	int result;
	if (!Initialized) return -1; // cannot write unitialized buffer
								 // write the parent (FixedLengthBuffer) header
	result = FixedLengthBuffer::WriteHeader(stream);
	if (!result) return -1;
	// write the header string 
	stream.write(headerStr4, headerStrSize4);
	if (!stream.good()) return -1;
	// write the record description
	//cout << "packing numfields "<<NumFields<<endl;
	stream.write((char*)&NumFields, sizeof(NumFields));
	for (int i = 0; i < NumFields; i++)
	{
		//cout << "packing fieldsize "<<FieldSize[i]<<endl;
		stream.write((char*)&FieldSize[i], sizeof(FieldSize[i]));
	}
	if (!stream) return -1;
	return stream.tellp();
}

int FixedFieldBuffer::Pack(const void * field, int size)
// set the value of the next field of the buffer;
//    if size != -1, it must be the same as the packSize
// return number of bytes packed, -1 if error
{
	//cout<<"Pack NumFields "<<NumFields<<" field "<<(char *)field<<endl;
	if (NextField == NumFields || !Packing) // buffer is full or not packing mode
		return -1;
	int start = NextByte; // first byte to be packed
	int packSize = FieldSize[NextField]; // number bytes to be packed
	if (size != -1 && packSize != size) return -1;
	memcpy(&Buffer[start], field, packSize); // move bytes to buffer
	NextByte += packSize;
	NextField++;
	if (NextField == NumFields) // all fields packed
	{
		Packing = -1;
		NextField = NextByte = 0;
	}
	return packSize;
}

int FixedFieldBuffer::Unpack(void * field, int maxBytes)
// extract the value of the next field of the buffer
// return the number of bytes extracted, -1 if error
{
	Packing = FALSE;
	if (NextField == NumFields) // buffer is full 
		return -1;
	int start = NextByte; // first byte to be unpacked
	int packSize = FieldSize[NextField]; // number bytes to be unpacked
	memcpy(field, &Buffer[start], packSize);
	NextByte += packSize;
	NextField++;
	if (NextField == NumFields) Clear(); // all fields unpacked
	return packSize;
}

void FixedFieldBuffer::Print(ostream & stream) const
{
	FixedLengthBuffer::Print(stream);
	stream << endl;
	stream << "\t max fields " << MaxFields << " and actual " << NumFields << endl;
	for (int i = 0; i < NumFields; i++)
		stream << "\tfield " << i << " size " << FieldSize[i] << endl;
	Buffer[BufferSize] = 0;
	stream << "NextByte " << NextByte << endl;
	stream << "Buffer '" << Buffer << "'" << endl;
}

int FixedFieldBuffer::Init(int maxFields)
// construct with a maximum of maxFields
{
	Clear();
	if (maxFields < 0) maxFields = 0;
	MaxFields = maxFields;
	FieldSize = new int[MaxFields];
	BufferSize = 0;
	NumFields = 0;
	return 1;
}

int FixedFieldBuffer::Init(int numFields, int * fieldSize)
// construct with fields of specific size
{
	// initialize
	Initialized = TRUE;
	Init(numFields);

	// add fields
	for (int j = 0; j < numFields; j++)
		AddField(FieldSize[j]);
	return TRUE;
}



class BufferFile
{
public:
	BufferFile(IOBuffer &);
	int Open(char * filename, int MODE);
	int Create(char * filename, int MODE);
	int Close();
	int Rewind();
	int Read(int recaddr = -1);
	int Write(int recaddr = -1);
	int Append();
	IOBuffer & GetBuffer();
protected:
	IOBuffer & Buffer;
	fstream File;
	int HeaderSize;
	int ReadHeader();
	int WriteHeader();
};

BufferFile::BufferFile(IOBuffer & from) :Buffer(from)
{}

int BufferFile::Open(char * filename, int mode)
{
	if (mode&ios::trunc) return FALSE;
	File.open(filename, ios::in);
	if (!File.good()) return FALSE;
	File.seekg(0, ios::beg);
	File.seekp(0, ios::beg);
	HeaderSize = ReadHeader();
	if (!HeaderSize) return FALSE;
	File.seekp(HeaderSize, ios::beg);
	File.seekg(HeaderSize, ios::beg);
	return File.good();
}

int BufferFile::Create(char * filename, int mode)
{
	if (!(mode&ios::out)) return FALSE;
	File.open(filename, ios::in | ios::out);
	if (!File.good())
	{
		File.close();
		return FALSE;
	}
	HeaderSize = WriteHeader();
	return HeaderSize != 0;
}

int BufferFile::Close()
{
	File.close();
	return TRUE;
}

int BufferFile::Rewind()
{
	File.seekg(HeaderSize, ios::beg);
	File.seekp(HeaderSize, ios::beg);
	return 1;
}

int BufferFile::Read(int recaddr)
{
	if (recaddr == -1)
		return Buffer.Read(File);
	else
		return Buffer.DRead(File, recaddr);
}

int BufferFile::Write(int recaddr)
{
	if (recaddr == -1)
		return Buffer.Write(File);
	else
		return Buffer.DWrite(File, recaddr);
}

int BufferFile::Append()
{
	File.seekp(0, ios::end);
	return Buffer.Write(File);
}

IOBuffer & BufferFile::GetBuffer()
{
	return Buffer;
}

int BufferFile::ReadHeader()
{
	return Buffer.ReadHeader(File);
}

int BufferFile::WriteHeader()
{
	return Buffer.WriteHeader(File);
}



template <class RecType>
class RecordFile : public BufferFile
{
public:
	int Read(RecType & record, int recaddr = -1);
	int Write(const RecType & record, int recaddr = -1);
	int Append(const RecType & record, int recaddr = -1);
	RecordFile(IOBuffer & buffer) : BufferFile(buffer) {}
};

// template method bodies
template <class RecType>
int RecordFile<RecType>::Read(RecType & record, int recaddr)
{
	int readAddr, result;
	readAddr = BufferFile::Read(recaddr);
	if (readAddr == -1) return -1;
	result = record.Unpack(Buffer);
	if (!result) return -1;
	return readAddr;
}

template <class RecType>
int RecordFile<RecType>::Write(const RecType & record, int recaddr)
{
	int result;
	result = record.Pack(Buffer);
	if (!result) return -1;
	return BufferFile::Write(recaddr);
}

template <class RecType>
int RecordFile<RecType>::Append(const RecType & record, int recaddr)
{
	int result;
	result = record.Pack(Buffer);
	if (!result) return -1;
	return BufferFile::Append();
}


class Student
{
public:
	//fields
	char id[6];
	char name[16];
	char address[11];
	char date[8];
	char grade_num[8];
	//Operations
	Student();
	static int InitBuffer(DelimFieldBuffer &);
	static int InitBuffer(LengthFieldBuffer &);
	static int InitBuffer(FixedFieldBuffer &);
	void Clear();
	int Unpack(IOBuffer &);
	int Pack(IOBuffer &) const;
	void Print(ostream &, const char * label = 0) const;
};

Student::Student() { Clear(); }

void Student::Clear()
{
	//set each field to an empty string
	id[0] = 0;
	name[0] = 0;
	address[0] = 0;
	date[0] = 0;
	grade_num[0] = 0;
}

int Student::Pack(IOBuffer & Buffer) const
{
	int numBytes;
	Buffer.Clear();
	numBytes = Buffer.Pack(id);
	if (numBytes == -1) return FALSE;
	numBytes = Buffer.Pack(name);
	if (numBytes == -1) return FALSE;
	numBytes = Buffer.Pack(address);
	if (numBytes == -1) return FALSE;
	numBytes = Buffer.Pack(date);
	if (numBytes == -1) return FALSE;
	numBytes = Buffer.Pack(grade_num);
	return TRUE;
}

int Student::Unpack(IOBuffer & Buffer)
{
	Clear();
	int numBytes;
	numBytes = Buffer.Unpack(id);
	if (numBytes == -1) return FALSE;
	id[numBytes] = 0;
	numBytes = Buffer.Unpack(name);
	if (numBytes == -1) return FALSE;
	name[numBytes] = 0;
	numBytes = Buffer.Unpack(address);
	if (numBytes == -1) return FALSE;
	address[numBytes] = 0;
	numBytes = Buffer.Unpack(date);
	if (numBytes == -1) return FALSE;
	date[numBytes] = 0;
	numBytes = Buffer.Unpack(grade_num);
	if (numBytes == -1) return FALSE;
	grade_num[numBytes] = 0;
	return TRUE;
}

int Student::InitBuffer(FixedFieldBuffer & Buffer)
{
	int result;
	result = Buffer.AddField(5);
	result = result && Buffer.AddField(15);
	result = result && Buffer.AddField(10);
	result = result && Buffer.AddField(7);
	result = result && Buffer.AddField(7);
	return result;
}

int Student::InitBuffer(DelimFieldBuffer & Buffer)
{
	return TRUE;
}

int Student::InitBuffer(LengthFieldBuffer & Buffer)
{
	return TRUE;
}

void Student::Print(ostream & stream,const char * label) const
{
	if (label == 0) stream << "Student: ";
	else stream << label;
	stream << "\n\t ID '" << id << "'\n"
		<< "\n\t Name '" << name << "'\n"
		<< "\n\t Address '" << address << "'\n"
		<< "\n\t Registration date '" << date << "'\n"
		<< "\n\t Total grade '" << grade_num << "'\n" << flush;
}


Student MaryAmes;
Student AlanMason;

template <class IOB>
void testBuffer(IOB & Buff, const char * myfile)
{
	Student student;
	int result;
	int recaddr1, recaddr2, recaddr3, recaddr4;

	Buff.Print(cout);
	ofstream TestOut(myfile, ios::out);
	result = Buff.WriteHeader(TestOut);
	cout << "write header " << result << endl;
	MaryAmes.Pack(Buff);
	Buff.Print(cout);
	recaddr1 = Buff.Write(TestOut);
	cout << "write at " << recaddr1 << endl;
	AlanMason.Pack(Buff);
	Buff.Print(cout);
	recaddr2 = Buff.Write(TestOut);
	cout << "write at " << recaddr2 << endl;
	TestOut.close();

	//test reading
	ifstream TestIn(myfile, ios::in);
	result = Buff.ReadHeader(TestIn);
	cout << "read header " << result << endl;
	Buff.DRead(TestIn, recaddr1);
	student.Unpack(Buff);
	student.Print(cout, "First record:");
	Buff.DRead(TestIn, recaddr2);
	student.Unpack(Buff);
	student.Print(cout, "Second record:");
}

void InitStudent()
{
	cout << "Initializing 2 studnets" << endl;
	strcpy_s(MaryAmes.id, "1");
	strcpy_s(MaryAmes.name, "MaryAmes");
	strcpy_s(MaryAmes.address, "NewYork");
	strcpy_s(MaryAmes.date, "110321");
	strcpy_s(MaryAmes.grade_num, "21");
	MaryAmes.Print(cout);
	strcpy_s(AlanMason.id, "2");
	strcpy_s(AlanMason.name, "AlanMason");
	strcpy_s(AlanMason.address, "LA");
	strcpy_s(AlanMason.date, "120412");
	strcpy_s(AlanMason.grade_num, "19");
	AlanMason.Print(cout);
}

void testFixedField()
{
	cout << "Testing Fixed Field Buffer" << endl;
	FixedFieldBuffer Buff(5);
	Student::InitBuffer(Buff);
	testBuffer(Buff, "fixlen.dat");
}

void testLength()
{
	cout << "\nTesting LengthTextBuffer" << endl;
	LengthFieldBuffer Buff;
	Student::InitBuffer(Buff);
	testBuffer(Buff, "length.dat");
}

void testDelim()
{
	cout << "\nTesting DelimTextBuffer" << endl;
	DelimFieldBuffer::SetDefaultDelim('|');
	DelimFieldBuffer Buff;
	Student::InitBuffer(Buff);
	testBuffer(Buff, "delim.dat");
}

int main(int argc, char ** argv)
{
	InitStudent();
	testFixedField();
	testLength();
	testDelim();
}


