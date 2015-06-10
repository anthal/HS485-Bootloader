/*************************************************************************************

Version: 1.2.0
Datum:   30.05.2015
Autor:   Andreas Thalmann

12.01.14 1.0.0  CHG - Flash mit festem Filenamen und ohne Verify funktioniert 
12.01.14 1.0.1  CHG - Flash mit auswählbarem Filenamen  
13.01.14 1.1.0  CHG - Flash mit Verify
30.05.15 1.2.0  CHG - New Code for read Intel-Hex-File

**************************************************************************************/

#define _CRT_SECURE_DEPRECATE_MEMORY
#include <memory.h>
#include <string.h>
#include <stdio.h>
#include <windows.h> 
//#include <sstream>

#pragma once

namespace test1 {
	using namespace std;
	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
	using namespace System::Text;
	using namespace System::IO::Ports;
	using namespace System::IO;
	using namespace Microsoft::Win32;

	#define FRAME_START_LONG  0xFD
	#define FRAME_START_SHORT 0xFE
	#define ESCAPE_CHAR     0xFC

	#define IS_IFRAME(x)        (((x) & 0x01) == 0x00)
	#define IS_ACK(x)           (((x) & 0x97) == 0x11)
	#define IS_DISCOVERY(x)     (((x) & 0x07) == 0x03)
	#define CONTAINS_SENDER(x)  (((x) & (1<<3)) !=0)

	#define MAX_RX_FRAME_LENGTH      255
	#define CRC16_POLYGON         0x1002

	#define START_SIGN              ":"      /* Hex-Datei Zeilenstartzeichen */
 
	/* Zustaende des Bootloader-Programms */
	#define BOOT_STATE_EXIT	        0        
	#define BOOT_STATE_PARSER       1
 
	/* Zustaende des Hex-File-Parsers */
	#define PARSER_STATE_START      0
	#define PARSER_STATE_SIZE       1
	#define PARSER_STATE_ADDRESS    2
	#define PARSER_STATE_TYPE       3
	#define PARSER_STATE_DATA       4
	#define PARSER_STATE_CHECKSUM   5
	#define PARSER_STATE_ERROR      6
 
	/* Page size */
	#define SPM_PAGESIZE            64

  struct stData
  {
	  //SYSTEMTIME    sysTime;
	  unsigned char ucStartByte;
	  unsigned char ucReceiverAddress[4];
	  unsigned char ucControlByte;
	  unsigned char ucSenderAddress[4];	
	  unsigned char ucDataLength;
	  unsigned char ucFrameData[MAX_RX_FRAME_LENGTH];
  };

  unsigned int crc16_register;				// Register mit CRC16 Code

