#include <Ice/Ice.h>
#include <Ice/Application.h>
#include <FileTransfer.h>
#include <stdio.h>

using namespace std;
using namespace Cathy;


class FileTransferClient : public Ice::Application
{
public:
	FileTransferClient();
	virtual int run(int, char*[]);
private:
	const char* menu;
	FileTransferPrx transferPrx;
	void showMenu();
	int parsCommand(const char* strCommand);
	void Upload(const char* strFilePath);
	void Download(const char* strServerPath,const char* strClientPath);
	void Delete(const char* strServerPath);
	void List(const char* strServerPath);
};



int main(int argc, char* argv[])
{
	FileTransferClient app;
	return app.main(argc, argv, "config.client");
}

int
FileTransferClient::run(int argc, char* argv[])
{
	int status = 1;//初始状态为1；若为0，则退出；若为-1，则出错；若为1，则正常处理
	Ice::CommunicatorPtr ic;
	try {
		ic = Ice::initialize(argc, argv);
		Ice::ObjectPrx base = ic->stringToProxy("SimpleTransferID:tcp -h localhost -p 7788");
		//Ice::ObjectPrx base = ic->propertyToProxy("FileStore.Proxy");
		transferPrx = FileTransferPrx::checkedCast(base);
		if (!transferPrx)
			throw "Invalid proxy";

		//进入菜单处理部分
		char command[100];
		while (status){
			showMenu();
			cin.getline(command, 100);
			status = parsCommand(command);
		}
	}
	catch (const Ice::Exception& ex) {
		cerr << ex << endl;
		status = 1;
	}
	catch (const char* msg) {
		cerr << msg << endl;
		status = 1;
	}
	if (ic)
		ic->destroy();
	return status;
}

FileTransferClient::FileTransferClient()
{
	menu =	
		"*****************test ice client *********************** \n"
		"\tupload [LocalFilePath]\n"
		"\tdownload [ServerFilePath] [LocalFilePath]\n"
		"\tlist \\[ServerFilePath]\n"
		"\tdelete [ServerFilePath/ServerDirectoryPath] \n"
		"\tq\n"
		"******************************************************** \n";
}

void FileTransferClient::showMenu()
{
	std::cout << menu << std::endl;
	return;
}

int FileTransferClient::parsCommand(const char* strCommand)
{
	//以下四种命令
	//upload [LocalFilePath]
	//download [ServerFilePath] [LocalFilePath]
	//list 
	//q
	char strCommandCopy[100];
	strcpy(strCommandCopy, strCommand);
	char element[3][100] = { { 0 }, { 0 }, { 0 } };
	int elementNum=0;
	char *temp = strtok(strCommandCopy, " \t");
	while (temp != NULL)
	{
		if (elementNum == 3){
			cout << "Command error : element more than three!" << endl;
			break;
			//throw "throw command error : element more than three!!";
		}
		strcpy(element[elementNum], temp);
		temp = strtok(NULL, " \t");
		elementNum++;
	}

	//处理分割完的命令
	if (strcmp(element[0], "upload") == 0){
		//处理 upload [LocalFilePath]
		Upload(element[1]);
		return 1;
	}
	else if (strcmp(element[0], "download") == 0){
		//处理 download [ServerFilePath] [LocalFilePath]
		Download(element[1], element[2]);
		//test  Download("output.txt", "e:\\a.txt");
		
		return 1;
	}
	else if (strcmp(element[0], "list") == 0){
		//处理 list [ServerFilePath]
		List(element[1]); 
		return 1;
	}
	else if (strcmp(element[0], "delete") == 0){
		//处理 delete [ServerFilePath/ServerDirectoryPath]
		Delete(element[1]);
		return 1;
	}
	else if (strcmp(element[0], "q") == 0){
		//处理 q
		return 0;
	}
	else{
		cout<< "unknown command!"<<endl;
		return -1;
	}

}

void FileTransferClient::Upload(const char* strFilePath)
{
	//string output = "output.txt";

	//FILE *wrfl = fopen(output.c_str(), "w+");
	//fwrite("asdfasd", 1, 7, wrfl);
	//fclose(wrfl);
	
	//FILE* file = fopen(output.c_str(), "rb");
	FILE* file = fopen(strFilePath, "rb");

	if (file == NULL)             /*判断文件是否打开成功*/
		cout << "File open error"<<endl;

	const int chunkSize = 10 * 1024;
	Ice::Int offset = 0;

	list<Ice::AsyncResultPtr> results;
	const int numRequests = 5;

	//解析文件名
	string strFileName=strFilePath;	// Remove directory if present.
	const size_t last_slash_idx = strFileName.find_last_of("\\/");
	if (std::string::npos != last_slash_idx)
	{
		strFileName.erase(0, last_slash_idx + 1);
	}

	while (!feof(file))
	{
		ByteSeq bs(chunkSize);
		fread(&bs[0], 1, chunkSize, file);

		// Send up to numRequests + 1 chunks asynchronously.
		Ice::AsyncResultPtr r = transferPrx->begin_write(strFileName, offset, bs);//让服务端写，即上传
		offset += bs.size();

		// Wait until this request has been passed to the transport.
		r->waitForSent();
		results.push_back(r);

		// Once there are more than numRequests, wait for the least
		// recent one to complete.
		while (results.size() > numRequests) {
			Ice::AsyncResultPtr r = results.front();
			results.pop_front();
			r->waitForCompleted();
		}
	}

	fclose(file);

	// Wait for any remaining requests to complete.
	while (!results.empty()) {
		Ice::AsyncResultPtr r = results.front();
		results.pop_front();
		r->waitForCompleted();
	}

}

void FileTransferClient::Download(const char* strServerPath, const char* strClientPath)
{
	FILE* file = fopen(strClientPath, "a+b");

	if (file == NULL)             /*判断文件是否打开成功*/
		cout << "File open error" << endl;

	const int chunkSize = 10 * 1024;
	Ice::Int offset = 0;

	list<Ice::AsyncResultPtr> results;
	const int numRequests = 5;
	while (1)
	{
		ByteSeq bs;
		//read from server
		Ice::AsyncResultPtr r = transferPrx->begin_read(strServerPath, offset, chunkSize);
		offset += chunkSize;
		r->waitForSent();
		results.push_back(r);
		if (results.size() > numRequests) {
			Ice::AsyncResultPtr rout = results.front();
			results.pop_front();
			rout->waitForCompleted();
			bs = transferPrx->end_read(rout);
			fwrite(&bs[0], 1, bs.size(), file);
			if (bs.size() < chunkSize){
				break;
			}
		}
	}
	fclose(file);
	// Wait for any remaining requests to complete.
	while (!results.empty()) {
		Ice::AsyncResultPtr r = results.front();
		results.pop_front();
	}
}

void FileTransferClient::Delete(const char* strServerPath)
{
	if (transferPrx->remove(strServerPath))
		return;
	else
		cout << "failed to delete file or directory!"<<endl;
}

void FileTransferClient::List(const char* strServerPath)
{
	ByteSeq listContent;
	listContent = transferPrx->list(strServerPath);
	//输出
	int i;
	for (i = 0; i < listContent.size();i++)
		cout << listContent[i];
}
