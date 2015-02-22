#include <Ice/Application.h>
#include <boost/filesystem.hpp>
#include "FileTransferI.h"

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

::Cathy::ByteSeq
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

void
FileTransferI::write(const ::std::string& name, ::Ice::Int offset, const ::Cathy::ByteSeq& bytes, const ::Ice::Current& )
{
	FILE* fp = fopen((_dataDir + "/" + name).c_str(), "a+b");
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

bool
FileTransferI::remove(const ::std::string& name, const ::Ice::Current&)
{
	if (std::remove((_dataDir + "/" + name).c_str()) == 0) return true;
	else return false;
}


/////////////////////////////////////////////////////////
//	list功能——使用Boost::filesystem
//////////////////////////////////////////////////////////
//::Cathy::ByteSeq
//FileTransferI::list(const ::std::string& path, const ::Ice::Current&)
//{
//	namespace bf = boost::filesystem;//简单别名
//
//	std:: string strInfo;
//	int offset = 0;
//	bf::path listPath(path);
//	listPath = _dataDir / listPath;
//	if (!bf::exists(listPath))
//	{
//		strInfo = path + "not exist, please try again.\n";
//	}
//	else if (bf::is_directory(listPath))
//	{
//		strInfo += "In directory : " + path + "\n";
//		bf::directory_iterator end_iter;
//		for (bf::directory_iterator dir_itr(listPath);
//			dir_itr != end_iter;
//			++dir_itr)
//		{
//			if (bf::is_directory(dir_itr->status()))
//			{
//				strInfo += dir_itr->path().filename().string() + " [directory]\n";
//			}
//			else if (bf::is_regular_file(dir_itr->status()))
//			{
//				strInfo += dir_itr->path().filename().string() + " [file]\n";
//			}
//			else
//			{
//				strInfo += dir_itr->path().filename().string() + " [other]\n";
//			}
//		}
//	}
//	else // must be a file
//	{
//		strInfo += listPath.filename().string() + "  not exist,please try again.\n";
//	}
//
//	int num = 1024;
//	ByteSeq data(num);
//	memcpy(&data[0], strInfo.c_str(), strInfo.size());
//	data.resize(strInfo.size());
//	return data;
//}

/////////////////////////////////////////////////////////
//	list功能——使用命令行
//////////////////////////////////////////////////////////
::Cathy::ByteSeq
FileTransferI::list(const ::std::string& path, const ::Ice::Current&)
{
	const char* tmpFileName = "../tmp/ls";
	boost::filesystem::path listPath(path);
	listPath = "..\\data" / listPath;
	string strCommand = "dir " + listPath.string() + " > " + tmpFileName;
	system(strCommand.c_str());
	FILE* lsFile = fopen(tmpFileName, "r");
	int num = 1024;
	ByteSeq data(num);
	std:: string strInfo;
	num = fread(&data[0], 1, num, lsFile);
	data.resize(num);
	fclose(lsFile);
	return data;

}