	/// <summary>
	/// Zusammenfassung für Form1
	/// </summary>
	public ref class Form1 : public System::Windows::Forms::Form
	{
	public:
		Form1(void)
		{
			InitializeComponent();
			//
			// Konstruktorcode hier hinzufügen.
			//
      array<String^>^ serialPorts = nullptr;
      try
      {
          // Get a list of serial port names.
          serialPorts = SerialPort::GetPortNames();
      }
      catch (Win32Exception^ ex)
      {
          Console::WriteLine(ex->Message);
      }
      // Display each port name to the console.
      for each(String^ port in serialPorts)
      {
        comboBox1->Items->Add(port);
      }
      comboBox1->Text = serialPorts[0];
      button2->Enabled = false;
      button3->Enabled = false;
      button4->Enabled = false;
    }

	protected:
		/// <summary>
		/// Verwendete Ressourcen bereinigen.
		/// </summary>
		~Form1()
		{
			if (components)
			{
				delete components;
			}
		}
  private: System::Windows::Forms::ListBox^  listBox1;
  protected: 

  protected: 

	protected: 
	private: System::Windows::Forms::Button^  button1;
	private: System::Windows::Forms::ComboBox^  comboBox1;
	private: System::Windows::Forms::Button^  button2;
	private: System::Windows::Forms::Button^  button3;
	private: System::IO::Ports::SerialPort^  serialPort1;
	private: System::Windows::Forms::OpenFileDialog^  openFileDialog1;

  private: System::Windows::Forms::GroupBox^  groupBox2;

  private: System::Windows::Forms::Label^  label2;

  private: System::Windows::Forms::Button^  button4;
  private: System::Windows::Forms::NumericUpDown^  numericUpDown1;
  private: System::Windows::Forms::Button^  button5;
  private: System::Windows::Forms::ProgressBar^  progressBar1;
  private: System::Windows::Forms::Label^  label3;


  private: System::Windows::Forms::Label^  label5;
  private: System::Windows::Forms::Label^  label4;
  private: System::Windows::Forms::TextBox^  textBox1;


  private: System::ComponentModel::IContainer^  components;

	private:
		/// <summary>
		/// Erforderliche Designervariable.
		/// </summary>


#pragma region Windows Form Designer generated code
		/// <summary>
		/// Erforderliche Methode für die Designerunterstützung.
		/// Der Inhalt der Methode darf nicht mit dem Code-Editor geändert werden.
		/// </summary>
		void InitializeComponent(void)
		{
			this->components = (gcnew System::ComponentModel::Container());
			this->listBox1 = (gcnew System::Windows::Forms::ListBox());
			this->button1 = (gcnew System::Windows::Forms::Button());
			this->comboBox1 = (gcnew System::Windows::Forms::ComboBox());
			this->button2 = (gcnew System::Windows::Forms::Button());
			this->button3 = (gcnew System::Windows::Forms::Button());
			this->serialPort1 = (gcnew System::IO::Ports::SerialPort(this->components));
			this->openFileDialog1 = (gcnew System::Windows::Forms::OpenFileDialog());
			this->groupBox2 = (gcnew System::Windows::Forms::GroupBox());
			this->textBox1 = (gcnew System::Windows::Forms::TextBox());
			this->label5 = (gcnew System::Windows::Forms::Label());
			this->label4 = (gcnew System::Windows::Forms::Label());
			this->progressBar1 = (gcnew System::Windows::Forms::ProgressBar());
			this->label3 = (gcnew System::Windows::Forms::Label());
			this->button5 = (gcnew System::Windows::Forms::Button());
			this->numericUpDown1 = (gcnew System::Windows::Forms::NumericUpDown());
			this->label2 = (gcnew System::Windows::Forms::Label());
			this->button4 = (gcnew System::Windows::Forms::Button());
			this->groupBox2->SuspendLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numericUpDown1))->BeginInit();
			this->SuspendLayout();
			// 
			// listBox1
			// 
			this->listBox1->FormattingEnabled = true;
			this->listBox1->HorizontalScrollbar = true;
			this->listBox1->Location = System::Drawing::Point(10, 46);
			this->listBox1->Name = L"listBox1";
			this->listBox1->SelectionMode = System::Windows::Forms::SelectionMode::MultiSimple;
			this->listBox1->Size = System::Drawing::Size(741, 368);
			this->listBox1->TabIndex = 0;
			this->listBox1->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::listBox1_SelectedIndexChanged);
			// 
			// button1
			// 
			this->button1->Location = System::Drawing::Point(147, 17);
			this->button1->Name = L"button1";
			this->button1->Size = System::Drawing::Size(75, 23);
			this->button1->TabIndex = 1;
			this->button1->Text = L"Open Port";
			this->button1->UseVisualStyleBackColor = true;
			this->button1->Click += gcnew System::EventHandler(this, &Form1::button1_Click);
			// 
			// comboBox1
			// 
			this->comboBox1->FormattingEnabled = true;
			this->comboBox1->Location = System::Drawing::Point(10, 19);
			this->comboBox1->Name = L"comboBox1";
			this->comboBox1->Size = System::Drawing::Size(121, 21);
			this->comboBox1->Sorted = true;
			this->comboBox1->TabIndex = 2;
			this->comboBox1->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::comboBox1_SelectedIndexChanged);
			// 
			// button2
			// 
			this->button2->Location = System::Drawing::Point(228, 17);
			this->button2->Name = L"button2";
			this->button2->Size = System::Drawing::Size(75, 23);
			this->button2->TabIndex = 3;
			this->button2->Text = L"Close Port";
			this->button2->UseVisualStyleBackColor = true;
			this->button2->Click += gcnew System::EventHandler(this, &Form1::button2_Click);
			// 
			// button3
			// 
			this->button3->Location = System::Drawing::Point(309, 17);
			this->button3->Name = L"button3";
			this->button3->Size = System::Drawing::Size(117, 23);
			this->button3->TabIndex = 4;
			this->button3->Text = L"Open Intel-Hex-File\r\n";
			this->button3->UseVisualStyleBackColor = true;
			this->button3->Click += gcnew System::EventHandler(this, &Form1::button3_Click);
			// 
			// serialPort1
			// 
			this->serialPort1->DataReceived += gcnew System::IO::Ports::SerialDataReceivedEventHandler(this, &Form1::serialPort1_DataReceived);
			// 
			// openFileDialog1
			// 
			this->openFileDialog1->FileName = L"openFileDialog1";
			this->openFileDialog1->FileOk += gcnew System::ComponentModel::CancelEventHandler(this, &Form1::openFileDialog1_FileOk);
			// 
			// groupBox2
			// 
			this->groupBox2->Controls->Add(this->textBox1);
			this->groupBox2->Controls->Add(this->label5);
			this->groupBox2->Controls->Add(this->label4);
			this->groupBox2->Controls->Add(this->progressBar1);
			this->groupBox2->Controls->Add(this->label3);
			this->groupBox2->Controls->Add(this->button5);
			this->groupBox2->Controls->Add(this->numericUpDown1);
			this->groupBox2->Controls->Add(this->label2);
			this->groupBox2->Controls->Add(this->button4);
			this->groupBox2->Controls->Add(this->listBox1);
			this->groupBox2->Controls->Add(this->button3);
			this->groupBox2->Controls->Add(this->comboBox1);
			this->groupBox2->Controls->Add(this->button2);
			this->groupBox2->Controls->Add(this->button1);
			this->groupBox2->Location = System::Drawing::Point(3, 13);
			this->groupBox2->Name = L"groupBox2";
			this->groupBox2->Size = System::Drawing::Size(761, 479);
			this->groupBox2->TabIndex = 6;
			this->groupBox2->TabStop = false;
			this->groupBox2->Text = L"groupBox2";
			this->groupBox2->Enter += gcnew System::EventHandler(this, &Form1::groupBox2_Enter);
			// 
			// textBox1
			// 
			this->textBox1->Location = System::Drawing::Point(89, 420);
			this->textBox1->Name = L"textBox1";
			this->textBox1->Size = System::Drawing::Size(662, 20);
			this->textBox1->TabIndex = 17;
			// 
			// label5
			// 
			this->label5->AutoSize = true;
			this->label5->Location = System::Drawing::Point(727, 456);
			this->label5->Name = L"label5";
			this->label5->Size = System::Drawing::Size(24, 13);
			this->label5->TabIndex = 14;
			this->label5->Text = L"0 %";
			// 
			// label4
			// 
			this->label4->AutoSize = true;
			this->label4->Location = System::Drawing::Point(9, 456);
			this->label4->Name = L"label4";
			this->label4->Size = System::Drawing::Size(56, 13);
			this->label4->TabIndex = 13;
			this->label4->Text = L"Fortschritt:";
			// 
			// progressBar1
			// 
			this->progressBar1->Location = System::Drawing::Point(89, 446);
			this->progressBar1->Name = L"progressBar1";
			this->progressBar1->Size = System::Drawing::Size(621, 23);
			this->progressBar1->TabIndex = 12;
			// 
			// label3
			// 
			this->label3->AutoSize = true;
			this->label3->Location = System::Drawing::Point(6, 427);
			this->label3->Name = L"label3";
			this->label3->Size = System::Drawing::Size(71, 13);
			this->label3->TabIndex = 11;
			this->label3->Text = L"Intel-Hex-File:";
			// 
			// button5
			// 
			this->button5->Location = System::Drawing::Point(676, 16);
			this->button5->Name = L"button5";
			this->button5->Size = System::Drawing::Size(75, 23);
			this->button5->TabIndex = 10;
			this->button5->Text = L"Clear Output";
			this->button5->UseVisualStyleBackColor = true;
			this->button5->Click += gcnew System::EventHandler(this, &Form1::button5_Click);
			// 
			// numericUpDown1
			// 
			this->numericUpDown1->Hexadecimal = true;
			this->numericUpDown1->Location = System::Drawing::Point(567, 19);
			this->numericUpDown1->Maximum = System::Decimal(gcnew cli::array< System::Int32 >(4) {65535, 0, 0, 0});
			this->numericUpDown1->Name = L"numericUpDown1";
			this->numericUpDown1->Size = System::Drawing::Size(103, 20);
			this->numericUpDown1->TabIndex = 9;
			this->numericUpDown1->Value = System::Decimal(gcnew cli::array< System::Int32 >(4) {4137, 0, 0, 0});
			this->numericUpDown1->ValueChanged += gcnew System::EventHandler(this, &Form1::numericUpDown1_ValueChanged);
			// 
			// label2
			// 
			this->label2->AutoSize = true;
			this->label2->Location = System::Drawing::Point(513, 22);
			this->label2->Name = L"label2";
			this->label2->Size = System::Drawing::Size(48, 13);
			this->label2->TabIndex = 8;
			this->label2->Text = L"Address:";
			// 
			// button4
			// 
			this->button4->Location = System::Drawing::Point(432, 17);
			this->button4->Name = L"button4";
			this->button4->Size = System::Drawing::Size(75, 23);
			this->button4->TabIndex = 6;
			this->button4->Text = L"Flash";
			this->button4->UseVisualStyleBackColor = true;
			this->button4->Click += gcnew System::EventHandler(this, &Form1::button4_Click);
			// 
			// Form1
			// 
			this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
			this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
			this->ClientSize = System::Drawing::Size(773, 499);
			this->Controls->Add(this->groupBox2);
			this->Name = L"Form1";
			this->Text = L"Form1";
			this->Load += gcnew System::EventHandler(this, &Form1::Form1_Load);
			this->groupBox2->ResumeLayout(false);
			this->groupBox2->PerformLayout();
			(cli::safe_cast<System::ComponentModel::ISupportInitialize^  >(this->numericUpDown1))->EndInit();
			this->ResumeLayout(false);

		}
