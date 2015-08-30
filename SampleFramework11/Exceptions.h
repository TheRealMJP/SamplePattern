//======================================================================
//
//	MJP's DX11 Sample Framework
//  http://mynameismjp.wordpress.com/
//
//  All code licensed under the MIT license
//
//======================================================================

#pragma once

#include "PCH.h"

namespace SampleFramework11
{

// Generic exeption, used as base class for other types
class Exception
{

public:

	Exception()
	{
	}

	// Specify an actual error message
	Exception (const std::wstring& exceptionMessage)
		: message(exceptionMessage)
	{
	}

	// Retrieve that error message
	const std::wstring& GetMessage() const throw()
	{
		return message;
	}

	void ShowErrorMessage() const throw ()
	{
		MessageBox(NULL, message.c_str(), L"Error", MB_OK|MB_ICONERROR);
	}

protected:

	std::wstring	message;		// The error message
};

// Exception thrown when a Win32 function fails.
class Win32Exception : public Exception
{

public:

	// Obtains a string for the specified Win32 error code
	Win32Exception (DWORD code)
		: errorCode(code)
	{
		WCHAR errorString [MAX_PATH];
		::FormatMessage(	FORMAT_MESSAGE_FROM_SYSTEM,
							0,
							errorCode,
							MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
							errorString,
							MAX_PATH,
							NULL	);

		message = L"Win32 Error: ";
		message += errorString;
	}

	// Retrieve the error code
	DWORD GetErrorCode() const throw ()
	{
		return errorCode;
	}

protected:

	DWORD	errorCode;		// The Win32 error code

};

// Exception thrown when a DirectX Function fails
class DXException : public Exception
{

public:

	// Obtains a string for the specified HRESULT error code
	DXException (HRESULT hresult)
		: errorCode(hresult)
	{
		std::wstring errorString = DXGetErrorDescriptionW(hresult);

		message = L"DirectX Error: ";
		message += errorString;
	}

	DXException (HRESULT hresult, LPCWSTR errorMsg)
		: errorCode(hresult)
	{
		message = L"DirectX Error: ";
		message += errorMsg;
	}

	// Retrieve the error code
	HRESULT GetErrorCode () const throw ()
	{
		return errorCode;
	}

protected:

	HRESULT		errorCode;		// The DX error code
};

// Exception thrown when a GDI+ function fails
class GdiPlusException : public Exception
{

public:

	// Obtains a string for the specified error code
	GdiPlusException (Gdiplus::Status status)
		: errorCode(status)
	{
		std::wstring errorString;

        if (status == Gdiplus::GenericError)
            errorString = L"Generic Error";
        else if (status == Gdiplus::InvalidParameter)
            errorString = L"Invalid Parameter";
        else if (status == Gdiplus::OutOfMemory)
            errorString = L"Out Of Memory";
        else if (status == Gdiplus::ObjectBusy)
            errorString = L"Object Busy";
        else if (status == Gdiplus::InsufficientBuffer)
            errorString = L"Insufficient Buffer";
        else if (status == Gdiplus::NotImplemented)
            errorString = L"Not Implemented";
        else if (status == Gdiplus::Win32Error)
            errorString = L"Win32 Error";
        else if (status == Gdiplus::WrongState)
            errorString = L"Wrong State";
        else if (status == Gdiplus::Aborted)
            errorString = L"Aborted";
        else if (status == Gdiplus::FileNotFound)
            errorString = L"File Not Found";
        else if (status == Gdiplus::ValueOverflow)
            errorString = L"Value Overflow";
        else if (status == Gdiplus::AccessDenied)
            errorString = L"Access Denied";
        else if (status == Gdiplus::UnknownImageFormat)
            errorString = L"Unknown Image Format";
        else if (status == Gdiplus::FontFamilyNotFound)
            errorString = L"Font Family Not Found";
        else if (status == Gdiplus::FontStyleNotFound)
            errorString = L"Font Style Not Found";
        else if (status == Gdiplus::NotTrueTypeFont)
            errorString = L"Not TrueType Font";
        else if (status == Gdiplus::UnsupportedGdiplusVersion)
            errorString = L"Unsupported GDI+ Version";
        else if (status == Gdiplus::GdiplusNotInitialized)
            errorString = L"GDI+ Not Initialized";
        else if (status == Gdiplus::PropertyNotFound)
            errorString = L"Property Not Found";
        else if (status == Gdiplus::PropertyNotSupported)
            errorString = L"Property Not Supported";

		message = L"GDI+ Error: ";
		message += errorString;
	}

	// Retrieve the error code
	Gdiplus::Status GetErrorCode () const throw ()
	{
		return errorCode;
	}

protected:

	Gdiplus::Status	errorCode;		// The GDI+ error code
};

}