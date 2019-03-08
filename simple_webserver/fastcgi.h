#pragma once

#ifndef FAST_CGI_H
#define FAST_CGI_H

#include <memory>

#pragma warning(disable : 4200)

#define FCGI_VERSION 1

/*
#define FCGI_BEGIN_REQUEST       1
#define FCGI_ABORT_REQUEST       2
#define FCGI_END_REQUEST         3
#define FCGI_PARAMS              4
#define FCGI_STDIN               5
#define FCGI_STDOUT              6
#define FCGI_STDERR              7
#define FCGI_DATA                8
#define FCGI_GET_VALUES          9
#define FCGI_GET_VALUES_RESULT  10
#define FCGI_UNKNOWN_TYPE       11
#define FCGI_MAXTYPE (FCGI_UNKNOWN_TYPE)

#define FCGI_RESPONDER  1           //role
#define FCGI_AUTHORIZER 2
#define FCGI_FILTER     3

#define FCGI_KEEP_CONN  1

#define FCGI_REQUEST_COMPLETE 0
#define FCGI_CANT_MPX_CONN    1
#define FCGI_OVERLOADED       2
#define FCGI_UNKNOWN_ROLE     3
*/

enum FCGI_Request_Type
{
	FCGI_BEGIN_REQUEST = 1,
	FCGI_ABORT_REQUEST = 2,
	FCGI_END_REQUEST = 3,
	FCGI_PARAMS = 4,
	FCGI_STDIN = 5,
	FCGI_STDOUT = 6,
	FCGI_STDERR = 7,
	FCGI_DATA = 8,
	FCGI_GET_VALUES = 9,
	FCGI_GET_VALUES_RESULT = 10,
	FCGI_UNKNOWN_TYPE = 11
};

enum FCGI_Role
{
	FCGI_RESPONDER = 1,
	FCGI_AUTHORIZER = 2,
	FCGI_FILTER = 3
};


struct FCGI_Header
{
	unsigned char version;
	unsigned char type;
	unsigned char requestID1;       //high 8 bits
	unsigned char requestID0;       //low
	unsigned char contentLength1;   //
	unsigned char contentLength0;
	unsigned char paddingLength;
	unsigned char reserved;
};

struct FCGI_BeginRequestBody
{
	unsigned char role1;
	unsigned char role0;
	unsigned char flags;            //whether close socket    flags&FCGI_KEEP_CONN = 0 close
	unsigned char reserved[5];
};

struct FCGI_EndRequestBody
{
	unsigned char appStatus3;
	unsigned char appStatus2;
	unsigned char appStatus1;
	unsigned char appStatus0;
	unsigned char protocolStatus;
	unsigned char reserved[3];
};

struct FCGI_BeginRequest
{
	FCGI_Header header;
	FCGI_BeginRequestBody body;
};

struct FCGI_Params
{
	FCGI_Header header;
	unsigned char keyLength;
	unsigned char valueLength;
	unsigned char data[0];
};

FCGI_Header makeHeader(int type, int requestID, int contentLength, int paddingLength);

FCGI_BeginRequestBody makeBeginRequestBody(int role);

#endif // !FAST_CGI_H