#pragma endregion
private: System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) 
  {
    serialPort1->PortName = comboBox1->Text;
    serialPort1->BaudRate = 19200;
    serialPort1->DataBits = 8;
    serialPort1->Parity = Parity::Even;
    serialPort1->StopBits = StopBits::One;
    serialPort1->ReadTimeout = 1000;
    serialPort1->Open();
    listBox1->Items->Add("Open Port " + comboBox1->Text + " with 19200 Baud");
    button2->Enabled = true;
    button3->Enabled = true;
  }


private: System::Void button3_Click(System::Object^  sender, System::EventArgs^  e) 
	{
    openFileDialog1->InitialDirectory = "d:\\Projects\\";
    openFileDialog1->Filter = "Intel-Hex Files (*.hex)|*.hex";
    openFileDialog1->FilterIndex = 2;
    openFileDialog1->RestoreDirectory = true;

    if ( openFileDialog1->ShowDialog() == System::Windows::Forms::DialogResult::OK )
    {
      openFileDialog1->OpenFile();
      textBox1->Text = openFileDialog1->FileName;
      button4->Enabled = true;
      // Filname in Registry speichern:
      /*
      RegistryKey^ filename_key;
      filename_key = Registry::LocalMachine->CreateSubKey("Software\\Thalmann\\HS485-Flash");
      filename_key->SetValue("Filename", openFileDialog1->FileName);
      */
    }
	}

private: System::Void button2_Click(System::Object^  sender, System::EventArgs^  e) 
	{
    serialPort1->Close();
    listBox1->Items->Add("Close Port " + comboBox1->Text);			 
	}

void crc16_shift(unsigned char w)
{
	unsigned char q;
	unsigned char flag;
	for(q=0;q<8;q++)
	{
		flag=(crc16_register & 0x8000)!=0;
		crc16_register<<=1;
		if(w&0x80)
		{
			crc16_register|=1;
		}
		w<<=1;
		if(flag)crc16_register ^= CRC16_POLYGON;
	}
}

void crc16_init(void)
{
	crc16_register=0xffff;
}

  // Wandelt eine 32-Bit Adresse in ein 4-Byte unsigned char array um
void AddressHexToChar(unsigned char *p_ucAddress, unsigned int p_ulAddress)
{
	int i;
	char *x = (char*)&p_ulAddress;
	for( i =0; i < 4; i++)
		p_ucAddress[i] = x[3-i];
}

  // Wandelt ein 4-Byte char array in eine 32-Bit Adresse um
void AddressCharToHex(unsigned char *p_ucAddress, unsigned long *p_ulAddress)
{
	int i;
	char *x = (char*)p_ulAddress;
	for(i = 0; i < 4; i++)
		x[i] = p_ucAddress[3 - i];
}

// Sendet ein Byte über die Schnittstelle
bool SendByte(unsigned char p_ucByte)
{
    // http://www.mikrocontroller.net/topic/39756
    array<unsigned char,1>^ BytesToSend = { p_ucByte };

    serialPort1->Write(BytesToSend, 0, 1);
    return true;
}

// Sendet ein Byte über die Schnittstelle, prüft jedoch auf Sonderzeichen und wandelt diese entsprechend um
bool SendDataByte(unsigned char p_ucByte)
{
	unsigned char c;
	if((p_ucByte==FRAME_START_LONG) || (p_ucByte==FRAME_START_SHORT) || (p_ucByte==ESCAPE_CHAR)){
		c=ESCAPE_CHAR;
		if(!SendByte(c))
			return false;
		p_ucByte &= 0x7f;
	}
	return SendByte(p_ucByte);
}

  // Sendet einen Datenframe 
