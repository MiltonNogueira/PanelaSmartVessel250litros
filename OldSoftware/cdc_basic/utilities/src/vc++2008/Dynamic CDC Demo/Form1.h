/********************************************************************
  File Information:
    FileName:     	Form1.h
    Dependencies:	Requires .NET framework redistributable 2.0 or later
					to run executable.  During development, also needs setupapi.h 
					header file.  This is included in the Windows Driver Kit (WDK).
					Please download and install the WDK, and add the appropriate
					header include paths to your VC++ IDE, if encountering
					build errors.
    Processor:		x86, x64
    Hardware:		Personal Computer
    Complier:  	    Microsoft Visual C++ 2008 Express edition (or higher/better)
    Company:		Microchip Technology, Inc.
    
    Software License Agreement:
    
    The software supplied herewith by Microchip Technology Incorporated
    (the "Company") for its PIC® Microcontroller is intended and
    supplied to you, the Company's customer, for use solely and
    exclusively on Microchip PIC Microcontroller products. The
    software is owned by the Company and/or its supplier, and is
    protected under applicable copyright laws. All rights are reserved.
    Any use in violation of the foregoing restrictions may subject the
    user to criminal sanctions under applicable laws, as well as to
    civil liability for the breach of the terms and conditions of this
    license.
    
    THIS SOFTWARE IS PROVIDED IN AN "AS IS" CONDITION. NO WARRANTIES,
    WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
    TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
    PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
    IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
    CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.



  Description:
    Example PC application demo showing how to connect to and communicate
	with a USB virtual COM port.  This demo uses dynamic concepts to
	find the COMx number based on the USB VID/PID, thus simplying the
	usage for the end user (human doesn't need to know the COMx number
	that was assigned for the device).


********************************************************************
 File Description:

 Change History:
  Rev    Description
  ----   -----------
  2.9	 Initial creation of this demo.  Started code from the 
		 "VC++ 2005 Simple CDC Demo" in the MCHPFSUSB Framework v2.8.
  2.12	 Updated to allow finding/connecting to CDC functions that may be
         part of a composite USB device (on Win 7 and later).
********************************************************************/


//Here are some useful articles when creating new PC applications for COM ports:
//(links valid as of June 3, 2008)
//
//"SerialPort Class"
//http://msdn.microsoft.com/en-us/library/system.io.ports.serialport.aspx
//
//"SerialPort Members"
//http://msdn.microsoft.com/en-us/library/system.io.ports.serialport_members.aspx
//
//"SerialPort.DataReceived Event"
//http://msdn.microsoft.com/en-us/library/system.io.ports.serialport.datareceived.aspx
//
//"How to: Make Thread-Safe Calls to Windows Forms Controls"
//http://msdn.microsoft.com/en-us/library/ms171728(VS.80).aspx

#pragma once


//-----------------------------------------------------------------------------------------------------------------------
//Modify the below values to match the VID and PID and other information from your USB device descriptor (in the USB firmware on the microcontroller).
//These values are needed to "find" the COMx number provided by the USB device.
//If the human user already knows the proper COMx number, then it is not required to know
//the VID/PID.  If the human user doesn't know the COMx number, then it is possible for
//this software to find the COMx number dynamically, by using the VID/PID (and possibly more info, if device is composite) of the USB device.

//Use below values when connecting to the CDC basic and CDC serial emulator demos in the Microchip Libraries for Applications
#define USB_VENDOR_ID							0x04D8	//The 16-bit Product ID used in the USB device firmware (see device descriptor)
#define USB_PRODUCT_ID							0x000A	//The 16-bit Vendor ID used in the USB device firmware (see device descriptor)
#define USB_DEVICE_INSTANCE						0		//Always use "0", unless there can be multiple instances of your hardware device attached to the same system simultaneously.  In this case, the instance index starts at 0, and increases for each device currently attached to the system (with matching VID/PID).
#define USB_IS_DEVICE_COMPOSITE					false	//Use "false" if the CDC "function" is not part of a composite USB device.  If the device is composite (ex: MSD + CDC), then use "true".
#define USB_COMPOSITE_INTERFACE_INDEX			0		//Irrelevant when USB_DEVICE_IS_COMPOSITE is false.  Required when USB_DEVICE_IS_COMPOSITE is true.  This should be set to the interface index of the first CDC related interface in the device (ex: must match USB descriptors from the firmware)

////Use below values when connecting to the composite MSD + CDC demo in the Microchip Libraries for Applications
//#define USB_VENDOR_ID						0x04D8	//The 16-bit Product ID used in the USB device firmware (see device descriptor)
//#define USB_PRODUCT_ID					0x0057	//The 16-bit Vendor ID used in the USB device firmware (see device descriptor)
//#define USB_DEVICE_INSTANCE				0		//Always use "0", unless there can be multiple instances of your hardware device attached to the same system simultaneously.  In this case, the instance index starts at 0, and increases for each device currently attached to the system (with matching VID/PID).
//#define USB_IS_DEVICE_COMPOSITE			true	//Use "false" if the CDC "function" is not part of a composite USB device.  If the device is composite (ex: MSD + CDC), then use "true".
//#define USB_COMPOSITE_INTERFACE_INDEX		1		//Irrelevant when USB_DEVICE_IS_COMPOSITE is false.  Required when USB_DEVICE_IS_COMPOSITE is true.  This should be set to the interface index of the first CDC related interface in the device (ex: must match USB descriptors from the firmware)
//-----------------------------------------------------------------------------------------------------------------------


#include <windows.h>	//Defines datatypes and other things needed for using the setupapi.dll functions.
#include <setupapi.h>	//Definitions useful for using the setupapi.dll functions.




namespace VCCDC {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Security::Permissions;
	using namespace Microsoft::Win32;
	using namespace System::Threading;
	using namespace System::Runtime::InteropServices;  //Need this to support "unmanaged" code.

	#ifdef UNICODE
	#define	Seeifdef	Unicode
	#else
	#define Seeifdef	Ansi
	#endif

