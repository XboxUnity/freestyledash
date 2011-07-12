#pragma once

#include "../Generic/tools.h"
#include "../Threads/cthread.h"


class FSDFtpC : public CThread
{
public:

	FSDFtpC() {
	CreateFSDFtpC();
	}
	~FSDFtpC( void ) {}
	
	static FSDFtpC& getInstance()
	{
		static FSDFtpC singleton;
		return singleton;
	}
	string host;
	int port;
	string username;
	string password;
	string lpath;
	string rpath;

	void UploadFile( string s_Server , int d_port , string s_username , string s_password , string s_lpath , string s_rpath);
	void cancel();

private:
	bool CreateFSDFtpC( void );

protected:

	virtual unsigned long Process (void* parameter);	
};