bool SendFrame(struct stData *p_pFrameData)
{
	int i;
	//if(!p_pFrameData || !p_cCom)				// Parameter prüfen
	if(!p_pFrameData)				// Parameter prüfen
		return false;
	if(IS_DISCOVERY(p_pFrameData->ucControlByte) || p_pFrameData->ucStartByte!=FRAME_START_LONG)		// Frame prüfen
		return false;
		
	crc16_init();
	if(!SendByte(p_pFrameData->ucStartByte))			// Startzeichen
		return false;
	crc16_shift(p_pFrameData->ucStartByte);
	for(i=0;i<4;i++)			// Zieladresse
	{
		if(!SendDataByte(p_pFrameData->ucReceiverAddress[i]))
			return false;
		crc16_shift(p_pFrameData->ucReceiverAddress[i]);
	}
	if(!SendDataByte(p_pFrameData->ucControlByte))	// Controllbyte
		return false;
	crc16_shift(p_pFrameData->ucControlByte);
	if(CONTAINS_SENDER(p_pFrameData->ucControlByte))
	{
		for(i=0;i<4;i++)	// Absenderadresse
		{
			if(!SendDataByte(p_pFrameData->ucSenderAddress[i]))
				return false;
			crc16_shift(p_pFrameData->ucSenderAddress[i]);
		}
	}
	if(!SendDataByte(p_pFrameData->ucDataLength+2))	// Framelänge
		return false;
	crc16_shift(p_pFrameData->ucDataLength+2);
	for(i=0;i<p_pFrameData->ucDataLength;i++)				// Framedaten
	{
		if(!SendDataByte(p_pFrameData->ucFrameData[i]))
			return false;
		crc16_shift(p_pFrameData->ucFrameData[i]);
	}
	crc16_shift(0);
	crc16_shift(0);
	if(!SendDataByte((crc16_register>>8)&0xff))	// CRC16-Checksumme
		return false;
	if(!SendDataByte((crc16_register)&0xff))
		return false;
	return true;
}

bool SendAck(unsigned char* p_ucReceiverAddress, unsigned char *p_ucSenderAddress, unsigned char p_ucEmpfangsfolgenummer)
{
	struct stData stAckData;
	stAckData.ucControlByte=((p_ucEmpfangsfolgenummer&0x03)<<5) | 0x19;
	stAckData.ucDataLength=0;
	stAckData.ucStartByte=FRAME_START_LONG;
	memcpy_s(stAckData.ucSenderAddress, 4, p_ucSenderAddress, 4);
	memcpy_s(stAckData.ucReceiverAddress,4, p_ucReceiverAddress, 4);
	SendFrame(&stAckData);
	return true;
}


/* Sendet eine Nachricht und wartet auf die Bestätigung
   Rückgabewert: Falls Bestätigung I-Frame, dann wird der I-Frame zurückgegeben
*/
struct stData *Send(struct stData *p_pFrameData)
{
	int iRetryCounter=3;			// Maximale Anzahl an Wiederholungen
	if (!p_pFrameData)				// Prüfen der Übergabeparameter
		return NULL;
	//p_cCom->setTimeOut(100);		// Maximale Zeit für die Antwort vom Modul
	while(iRetryCounter)
	{
		struct stData *pReturnFrame;

		if(!SendFrame(p_pFrameData))		// Nachricht senden
			return NULL;
		if(IS_IFRAME(p_pFrameData->ucControlByte))
		{		
			if(pReturnFrame = ReadFrame())		// Auf Antwort warten
			{
				unsigned long ulAddress1,ulAddress2;
				AddressCharToHex(pReturnFrame->ucReceiverAddress,&ulAddress1);		// Umwandeln der empfangenen Adresse in unsigned long
				AddressCharToHex(p_pFrameData->ucSenderAddress,&ulAddress2);		// Umwandeln der empfangenen Adresse in unsigned long			
				/*
				listBox1->Items->Add("ucReceiverAddress: " + pReturnFrame->ucReceiverAddress[0].ToString("X2") + pReturnFrame->ucReceiverAddress[1].ToString("X2") + pReturnFrame->ucReceiverAddress[2].ToString("X2") + pReturnFrame->ucReceiverAddress[3].ToString("X2"));
				listBox1->Items->Add("ucSenderAddress: " + p_pFrameData->ucSenderAddress[0].ToString("X2") + p_pFrameData->ucSenderAddress[1].ToString("X2") + p_pFrameData->ucSenderAddress[2].ToString("X2") + p_pFrameData->ucSenderAddress[3].ToString("X2"));
				listBox1->Items->Add("ucReceiverAddress: " + ulAddress1.ToString("X8"));
				listBox1->Items->Add("ucSenderAddress: " + ulAddress2.ToString("X8"));
				*/
				listBox1->Items->Add("Send - ucControlByte (answer): " + pReturnFrame->ucControlByte.ToString("X2"));
				if ( IS_IFRAME( pReturnFrame->ucControlByte ))
				{
					listBox1->Items->Add("  ucControlByte (answer): I-Nachricht");  
				}
				if ( IS_ACK( pReturnFrame->ucControlByte ))
				{
					listBox1->Items->Add("  ucControlByte (answer): ACK-Nachricht \n");  
				}
				if ( IS_DISCOVERY( pReturnFrame->ucControlByte ))
				{
					listBox1->Items->Add("  ucControlByte (answer): Discovery-Nachricht \n");  
				}
          
				// Bit B (Bit 3 - Eigene Sender-Adresse) gesetzt? 
				if (((pReturnFrame->ucControlByte>>3)&0x01) == 1 )
				{
					listBox1->Items->Add("Mit Absenderadresse 1\n");
					if ((ulAddress1 == ulAddress2) && pReturnFrame 
						&& (IS_IFRAME(pReturnFrame->ucControlByte) || IS_ACK(pReturnFrame->ucControlByte))) // Bestätigung vom angesprochenen Modul? (I-Frame oder ACK)
					{
						listBox1->Items->Add("Mit Absenderadresse 2");
						//listBox1->Items->Add("ucReceiverAddress (answer): 0x%08x", ulAddress1);
						//listBox1->Items->Add("ucSenderAddress   (sends) : 0x%08x", ulAddress2);
						if (((pReturnFrame->ucControlByte>>5)&0x03) == ((p_pFrameData->ucControlByte>>1)&0x03)) // Stimmt Sendefolgenummer mit Empfangsfolgenummer überein?     
						{
							if (IS_IFRAME(pReturnFrame->ucControlByte))    // Wenn I-Frame, dann Antwort schicken
							{
								SendAck(pReturnFrame->ucSenderAddress, pReturnFrame->ucReceiverAddress,  (pReturnFrame->ucControlByte>>1)&0x03);
								return pReturnFrame;
							}
							else
							    return NULL;
						}
					}
				}
				else
				{
				  listBox1->Items->Add("Ohne Absenderadresse 1");
				  if (pReturnFrame && (IS_IFRAME(pReturnFrame->ucControlByte) || IS_ACK(pReturnFrame->ucControlByte))) // Bestätigung vom angesprochenen Modul? (I-Frame oder ACK)
				  {
					listBox1->Items->Add("Ohne Absenderadresse 2");
					if (((pReturnFrame->ucControlByte>>5)&0x03) == ((p_pFrameData->ucControlByte>>1)&0x03)) // Stimmt Sendefolgenummer mit Empfangsfolgenummer überein?     
					{
						if (IS_IFRAME(pReturnFrame->ucControlByte))    // Wenn I-Frame, dann Antwort schicken
						{
							SendAck(pReturnFrame->ucSenderAddress, pReturnFrame->ucReceiverAddress,  (pReturnFrame->ucControlByte>>1)&0x03);
							return pReturnFrame;
						}
						else
							return NULL;
					}
				}
			}
				delete pReturnFrame;
		  }
		  iRetryCounter--;
		}
		else
			break;
	}  // while
	return NULL;
}