	//Returns a HDEVINFO type for a device information set (USB devices in
	//our case).  We will need the HDEVINFO as in input parameter for calling many of
	//the other SetupDixxx() functions.
	[DllImport("setupapi.dll" , CharSet = CharSet::Seeifdef, EntryPoint="SetupDiGetClassDevs", CallingConvention=CallingConvention::Winapi)]		
	extern "C" HDEVINFO  SetupDiGetClassDevsUM(
		LPGUID  ClassGuid,					//Input: Supply the class GUID here. 
		PCTSTR  Enumerator,					//Input: Use NULL here, not important for our purposes
		HWND  hwndParent,					//Input: Use NULL here, not important for our purposes
		DWORD  Flags);						//Input: Flags describing what kind of filtering to use.

	//Gives us "PSP_DEVICE_INTERFACE_DATA" which contains the Interface specific GUID (different
	//from class GUID).  We need the interface GUID to get the device path.
	[DllImport("setupapi.dll" , CharSet = CharSet::Seeifdef, EntryPoint="SetupDiEnumDeviceInterfaces", CallingConvention=CallingConvention::Winapi)]				
	extern "C" WINSETUPAPI BOOL WINAPI  SetupDiEnumDeviceInterfacesUM(
		HDEVINFO  DeviceInfoSet,			//Input: Give it the HDEVINFO we got from SetupDiGetClassDevs()
		PSP_DEVINFO_DATA  DeviceInfoData,	//Input (optional)
		LPGUID  InterfaceClassGuid,			//Input 
		DWORD  MemberIndex,					//Input: "Index" of the device you are interested in getting the path for.
		PSP_DEVICE_INTERFACE_DATA  DeviceInterfaceData);//Output: This function fills in an "SP_DEVICE_INTERFACE_DATA" structure.

	//SetupDiDestroyDeviceInfoList() frees up memory by destroying a DeviceInfoList
	[DllImport("setupapi.dll" , CharSet = CharSet::Seeifdef, EntryPoint="SetupDiDestroyDeviceInfoList", CallingConvention=CallingConvention::Winapi)]
	extern "C" WINSETUPAPI BOOL WINAPI  SetupDiDestroyDeviceInfoListUM(			
		HDEVINFO  DeviceInfoSet);			//Input: Give it a handle to a device info list to deallocate from RAM.

	//SetupDiEnumDeviceInfo() fills in an "SP_DEVINFO_DATA" structure, which we need for SetupDiGetDeviceRegistryProperty()
	[DllImport("setupapi.dll" , CharSet = CharSet::Seeifdef, EntryPoint="SetupDiEnumDeviceInfo", CallingConvention=CallingConvention::Winapi)]
	extern "C" WINSETUPAPI BOOL WINAPI  SetupDiEnumDeviceInfoUM(
		HDEVINFO  DeviceInfoSet,
		DWORD  MemberIndex,
		PSP_DEVINFO_DATA  DeviceInfoData);

	//SetupDiGetDeviceRegistryProperty() gives us the hardware ID, which we use to check to see if it has matching VID/PID
	[DllImport("setupapi.dll" , CharSet = CharSet::Seeifdef, EntryPoint="SetupDiGetDeviceRegistryProperty", CallingConvention=CallingConvention::Winapi)]
	extern "C"	WINSETUPAPI BOOL WINAPI  SetupDiGetDeviceRegistryPropertyUM(
		HDEVINFO  DeviceInfoSet,
		PSP_DEVINFO_DATA  DeviceInfoData,
		DWORD  Property,
		PDWORD  PropertyRegDataType,
		PBYTE  PropertyBuffer,   
		DWORD  PropertyBufferSize,  
		PDWORD  RequiredSize);

	//SetupDiGetDeviceInterfaceDetail() gives us a device path, which is needed before CreateFile() can be used.
	[DllImport("setupapi.dll" , CharSet = CharSet::Seeifdef, EntryPoint="SetupDiGetDeviceInterfaceDetail", CallingConvention=CallingConvention::Winapi)]
	extern "C" BOOL SetupDiGetDeviceInterfaceDetailUM(
		HDEVINFO DeviceInfoSet,										//Input: Wants HDEVINFO which can be obtained from SetupDiGetClassDevs()
		PSP_DEVICE_INTERFACE_DATA DeviceInterfaceData,				//Input: Pointer to an structure which defines the device interface.  
		PSP_DEVICE_INTERFACE_DETAIL_DATA DeviceInterfaceDetailData,	//Output: Pointer to a strucutre, which will contain the device path.
		DWORD DeviceInterfaceDetailDataSize,						//Input: Number of bytes to retrieve.
		PDWORD RequiredSize,										//Output (optional): Te number of bytes needed to hold the entire struct 
		PSP_DEVINFO_DATA DeviceInfoData);							//Output

	//Note: This function is only supported in Windows Server 2003 and later (not implemented in XP).
	//It is uncertain if this function could be used to re-initialize a "Status: Attached but broken." device.
	[DllImport("setupapi.dll" , CharSet = CharSet::Seeifdef, EntryPoint="SetupDiRestartDevices", CallingConvention=CallingConvention::Winapi)]					
	extern "C" BOOL WINAPI SetupDiRestartDevices(
    HDEVINFO  DeviceInfoSet,
    PSP_DEVINFO_DATA  DeviceInfoData);

	[DllImport("setupapi.dll" , CharSet = CharSet::Seeifdef, EntryPoint="SetupDiGetCustomDeviceProperty", CallingConvention=CallingConvention::Winapi)]
	extern "C" BOOL SetupDiGetCustomDevicePropertyUM(
		HDEVINFO DeviceInfoSet,
		PSP_DEVINFO_DATA DeviceInfoData,
		PCTSTR CustomPropertyName,
		DWORD Flags,PDWORD PropertyRegDataType,
		PBYTE PropertyBuffer,DWORD PropertyBufferSize,
		PDWORD RequiredSize);

	[DllImport("setupapi.dll" , CharSet = CharSet::Seeifdef, EntryPoint="SetupDiGetDeviceProperty", CallingConvention=CallingConvention::Winapi)]
	extern "C" BOOL SetupDiGetDevicePropertyUM(
		HDEVINFO  DeviceInfoSet,
		PSP_DEVINFO_DATA  DeviceInfoData,
		CONST DEVPROPKEY  *PropertyKey,
		DEVPROPTYPE  *PropertyType,
		PBYTE  PropertyBuffer, OPTIONAL
		DWORD  PropertyBufferSize,
		PDWORD  RequiredSize, OPTIONAL
		DWORD  Flags);

