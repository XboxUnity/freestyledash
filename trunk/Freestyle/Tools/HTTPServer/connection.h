/********************************************************************

	Connection Helper Class for Example Programs for:

	EhttpD - EasyHTTP Server/Parser C++ Class

	http://www.littletux.com

	Copyright (c) 2007, Barry Sprajc


	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS".  USE AT YOUR OWN RISK.

********************************************************************/

#include <sys/types.h>



#include <fcntl.h>
#include <stdio.h>

#include <stdlib.h>




/*
Example Usage


	// Listen on port 80
	Connection connection(1);
	if( connection.init(80) )
		exit(1)


	// Process incomming requests
	int s=0;
	while(s>-1)
		{
		int s=connection.accept();
		if( s >-1 )
			{
			// Do something with the connected socket
			//
			// 
			connection.close(s);
			}
		}

	// Prepare to exit the application
	connection.terminate();

*/

class Connection
{

   static const int BUFFER_SIZE = 256;
//   char myhostname[BUFFER_SIZE];
  // struct hostent *hp;
   int sd;
   int connect_d;
   int fromlen;
   struct sockaddr_in sin;
   struct sockaddr_in fsin;
   int debug;				
public:



	Connection(int debugprint=0);
	~Connection();

	void terminate( void );
	int init( unsigned short port  );
	int accept( void );
	int connect( char *ipAddress, unsigned short destPort );
	void close( int sock );

};