// Liest einen Nachrichtenframe 
struct stData *ReadFrame()
{
	  unsigned char ucRxEscape=false;
	  unsigned char ucRxByte;
	  unsigned char ucRxStartByte;
	  unsigned char ucRxAddressPointer;
	  unsigned char ucRxAddresslen;
	  unsigned char ucRxReceiverAddress[4];
	  unsigned char ucRxSenderAddress[4];
	  unsigned char ucRxControlByte;
	  unsigned char ucRxDataLength;
	  unsigned char ucRxFramePointer;
	  unsigned char ucRxFrameData[MAX_RX_FRAME_LENGTH];
	  unsigned int  uiLocalCRCRegister;
	  //unsigned int  i;

		ucRxReceiverAddress[0] = 0x00;
		ucRxReceiverAddress[1] = 0x00;
		ucRxReceiverAddress[2] = 0x00;
		ucRxReceiverAddress[3] = 0x00;

	  while(true)
	  {
      if (1 == 1 )
      {
        Sleep(20);
        if ( serialPort1->BytesToRead > 0 )
        {
            ucRxByte = serialPort1->ReadByte();
            //listBox1->Items->Add("Byte empfangen: " + ucRxByte.ToString("X2"));
			    if (ucRxByte == ESCAPE_CHAR && !ucRxEscape)
			    {
				    ucRxEscape=true;
				    continue;		
			    }
			    if (ucRxByte == FRAME_START_LONG)			// Startzeichen 0xFD empfangen
			    {
				    ucRxStartByte = FRAME_START_LONG;
				    ucRxEscape=false;
				    ucRxAddressPointer = 0;
				    ucRxAddresslen = 4;
				    ucRxFramePointer = 0;
				    crc16_init();
				    crc16_shift(ucRxByte);
			    }
			    else if (ucRxByte == FRAME_START_SHORT)	// Startzeichen 0xFE empfangen
			    {
				    ucRxStartByte = FRAME_START_SHORT;
				    ucRxEscape=false;
				    ucRxAddressPointer = 0;
				    ucRxAddresslen = 1;
				    ucRxFramePointer = 0;
				    crc16_init();
				    crc16_shift(ucRxByte);
			    }
			    else												// Frameinhalt empfangen
			    {
				    if(ucRxEscape)
				    {
					    ucRxByte|=0x80;
					    ucRxEscape = false;
				    }
				    if(ucRxAddressPointer<ucRxAddresslen)		// Adressbytes empfangen
				    {
					    ucRxReceiverAddress[ucRxAddressPointer] = ucRxByte;
					    ucRxAddressPointer++;
					    crc16_shift(ucRxByte);
					    //listBox1->Items->Add("Addressbyte empfangen");
				    }
				    else if(ucRxAddressPointer==ucRxAddresslen)	// Controllbyte empfangen
				    {
					    ucRxControlByte=ucRxByte;
					    ucRxAddressPointer++;
					    crc16_shift(ucRxByte);
					    //listBox1->Items->Add("Controllbyte empfangen");
				    }
				    else if(CONTAINS_SENDER(ucRxControlByte) && (ucRxAddressPointer < (2*ucRxAddresslen+1)))
				    {
					    ucRxSenderAddress[ucRxAddressPointer-ucRxAddresslen-1]=ucRxByte;
					    ucRxAddressPointer++;
					    crc16_shift(ucRxByte);
					    //listBox1->Items->Add("Absenderadresse empfangen");
				    }
				    else if(ucRxAddressPointer!=0xFF)		// Länge empfangen
				    {
					    ucRxAddressPointer=0xFF;
					    ucRxDataLength=ucRxByte;
					    crc16_shift(ucRxByte);
					    //listBox1->Items->Add("Länge empfangen: " + ucRxDataLength);
				    }
				    else	// Daten empfangen
				    {
					    //listBox1->Items->Add("Daten empfangen: Datalength: " + ucRxFramePointer);
					    if(ucRxFramePointer==ucRxDataLength-2)		// CRC Prüfsumme folgt
						    uiLocalCRCRegister=crc16_register;
					    ucRxFrameData[ucRxFramePointer]=ucRxByte;
					    crc16_shift(ucRxByte);
					    if(ucRxFramePointer==(ucRxDataLength-1))		// Daten komplett empfangen
					    {
						    crc16_shift(0);
						    crc16_shift(0);
						    ucRxFramePointer=ucRxAddressPointer=0;
						    if(crc16_register!=0)
                {	// Checksumme überprüfen
							    listBox1->Items->Add("Fehler in der Checksumme");
						    }
                else // Daten ausgeben
                {	
							    //listBox1->Items->Add("Daten empfangen");
							    struct stData *pNewData=new struct stData;

							    //SYSTEMTIME sysTime;
							    //GetSystemTime(&pNewData->sysTime);
							    pNewData->ucStartByte=ucRxStartByte;
							    memcpy_s(pNewData->ucReceiverAddress, 4, ucRxReceiverAddress,4);
							    pNewData->ucControlByte=ucRxControlByte;
							    memcpy_s(pNewData->ucSenderAddress, 4, ucRxSenderAddress,4);
							    pNewData->ucDataLength=ucRxDataLength;
							    memcpy_s(pNewData->ucFrameData, ucRxDataLength, ucRxFrameData,ucRxDataLength);
							    return pNewData;
						    }
					    }
					    if(ucRxFramePointer==MAX_RX_FRAME_LENGTH)
              {
						    listBox1->Items->Add("Maximale Framelänge überschritten!");
					    }
					    ucRxFramePointer++;
				    }
			    }
		    }
		    else
        {
			// Wenn nichts empfangen wurde:
		    return NULL;
        }
      }
    }
  }

  // File einlesen zum Zaehlen der Zeilen für die Fortschrittsanzeige:
  int get_file_lines(String^ fileName)
  {
    int count = 0;
    try 
    {
      StreamReader^ din = File::OpenText(fileName);
      String^ str;
      
      while ((str = din->ReadLine()) != nullptr) 
      {
        count++;
        //listBox1->Items->Add("line:" + count + ":" + str);
      }
    }
    catch (Exception^ e)
    {
      if (dynamic_cast<FileNotFoundException^>(e))
      {
        listBox1->Items->Add("file " + fileName+ " not found");
      }
      else
      {
        listBox1->Items->Add("problem reading file (progress indicator)" + fileName);
      }
    }
    //listBox1->Items->Add("count:" + count );
    return count;
  }