	//Note: To use the SetupDiGetDeviceProperty(), one needs some custom types, which can be obtained from: 
	//#include <devpkey.h>	//However, this header is from the Windows Driver Kit (WDK).  If you don't want 
	//to download/install the WDK, you can just use this subset of definitions instead.
	#ifndef DEVPROPKEY_DEFINED
	#define DEVPROPKEY_DEFINED

	typedef GUID  DEVPROPGUID, *PDEVPROPGUID;
	typedef ULONG DEVPROPID,   *PDEVPROPID;

	typedef struct _DEVPROPKEY {
		DEVPROPGUID fmtid;
		DEVPROPID   pid;
	} DEVPROPKEY, *PDEVPROPKEY;
	#endif // DEVPROPKEY_DEFINED




	//-------------------------------------------
	//Global variables
	//-------------------------------------------
	BOOL DeviceAttachedState = FALSE;		//Used for hardware state tracking.
	BOOL OldDeviceAttachedState = FALSE;	//Used for hardware state tracking.



	/// <summary>
	/// Summary for Form1
	///
	/// WARNING: If you change the name of this class, you will need to change the
	///          'Resource File Name' property for the managed resource compiler tool
	///          associated with all .resx files this class depends on.  Otherwise,
	///          the designers will not be able to interact properly with localized
	///          resources associated with this form.
	/// </summary>


	public ref class Form1 : public System::Windows::Forms::Form
	{
		//Create a delegate function for this thread that will take
        //  in a string and will write it to the txtDataReceived textbox

		delegate void SetTextCallback(String^ text);

		public:
		/****************************************************************************
			Function:
				public Form1()

			Summary:
				The main contructor for the Form1 class.

			Description:
				The main contructor for the Form1 class.  This function creates and
				initializes all of the form objects.

			Precondition:
				None

			Parameters:
				None

			Return Values:
				None

			Remarks:
				None
		***************************************************************************/
		Form1(void)
		{
			InitializeComponent();
		}

	protected:
		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		~Form1()
		{
			if (components)
			{
				delete components;
			}
		}

	protected: 
	private: System::IO::Ports::SerialPort^  serialPort1;

	private: System::Windows::Forms::Button^  btnSendData;
	private: System::Windows::Forms::TextBox^  txtSendData;
	private: System::Windows::Forms::Timer^  timer1;
	private: System::Windows::Forms::TextBox^  txtDataReceived;
	private: System::Windows::Forms::Label^  COMStatus_lbl;

	private: System::ComponentModel::IContainer^  components;

	private:
		/// <summary>
		/// Required designer variable.
		/// </summary>


#pragma region Windows Form Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify
		/// the contents of this method with the code editor.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			this->txtDataReceived = (gcnew System::Windows::Forms::TextBox());
			this->serialPort1 = (gcnew System::IO::Ports::SerialPort(this->components));
			this->btnSendData = (gcnew System::Windows::Forms::Button());
			this->txtSendData = (gcnew System::Windows::Forms::TextBox());
			this->timer1 = (gcnew System::Windows::Forms::Timer(this->components));
			this->COMStatus_lbl = (gcnew System::Windows::Forms::Label());
			this->SuspendLayout();
			// 
			// txtDataReceived
			// 
			this->txtDataReceived->Location = System::Drawing::Point(12, 66);
			this->txtDataReceived->Multiline = true;
			this->txtDataReceived->Name = L"txtDataReceived";
			this->txtDataReceived->ScrollBars = System::Windows::Forms::ScrollBars::Vertical;
			this->txtDataReceived->Size = System::Drawing::Size(522, 155);
			this->txtDataReceived->TabIndex = 0;
			// 
			// serialPort1
			// 
			this->serialPort1->DataReceived += gcnew System::IO::Ports::SerialDataReceivedEventHandler(this, &Form1::serialPort1_DataReceived);
			// 
			// btnSendData
			// 
			this->btnSendData->Location = System::Drawing::Point(12, 39);
			this->btnSendData->Name = L"btnSendData";
			this->btnSendData->Size = System::Drawing::Size(76, 21);
			this->btnSendData->TabIndex = 2;
			this->btnSendData->Text = L"Send Data";
			this->btnSendData->UseVisualStyleBackColor = true;
			this->btnSendData->Click += gcnew System::EventHandler(this, &Form1::btnSendData_Click);
			// 
			// txtSendData
			// 
			this->txtSendData->Location = System::Drawing::Point(94, 40);
			this->txtSendData->Name = L"txtSendData";
			this->txtSendData->Size = System::Drawing::Size(440, 20);
			this->txtSendData->TabIndex = 3;
			// 
			// timer1
			// 
			this->timer1->Enabled = true;
			this->timer1->Interval = 30;
			this->timer1->Tick += gcnew System::EventHandler(this, &Form1::timer1_Tick);
			// 
			// COMStatus_lbl
			// 
			this->COMStatus_lbl->AutoSize = true;
			this->COMStatus_lbl->Location = System::Drawing::Point(12, 9);
			this->COMStatus_lbl->Name = L"COMStatus_lbl";
			this->COMStatus_lbl->Size = System::Drawing::Size(167, 13);
			this->COMStatus_lbl->TabIndex = 6;
			this->COMStatus_lbl->Text = L"Status: Unknown or Not Attached";
			// 
			// Form1
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(546, 236);
			this->Controls->Add(this->COMStatus_lbl);
			this->Controls->Add(this->txtSendData);
			this->Controls->Add(this->btnSendData);
			this->Controls->Add(this->txtDataReceived);
			this->Name = L"Form1";
			this->Text = L"Dynamic COM port example";
			this->ResumeLayout(false);
			this->PerformLayout();

		}
#pragma endregion


