#ifndef CATHY_FILETRANSFER_H_
#define CATHY_FILETRANSFER_H_


module Cathy
{
	sequence<byte> ByteSeq;

	interface FileTransfer
	{
		void send(int offset, ByteSeq bytes);
	};
};

#endif