/************************************************************
 In Bootloader wechseln 
************************************************************/
int set_bootloader (unsigned long ulAddress)
{
    struct stData pFrame;
    struct stData *pReturnFrame;
    
    listBox1->Items->Add("Modul wird upgedated");
    crc16_init();
    listBox1->Items->Add("Senden von 'u' 10mal an alle ");
    pFrame.ucStartByte=FRAME_START_LONG;
    pFrame.ucControlByte=0x94;
    pFrame.ucDataLength=2;
    AddressHexToChar(pFrame.ucReceiverAddress,0xFFFFFFFF);
    pFrame.ucFrameData[0]='u';		// 0x75
    pFrame.ucFrameData[1]=0x00;   // Sensor; hier nur Dummy, da nicht erforderlich
    for (int i=0;i<10;i++)
    {
        SendFrame(&pFrame);
    }

    listBox1->Items->Add("Senden von 'p' (Paketgroesse abfragen) an Modul 0x" + ulAddress.ToString("X8"));
    pFrame.ucStartByte=FRAME_START_LONG;
    pFrame.ucControlByte=0x94;
    pFrame.ucDataLength=1;
    AddressHexToChar(pFrame.ucReceiverAddress,ulAddress);
    pFrame.ucFrameData[0]='p';		// 0x70
	pReturnFrame = Send(&pFrame);
	if ( pReturnFrame )
    {   
        listBox1->Items->Add("Antwort vom Modul mit Adresse 0x" + ulAddress.ToString("X8"));
        listBox1->Items->Add("Datenbyte 1 : 0x" + pReturnFrame->ucFrameData[0].ToString("X8"));
        listBox1->Items->Add("Datenbyte 2 : 0x" + pReturnFrame->ucFrameData[1].ToString("X8"));
        delete(pReturnFrame);
    }
    else
    {
        listBox1->Items->Add("ERROR if send 'p': Keine Antwort vom Modul, oder Fehler beim Senden ==> EXIT");
        return 1;
    }   
	
    listBox1->Items->Add("Senden von '0' (Paketgroesse abfragen) an Modul 0x" + ulAddress.ToString("X8"));
    pFrame.ucStartByte=FRAME_START_LONG;
    pFrame.ucControlByte=0x39;
    pFrame.ucDataLength=0;
    AddressHexToChar(pFrame.ucReceiverAddress,ulAddress);
    AddressHexToChar(pFrame.ucSenderAddress,0x00000000);
    SendFrame(&pFrame);
    SendFrame(&pFrame);
    return 0;
}
 
/************************************************************
 Sende Programmierdaten
************************************************************/
void program_page (unsigned long ulAddress, unsigned int page, unsigned int *buf)
{
    unsigned int i;
    struct stData pFrame;
    unsigned int  sendefolgenummer = 0;
    
    // HS485-Header neu setzen:
    // printf( " pFrame.ucDataLength: 0x%04x \n", pFrame.ucDataLength );
    AddressHexToChar(pFrame.ucReceiverAddress, ulAddress);
    pFrame.ucStartByte=FRAME_START_LONG;
    // Set Controlbyte:
    if ( page == 0)
    {
      pFrame.ucControlByte=0xB6;
    }
    else
    {
      pFrame.ucControlByte = ((sendefolgenummer * 2) & 0x06) + 0x30;
      // printf( "sendefolgenummer: %02x\n", sendefolgenummer );
      // printf( "pFrame.ucControlByte: %02x\n", pFrame.ucControlByte );
      sendefolgenummer++;
      if ( sendefolgenummer >= 4 )
      {
        sendefolgenummer = 0;
      }
    }
    pFrame.ucFrameData[0] = 'w';
    pFrame.ucFrameData[1] = page / 0x100;  // High-Adresse im Flash
    pFrame.ucFrameData[2] = page % 0x100;  // Low-Adresse im Flash
    pFrame.ucFrameData[3] = 0x40;    // Anzahl der Daten-Bytes
    //listBox1->Items->Add( "->Program HS485-Flash-Addr.: %02x%02x \n", pFrame.ucFrameData[1], pFrame.ucFrameData[2] );

    // pFrame.ucDataLength = hs485_databyte + 4;   // Datenlaenge (0x46)
    pFrame.ucDataLength = 0x40 + 4;   // Datenlaenge (0x46)
    for (i=0; i<SPM_PAGESIZE; i+=1)
    {
        unsigned int w = *buf++;
        pFrame.ucFrameData[i + 4] = w;   // Zu schreibendes Byte
        // printf("page: 0x%04x; i: %i, w: 0x%02x\n", page, i, w);
    }
    Send(&pFrame);

}
 