		/****************************************************************************
			Function:
				private void timer1_Tick(object sender, EventArgs e)

			Summary:
				This function periodically runs and checks if the USB virtual COM port
				devices attached.  If so, it connects to it.  If not, it closes
				the COM port.

			Description:
				This function periodically runs and checks if the USB virtual COM port
				devices attached.  If so, it connects to it.  If not, it closes
				the COM port.

			Precondition:
				None

			Parameters:
				object sender     - Sender of the event (this form)
				EventArgs e       - The event arguments

			Return Values:
				None

			Remarks:
				None
		***************************************************************************/
		private: System::Void timer1_Tick(System::Object^  sender, System::EventArgs^  e) 
		{
			String^ COMNumberString = "";

			//Check if the device is currently attached to the system, and if so, get the "COMxx" string
			//associated with the device (using the VID/PID and other info).
			COMNumberString = USB_CheckIfPresentAndGetCOMString(USB_VENDOR_ID, USB_PRODUCT_ID, USB_DEVICE_INSTANCE, USB_IS_DEVICE_COMPOSITE, USB_COMPOSITE_INTERFACE_INDEX);
			if(COMNumberString == "")
			{
				DeviceAttachedState = FALSE;
				COMStatus_lbl->Text = "Status: Unknown or Not Attached";
			}
			else
			{
				DeviceAttachedState = TRUE;
			}

			//Check if we are switching from "was not attached" to "is now attached" state.
			if((OldDeviceAttachedState != DeviceAttachedState) && (DeviceAttachedState == TRUE))
			{
				try
				{
					//Set the name of the com port we are trying to attach to (ex: "COM4")
					serialPort1->PortName = COMNumberString;

					//Try to open the COM port.
					serialPort1->Open();

					//Enable user interface to the device.
					btnSendData->Enabled = TRUE;
					COMStatus_lbl->Text = "Status: Attached as " + COMNumberString;
				}
				catch(...)
				{
					//If there was an exception, then close the handle to 
					//  the device and assume that the device was removed
					//btnClose_Click(this, gcnew EventArgs());
					OldDeviceAttachedState = TRUE;
					DeviceAttachedState = FALSE;
					try
					{
						//Close the COM port
						serialPort1->Close();
					}
					catch(...){}
					COMStatus_lbl->Text = "Status: Attached but broken.  Please unplug device, wait a few seconds, and re-attach device.";
				}
			}
			
			//Check if we are switching from "was attached" to "not attached anymore" state.
			if((OldDeviceAttachedState != DeviceAttachedState) && (DeviceAttachedState == FALSE))
			{
				//Reset the state of the application objects
				btnSendData->Enabled = FALSE;


				//This section of code will try to close the COM port.
				//  Please note that it is important to use a try/catch
				//  statement when closing the COM port.  If a USB virtual
				//  COM port is removed and the PC software tries to close
				//  the COM port before it detects its removal then
				//  an exeception is thrown.  If the execption is not in a
				//  try/catch statement this could result in the application
				//  crashing.
				try
				{
					//Dispose the In and Out buffers;
					serialPort1->DiscardInBuffer();
					serialPort1->DiscardOutBuffer();
				}
				//If there was an exeception then there isn't much we can
				//  do.  The port is no longer available.
				catch(...){}

				try
				{
					//Close the COM port
					serialPort1->Close();
				}
				catch(...){}
			}

			OldDeviceAttachedState = DeviceAttachedState;
		}




