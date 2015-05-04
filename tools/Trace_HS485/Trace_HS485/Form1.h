#include <string.h>

#pragma once

namespace Trace_HS485 {

	using namespace System;
	using namespace System::ComponentModel;
	using namespace System::Collections;
	using namespace System::Windows::Forms;
	using namespace System::Data;
	using namespace System::Drawing;
  using namespace System::IO::Ports;
  using namespace System::IO;

	/// <summary>
	/// Zusammenfassung für Form1
	/// </summary>
	public ref class Form1 : public System::Windows::Forms::Form
	{
	public:
    String^ ByteString;
    //ByteString = gcnew String^; 

		Form1(void)
		{
			InitializeComponent();
			//
			//TODO: Konstruktorcode hier hinzufügen.
			//
      ByteString = "";

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
  private: System::Windows::Forms::GroupBox^  groupBox1;
  protected: 
  private: System::Windows::Forms::Button^  button3;
  private: System::Windows::Forms::Button^  button2;
  private: System::Windows::Forms::Button^  button1;
  private: System::Windows::Forms::ComboBox^  comboBox1;
  private: System::Windows::Forms::ListBox^  listBox1;
  private: System::IO::Ports::SerialPort^  serialPort1;
  private: System::Windows::Forms::Timer^  timer1;
  private: System::Windows::Forms::GroupBox^  groupBox2;
  private: System::Windows::Forms::Timer^  timer2;
  private: System::Windows::Forms::Button^  button4;
  private: System::Windows::Forms::Button^  button5;
  private: System::Windows::Forms::SaveFileDialog^  saveFileDialog1;
  private: System::Windows::Forms::CheckBox^  checkBox1;




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
      this->groupBox1 = (gcnew System::Windows::Forms::GroupBox());
      this->checkBox1 = (gcnew System::Windows::Forms::CheckBox());
      this->button5 = (gcnew System::Windows::Forms::Button());
      this->button4 = (gcnew System::Windows::Forms::Button());
      this->button3 = (gcnew System::Windows::Forms::Button());
      this->button2 = (gcnew System::Windows::Forms::Button());
      this->button1 = (gcnew System::Windows::Forms::Button());
      this->comboBox1 = (gcnew System::Windows::Forms::ComboBox());
      this->listBox1 = (gcnew System::Windows::Forms::ListBox());
      this->serialPort1 = (gcnew System::IO::Ports::SerialPort(this->components));
      this->timer1 = (gcnew System::Windows::Forms::Timer(this->components));
      this->groupBox2 = (gcnew System::Windows::Forms::GroupBox());
      this->timer2 = (gcnew System::Windows::Forms::Timer(this->components));
      this->saveFileDialog1 = (gcnew System::Windows::Forms::SaveFileDialog());
      this->groupBox1->SuspendLayout();
      this->groupBox2->SuspendLayout();
      this->SuspendLayout();
      // 
      // groupBox1
      // 
      this->groupBox1->Anchor = static_cast<System::Windows::Forms::AnchorStyles>(((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Left) 
        | System::Windows::Forms::AnchorStyles::Right));
      this->groupBox1->Controls->Add(this->checkBox1);
      this->groupBox1->Controls->Add(this->button5);
      this->groupBox1->Controls->Add(this->button4);
      this->groupBox1->Controls->Add(this->button3);
      this->groupBox1->Controls->Add(this->button2);
      this->groupBox1->Controls->Add(this->button1);
      this->groupBox1->Location = System::Drawing::Point(12, 12);
      this->groupBox1->Name = L"groupBox1";
      this->groupBox1->Size = System::Drawing::Size(748, 54);
      this->groupBox1->TabIndex = 0;
      this->groupBox1->TabStop = false;
      this->groupBox1->Text = L"Control";
      this->groupBox1->Enter += gcnew System::EventHandler(this, &Form1::groupBox1_Enter);
      // 
      // checkBox1
      // 
      this->checkBox1->AutoSize = true;
      this->checkBox1->Location = System::Drawing::Point(456, 23);
      this->checkBox1->Name = L"checkBox1";
      this->checkBox1->Size = System::Drawing::Size(77, 17);
      this->checkBox1->TabIndex = 4;
      this->checkBox1->Text = L"Auto-Scroll";
      this->checkBox1->UseVisualStyleBackColor = true;
      // 
      // button5
      // 
      this->button5->Location = System::Drawing::Point(375, 19);
      this->button5->Name = L"button5";
      this->button5->Size = System::Drawing::Size(75, 23);
      this->button5->TabIndex = 3;
      this->button5->Text = L"Save";
      this->button5->UseVisualStyleBackColor = true;
      this->button5->Click += gcnew System::EventHandler(this, &Form1::button5_Click);
      // 
      // button4
      // 
      this->button4->Location = System::Drawing::Point(191, 19);
      this->button4->Name = L"button4";
      this->button4->Size = System::Drawing::Size(97, 23);
      this->button4->TabIndex = 3;
      this->button4->Text = L"Refresh Ports";
      this->button4->UseVisualStyleBackColor = true;
      this->button4->Click += gcnew System::EventHandler(this, &Form1::button4_Click);
      // 
      // button3
      // 
      this->button3->Anchor = System::Windows::Forms::AnchorStyles::Right;
      this->button3->Location = System::Drawing::Point(667, 19);
      this->button3->Name = L"button3";
      this->button3->Size = System::Drawing::Size(75, 23);
      this->button3->TabIndex = 2;
      this->button3->Text = L"Clear Output";
      this->button3->UseVisualStyleBackColor = true;
      this->button3->Click += gcnew System::EventHandler(this, &Form1::button3_Click);
      // 
      // button2
      // 
      this->button2->Location = System::Drawing::Point(294, 19);
      this->button2->Name = L"button2";
      this->button2->Size = System::Drawing::Size(75, 23);
      this->button2->TabIndex = 1;
      this->button2->Text = L"Close Port";
      this->button2->UseVisualStyleBackColor = true;
      this->button2->Click += gcnew System::EventHandler(this, &Form1::button2_Click);
      // 
      // button1
      // 
      this->button1->Location = System::Drawing::Point(106, 19);
      this->button1->Name = L"button1";
      this->button1->Size = System::Drawing::Size(75, 23);
      this->button1->TabIndex = 0;
      this->button1->Text = L"Open Port";
      this->button1->UseVisualStyleBackColor = true;
      this->button1->Click += gcnew System::EventHandler(this, &Form1::button1_Click);
      // 
      // comboBox1
      // 
      this->comboBox1->FormattingEnabled = true;
      this->comboBox1->Location = System::Drawing::Point(22, 31);
      this->comboBox1->Name = L"comboBox1";
      this->comboBox1->Size = System::Drawing::Size(90, 21);
      this->comboBox1->Sorted = true;
      this->comboBox1->TabIndex = 1;
      // 
      // listBox1
      // 
      this->listBox1->Dock = System::Windows::Forms::DockStyle::Fill;
      this->listBox1->Font = (gcnew System::Drawing::Font(L"Courier New", 11, System::Drawing::FontStyle::Regular, System::Drawing::GraphicsUnit::Point, 
        static_cast<System::Byte>(0)));
      this->listBox1->ForeColor = System::Drawing::Color::Green;
      this->listBox1->HorizontalScrollbar = true;
      this->listBox1->ItemHeight = 17;
      this->listBox1->Location = System::Drawing::Point(3, 16);
      this->listBox1->Name = L"listBox1";
      this->listBox1->SelectionMode = System::Windows::Forms::SelectionMode::None;
      this->listBox1->Size = System::Drawing::Size(743, 620);
      this->listBox1->TabIndex = 2;
      this->listBox1->SelectedIndexChanged += gcnew System::EventHandler(this, &Form1::listBox1_SelectedIndexChanged);
      // 
      // serialPort1
      // 
      this->serialPort1->Parity = System::IO::Ports::Parity::Even;
      // 
      // timer1
      // 
      this->timer1->Interval = 10;
      this->timer1->Tick += gcnew System::EventHandler(this, &Form1::timer1_Tick);
      // 
      // groupBox2
      // 
      this->groupBox2->Anchor = static_cast<System::Windows::Forms::AnchorStyles>((((System::Windows::Forms::AnchorStyles::Top | System::Windows::Forms::AnchorStyles::Bottom) 
        | System::Windows::Forms::AnchorStyles::Left) 
        | System::Windows::Forms::AnchorStyles::Right));
      this->groupBox2->AutoSize = true;
      this->groupBox2->Controls->Add(this->listBox1);
      this->groupBox2->Location = System::Drawing::Point(12, 81);
      this->groupBox2->Name = L"groupBox2";
      this->groupBox2->Size = System::Drawing::Size(749, 639);
      this->groupBox2->TabIndex = 3;
      this->groupBox2->TabStop = false;
      this->groupBox2->Text = L"Trace HS485";
      this->groupBox2->Enter += gcnew System::EventHandler(this, &Form1::groupBox2_Enter);
      // 
      // timer2
      // 
      this->timer2->Interval = 1000;
      this->timer2->Tick += gcnew System::EventHandler(this, &Form1::timer2_Tick);
      // 
      // Form1
      // 
      this->AutoScaleDimensions = System::Drawing::SizeF(6, 13);
      this->AutoScaleMode = System::Windows::Forms::AutoScaleMode::Font;
      this->ClientSize = System::Drawing::Size(773, 732);
      this->Controls->Add(this->comboBox1);
      this->Controls->Add(this->groupBox1);
      this->Controls->Add(this->groupBox2);
      this->Name = L"Form1";
      this->Text = L"HS485-Tracer";
      this->Load += gcnew System::EventHandler(this, &Form1::Form1_Load);
      this->groupBox1->ResumeLayout(false);
      this->groupBox1->PerformLayout();
      this->groupBox2->ResumeLayout(false);
      this->ResumeLayout(false);
      this->PerformLayout();

    }
#pragma endregion
  private: System::Void button1_Click(System::Object^  sender, System::EventArgs^  e) 
  {
    serialPort1->PortName = comboBox1->Text;
    serialPort1->BaudRate = 19200;
    serialPort1->DataBits = 8;
    serialPort1->Parity = Parity::Even;
    serialPort1->StopBits = StopBits::One;
    serialPort1->ReadTimeout = 100;
    serialPort1->Open();

    DateTime datum = DateTime::Now;
    String ^myTime = datum.ToString("T");

    listBox1->Items->Add(myTime + " Open Port " + comboBox1->Text + " with 19200 Baud");
    // Listbox scrollt an das Ende:
    listBox1->TopIndex = listBox1->Items->Count-1;

    timer1->Enabled = true;

  }
  private: System::Void button2_Click(System::Object^  sender, System::EventArgs^  e) 
  {
    serialPort1->Close();
    listBox1->Items->Add("Close Port " + comboBox1->Text);	
    // Listbox scrollt an das Ende:
    listBox1->TopIndex = listBox1->Items->Count-1;

    timer1->Enabled = false;
  }

  private: System::Void serialPort1_DataReceived(System::Object^  sender, System::IO::Ports::SerialDataReceivedEventArgs^  e) 
  {
  }

  private: System::Void timer1_Tick(System::Object^  sender, System::EventArgs^  e) 
  {
    int RecvByte;

    while (serialPort1->BytesToRead > 0 )
    {
      timer2->Enabled = false;
      RecvByte = serialPort1->ReadByte();
      if (RecvByte == 0xFD || RecvByte == 0xFE)
      {
        if ( Form1::ByteString->Length > 1 )
        {
          DateTime datum = DateTime::Now;
          String ^myTime = datum.ToString("T");
          listBox1->Items->Add(myTime + " " + Form1::ByteString);
          // Listbox scrollt an das Ende:
          if (checkBox1->Checked == true)
          {
            listBox1->TopIndex = listBox1->Items->Count-1;
          }
        }
        Form1::ByteString = "";
        
      }
      Form1::ByteString = Form1::ByteString + RecvByte.ToString("X2") + " ";
    }
    timer2->Enabled = true;
    /*
    if ( Form1::ByteString->Length > 1 )
    {
      listBox1->Items->Add(Form1::ByteString);
      Form1::ByteString = ".";
      // Listbox scrollt an das Ende:
      listBox1->TopIndex = listBox1->Items->Count-1;
    }
    */
  }

private: System::Void button3_Click(System::Object^  sender, System::EventArgs^  e) 
  { 
    // Clear List Box:
    listBox1->Items->Clear();
  }

private: System::Void Form1_Load(System::Object^  sender, System::EventArgs^  e) 
  {
  }

private: System::Void groupBox1_Enter(System::Object^  sender, System::EventArgs^  e) {
         }
private: System::Void listBox1_SelectedIndexChanged(System::Object^  sender, System::EventArgs^  e) {
         }
private: System::Void groupBox2_Enter(System::Object^  sender, System::EventArgs^  e) {
         }
private: System::Void timer2_Tick(System::Object^  sender, System::EventArgs^  e) 
  {
    timer2->Enabled = false;
    if ( Form1::ByteString->Length > 1 )
    {
      DateTime datum = DateTime::Now;
      String ^myTime = datum.ToString("T");
      listBox1->Items->Add(myTime + " " + Form1::ByteString);
      Form1::ByteString = ",";
      // Listbox scrollt an das Ende:
      listBox1->TopIndex = listBox1->Items->Count-1;
    }
  }

private: System::Void button4_Click(System::Object^  sender, System::EventArgs^  e) 
  {
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
    comboBox1->Items->Clear();
    // Display each port name to the console.
    for each(String^ port in serialPorts)
    {
      comboBox1->Items->Add(port);
    }
    comboBox1->Text = serialPorts[0];
  }

private: System::Void button5_Click(System::Object^  sender, System::EventArgs^  e) 
  {
    saveFileDialog1->Filter = "Log file - *.log | *.log";
    if (saveFileDialog1->ShowDialog() == System::Windows::Forms::DialogResult::OK )
    {
      if (saveFileDialog1->FileName != "")
      {
        StreamWriter^ sw = gcnew StreamWriter(saveFileDialog1->FileName);
        for each (String^ item in listBox1->Items)
        {
          sw->WriteLine(item);
        }
        sw->Close();
      }
    }
  }
};
}