/************************************************************
 Umwandlung Hex String in numerischen Wert
************************************************************/
static unsigned int hex2num (const unsigned int * ascii, unsigned int num)
{
    unsigned int  i;
    unsigned int val = 0;
 
    for (i=0; i<num; i++)
    {
        unsigned int c = ascii[i];
 
        /* Hex-Ziffer auf ihren Wert abbilden */
        if (c >= '0' && c <= '9')            c -= '0';  
        else if (c >= 'A' && c <= 'F')       c -= 'A' - 10;
        else if (c >= 'a' && c <= 'f')       c -= 'a' - 10;
 
        val = 16 * val + c;
    }
 
    return val;  
}

/************************************************************
    Bootloadermodus / Updatemodus beenden:
************************************************************/  
void ExitBootloadermode(unsigned long ulAddress)  
{
  struct stData pFrame;
  
  listBox1->Items->Add("Bootloadermodus beenden");
  //listBox1->Items->Add(" Senden von '0' (Paketgroesse abfragen) an Modul 0x%08x\n",ulAddress);
  pFrame.ucStartByte=FRAME_START_LONG;
  pFrame.ucControlByte=0x59;
  pFrame.ucDataLength=0;
  AddressHexToChar(pFrame.ucReceiverAddress,ulAddress);
  AddressHexToChar(pFrame.ucSenderAddress,0x00000000);
  SendFrame(&pFrame);
  
  listBox1->Items->Add(" Senden von 'u' 10mal an alle ");
  pFrame.ucStartByte=FRAME_START_LONG;
  pFrame.ucControlByte=0xD0;
  pFrame.ucDataLength=2;
  AddressHexToChar(pFrame.ucReceiverAddress,0xFFFFFFFF);
  pFrame.ucFrameData[0]='g';		// 0x67
  pFrame.ucFrameData[1]=0x00;     // Sensor; hier nur Dummy, da nicht erforderlich
  for (int i=0;i<10;i++)
  {
    SendFrame(&pFrame);
  }
  //fclose(fp);
  listBox1->Items->Add("UPDATE beendet !");
}