		/****************************************************************************
			Function:
				String^	USB_CheckIfPresentAndGetCOMString(WORD USBVendorID, WORD USBProductID, DWORD deviceInstanceIndex, bool deviceIsComposite, BYTE compositeInterfaceIndex)

			Summary:
				This function checks if a USB CDC device is currently plugged in with 
				a matching USB VID and PID string.  If a matching device is found, it
				returns the COMx number string associated with the USB device.

			Description:
				This function checks if a USB CDC device is currently plugged in with 
				a matching USB VID and PID string.  If a matching device is found, it
				returns the COMx number string associated with the USB device.

			Precondition:
				None

			Parameters:
				WORD USBVendorID - The 16-bit Vendor ID (VID) of the USB device to try to find the COMx port number for
				WORD USBProductID - The 16-bit Product ID (PID) of the USB device to try to find the COMx port number for
				DWORD deviceInstanceIndex - The "instance index" of the USB device.  Always use '0', unless there are more than one instance of the hardware attached to the same computer simultaneously.
				bool deviceIsComposite - Use "true" if the CDC function is part of a composite USB device, otherwise use "false"
				BYTE compositeInterfaceIndex - The CDC interface index number, when the device is a composite USB device.  Value is irrelevant and may be '0' when deviceIsComposite is false.

			Return Values:
				String^ containing the "COMxx" string number (useful when trying to open the COM port).
						If no USB CDC COMx ports are currently provided by a USB with matching VID/PID,
						then the return value is: ""

			Remarks:
				This function is passive and does not attempt to open/close any COMx ports.  All it
				does is search for attached USB devices, that are currently providing COMx ports.
				
				This function is currently written to work only with a single instance of the USB device
				attached to the computer, with the given VID/PID.  If more than one device of matching
				VID/PID is simultaneously attached to the machine, then more than one COMx port number
				will exist (one for each CDC device).  This function will only return the first COMx
				number that it finds for the matching VID/PID.  If more than one exists, the additional
				COMx numbers will remain unknown.  To expand the functionality of this function, it would
				be possible to remove the "return COMString;" line of code, and instead keep looping
				until the "if(ErrorStatus == ERROR_NO_MORE_ITEMS)" exit condition is hit instead.
				In this case, the loop will find multiple COMx numbers and instances, and these
				could be stored and returned in an array of strings instead.
		***************************************************************************/
String^	USB_CheckIfPresentAndGetCOMString(WORD USBVendorID, WORD USBProductID, DWORD deviceInstanceIndex, bool deviceIsComposite, BYTE compositeInterfaceIndex)
{
		//Globally Unique Identifier (GUID) for COM port devices.  Windows uses GUIDs to identify things.  GUIDs are a structure containing 128-bits worth of numbers.
		GUID InterfaceClassGuid = {0xa5dcbf10, 0x6530, 0x11d2, 0x90, 0x1f, 0x00, 0xc0, 0x4f, 0xb9, 0x51, 0xed};					//GUID_DEVINTERFACE_USB_DEVICE (common for any USB device) - used to find the USB device
		DEVPROPKEY devPropKeyContainerID = {{0x8c7ed206, 0x3f8a, 0x4827, 0xb3, 0xab, 0xae, 0x9e, 0x1f, 0xae, 0xfc, 0x6c}, 2};	//Needed for calling SetupDiGetDeviceProperty() on composite devices only.
		
		HDEVINFO DeviceInfoTable = INVALID_HANDLE_VALUE;
		PSP_DEVICE_INTERFACE_DATA InterfaceDataStructure = new SP_DEVICE_INTERFACE_DATA;
		PSP_DEVICE_INTERFACE_DETAIL_DATA DetailedInterfaceDataStructure = new SP_DEVICE_INTERFACE_DETAIL_DATA;
		SP_DEVINFO_DATA DevInfoData;
		DWORD InterfaceIndex = 0;
		DWORD StatusLastError = 0;
		DWORD dwRegType;
		DWORD dwRegSize;
		DWORD StructureSize = 0;
		PBYTE PropertyValueBuffer;
		bool MatchFound = false;
		DWORD ErrorStatus;
		BOOL BoolStatus = FALSE;
		DWORD LoopCounter = 0;
		String^ deviceIDToFind;
		String^ completeIdentifier;
		String^ FriendlyNameString = "";
		int IndexOfStartofCOMChars;
		int IndexOfEndOfCOMChar = 0;
		String^ COMString = "";
		String^ registryPathString;
		RegistryKey^ regKey;
		RegistryKey^ regSubKey;
		DWORD i;
		DWORD instanceCounter = 0;
		String^ containerIDToCheck;
		String^ parentDeviceGUIDString;
		DEVPROPTYPE devPropType;
		GUID containerIDGUID;


		//Craft a Windows "HardwareID" string from the user's specified VID/PID/interface index info.
		//the HardwareID string is formatted like: "Vid_04d8&Pid_000a".  If the device is part of a composite
		//USB device however, then an additional "&MI_xx" gets appended at the end, where "MI" represents "multiple interface" and "xx" is a number 00, 01, 02, etc.,
		//representing the interface index of the first interface associated with the CDC function.  See: http://msdn.microsoft.com/en-us/library/windows/hardware/ff553356(v=vs.85).aspx
		deviceIDToFind = "Vid_" + USBVendorID.ToString("x4") + "&Pid_" + USBProductID.ToString("x4");	//The "x4" format strings make the 16-bit numbers into hexadecimal (0-9 + A-F) text strings.
		if(deviceIsComposite == true)
		{
			completeIdentifier = deviceIDToFind + "&MI_" + compositeInterfaceIndex.ToString("d2");	//Append &MI_xx to end of the string (ex: "Vid_04d8&Pid_0057&MI_01").
		}
		else
		{
			completeIdentifier = deviceIDToFind;
		}
		//Convert string to pure lower case, for better/more robust string comparision checks
		deviceIDToFind = deviceIDToFind->ToLowerInvariant();	

		//Now Populate a list of currently plugged in hardware devices (by specifying "DIGCF_PRESENT"), which are of the specified class GUID. 
		DeviceInfoTable = INVALID_HANDLE_VALUE;
		DeviceInfoTable = SetupDiGetClassDevsUM(&InterfaceClassGuid, NULL, NULL, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);	//Creates a device list in memory, kind of like looking at a cut down internal version of the Windows device manager.
		ErrorStatus = GetLastError();
		if(DeviceInfoTable == INVALID_HANDLE_VALUE)
		{
			//Failed to get the device list.  Not much we can do in this case, just bug out.
			delete InterfaceDataStructure;	
			delete DetailedInterfaceDataStructure;
			return "";
		}


		//Now look through the list we just populated.  We are trying to see if any of them match our device. 
		while(true)
		{
			InterfaceDataStructure->cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
			if(SetupDiEnumDeviceInterfacesUM(DeviceInfoTable, NULL, &InterfaceClassGuid, InterfaceIndex, InterfaceDataStructure))
			{
				ErrorStatus = GetLastError();
				if(ErrorStatus == ERROR_NO_MORE_ITEMS)	//Did we reach the end of the list of matching devices in the DeviceInfoTable?
				{	//Cound not find the device.  Must not have been attached.
					SetupDiDestroyDeviceInfoListUM(DeviceInfoTable);	//Clean up the old structure we no longer need.
					delete InterfaceDataStructure;	
					delete DetailedInterfaceDataStructure;
					return "";		
				}
			}
			else	//Else some other kind of unknown error ocurred...
			{
				ErrorStatus = GetLastError();
				SetupDiDestroyDeviceInfoListUM(DeviceInfoTable);	//Clean up the old structure we no longer need.
				delete InterfaceDataStructure;	
				delete DetailedInterfaceDataStructure;
				return "";	
			}

			//Now retrieve the hardware ID from the registry.  The hardware ID contains the VID and PID, which we will then 
			//check to see if it is the correct device or not.

			//Initialize an appropriate SP_DEVINFO_DATA structure.  We need this structure for SetupDiGetDeviceRegistryProperty().
			DevInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
			SetupDiEnumDeviceInfoUM(DeviceInfoTable, InterfaceIndex, &DevInfoData);

			//First query for the size of the hardware ID, so we can know how big a buffer to allocate for the data.
			SetupDiGetDeviceRegistryPropertyUM(DeviceInfoTable, &DevInfoData, SPDRP_HARDWAREID, &dwRegType, NULL, 0, &dwRegSize);

			//Allocate a buffer for the hardware ID.
			PropertyValueBuffer = (BYTE *) malloc (dwRegSize);
			if(PropertyValueBuffer == NULL)	//if null, error, couldn't allocate enough memory
			{	//Can't really recover from this situation, just exit instead.
				SetupDiDestroyDeviceInfoListUM(DeviceInfoTable);	//Clean up the old structure we no longer need.
				delete InterfaceDataStructure;	
				delete DetailedInterfaceDataStructure;
				return "";		
			}

			//Retrieve the hardware IDs for the current device we are looking at.  PropertyValueBuffer gets filled with a 
			//REG_MULTI_SZ (array of null terminated strings).  To find a device, we only care about the very first string in the
			//buffer, which will be the "device ID".  The device ID is a string which contains the VID and PID, in the example 
			//format "Vid_04d8&Pid_000a".
			SetupDiGetDeviceRegistryPropertyUM(DeviceInfoTable, &DevInfoData, SPDRP_HARDWAREID, &dwRegType, PropertyValueBuffer, dwRegSize, NULL);

			//Now check if the first string in the hardware ID matches the device ID of my USB device.
			#ifdef UNICODE
			String^ DeviceIDFromRegistry = gcnew String((wchar_t *)PropertyValueBuffer);
			#else
			String^ DeviceIDFromRegistry = gcnew String((char *)PropertyValueBuffer);
			#endif

			free(PropertyValueBuffer);		//No longer need the PropertyValueBuffer, free the memory to prevent potential memory leaks

			//Convert string to lower case.  This makes the code more robust/portable accross OS Versions
			DeviceIDFromRegistry = DeviceIDFromRegistry->ToLowerInvariant();	
			//Now check if the hardware ID we are looking at contains the correct VID/PID
			MatchFound = DeviceIDFromRegistry->Contains(deviceIDToFind);		
			if(MatchFound == true)
			{
				//We just found a device with matching VID/PID.  

				//Check if the instance of the matching hardware matches the user's desired instance number.  If not,
				//just keep looking until we find a device with fully matching VID+PID+Instance Index.
				if(instanceCounter == deviceInstanceIndex)
				{
					//Device must have been found.  Now we should read the "FriendlyName".  The FriendlyName is the text
					//that appears in the device manager entry for the device.  The friendlyname for the virtual com port
					//is composed of the text from the .inf file used to install the driver, combined with the COMx number
					//assigned to the device.  The COMx number also appears in the registry "PortName" entry for the device.
					//First call the function to determine the size of the required buffer.
					SetupDiGetDeviceRegistryPropertyUM(DeviceInfoTable, &DevInfoData, SPDRP_FRIENDLYNAME, &dwRegType, NULL, 0, &dwRegSize);
					//Allocate a buffer for the hardware ID.
					PropertyValueBuffer = (BYTE *) malloc (dwRegSize);
					if(PropertyValueBuffer == NULL)	//if null, error, couldn't allocate enough memory
					{	//Can't really recover from this situation, just exit instead.
						SetupDiDestroyDeviceInfoListUM(DeviceInfoTable);	//Clean up the old structure we no longer need.
						delete InterfaceDataStructure;	
						delete DetailedInterfaceDataStructure;
						return "";		
					}

					//Try to retrieve the FriendlyName string containing the COMx number.
					SetupDiGetDeviceRegistryPropertyUM(DeviceInfoTable, &DevInfoData, SPDRP_FRIENDLYNAME, &dwRegType, PropertyValueBuffer, dwRegSize, NULL);

					//Now copy the resulting string (which is returned by the API as a plain byte array) into a managed String^ type for easier 
					//manipulation and comparisons using managed functions.
					#ifdef UNICODE
						FriendlyNameString = gcnew String((wchar_t *)PropertyValueBuffer);
					#else
						FriendlyNameString = gcnew String((char *)PropertyValueBuffer);
					#endif
					free(PropertyValueBuffer);		//No longer need the PropertyValueBuffer, free the memory to prevent potential memory leaks


					//Check if it contains the COMx string like expected, and if so, extract it, so it can
					//be returned in the formatting used when opening a COMx port.
					if((FriendlyNameString->Contains("(COM")) && (FriendlyNameString->Contains(")")))
					{
						IndexOfStartofCOMChars = FriendlyNameString->IndexOf("COM");
						IndexOfEndOfCOMChar = FriendlyNameString->IndexOf(")");
						if((IndexOfEndOfCOMChar > IndexOfStartofCOMChars) && (IndexOfEndOfCOMChar <= FriendlyNameString->Length))
						{
							//Copy the "COMxx" string value from the FriendlyNameString to the COMString.
							//The COMString is formatted in the exact format needed when setting a serial port object's COM number.
							COMString = FriendlyNameString->Substring(IndexOfStartofCOMChars, IndexOfEndOfCOMChar - IndexOfStartofCOMChars);
						}

						//We have now presumable found the COMx number and should return.
						SetupDiDestroyDeviceInfoListUM(DeviceInfoTable);	//Clean up the old structure we no longer need.
						delete InterfaceDataStructure;	
						delete DetailedInterfaceDataStructure;
						return COMString;	//May contain "" if we couldn't find the device COMxx number.  Otherwise, returns "COMxxx" where xxx is the COM number (could be only one or two chars, depending upon COM number)
					}
					else
					{
						//The FriendlyNameString didn't contain the "COMxx" number string we were expecting.  This can happen (and is expected) if the 
						//USB CDC ACM function/interface is part of a composite USB device.  
						//In a composite device, at least three entries get made in the registry.  For example an MSD + CDC composite device would generate:
						//1. A top level "parent device" entry.
						//2. A lower level entry specific to the first function implemented in the composite device (ex: MSD).
						//3. Another lower level entry specific to the next function implemented in the composite device (ex: CDC).

						//In the code above, if the CDC function is part of a composite USB device, the above algorithm will find the
						//parent device (since it has matching VID/PID and other info).  However, the COMx port number information will
						//not be present in the top level parent device registry entries.  Instead, you have to find the proper corresponding
						//lower level entry, for the CDC interface on the device, that is specifically associated with that parent device.

						//Windows 7 and up implements the concept of "Container ID" numbers (which are 128-bit GUIDs), which link/associate
						//the parent device with the lower level functions.  It is therefore possible to cross reference the three+ registry entries
						//with each other, since the ContainerID for each entry will match.

						//Therefore the process for getting the COMxx number for a composite device is to:
						//1. Find the top level parent device based on VID/PID/current attachment status, index, etc.
						//2. Read and save the Container ID value for the parent device.
						//3. Find all the lower level CDC entries that also have matching VID/PID.
						//4. For each entry found in #3 above, read the Container ID and check it against the parent device Container ID.
						//5. If a match is found, read the COMx port "FriendlyName" string from the registry, which is stored in the subkey associated with that CDC function.

					
						//Going to do some registry read only operations, which is dymanic and can change at runtime based on plug and play
						//activity elsewhere in the OS.  Additionally, some of the below code will only work on >= Windows 7 (ex: ContainerIDs and the SetupDiGetDeviceProperty()
						//API doesn't exist on XP, so won't actually link to the setupapi.dll properly on the old OSes).  Therefore, we use a try/catch
						//block in case of this type of failure.
						try
						{
							//Get the "ContainerID" (Note: only available in Windows 7+) for the top level parent device
							if(SetupDiGetDevicePropertyUM(DeviceInfoTable, &DevInfoData, &devPropKeyContainerID, &devPropType, (PBYTE)&containerIDGUID, sizeof(GUID), &dwRegSize, 0))
							{
								ErrorStatus = GetLastError();
								parentDeviceGUIDString = ContainerIDGUIDToString(&containerIDGUID);
							}
							else
							{
								//Failed to get the ContainerID GUID for some reason.
								ErrorStatus = GetLastError();
								parentDeviceGUIDString = "";
							}

							//Now open the composite CDC "child" specific registry key for the device.
							//The full key will look something like:
							//"SYSTEM\CurrentControlSet\Enum\USB\VID_04D8&PID_0057&MI_01\9&2e9bdd3b&0&0001"		//Note the cryptic subkey "9&2e9bdd3b&0&0001" at the end (which changes, among other things, based on hub/port number the device is attached to).
							registryPathString = "SYSTEM\\CurrentControlSet\\Enum\\USB\\" + completeIdentifier->ToUpperInvariant();
							//Try to open the registry key (without cryptic subkey)
							regKey = Registry::LocalMachine->OpenSubKey(registryPathString);
							if(regKey != nullptr)
							{
								//We successfully opened the key.  However, the actual FriendlyName value
								//is stored in an obscure named subkey to the key we just opened.  Therefore,
								//we need to figure out the name(s) of any subkeys first, then look through each of them.
								array<String^>^subKeyNames = regKey->GetSubKeyNames();
								
								//Done with the upper level key.  Close it.
								regKey->Close();

								//Look through each of the subkey(s) to extract the FriendlyName value for the desired device
								for(i = 0; i < (DWORD)subKeyNames->Length; i++)
								{
									regSubKey = Registry::LocalMachine->OpenSubKey(registryPathString + "\\" + subKeyNames[i]);
									if(regSubKey != nullptr)
									{
										//Fetch the ContainerID value from the current subkey that we are looking at.  This particular
										//subkey may or may not be the correct hardware device that is currently attached (ex: it could
										//be for a separate plug and play device node in the registry, for a device with matching VID/PID,
										//but which is not currently plugged in or, is attached to a different USB port from our device of interest).
										containerIDToCheck = (String^)regSubKey->GetValue("ContainerID");

										//We now have two ContainerID strings, one from this subkey, and one for the "parent" USB 
										//device (that is currently attached to the machine with matching VID/PID).

										//Convert both strings to all lower case, prior to comparisons, to ensure case insensitive hex number (as string) comparison.
										containerIDToCheck = containerIDToCheck->ToLowerInvariant();
										parentDeviceGUIDString = parentDeviceGUIDString->ToLowerInvariant();

										//Check if we have found the correct/matching registry entry, associated with the CDC function of the
										//device we are interested in, which is currently plugged in (which has matching ContainerID value
										//as the parent device that we found earlier).
										if(containerIDToCheck == parentDeviceGUIDString)
										{
											//We found the correct match, try to read the "FriendlyName" string (which contains the COMxx port info), stored in the registry entry.
											FriendlyNameString = (String^)regSubKey->GetValue("FriendlyName", "");
											if(FriendlyNameString != "")
											{
												//Check if the friendly name contains the desired COMx number information, and extract it if present.
												if((FriendlyNameString->Contains("(COM")) && (FriendlyNameString->Contains(")")))
												{
													IndexOfStartofCOMChars = FriendlyNameString->IndexOf("COM");
													IndexOfEndOfCOMChar = FriendlyNameString->IndexOf(")");
													if((IndexOfEndOfCOMChar > IndexOfStartofCOMChars) && (IndexOfEndOfCOMChar <= FriendlyNameString->Length))
													{
														//Copy the "COMxx" string value from the FriendlyNameString to the COMString.
														//The COMString is formatted in the exact format needed when setting a serial port object's COM number.
														COMString = FriendlyNameString->Substring(IndexOfStartofCOMChars, IndexOfEndOfCOMChar - IndexOfStartofCOMChars);
													}

													//We have now found the COMx number and should return.
													//Close the registry keys, now that we are done with it.
													regSubKey->Close();
													SetupDiDestroyDeviceInfoListUM(DeviceInfoTable);	//Clean up the old structure we no longer need.
													delete InterfaceDataStructure;	
													delete DetailedInterfaceDataStructure;
													return COMString;	//May contain "" if we couldn't find the device COMxx number.  Otherwise, returns "COMxxx" where xxx is the COM number (could be only one or two chars, depending upon COM number)
												}
											}
										}//if(containerIDToCheck == parentDeviceGUIDString)

										//Close the registry key, now that we are done with it.
										regSubKey->Close();
									}//if(regSubKey != nullptr)
								}//for(i = 0; i < subKeyNames->Length; i++)
							}//if(regKey != nullptr)
							else
							{
								//Failed to open the expected registry key with the crytptic subkeys...
								//Can't do much in this case.
							}
						}//try
						catch(...)
						{
							//Most likely reason for failure is due to fail to obtain the SetupDiGetDeviceProperty() entry point
							//in the setupapi.dll.  Presumably, this API didn't exist in XP.  In this case, we can't really find
							//the device (with full confidence anyway), and the user must still manually figure out the COMxx 
							//number themselves and open it manually.
						};
					}//else of: if((FriendlyNameString->Contains("(COM")) && (FriendlyNameString->Contains(")")))
				}//if(instanceCounter == deviceInstanceIndex)
				else
				{
					//Although we found a device with matching VID/PID, the user didn't want to open it,
					//presumably because they wanted to attached to a higher order instance of the hardware.
					instanceCounter++;
				}
			}//if(MatchFound == true)

			//If we get to here, this means we just finished looking through one entry in the populated
			//virutal Windows device manager list of attached hardware devices (of matching interface GUID).
			//Since we didn't find a match this pass, we need to continue the loop to look at the next 
			//entry in the table.
			InterfaceIndex++;	

			//Keep looping until we either find a device with matching VID and PID, or until we run out of devices to check.
			//However, just in case some unexpected error occurs, keep track of the number of loops executed.
			//If the number of loops exceeds a very large number, exit anyway, to prevent inadvertent infinite looping.
			LoopCounter++;
			if(LoopCounter == 10000000)	//Surely there aren't more than 10 million devices attached to any forseeable PC...
			{
				//Bug out to avoid infinite looping, in case the "if(ErrorStatus == ERROR_NO_MORE_ITEMS)" 
				//check isn't working for whatever reason.
				SetupDiDestroyDeviceInfoListUM(DeviceInfoTable);	//Clean up the old structure we no longer need.
				delete InterfaceDataStructure;	
				delete DetailedInterfaceDataStructure;
				return "";
			}
		}//end of while(true)
}


