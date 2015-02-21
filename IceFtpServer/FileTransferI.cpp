#include "FileTransferI.h"
#include <Ice/Application.h>

FileTransferI::FileTransferI()
{
	_dataDir = Ice::Application::communicator()->getProperties()->getPropertyWithDefault("DataDir", ".");

}


FileTransferI::~FileTransferI()
{
}

FileTransferI::FileTransferI(const Ice::CommunicatorPtr& communicator)
	//:_dataDir(communicator->getProperties()->getPropertyWithDefault("DataDir", "."))
{
}

ByteSeq
FileTransferI::read(const ::std::string& name, ::Ice::Int offset, ::Ice::Int num, const ::Ice::Current& )
{
	FILE* fp = fopen((_dataDir + "/" + name).c_str(), "rb");
	if (fp == 0)
	{
		//FileAccessException ex;
		//ex.reason = "cannot open `" + name + "' for reading: " + strerror(errno);
		Ice::Exception ex;
		throw ex;
	}

	if (fseek(fp, offset, SEEK_SET) != 0)
	{
		fclose(fp);
		return ByteSeq();
	}

	ByteSeq data(num);
	int r = fread(&data[0], 1, num, fp);
	fclose(fp);
	if (r != num)
	{
		data.resize(r);
	}

	return data;
}

void FileTransferI::write(const ::std::string& name, ::Ice::Int offset, const ::Cathy::ByteSeq& bytes, const ::Ice::Current& )
{
	FILE* fp = fopen((_dataDir + "/" + name).c_str(), "wb+");
	if (fp == 0)
	{
		Ice::Exception ex;
		throw ex;
	}

	if (fseek(fp, offset, SEEK_SET) != 0)
	{
		fclose(fp);
		return;
	}

	if (fwrite(&bytes[0], 1, bytes.size(), fp)
		!= bytes.size())
	{
		cerr << "error writing:" << strerror(errno) << endl;
	}
	fclose(fp);
}