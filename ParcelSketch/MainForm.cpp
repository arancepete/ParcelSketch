#include "MainForm.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Linq;
using namespace System::Windows::Forms;


[STAThread]
void Main(array<String^>^ args)
{
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);

	ParcelSketch::MainForm form;
	Application::Run(%form);

}