		/****************************************************************************
			Function:
				String^ ContainerIDGUIDToString(GUID* pGUIDToConvert)

			Summary:
				This function converts a "GUID" type into a String^, formatted like: "{4f1763bd-8195-11e4-935e-3c970e83569e}"

			Description:
				This function converts a "GUID" type (a 128-bit integer) into a text String^, formatted like: "{4f1763bd-8195-11e4-935e-3c970e83569e}"

			Precondition:
				None

			Parameters:
				GUID* pGUIDToConvert - a pointer to a GUID that needs converting into the String^ format.

			Return Values:
				String^ containing the string formatted version of the GUID value.

			Remarks:
				The returned string contains the extra characters like the "{" and "}" braces,
				as well as dashes separating number groups.  There are however different methods of
				representing GUIDs as strings (ex: different digit grouping/dash locations), so make
				sure this format ("{4f1763bd-8195-11e4-935e-3c970e83569e}") is what you actually want.
				
		***************************************************************************/
		String^ ContainerIDGUIDToString(GUID* pGUIDToConvert)
		{
			String^ returnString;
			String^ data4String = "";
			int i;


			//First convert the "Data4" portion of the GUID number.  Data4 is an 8-byte char array, but we want the formatted string to be like "xxxx-xxxxxxxxxxxx"
			data4String = pGUIDToConvert->Data4[0].ToString("x2") + pGUIDToConvert->Data4[1].ToString("x2") + "-";
			for(i = 2; i < 8; i++)
			{
				data4String += pGUIDToConvert->Data4[i].ToString("x2");
			}

			//Now convert the Data0-Data3 portions of the GUID (which are 32-bit long, 16-bit short, 16-bit short, separated by dashes, and append the {} braces.
			returnString = "{" + pGUIDToConvert->Data1.ToString("x8") + "-" + pGUIDToConvert->Data2.ToString("x4") + "-" + pGUIDToConvert->Data3.ToString("x4") + "-" + data4String + "}";

			//The final output should be something like this: "{4f1763bd-8195-11e4-935e-3c970e83569e}"
			return returnString;
		}



