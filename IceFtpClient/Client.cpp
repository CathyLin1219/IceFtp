#include <Ice/Ice.h>
#include <Ice/Application.h>
#include <FileTransfer.h>
#include <stdio.h>

using namespace std;
using namespace Cathy;

class FileTransferClient : public Ice::Application
{
public:
	virtual int run(int, char*[]);
};



int main(int argc, char* argv[])
{
	FileTransferClient app;
	return app.main(argc, argv, "config.client");
}

int
FileTransferClient::run(int argc, char* argv[])
{
	int status = 0;
	Ice::CommunicatorPtr ic;
	try {
		ic = Ice::initialize(argc, argv);
		Ice::ObjectPrx base = ic->stringToProxy(ic->getProperties()->getProperty("FileStore.Proxy"));
		FileTransferPrx transferPrx = FileTransferPrx::checkedCast(base);
		if (!transferPrx)
			throw "Invalid proxy";

		//FileHandle file = open(...);
		string output = "output.txt";
		FILE* file = fopen(output.c_str(), "wb");
		const int chunkSize = 1000 * 1024;
		Ice::Int offset = 0;

		list<Ice::AsyncResultPtr> results;
		const int numRequests = 5;

		while (!feof(file))
		{
			ByteSeq bs;
			//bs = file.read(chunkSize);
			fread(&bs[0], 1, chunkSize, file);

			// Send up to numRequests + 1 chunks asynchronously.
			//Ice::AsyncResultPtr r = transferPrx->begin_send(offset, bs);
			Ice::AsyncResultPtr r = transferPrx->begin_write(output, offset, bs);//让服务端写，即上传
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

		// Wait for any remaining requests to complete.
		while (!results.empty()) {
			Ice::AsyncResultPtr r = results.front();
			results.pop_front();
			r->waitForCompleted();
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