/************************************************************
 Firmwareupdate von einem Modul
************************************************************/
int FirmwareUpdate()
{	
	/* Empfangenes Zeichen + Statuscode */
	//unsigned int c = 0, 
	String^ str;
	/* Intel-HEX Zieladresse */
	unsigned int hex_addr = 0,
	/* Zu schreibende Flash-Page */
	flash_page = 0,
	/* Intel-HEX Checksumme zum Ueberpruefen des Daten */
	hex_check = 0,
	/* Positions zum Schreiben in der Datenpuffer */
	flash_cnt = 0;
	/* temporaere Variable */
	unsigned int temp,
	/* Flag zum steuern des Programmiermodus */
	boot_state = BOOT_STATE_EXIT,
	/* Empfangszustandssteuerung */
	parser_state = PARSER_STATE_START,
	/* Flag zum ermitteln einer neuen Flash-Page */
	flash_page_flag = 1,
	/* Datenpuffer fuer die Hexdaten*/
	flash_data[SPM_PAGESIZE], 
	/* Position zum Schreiben in den HEX-Puffer */
	hex_cnt = 0, 
	/* Puffer fuer die Umwandlung der ASCII in Binaerdaten */
	hex_buffer[5], 
	/* Intel-HEX Datenlaenge */
	hex_size = 0,
	/* Zaehler fuer die empfangenen HEX-Daten einer Zeile */
	hex_data_cnt = 0, 
	/* Intel-HEX Recordtype */
	hex_type = 0, 
	/* empfangene HEX-Checksumme */
	hex_checksum=0;
	/*  */
	int count = 0;
	/* Target Address */
	unsigned int  ulAddress;
	/* Progress Bar */
	unsigned int  iZeile = 0;

	/* Fuellen der Puffer mit definierten Werten */
	memset(hex_buffer, 0x00, sizeof(hex_buffer));
	memset(flash_data, 0xFF, sizeof(flash_data));
	
	listBox1->Items->Add("Start Firmware Update");
	listBox1->Items->Add("Open File " + openFileDialog1->FileName);
	//String^ fileName;
	String^ fileName = "d:\\Projekte\\HAUS_BUS\\HS485\\Module\\HS485S\\Firmware-ATmega8\\hs485s_hw1_sw2_00.hex";

	// File einlesen zum Zaehlen der Zeilen für die Fortschrittsanzeige:
	count = get_file_lines(fileName);
	listBox1->Items->Add("Zeilen in File: " + count);

    
	//if ( fp == NULL )
	if ( 1 == 2 )
	{
		listBox1->Items->Add("Fehler beim Öffnen des Files " + openFileDialog1->FileName );
	}
	else
	{
		listBox1->Items->Add("Öffnen des Files " + openFileDialog1->FileName + " erfolgreich" );
		// Adresse uebernehmen:
		ulAddress = Decimal::ToUInt32(numericUpDown1->Value) ;
		listBox1->Items->Add("ulAddress: 0x" + ulAddress.ToString("X2") );

		// Wechsle zu Bootloader: 
		if (set_bootloader(ulAddress) != 0)
		{
			ExitBootloadermode(ulAddress);
			listBox1->Items->Add(" ==> ExitBootloadermode");
			return 1;
		}
		unsigned int bytecount, address, recordtype, checksum;
		
		// Listbox scrollt an das Ende:
		listBox1->TopIndex = listBox1->Items->Count-1;

		boot_state = BOOT_STATE_PARSER;   
		try 
		{
			StreamReader^ din = File::OpenText(openFileDialog1->FileName);
			String^ str;

			while (!din->EndOfStream) 
			//do
			{
				str = din->ReadLine();
				iZeile++;
          
				//listBox1->Items->Add("Zeile " + iZeile + " aus Intel-Hex-Datei lesen");
				//label7->Text = iZeile.ToString("d");
				// Progress Bar:
				progressBar1->Value = iZeile * 100 / count;
				label5->Text = iZeile * 100 / count + " %";
				listBox1->Items->Add("line:" + iZeile + ", " + str);
				listBox1->Items->Add("parser_state: " + parser_state);

				/* Programmzustand: Parser */
				if (boot_state == BOOT_STATE_PARSER)
				{
					//listBox1->Items->Add("BOOT_STATE_PARSER");
					switch(parser_state)
					{
						/* Warte auf Zeilen-Startzeichen */
						case PARSER_STATE_START:			
							if ( str->StartsWith(START_SIGN) )
							{
								//hex_cnt = 0;
								hex_check = 0;
								listBox1->Items->Add("PARSER_STATE_START");

								/* Datengroesse */
								hex_size = Convert::ToUInt32(str->Substring(1,2), 16);
								listBox1->Items->Add("Intel-Hex-bytecount 0x" + hex_size.ToString("X2") );
								if (hex_size != 0x10)
								{
									//listBox1->Items->Add("- hex_size: 0x%02x\n", hex_size);
								}
								hex_check += hex_size;

								/* Zieladresse */
								hex_addr = Convert::ToUInt32(str->Substring(3,4), 16);
								listBox1->Items->Add("Intel-Hex-address: 0x" + hex_addr.ToString("X4") );
								if (flash_page_flag) 
								{
									flash_page = hex_addr - hex_addr % SPM_PAGESIZE;
									flash_page_flag = 0;
								}
								hex_check += (int) hex_addr;
								hex_check += (int) (hex_addr >> 8);

								/* Zeilentyp */
								hex_type = Convert::ToUInt32(str->Substring(7,2), 16);
								listBox1->Items->Add("Intel-Hex-recordtype: " + hex_type.ToString("X2") );
								hex_data_cnt = 0;
								if (hex_type != 0)
								{
									listBox1->Items->Add("- hex_type: 0x" + hex_type.ToString("X2"));
								}
								switch(hex_type)
								{
									case 0: parser_state = PARSER_STATE_DATA; break;
									case 1: parser_state = PARSER_STATE_CHECKSUM; break;
									default: parser_state = PARSER_STATE_DATA; break;
								}
								hex_check += hex_type;

								/* Flash-Daten */
								for (unsigned int data_cnt = 0; data_cnt < hex_size ; data_cnt++ )
								{
									unsigned int hex_data = Convert::ToUInt32(str->Substring(data_cnt * 2 + 9, 2), 16);
									flash_data[flash_cnt] = hex_data;
									listBox1->Items->Add("Intel-Hex-Data: " + hex_data.ToString("X2") );
									flash_cnt++;
									hex_check += hex_data;
									hex_data_cnt++;
									if (hex_data_cnt == hex_size)
									{
										parser_state = PARSER_STATE_CHECKSUM;
										hex_data_cnt = 0;
										hex_cnt = 0;
									}

									/* Puffer voll -> schreibe Page */
									if (flash_cnt == SPM_PAGESIZE)
									{
										listBox1->Items->Add("Puffer voll -> schreibe Page");
										program_page(ulAddress, (int)flash_page, flash_data);
										memset(flash_data, 0xFF, sizeof(flash_data));
										flash_cnt = 0;
										flash_page_flag = 1;
									}
								}

								/* Checksumme*/
								hex_checksum = Convert::ToUInt32(str->Substring(hex_size * 2 + 9, 2), 16);
								listBox1->Items->Add("Intel-Hex-checksum: 0x" + hex_checksum.ToString("X2") );
								hex_check += hex_checksum;
								hex_check &= 0x00FF;

								/* Dateiende -> schreibe Restdaten */ 
								if (hex_type == 1)
								{
									listBox1->Items->Add("Dateiende -> schreibe Restdaten");
									program_page(ulAddress, (int)flash_page, flash_data);
									boot_state = BOOT_STATE_EXIT;
								}

								/* Ueberpruefe Checksumme -> muss '0' sein */
								if (hex_check == 0) 
								{
									// listBox1->Items->Add("Checksum OK");
									parser_state = PARSER_STATE_START;
								}
								else
								{                                  
									listBox1->Items->Add("Checksum ERROR");
									parser_state = PARSER_STATE_ERROR;
								}
							}
							break;
                          
                      /* Parserfehler (falsche Checksumme) */
                      case PARSER_STATE_ERROR:
                          listBox1->Items->Add("Parserfehler (falsche Checksumme) ==> EXIT!");
                          return 1;
                          break;			
                      default:
                          break;
					  }  // switch
				 }  // if
			}  // do
	        //while (boot_state!=BOOT_STATE_EXIT);
		}  // try
	
		catch (Exception^ e)
		{
			if (dynamic_cast<FileNotFoundException^>(e))
			{
				listBox1->Items->Add("Exception: file " + openFileDialog1->FileName+ " not found");
			}
			else
			{
				listBox1->Items->Add("Exception: problem reading file" + openFileDialog1->FileName);
			}
		listBox1->Items->Add("Close File");
		}
    
    }  // else
	listBox1->Items->Add("Exit to Bootloadermode of AVR!");
	ExitBootloadermode(ulAddress);

	//} // end-less-while
	//progressBar1->Value = 100;
	//label5->Text = "100 %";

	return 0;
}

private: System::Void listBox1_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e)
  {
  }

private: System::Void Form1_Load(System::Object^  sender, System::EventArgs^  e) 
  {
  }

private: System::Void comboBox1_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) 
  {
  }

private: System::Void groupBox2_Enter(System::Object^  sender, System::EventArgs^  e) 
  {
  }

private: System::Void label1_Click(System::Object^  sender, System::EventArgs^  e) 
  {
  }

private: System::Void label1_Click_1(System::Object^  sender, System::EventArgs^  e) 
  {
  }

private: System::Void openFileDialog1_FileOk(System::Object^  sender, System::ComponentModel::CancelEventArgs^  e) 
  {
  }

private: System::Void serialPort1_DataReceived(System::Object^  sender, System::IO::Ports::SerialDataReceivedEventArgs^  e) 
  {
    // recv. Byte
    //ucRByetx = serialPort1->ReadByte();
  }

private: System::Void button4_Click(System::Object^  sender, System::EventArgs^  e) 
  {
    FirmwareUpdate();
  }

private: System::Void numericUpDown1_ValueChanged(System::Object^  sender, System::EventArgs^  e) 
  {
  }

private: System::Void listBox1_MouseDown(System::Object^  sender, System::Windows::Forms::MouseEventArgs^  e) 
  {
  }

private: System::Void button5_Click(System::Object^  sender, System::EventArgs^  e) 
  {
    // Clear List Box:
    listBox1->Items->Clear();

  }

};
}