		/****************************************************************************
			Function:
				private void serialPort1_DataReceived(  object sender, 
														SerialDataReceivedEventArgs e)

			Summary:
				This function prints any data received on the COM port.

			Description:
				This function is called when the data is received on the COM port.  This
				function attempts to write that data to the txtDataReceived textbox.  If
				an exception occurs the btnClose_Click() function is called in order to
				close the COM port that caused the exception.

			Precondition:
				None

			Parameters:
				object sender     - Sender of the event (this form)
				SerialDataReceivedEventArgs e       - The event arguments

			Return Values:
				None

			Remarks:
				None
		***************************************************************************/
		private: System::Void serialPort1_DataReceived(System::Object^  sender, System::IO::Ports::SerialDataReceivedEventArgs^  e) 
		{
			//The ReadExisting() function will read all of the data that
			//  is currently available in the COM port buffer.  In this 
			//  example we are sending all of the available COM port data
			//  to the SetText() function.
			//
			//  NOTE: the <SerialPort>_DataReceived() function is launched
			//  in a seperate thread from the rest of the application.  A
			//  delegate function is required in order to properly access
			//  any managed objects inside of the other thread.  Since we
			//  will be writing to a textBox (a managed object) the delegate
			//  function is required.  Please see the SetText() function for 
			//  more information about delegate functions and how to use them.
			try
			{
				SetText(serialPort1->ReadExisting());
			}
			catch(...)
			{
			}
		}

