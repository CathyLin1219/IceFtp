#pragma once

#include <FileTransfer.h>

using namespace Cathy;
using namespace std;

class FileTransferI : public FileTransfer
{
public:
	FileTransferI();
	FileTransferI(const Ice::CommunicatorPtr& communicator);
	~FileTransferI();

	virtual ::Cathy::ByteSeq read(const ::std::string&, ::Ice::Int, ::Ice::Int, const ::Ice::Current& = ::Ice::Current());
	virtual void write(const ::std::string&, ::Ice::Int, const ::Cathy::ByteSeq&, const ::Ice::Current& = ::Ice::Current());

private:
	::std::string _dataDir;

};

