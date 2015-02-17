#ifndef CATHY_FILETRANSFER_H_
#define CATHY_FILETRANSFER_H_

module Cathy
{
	sequence<byte> ByteSeq;

	interface FileTransfer
	{
		ByteSeq read( string name,int  offset,int num);
		void write(string name,int offset,ByteSeq bytes);
		//void send(int offset, ByteSeq bytes);
	};
};

#endif