		/****************************************************************************
			Function:
				private void SetText(string text)

			Summary:
				This function prints the input text to the txtDataReceived textbox.

			Description:
				This function prints the input text to the txtDataReceived textbox.  If
				the calling thread is the same as the thread that owns the textbox, then
				the AppendText() method is called directly.  If a thread other than the
				main thread calls this function, then an instance of the delegate function
				is created so that the function runs again in the main thread.

			Precondition:
				None

			Parameters:
				string text     - Text that needs to be printed to the textbox

			Return Values:
				None

			Remarks:
				None
		***************************************************************************/
		private: void SetText(String^ text)
		{
			//InvokeRequired required compares the thread ID of the
			//  calling thread to the thread ID of the creating thread.
			//  If these threads are different, it returns true.  We can
			//  use this attribute to determine if we can append text
			//  directly to the textbox or if we must launch an a delegate
			//  function instance to write to the textbox.
			if (this->txtDataReceived->InvokeRequired)
			{
				//InvokeRequired returned TRUE meaning that this function
				//  was called from a thread different than the current
				//  thread.  We must launch a deleage function.

				//Create an instance of the SetTextCallback delegate and
				//  assign the delegate function to be this function.  This
				//  effectively causes this same SetText() function to be
				//  called within the main thread instead of the second
				//  thread.
				SetTextCallback^ d = gcnew SetTextCallback(this,&VCCDC::Form1::SetText);

				//Invoke the new delegate sending the same text to the
				//  delegate that was passed into this function from the
				//  other thread.
				this->Invoke(d,gcnew String(text));
			}
			else
			{
				//If this function was called from the same thread that 
				//  holds the required objects then just add the text.
				txtDataReceived->AppendText(text);
			}
		}

		/****************************************************************************
			Function:
				private void btnSendData_Click(object sender, EventArgs e)

			Summary:
				This function will attempt to send the contents of txtData over the COM port

			Description:
				This function is called when the btnSendData button is clicked.  It will 
				attempt to send the contents of txtData over the COM port.  If the attempt
				is unsuccessful this function will call the btnClose_Click() in order to
				close the COM port that just failed.

			Precondition:
				None

			Parameters:
				object sender     - Sender of the event (this form)
				EventArgs e       - The event arguments

			Return Values:
				None

			Remarks:
				None
		***************************************************************************/
		private: System::Void btnSendData_Click(System::Object^  sender, System::EventArgs^  e)
		{
			//This section of code will try to write to the COM port.
			//  Please note that it is important to use a try/catch
			//  statement when writing to the COM port.  If a USB virtual
			//  COM port is removed and the PC software tries to write
			//  to the COM port before it detects its removal then
			//  an exeception is thrown.  If the execption is not in a
			//  try/catch statement this could result in the application
			//  crashing.

			try
			{
				//Write the data in the text box to the open serial port
				serialPort1->Write(txtSendData->Text);
			}
			catch(...)
			{
			}
		}
};
}

