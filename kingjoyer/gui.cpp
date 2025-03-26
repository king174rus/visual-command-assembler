#include "gui.h"

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx9.h"
#include "../imgui/imgui_impl_win32.h"

#include <iostream>
#include "string"
#include <chrono>
#include <algorithm> // для std::transform
#include <cctype>    // для std::toupper

using namespace std::chrono;
using namespace std;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND window,
	UINT message,
	WPARAM wideParameter,
	LPARAM longParameter
);

long __stdcall WindowProcess(
	HWND window,
	UINT message,
	WPARAM wideParameter,
	LPARAM longParameter)
{
	if (ImGui_ImplWin32_WndProcHandler(window, message, wideParameter, longParameter))
		return true;

	switch (message)
	{
	case WM_SIZE: {
		if (gui::device && wideParameter != SIZE_MINIMIZED)
		{
			gui::presentParameters.BackBufferWidth = LOWORD(longParameter);
			gui::presentParameters.BackBufferHeight = HIWORD(longParameter);
			gui::ResetDevice();
		}
	}return 0;

	case WM_SYSCOMMAND: {
		if ((wideParameter & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
	}break;

	case WM_DESTROY: {
		PostQuitMessage(0);
	}return 0;

	case WM_LBUTTONDOWN: {
		gui::position = MAKEPOINTS(longParameter); // set click points
	}return 0;
	/**	case WM_ERASEBKGND:
	{
		HDC hdc = (HDC)wideParameter;

		// Задайте цвет фона
		HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 0)); // Красный цвет фона

		RECT rect;
		GetClientRect(window, &rect);
		FillRect(hdc, &rect, hBrush);

		DeleteObject(hBrush);

		return 1;
	}
	break;	**/
	case WM_MOUSEMOVE: {
		if (wideParameter == MK_LBUTTON)
		{
			const auto points = MAKEPOINTS(longParameter);
			auto rect = ::RECT{ };

			GetWindowRect(gui::window, &rect);

			rect.left += points.x - gui::position.x;
			rect.top += points.y - gui::position.y;

			if (gui::position.x >= 0 &&
				gui::position.x <= gui::WIDTH &&
				gui::position.y >= 0 && gui::position.y <= 19)
				SetWindowPos(
					gui::window,
					HWND_TOPMOST,
					rect.left,
					rect.top,
					0, 0,
					SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER
				);
		}

	}return 0;

	}

	return DefWindowProc(window, message, wideParameter, longParameter);
}

void gui::CreateHWindow(const char* windowName) noexcept
{
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_CLASSDC;
	windowClass.lpfnWndProc = WindowProcess;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandleA(0);
	windowClass.hIcon = 0;
	windowClass.hCursor = 0;
	windowClass.hbrBackground = 0;
	windowClass.lpszMenuName = 0;
	windowClass.lpszClassName = "class001";
	windowClass.hIconSm = 0;

	RegisterClassEx(&windowClass);

	window = CreateWindowEx(
		0,
		"class001",
		windowName,
		WS_POPUP,
		100,
		100,
		WIDTH,
		HEIGHT,
		0,
		0,
		windowClass.hInstance,
		0
	);

	ShowWindow(window, SW_SHOWDEFAULT);
	UpdateWindow(window);
}

void gui::DestroyHWindow() noexcept
{
	DestroyWindow(window);
	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
}

bool gui::CreateDevice() noexcept
{
	d3d = Direct3DCreate9(D3D_SDK_VERSION);

	if (!d3d)
		return false;

	ZeroMemory(&presentParameters, sizeof(presentParameters));

	presentParameters.Windowed = TRUE;
	presentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentParameters.BackBufferFormat = D3DFMT_UNKNOWN;
	presentParameters.EnableAutoDepthStencil = TRUE;
	presentParameters.AutoDepthStencilFormat = D3DFMT_D16;
	presentParameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	if (d3d->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		window,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&presentParameters,
		&device) < 0)
		return false;

	return true;
}

void gui::ResetDevice() noexcept
{
	ImGui_ImplDX9_InvalidateDeviceObjects();

	const auto result = device->Reset(&presentParameters);

	if (result == D3DERR_INVALIDCALL)
		IM_ASSERT(0);

	ImGui_ImplDX9_CreateDeviceObjects();
}

void gui::DestroyDevice() noexcept
{
	if (device)
	{
		device->Release();
		device = nullptr;
	}

	if (d3d)
	{
		d3d->Release();
		d3d = nullptr;
	}
}

void gui::CreateImGui() noexcept
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\courbd.ttf", 16.5f, NULL, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
	io.IniFilename = NULL;

	ImGui::StyleColorsDark();

	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(device);
}

void gui::DestroyImGui() noexcept
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void gui::BeginRender() noexcept
{
	MSG message;
	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);

		if (message.message == WM_QUIT)
		{
			isRunning = !isRunning;
			return;
		}
	}

	// Start the Dear ImGui frame
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void gui::EndRender() noexcept
{
	ImGui::EndFrame();

	device->SetRenderState(D3DRS_ZENABLE, FALSE);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 255), 1.0f, 0);

	if (device->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		device->EndScene();
	}

	const auto result = device->Present(0, 0, 0, 0);

	// Handle loss of D3D9 device
	if (result == D3DERR_DEVICELOST && device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		ResetDevice();
}

auto start = high_resolution_clock::now();

char buf[255];
char buf1[255];
char bufznach[9];
char bufaddress[9];

string byte1 = "00";
string byte2 = "00";
string byte3 = "00";
string byte4 = "00";
string adresbyte1 = "00";
string adresbyte2 = "05";
string adresbyte3 = "A4";
string adresbyte4 = "20";
string znachbyte1 = "20";
string znachbyte2 = "11";
string znachbyte3 = "5A";
string znachbyte4 = "4B";
string znachperemenbyte1 = "20";
string znacperemenhbyte2 = "11";
string znacperemenhbyte3 = "5A";
string znacperemenhbyte4 = "4B";
string registr = " ";
string peremennai = "4B5A1120";
string adres = "0005A420";
string resultString = "";
string glavregistr = "00000000";

float greenznach = 1;
float blueznach = 1;
float redznach = 1;
float greenadres = 1;
float blueadres = 1;
float redadres = 1;
float green = 1;
float blue = 1;
float red = 1;
float green1 = 0;
float blue1 = 0;
float red1 = 0;
float redbutton = 0.11;
float greenbutton = 0.56;
float bluebutton = 1;

const char* Registres[] = { "EAX", "AX", "AH", "AL","EBX", "BX", "BH", "BL" ,"ECX", "CX", "CH", "CL" ,"EDX", "DX", "DH", "DL", "ESI", "SI", "EDI", "DI"};
static int NumberRegistr = 0;
const char* TypePeremennoi[] = { "dd", "dw", "db"};
static int NumberTypePeremennoi = 0;
const char* Elements[] = { "x1", "[x1]", "EAX", "AX", "AH", "AL","EBX", "BX", "BH", "BL" ,"ECX", "CX", "CH", "CL" ,"EDX", "DX", "DH", "DL", "ESI", "SI", "EDI", "DI",
						  "[EAX]", "[AX]", "[AH]", "[AL]","[EBX]", "[BX]", "[BH]", "[BL]" ,"[ECX]", "[CX]", "[CH]", "[CL]" ,"[EDX]", "[DX]", "[DH]", "[DL]", "[ESI]", "[SI]", "[EDI]", "[DI]" };
static int NumberElements = 0;
string ElementsCommands[2][5] = { {"0005A420", "00", "05", "A4", "20"}, {"00401000", "00", "40", "10","00"} };
static int NumberElementCommands = 0;

const char* Commads[] = {"LEA", "MOV", "LODSB", "LODSW", "LODSD", "STOSB", "STOSW", "STOSD"};
static int NumberCommand = -1;
float timer = 0.0f;
float chet = 0.0f;
int error = 0;
bool isBlinking = false;
string byteregistrs[6][4] = { {"00","00","00","00"},{"00","00","00","00"} ,{"00","00","00","00"} ,{"00","00","00","00"},{"00","40","10","00"},{"00","40","10","00"} };
int otchet = 0;
int bitysInRegistr = 4;
int Registr = 0;


void BlinkingText(const char* text)
{
	if (isBlinking)
	{
		timer += ImGui::GetIO().DeltaTime;

		if (timer > 3.0f)
		{
			timer = 0.0f;
		}
		else if (timer > 1.5f)
		{
			chet += ImGui::GetIO().DeltaTime;
			ImGui::TextColored(ImVec4(1.0, 1.0, 1.0, 0.0), text);
			return;
		}
	}
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1),text);
}
void ByteText(string byte[4])
{
	ImGuiID id;
	for (int i = 0; i < 4; i++)
	{
		id = i;
		ImGui::BeginChild(id, ImVec2(40, 40), true);
		if (byte[i] == "Blink")
		{
			BlinkingText((const char*)byteregistrs[Registr][i].c_str());
		}
		else if (byte[i] == "White")
		{
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)byteregistrs[Registr][i].c_str());
		}
		else if (byte[i] == "Red")
		{
			ImGui::TextColored(ImVec4(1, 0, 0, 1), (const char*)byteregistrs[Registr][i].c_str());
		}
		else if (byte[i] == "Gray")
		{
			ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[Registr][i].c_str());
		}
		ImGui::EndChild();
		if(i != 3) ImGui::SameLine(0, 0);
	}
	return;
}
unsigned long long convertStringtoHex(string str)
{
	unsigned long long hex;
	std::stringstream ss;
	ss << std::hex << str;
	ss >> hex;
	return hex;
}
stringstream convertHextoString(unsigned long long hex)
{
	std::stringstream ss;
	ss << std::hex << hex;
	return ss;
}
void gui::Render() noexcept
{
	if (NumberRegistr % 4 == 0 or NumberRegistr == 18) bitysInRegistr = 4;
	else if (NumberRegistr % 4 == 1 or NumberRegistr == 19) bitysInRegistr = 2;
	else bitysInRegistr = 1;
	if (NumberRegistr > 17) Registr = 5;
	else Registr = NumberRegistr / 4;
	ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::SetNextWindowSize({ WIDTH, HEIGHT });
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(red, green, blue, 1.0f)); // Устанавливаем цвет фона окна на тёмно-серый


	ImGui::Begin(
		(const char*)u8"Visual Command Assembler",
		&isRunning,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoMove 
	);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(red1, green1, blue1, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(redbutton, greenbutton, bluebutton, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(red, green, blue, 1.0f)); // Установите цвет для выпадающего окна
	
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)u8"Регистр");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.08f);
	ImGui::Combo((const char*)u8"", &NumberRegistr, Registres, IM_ARRAYSIZE(Registres));
	registr = Registres[NumberRegistr];
	ImGui::SameLine(670, 0);
	std::transform(znachbyte1.begin(), znachbyte1.end(), znachbyte1.begin(),
		[](unsigned char c) { return std::toupper(c); });
	std::transform(znachbyte2.begin(), znachbyte2.end(), znachbyte2.begin(),
		[](unsigned char c) { return std::toupper(c); });
	std::transform(znachbyte3.begin(), znachbyte3.end(), znachbyte3.begin(),
		[](unsigned char c) { return std::toupper(c); });
	std::transform(znachbyte4.begin(), znachbyte4.end(), znachbyte4.begin(),
		[](unsigned char c) { return std::toupper(c); });
	std::transform(byte1.begin(), byte1.end(), byte1.begin(),
		[](unsigned char c) { return std::toupper(c); });
	std::transform(byte2.begin(), byte2.end(), byte2.begin(),
		[](unsigned char c) { return std::toupper(c); });
	std::transform(byte3.begin(), byte3.end(), byte3.begin(),
		[](unsigned char c) { return std::toupper(c); });
	std::transform(byte4.begin(), byte4.end(), byte4.begin(),
		[](unsigned char c) { return std::toupper(c); });
	if (ImGui::Button((const char*)u8"Сменить тему")) {
		if (green == 1) {
			green = 0;
			blue = 0;
			red = 0;
			green1 = 1;
			blue1 = 1;
			red1 = 1;
			redbutton = 0.1;
			greenbutton = 0.23;
			bluebutton = 0.39;
		}
		else {
			green1 = 0;
			blue1 = 0;
			red1 = 0;
			green = 1;
			blue = 1;
			red = 1;
			redbutton = 0.11;
			greenbutton = 0.56;
			bluebutton = 1;
		}
	}
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(red1, green1, blue1, 1.0f));
	ImGui::BeginChild("Левая панель", ImVec2(404, 300), true);
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1),"31           ");
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)registr.c_str());
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1),"             0");
	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.00, 1.00, 0, 1));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1.0f));
	if (ImGui::Button((const char*)u8"Очистить регистр"))
	{
		byteregistrs[Registr][0] = "00";
		byteregistrs[Registr][1] = "00";
		byteregistrs[Registr][2] = "00";
		byteregistrs[Registr][3] = "00";
	}
	ImGui::PopStyleColor(2);
	if (bitysInRegistr == 4 and NumberCommand < 2)
	{
		if (chet >= 9 and chet <= 13.5 and NumberCommand == 0)
		{
			string bytes[4] = { "Blink", "Blink", "Blink", "Blink" };
			ByteText(bytes);
		}
		else if (chet >= 4.5 and chet <= 9 and NumberCommand == 1)
		{
			string bytes[4] = { "White", "White", "White", "Blink" };
			ByteText(bytes);
		}
		else if (chet >= 9 and chet <= 13.5 and NumberCommand == 1)
		{
			byteregistrs[Registr][3] = znachbyte1;
			string bytes[4] = { "White", "White", "Blink", "White" };
			ByteText(bytes);
		}
		else if (chet >= 13.5 and chet <= 18 and NumberCommand == 1)
		{
			byteregistrs[Registr][2] = znachbyte2;
			string bytes[4] = { "White", "Blink", "White", "White" };
			ByteText(bytes);
		}
		else if (chet >= 18 and chet <= 22.5 and NumberCommand == 1)
		{
			byteregistrs[Registr][1] = znachbyte3;
			string bytes[4] = { "Blink", "White", "White", "White" };
			ByteText(bytes);
		}
		else if (chet >= 22.5 and NumberCommand == 1) {
			chet = 0.0f;
			timer = 0.0f;
			isBlinking = !isBlinking;
			byteregistrs[Registr][0] = znachbyte4;
			string bytes[4] = { "White", "White", "White", "White" };
			ByteText(bytes);
		}
		else if (chet >= 13.5 and NumberCommand == 0 and NumberElements<=1) {
			chet = 0.0f;
			timer = 0.0f;
			isBlinking = !isBlinking;
			byteregistrs[Registr][0] = ElementsCommands[NumberElementCommands][1];
			byteregistrs[Registr][1] = ElementsCommands[NumberElementCommands][2];
			byteregistrs[Registr][2] = ElementsCommands[NumberElementCommands][3];
			byteregistrs[Registr][3] = ElementsCommands[NumberElementCommands][4];
			string bytes[4] = { "White", "White", "White", "White" };
			ByteText(bytes);
		}
		else if (chet >= 13.5 and NumberCommand == 0 and ((NumberElements >= 22 and (NumberElements-2)%4 == 0 and NumberElements <= 38) or NumberElements == 40)) {
			chet = 0.0f;
			timer = 0.0f;
			isBlinking = !isBlinking;
			byteregistrs[Registr][0] = byteregistrs[(NumberElements-22) / 4][0];
			byteregistrs[Registr][1] = byteregistrs[(NumberElements - 22) / 4][1];
			byteregistrs[Registr][2] = byteregistrs[(NumberElements - 22) / 4][2];
			byteregistrs[Registr][3] = byteregistrs[(NumberElements - 22) / 4][3];
			string bytes[4] = { "White", "White", "White", "White" };
			ByteText(bytes);
			}
		else if (error == 3 and NumberTypePeremennoi == 1)
		{
			string bytes[4] = { "White", "White", "Red", "Red" };
			ByteText(bytes);
			}
		else if (error == 3 and NumberTypePeremennoi == 2)
		{
			string bytes[4] = { "White", "White", "Red", "Red" };
			ByteText(bytes);
			}
		else
		{
			string bytes[4] = {"White", "White", "White", "White"};
			ByteText(bytes);
		}

	}
	else if (bitysInRegistr == 2 and NumberCommand < 2)
	{
		if (chet >= 9 and chet <= 13.5 and NumberCommand == 0)
		{
			string bytes[4] = { "Gray", "Gray", "Blink", "Blink" };
			ByteText(bytes);
		}
		else if (chet >= 4.5 and chet <= 9 and NumberCommand == 1)
		{
			string bytes[4] = { "Gray", "Gray", "White", "Blink" };
			ByteText(bytes);
		}
		else if (chet >= 9 and chet <= 13.5 and NumberCommand == 1)
		{
			byteregistrs[Registr][3] = znachbyte1;
			string bytes[4] = { "Gray", "Gray", "Blink", "White" };
			ByteText(bytes);
		}
		else if (chet >= 13.5 and NumberCommand == 1) {
			chet = 0.0f;
			timer = 0.0f;
			isBlinking = !isBlinking;
			byteregistrs[Registr][2] = znachbyte2;
			ImGui::BeginChild("byte1", ImVec2(40, 40), true);
			string bytes[4] = { "Gray", "Gray", "White", "White" };
			ByteText(bytes);
		}
		else if (chet >= 13.5 and NumberCommand == 0) {
			chet = 0.0f;
			timer = 0.0f;
			isBlinking = !isBlinking;
			byteregistrs[Registr][2] = ElementsCommands[NumberElementCommands][3];
			byteregistrs[Registr][3] = ElementsCommands[NumberElementCommands][4];
			string bytes[4] = { "Gray", "Gray", "White", "White" };
			ByteText(bytes);
		}
		else if (error == 4 and NumberTypePeremennoi == 0)
		{
			string bytes[4] = { "Red", "Red", "Red", "Red" };
			ByteText(bytes);
		}
		else if (error == 3 and NumberTypePeremennoi == 2)
		{
			string bytes[4] = { "Gray", "Gray", "White", "Red" };
			ByteText(bytes);
		}
		else
		{
			string bytes[4] = { "Gray", "Gray", "White", "White" };
			ByteText(bytes);
		}
	}
	else if (NumberRegistr % 4 == 2 and NumberCommand < 2 and bitysInRegistr != 2)
		{
			if (NumberCommand == 0 and error == 1)
			{

				string bytes[4] = { "Red", "Red", "Red", "Red" };
				ByteText(bytes);
			}
			else if (chet >= 4.5 and chet <= 9 and NumberCommand == 1)
			{
				string bytes[4] = { "Gray", "Gray", "Blink", "Gray" };
				ByteText(bytes);
			}
			else if (chet >= 9 and NumberCommand == 1) {
				chet = 0.0f;
				timer = 0.0f;
				isBlinking = !isBlinking;
				byteregistrs[Registr][2] = znachbyte1;
				string bytes[4] = { "Gray", "Gray", "White", "Gray" };
				ByteText(bytes);
			}
			else if (error == 4 and NumberTypePeremennoi == 0)
			{
				string bytes[4] = { "Red", "Red", "Red", "Red" };
				ByteText(bytes);
			}
			else if (error == 4 and NumberTypePeremennoi == 1)
			{
				string bytes[4] = { "Gray", "Gray", "Red", "Red" };
				ByteText(bytes);
			}
			else
			{
				string bytes[4] = { "Gray", "Gray", "White", "Gray" };
				ByteText(bytes);
			}
		}
		else if (NumberRegistr % 4 == 3 and NumberCommand < 2 and bitysInRegistr!=2)
		{
			if (NumberCommand == 0 and error==1)
			{
				string bytes[4] = { "Red", "Red", "Red", "Red" };
				ByteText(bytes);
			}
			else if (chet >= 4.5 and chet <= 9 and NumberCommand == 1)
			{
				string bytes[4] = { "Gray", "Gray", "Gray", "Blink" };
				ByteText(bytes);
			}
			else if (chet >= 9 and NumberCommand == 1) {
				chet = 0.0f;
				timer = 0.0f;
				isBlinking = !isBlinking;
				byteregistrs[Registr][3] = znachbyte1;
				string bytes[4] = { "Gray", "Gray", "Gray", "White" };
				ByteText(bytes);
			}
			else if (error == 4 and NumberTypePeremennoi == 0)
			{
				string bytes[4] = { "Red", "Red", "Red", "Red" };
				ByteText(bytes);
			}
			else if (error == 4 and NumberTypePeremennoi == 1)
			{
				string bytes[4] = { "Gray", "Gray", "Red", "Red" };
				ByteText(bytes);
			}
			else
			{
				string bytes[4] = { "Gray", "Gray", "Gray", "White" };
				ByteText(bytes);
			}
		}
		else if (NumberCommand == 4)
		{
			if (chet >= 4.5 and chet <= 9)
			{
				NumberRegistr = 0;
				string bytes[4] = { "White", "White", "White", "Blink" };
				ByteText(bytes);
			}
			else if (chet >= 9 and chet <= 13.5)
			{
				byteregistrs[Registr][3] = znachbyte1;
				string bytes[4] = { "White", "White", "Blink", "White" };
				ByteText(bytes);
			}
			else if (chet >= 13.5 and chet <= 18)
			{
				byteregistrs[Registr][2] = znachbyte2;
				string bytes[4] = { "White", "Blink", "White", "White" };
				ByteText(bytes);
			}
			else if (chet >= 18 and chet <= 22.5)
			{
				byteregistrs[Registr][1] = znachbyte3;
				string bytes[4] = { "Blink", "White", "White", "White" };
				ByteText(bytes);
			}
			else if (chet >= 22.5) {
				chet = 0.0f;
				timer = 0.0f;
				isBlinking = !isBlinking;
				byteregistrs[Registr][0] = znachbyte4;
				string bytes[4] = { "White", "White", "White", "White" };
				ByteText(bytes);
			}
			else
			{
				string bytes[4] = { "White", "White", "White", "White" };
				ByteText(bytes);
			}
		}
		else if (NumberCommand == 3)
		{
			if (chet >= 4.5 and chet <= 9)
			{
				NumberRegistr = 0;
				string bytes[4] = { "Gray", "Gray", "White", "Blink" };
				ByteText(bytes);
			}
			else if (chet >= 9 and chet <= 13.5)
			{
				byteregistrs[Registr][3] = znachbyte1;
				string bytes[4] = { "Gray", "Gray", "Blink", "White" };
				ByteText(bytes);
			}
			else if (chet >= 13.5) 
			{
				chet = 0.0f;
				timer = 0.0f;
				isBlinking = !isBlinking;
				byteregistrs[Registr][2] = znachbyte2;
				string bytes[4] = { "Gray", "Gray", "White", "White" };
				ByteText(bytes);
			}
			else
			{
				string bytes[4] = { "Gray", "Gray", "White", "White" };
				ByteText(bytes);
			}
		}
		else if (NumberCommand == 2)
		{
			if (chet >= 4.5 and chet <= 9)
			{
				NumberRegistr = 0;
				string bytes[4] = { "Gray", "Gray", "Gray", "Blink" };
				ByteText(bytes);
			}
			else if (chet >= 9)
			{
				chet = 0.0f;
				timer = 0.0f;
				isBlinking = !isBlinking;
				byteregistrs[Registr][3] = znachbyte1;
				string bytes[4] = { "Gray", "Gray", "Gray", "White" };
				ByteText(bytes);
			}
			else
			{
				string bytes[4] = { "Gray", "Gray", "Gray", "White" };
				ByteText(bytes);
			}
		}
	ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
	ImGui::Combo((const char*)u8"Выбрать команду", &NumberCommand, Commads, IM_ARRAYSIZE(Commads));
	ImGui::SameLine();
	if (NumberCommand < 2) NumberElementCommands = 0;
	else NumberElementCommands = 1;
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0.5, 0, 1));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1.0f));
	if (ImGui::Button((const char*)u8"Активировать команду"))
	{
		if ((NumberElements - 2) % 4 != 0 and NumberElements >= 22 and NumberElements != 40) error = 8;
		else
		{
			error = 0;
			if (NumberCommand == -1) error = 6;
			else if (NumberCommand == 0)
			{
				if ((bitysInRegistr == 4 or bitysInRegistr == 2) and (NumberElements <= 1 or NumberElements >= 22)) isBlinking = !isBlinking;
				else if (NumberElements > 1 and NumberElements < 22) error = 7;
				else if (NumberElements >= 22 and (NumberElements - 2) % 4 != 0) error = 8;
				else error = 1;
			}

			else if (NumberCommand == 1)
			{
				if (otchet == 4 or ((NumberElements >= 22 and otchet != 0) and bitysInRegistr == 4) or ((NumberElements >= 22 and otchet == 3) and NumberRegistr % 4 == 1)) error = 9;
				else if (bitysInRegistr == 4 and ((NumberTypePeremennoi == 0 and NumberElements <= 1) or (((NumberElements - 2) % 4 == 0) and NumberElements < 22) or (NumberElements >= 22 and otchet == 0))) isBlinking = !isBlinking;
				else if (bitysInRegistr == 4 and ((NumberTypePeremennoi != 0 and NumberElements <= 1) or ((NumberElements - 2) % 4 != 0 and NumberElements < 22))) error = 3;
				else if (bitysInRegistr == 2 and ((NumberTypePeremennoi == 1 and NumberElements <= 1) or ((NumberElements - 2) % 4 == 1 and NumberElements < 22) or (NumberElements >= 22 and otchet != 3))) isBlinking = !isBlinking;
				else if (bitysInRegistr == 2 and ((NumberTypePeremennoi == 0 and NumberElements <= 1) or ((NumberElements - 2) % 4 == 0 and NumberElements < 22))) error = 4;
				else if (bitysInRegistr == 2 and ((NumberTypePeremennoi == 2 and NumberElements <= 1) or ((NumberElements - 2) % 4 >= 2 and NumberElements < 22))) error = 3;
				else if (bitysInRegistr == 1 and ((NumberTypePeremennoi == 2 and NumberElements <= 1) or ((NumberElements - 2) % 4 >= 2 and NumberElements < 22) or NumberElements >= 22)) isBlinking = !isBlinking;
				else error = 4;
			}
			else if (NumberCommand >= 2 and NumberCommand <= 4) 
			{
				if (NumberCommand == 4 and znachbyte4 != "") isBlinking = !isBlinking;
				else if (NumberCommand == 3 and znachbyte2 != "") isBlinking = !isBlinking;
				else if (NumberCommand == 2 and znachbyte1 != "") isBlinking = !isBlinking;
				else error = 9;
			}
		}
	}
	ImGui::PopStyleColor(2);
	if (NumberCommand == 0) 
	{
		if (redadres != 1) {
			redadres = red1;
			greenadres = green1;
			blueadres = blue1;
		}
		if (chet >= 0 and chet <= 4.5)
		{
			BlinkingText("LEA");
			ImGui::SameLine();
			BlinkingText((const char*)registr.c_str());
			ImGui::SameLine();
			ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
			if (!isBlinking) ImGui::Combo((const char*)u8"", &NumberElements, Elements, IM_ARRAYSIZE(Elements));
			else BlinkingText((const char*)Elements[NumberElements]);
		}
		else
		{
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "LEA");
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)registr.c_str());
			ImGui::SameLine();
			ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
			if (!isBlinking) ImGui::Combo((const char*)u8"", &NumberElements, Elements, IM_ARRAYSIZE(Elements));
			else ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)Elements[NumberElements]);
		}

	}
	else if (NumberCommand == 1)
	{
		if (redadres != 1) {
			redadres = 0.5;
			greenadres = 0.5;
			blueadres = 0.5;
		}
		if (chet >= 0 and chet <= 4.5)
		{
			BlinkingText("MOV");
			ImGui::SameLine();
			BlinkingText((const char*)registr.c_str());
			ImGui::SameLine();
			ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
			if (!isBlinking) ImGui::Combo((const char*)u8"", &NumberElements, Elements, IM_ARRAYSIZE(Elements));
			else BlinkingText((const char*)Elements[NumberElements]);
	
		}
		else
		{
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),"MOV");
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)registr.c_str());
			ImGui::SameLine();
			ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
			if (!isBlinking) ImGui::Combo((const char*)u8"", &NumberElements, Elements, IM_ARRAYSIZE(Elements));
			else ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)Elements[NumberElements]);
	
		}
	}
	else if (NumberCommand >= 2 and NumberCommand <= 4)
	{
		if (redadres != 1) {
			redadres = red1;
			greenadres = green1;
			blueadres = blue1;
		}
		if (chet >= 0 and chet <= 4.5)
		{
			BlinkingText(Commads[NumberCommand]);
		}
		else
		{
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1), Commads[NumberCommand]);
		}

	}
	if (!isBlinking)
	{
		if (NumberCommand >= 2 and NumberCommand <= 4)
		{
			NumberElements = -1;
			//NumberTypePeremennoi = -1;
		}
		if (NumberElements > 1 and NumberElements < 22)
		{
			otchet = 0;
			znachbyte1 = byteregistrs[(NumberElements - 2) / 4][3];
			znachbyte2 = byteregistrs[(NumberElements - 2) / 4][2];
			znachbyte3 = byteregistrs[(NumberElements - 2) / 4][1];
			znachbyte4 = byteregistrs[(NumberElements - 2) / 4][0];
		}
		else if (NumberElements >= 22 or (NumberCommand >=2 and NumberCommand <=4))
		{
			if ((NumberTypePeremennoi == 0 and NumberCommand == 1) or NumberCommand == 4)
			{
				int KosAddress = (NumberElements - 22) / 4;
				if (NumberCommand == 4) KosAddress = 5;
				if (ElementsCommands[NumberElementCommands][1] == byteregistrs[KosAddress][0] and ElementsCommands[NumberElementCommands][2] == byteregistrs[KosAddress][1] and ElementsCommands[NumberElementCommands][3] == byteregistrs[KosAddress][2] and ElementsCommands[NumberElementCommands][4] == byteregistrs[KosAddress][3])
				{
					otchet = 0;
					znachbyte4 = peremennai.substr(0, 2);
					znachbyte3 = peremennai.substr(2, 2);
					znachbyte2 = peremennai.substr(4, 2);
					znachbyte1 = peremennai.substr(6, 2);
				}
				else
				{
					glavregistr = byteregistrs[KosAddress][0] + byteregistrs[KosAddress][1] + byteregistrs[KosAddress][2] + byteregistrs[KosAddress][3];
					int otchet1;
					otchet1 = convertStringtoHex(ElementsCommands[NumberElementCommands][0]) - convertStringtoHex(glavregistr);
					if (otchet1 == 1)
					{
						otchet = 1;
						znachbyte4 = "";
						znachbyte3 = peremennai.substr(0, 2);
						znachbyte2 = peremennai.substr(2, 2);
						znachbyte1 = peremennai.substr(4, 2);
					}
					else if (otchet1 == 2)
					{
						otchet = 2;
						znachbyte4 = "";
						znachbyte3 = "";
						znachbyte2 = peremennai.substr(0, 2);
						znachbyte1 = peremennai.substr(2, 2);
					}
					else if (otchet1 == 3)
					{
						otchet = 3;
						znachbyte4 = "";
						znachbyte3 = "";
						znachbyte2 = "";
						znachbyte1 = peremennai.substr(0, 2);
					}
					else
					{
						otchet = 4;
						znachbyte4 = "";
						znachbyte3 = "";
						znachbyte2 = "";
						znachbyte1 = "";
					}
				}
			}
			else if ((NumberTypePeremennoi == 2 and NumberCommand == 1) or NumberCommand == 2)
			{
				int KosAddress = (NumberElements - 22) / 4;
				if (NumberCommand == 2) KosAddress = 5;
					if (ElementsCommands[NumberElementCommands][1] == byteregistrs[KosAddress][0] and ElementsCommands[NumberElementCommands][2] == byteregistrs[KosAddress][1] and ElementsCommands[NumberElementCommands][3] == byteregistrs[KosAddress][2] and ElementsCommands[NumberElementCommands][4] == byteregistrs[KosAddress][3])
					{
						otchet = 3;
						znachbyte4 = "";
						znachbyte3 = "";
						znachbyte2 = "";
						znachbyte1 = peremennai.substr(6, 2);
					}
					else
					{
						otchet = 4;
						znachbyte4 = "";
						znachbyte3 = "";
						znachbyte2 = "";
						znachbyte1 = "";
					}
			}
			else if ((NumberTypePeremennoi == 1 and NumberCommand == 1) or NumberCommand == 3)
			{
				int KosAddress = (NumberElements - 22) / 4;
				if (NumberCommand == 3) KosAddress = 5;
				if (ElementsCommands[NumberElementCommands][1] == byteregistrs[KosAddress][0] and ElementsCommands[NumberElementCommands][2] == byteregistrs[KosAddress][1] and ElementsCommands[NumberElementCommands][3] == byteregistrs[KosAddress][2] and ElementsCommands[NumberElementCommands][4] == byteregistrs[KosAddress][3])
				{
					otchet = 2;
					znachbyte4 = "";
					znachbyte3 = "";
					znachbyte2 = peremennai.substr(4, 2);
					znachbyte1 = peremennai.substr(6, 2);
				}
				else
				{
					glavregistr = byteregistrs[KosAddress][0] + byteregistrs[KosAddress][1] + byteregistrs[KosAddress][2] + byteregistrs[KosAddress][3];
					int otchet1;
					otchet1 = convertStringtoHex(ElementsCommands[NumberElementCommands][0]) - convertStringtoHex(glavregistr);

					if (otchet1 == 1)
					{
						otchet = 3;
						znachbyte4 = "";
						znachbyte3 = "";
						znachbyte2 = "";
						znachbyte1 = peremennai.substr(4, 2);
					}
					else
					{
						otchet = 4;
						znachbyte4 = "";
						znachbyte3 = "";
						znachbyte2 = "";
						znachbyte1 = "";
					}
				}
			}
		}
		else 
		{
			otchet = 0;
			if (NumberTypePeremennoi == 0)
			{
				znachbyte4 = peremennai.substr(0, 2);
				znachbyte3 = peremennai.substr(2, 2);
				znachbyte2 = peremennai.substr(4, 2);
				znachbyte1 = peremennai.substr(6, 2);
			}
			else if (NumberTypePeremennoi == 1)
			{
				znachbyte2 = peremennai.substr(0, 2);
				znachbyte1 = peremennai.substr(2, 2);
				znachbyte4 = "";
				znachbyte3 = "";
			}
			else
			{
				znachbyte1 = peremennai.substr(0, 2);
				znachbyte4 = "";
				znachbyte3 = "";
				znachbyte2 = "";
			}
		}
	}
	ImGui::SetCursorPos(ImVec2(7, 200));
	if (error != 2) {
		redznach = red1;
		greenznach = green1;
		blueznach = blue1;
	}
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(redznach, greenznach, blueznach, 1.0f));
	ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.2f);
	ImGui::InputText("##hidden", bufznach, IM_ARRAYSIZE(bufznach), ImGuiInputTextFlags_CharsHexadecimal);
	ImGui::PopStyleColor(1);
	ImGui::SameLine();
	if (ImGui::Button((const char*)u8"Ввести переменную х1"))
	{
		peremennai = bufznach;
		error = 0;
		if (NumberTypePeremennoi == 0)
		{
			while (size(peremennai) < 8) peremennai = "0" + peremennai;
			if (bitysInRegistr == 4)
			{
				znachbyte4 = peremennai.substr(0, 2);
				znachbyte3 = peremennai.substr(2, 2);
				znachbyte2 = peremennai.substr(4, 2);
				znachbyte1 = peremennai.substr(6, 2);
			}
		}
		else if (NumberTypePeremennoi == 1)
		{
			while (size(peremennai) < 4) peremennai = "0" + peremennai;
			if (size(peremennai) <= 4)
			{
					znachbyte2 = peremennai.substr(0, 2);
					znachbyte1 = peremennai.substr(2, 2);
					znachbyte4 = "";
					znachbyte3 = "";
			}
			else error = 2;
		} 
		else
		{
			while (size(peremennai) < 2) peremennai = "0" + peremennai;
			if (size(peremennai) <= 2)
			{
					znachbyte1 = peremennai.substr(0, 2);
					znachbyte4 = "";
					znachbyte3 = "";
					znachbyte2 = "";
			}
			else error = 2;
		}
	}
	ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
	ImGui::Combo((const char*)u8"Выбрать тип данных переменной x1", &NumberTypePeremennoi, TypePeremennoi, IM_ARRAYSIZE(TypePeremennoi));
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1),"x1");
	ImGui::SameLine();
	std::transform(peremennai.begin(), peremennai.end(), peremennai.begin(),
		[](unsigned char c) { return std::toupper(c); });
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1),peremennai.c_str());
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(TypePeremennoi[NumberTypePeremennoi]));
	ImGui::EndChild();
	ImGui::SameLine(410, 0);
	ImGui::BeginChild("Адрес", ImVec2(115, 300), true);
	ImGui::InputText("##hidden", bufaddress, IM_ARRAYSIZE(bufaddress), ImGuiInputTextFlags_CharsHexadecimal);
	if (ImGui::Button((const char*)u8"Ввести адрес"))
	{
		ElementsCommands[NumberElementCommands][0] = bufaddress;
		error = 0;
		while (size(ElementsCommands[NumberElementCommands][0]) < 8) ElementsCommands[NumberElementCommands][0] = ElementsCommands[NumberElementCommands][0] + "0";
		ElementsCommands[NumberElementCommands][1] = ElementsCommands[NumberElementCommands][0].substr(0, 2);
		ElementsCommands[NumberElementCommands][2] = ElementsCommands[NumberElementCommands][0].substr(2, 2);
		ElementsCommands[NumberElementCommands][3] = ElementsCommands[NumberElementCommands][0].substr(4, 2);
		ElementsCommands[NumberElementCommands][4] = ElementsCommands[NumberElementCommands][0].substr(6, 2);
	}
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1)," ");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1)," ");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)u8"Адрес");
	std::transform(ElementsCommands[NumberElementCommands][1].begin(), ElementsCommands[NumberElementCommands][1].end(), ElementsCommands[NumberElementCommands][1].begin(),
		[](unsigned char c) { return std::toupper(c); });
	std::transform(ElementsCommands[NumberElementCommands][2].begin(), ElementsCommands[NumberElementCommands][2].end(), ElementsCommands[NumberElementCommands][2].begin(),
		[](unsigned char c) { return std::toupper(c); });
	std::transform(ElementsCommands[NumberElementCommands][3].begin(), ElementsCommands[NumberElementCommands][3].end(), ElementsCommands[NumberElementCommands][3].begin(),
		[](unsigned char c) { return std::toupper(c); });
	std::transform(ElementsCommands[NumberElementCommands][4].begin(), ElementsCommands[NumberElementCommands][4].end(), ElementsCommands[NumberElementCommands][4].begin(),
		[](unsigned char c) { return std::toupper(c); });
	if (NumberCommand == 0) {
		if (chet >= 4.5 and chet <= 9  and bitysInRegistr == 4 and NumberElements<=1)
		{

			BlinkingText(ElementsCommands[NumberElementCommands][1].c_str());
			ImGui::SameLine(0, 0);
			BlinkingText(ElementsCommands[NumberElementCommands][2].c_str());
			ImGui::SameLine(0, 0);
			BlinkingText(ElementsCommands[NumberElementCommands][3].c_str());
			ImGui::SameLine(0, 0);
			BlinkingText(ElementsCommands[NumberElementCommands][4].c_str());
			ImGui::SameLine(0, 0);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),"h==>");
		}
		else if (chet >= 4.5 and chet <= 9 and bitysInRegistr == 2)
		{
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1), ElementsCommands[NumberElementCommands][1].c_str());
			ImGui::SameLine(0, 0);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1), ElementsCommands[NumberElementCommands][2].c_str());
			ImGui::SameLine(0, 0);			
			BlinkingText(ElementsCommands[NumberElementCommands][3].c_str());
			ImGui::SameLine(0, 0);
			BlinkingText(ElementsCommands[NumberElementCommands][4].c_str());
			ImGui::SameLine(0, 0);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "h==>");
		}
		else 
		{
			ImGui::TextColored(ImVec4(redadres, greenadres, blueadres, 1),ElementsCommands[NumberElementCommands][1].c_str());
			ImGui::SameLine(0, 0);
			ImGui::TextColored(ImVec4(redadres, greenadres, blueadres, 1),ElementsCommands[NumberElementCommands][2].c_str());
			ImGui::SameLine(0, 0);
			ImGui::TextColored(ImVec4(redadres, greenadres, blueadres, 1),ElementsCommands[NumberElementCommands][3].c_str());
			ImGui::SameLine(0, 0);
			ImGui::TextColored(ImVec4(redadres, greenadres, blueadres, 1),ElementsCommands[NumberElementCommands][4].c_str());
			ImGui::SameLine(0, 0);
			ImGui::TextColored(ImVec4(redadres, greenadres, blueadres, 1),"h==>");
		}
	}
	else
	{
		ImGui::TextColored(ImVec4(redadres, greenadres, blueadres, 1), ElementsCommands[NumberElementCommands][1].c_str());
		ImGui::SameLine(0, 0);
		ImGui::TextColored(ImVec4(redadres, greenadres, blueadres, 1), ElementsCommands[NumberElementCommands][2].c_str());
		ImGui::SameLine(0, 0);
		ImGui::TextColored(ImVec4(redadres, greenadres, blueadres, 1), ElementsCommands[NumberElementCommands][3].c_str());
		ImGui::SameLine(0, 0);
		ImGui::TextColored(ImVec4(redadres, greenadres, blueadres, 1), ElementsCommands[NumberElementCommands][4].c_str());
		ImGui::SameLine(0, 0);
		ImGui::TextColored(ImVec4(redadres, greenadres, blueadres, 1), "h==>");
	}
		if (NumberTypePeremennoi % 4 == 1)
		{

			unsigned long long hexadres;
			std::stringstream ss;
			ss << std::hex << ElementsCommands[NumberElementCommands][0];
			ss >> hexadres;
			hexadres += 1;
			std::stringstream ss2;
			ss2 << std::hex << hexadres;
			


			if (size(ss2.str()) == 9) error = 5;
			else {
				resultString = ss2.str();
				std::transform(resultString.begin(), resultString.end(), resultString.begin(),
					[](unsigned char c) { return std::toupper(c); });
				while (size(resultString) < 8) resultString = "0" + resultString;
				resultString += "h==>";
				ImGui::TextColored(ImVec4(red1, green1, blue1, 1),"");
				ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), resultString.c_str());
			}
		}
		else if (NumberTypePeremennoi % 4 == 0)
		{

			unsigned long long hexadres;
			std::stringstream ss;
			ss << std::hex << ElementsCommands[NumberElementCommands][0];
			ss >> hexadres;
			hexadres += 1;
			std::stringstream ss2;
			ss2 << std::hex << hexadres;
			if (size(ss2.str()) == 9) error = 5;
			else {
			resultString = ss2.str();
			std::transform(resultString.begin(), resultString.end(), resultString.begin(),
				[](unsigned char c) { return std::toupper(c); });
				while (size(resultString) < 8) resultString = "0" + resultString;
				resultString += "h==>";
				ImGui::TextColored(ImVec4(red1, green1, blue1, 1),"");
				ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), resultString.c_str());
			}
			resultString = "";
			unsigned long long hexadres1;
			std::stringstream ss3;
			ss3 << std::hex << ElementsCommands[NumberElementCommands][0];
			ss3 >> hexadres1;
			hexadres1 += 2;
			std::stringstream ss4;
			ss4 << std::hex << hexadres1;
			if (size(ss4.str()) == 9) error = 5;
			else {
				resultString = ss4.str();
				std::transform(resultString.begin(), resultString.end(), resultString.begin(),
					[](unsigned char c) { return std::toupper(c); });
				while (size(resultString) < 8) resultString = "0" + resultString;
				resultString += "h==>";
				ImGui::TextColored(ImVec4(red1, green1, blue1, 1),"");
				ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), resultString.c_str());
			}
			resultString = "";
			unsigned long long hexadres2;
			std::stringstream ss5;
			ss5 << std::hex << ElementsCommands[NumberElementCommands][0];
			ss5 >> hexadres2;
			hexadres2 += 3;
			std::stringstream ss6;
			ss6 << std::hex << hexadres2;
			if (size(ss6.str()) == 9) error = 5;
			else {
				resultString = ss6.str();
				std::transform(resultString.begin(), resultString.end(), resultString.begin(),
					[](unsigned char c) { return std::toupper(c); });
				while (size(resultString) < 8) resultString = "0" + resultString;
				resultString += "h==>";
				ImGui::TextColored(ImVec4(red1, green1, blue1, 1),"");
				ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), resultString.c_str());
			}
		}

	ImGui::EndChild();
	ImGui::SameLine(525, 0);
	ImGui::BeginChild("Оперативка", ImVec2(110, 300), ImGuiWindowFlags_NoMove);
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)u8"Оперативная");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)u8"Память");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1)," ");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1)," ");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1)," ");
	ImGui::BeginChild("Значение1", ImVec2(87, 40), true);
	if (chet >= 4.5f and chet <= 9.0f and NumberCommand != 0)
	{
		ImGui::TextColored(ImVec4(red1, green1, blue1, 1),"  ");
		ImGui::SameLine(0, 0);
		BlinkingText(znachbyte1.c_str());
	}
	else
	{
		ImGui::TextColored(ImVec4(red1, green1, blue1, 1),"  ");
		ImGui::SameLine(0, 0);
		ImGui::TextColored(ImVec4(red1, green1, blue1, 1),znachbyte1.c_str());
	}
	ImGui::EndChild();
		if ((NumberTypePeremennoi % 4 == 0 and NumberCommand < 2) or NumberCommand==4)
		{
			ImGui::BeginChild("Значение2", ImVec2(87, 40), true);
			if (chet >= 9.0f and chet <= 13.5f and NumberCommand != 0)
			{
				ImGui::TextColored(ImVec4(red1, green1, blue1, 1),"  ");
				ImGui::SameLine(0, 0);
				BlinkingText(znachbyte2.c_str());
			}
			else
			{
				ImGui::TextColored(ImVec4(red1, green1, blue1, 1),"  ");
				ImGui::SameLine(0, 0);
				ImGui::TextColored(ImVec4(red1, green1, blue1, 1),znachbyte2.c_str());
			}
			ImGui::EndChild();
			
			ImGui::BeginChild("Значение3", ImVec2(87, 40), true);
			if (chet >= 13.5 and chet <= 18 and NumberCommand != 0)
			{
				ImGui::TextColored(ImVec4(red1, green1, blue1, 1),"  ");
				ImGui::SameLine(0, 0);
				BlinkingText(znachbyte3.c_str());
			}
			else
			{
				ImGui::TextColored(ImVec4(red1, green1, blue1, 1),"  ");
				ImGui::SameLine(0, 0);
				ImGui::TextColored(ImVec4(red1, green1, blue1, 1),znachbyte3.c_str());
			}
			ImGui::EndChild();
			ImGui::BeginChild("Значение4", ImVec2(87, 40), true);
			if (chet >= 18 and chet <= 22.5 and NumberCommand != 0)
			{
				ImGui::TextColored(ImVec4(red1, green1, blue1, 1),"  ");
				ImGui::SameLine(0, 0);
				BlinkingText(znachbyte4.c_str());
			}
			else
			{
				ImGui::TextColored(ImVec4(red1, green1, blue1, 1),"  ");
				ImGui::SameLine(0, 0);
				ImGui::TextColored(ImVec4(red1, green1, blue1, 1),znachbyte4.c_str());
			}
			ImGui::EndChild();
		}
		else if ((NumberTypePeremennoi % 4 == 1 and NumberCommand < 2) or NumberCommand == 3)
		{
			ImGui::BeginChild("Значение2", ImVec2(87, 40), true);
			if (chet >= 9.0f and chet <= 13.5f and NumberCommand != 0)
			{
				ImGui::TextColored(ImVec4(red1, green1, blue1, 1),"  ");
				ImGui::SameLine(0, 0);
				BlinkingText(znachbyte2.c_str());

			}
			else
			{
				ImGui::TextColored(ImVec4(red1, green1, blue1, 1),"  ");
				ImGui::SameLine(0, 0);
				ImGui::TextColored(ImVec4(red1, green1, blue1, 1),znachbyte2.c_str());
			}
			ImGui::EndChild();

		}
	ImGui::EndChild();
	ImGui::SameLine(635, 0);
	ImGui::BeginChild("Регистры", ImVec2(140, 300), ImGuiWindowFlags_NoMove);
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)u8"  Регистры");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1), " ");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1), " ");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "EAX=");
	ImGui::SameLine(0,0);
	for (int i = 0; i < 4; i++) {
		if (NumberCommand==0 and chet >= 4.5 and chet <= 9 and bitysInRegistr == 4 and NumberElements >= 22  and (NumberElements-2)/4 == 5)
		{
			BlinkingText(byteregistrs[0][i].c_str());
			if (i != 3) ImGui::SameLine(0, 0);
		}
		else
		{
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1), byteregistrs[0][i].c_str());
			if (i != 3) ImGui::SameLine(0, 0);
		}
	}
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1), " ");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "EBX=");
	ImGui::SameLine(0, 0);
	for (int i = 0; i < 4; i++) {
		if (NumberCommand == 0 and chet >= 4.5 and chet <= 9 and bitysInRegistr == 4 and NumberElements >= 22 and (NumberElements - 2) / 4 == 6)
		{
			BlinkingText(byteregistrs[1][i].c_str());
			if (i != 3) ImGui::SameLine(0, 0);
		}
		else
		{
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1), byteregistrs[1][i].c_str());
			if (i != 3) ImGui::SameLine(0, 0);
		}
	}
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1), " ");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "ECX=");
	ImGui::SameLine(0, 0);
	for (int i = 0; i < 4; i++) {
		if (NumberCommand == 0 and chet >= 4.5 and chet <= 9 and bitysInRegistr == 4 and NumberElements >= 22 and (NumberElements - 2) / 4 == 7)
		{
			BlinkingText(byteregistrs[2][i].c_str());
			if (i != 3) ImGui::SameLine(0, 0);
		}
		else
		{
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1), byteregistrs[2][i].c_str());
			if (i != 3) ImGui::SameLine(0, 0);
		}
	}
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1), " ");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "EDX=");
	ImGui::SameLine(0, 0);
	for (int i = 0; i < 4; i++) {
		if (NumberCommand == 0 and chet >= 4.5 and chet <= 9 and bitysInRegistr == 4 and NumberElements >= 22 and (NumberElements - 2) / 4 == 8)
		{
			BlinkingText(byteregistrs[3][i].c_str());
			if (i != 3) ImGui::SameLine(0, 0);
		}
		else
		{
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1), byteregistrs[3][i].c_str());
			if (i != 3) ImGui::SameLine(0, 0);
		}
	}
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1), " ");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "ESI=");
	ImGui::SameLine(0, 0);
	for (int i = 0; i < 4; i++) {
		if (NumberCommand == 0 and chet >= 4.5 and chet <= 9 and bitysInRegistr == 4 and NumberElements == 38)
		{
			BlinkingText(byteregistrs[4][i].c_str());
			if (i != 3) ImGui::SameLine(0, 0);
		}
		else
		{
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1), byteregistrs[4][i].c_str());
			if (i != 3) ImGui::SameLine(0, 0);
		}
	}
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1), " ");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "EDI=");
	ImGui::SameLine(0, 0);
	for (int i = 0; i < 4; i++) {
		if (NumberCommand == 0 and chet >= 4.5 and chet <= 9 and bitysInRegistr == 4 and NumberElements == 40)
		{
			BlinkingText(byteregistrs[5][i].c_str());
			if (i != 3) ImGui::SameLine(0, 0);
		}
		else
		{
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1), byteregistrs[5][i].c_str());
			if (i != 3) ImGui::SameLine(0, 0);
		}
	}
	ImGui::EndChild();
	ImGui::BeginChild("Откладка", ImVec2(625, 150), true);
	if (error == 0)
	{
		greenznach = green1;
		blueznach = blue1;
		redznach = red1;
		greenadres = green1;
		blueadres = blue1;
		redadres = red1;
	}
	else if (error == 1)
	{
		ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), (const char*)u8"Ошибка: Этот регистр не может быть использован для команды lea.");
		ImGui::TextColored(ImVec4(0.0, 1.0, 0.0, 1.0), (const char*)u8"Нужно использовать регистр размера x16 или x32");
	}
	else if (error == 2)
	{
		ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), (const char*)u8"Ошибка: Числовое значение переменной больше положенного");
		ImGui::TextColored(ImVec4(0, 1.0, 0.0, 1.0), (const char*)u8"Нужно увеличить тип переменной или уменьшить ее числовое значение");
		greenznach = 0;
		blueznach = 0;
		redznach = 1;
	}
	else if (error == 3)
	{
		ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), (const char*)u8"Ошибка: Основной регистр больше переменной x1 или другого регистра");
		ImGui::TextColored(ImVec4(0, 1.0, 0.0, 1.0), (const char*)u8"Нужно увеличить переменную или уменьшить регистр");
	}
	else if (error == 4)
	{
		ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), (const char*)u8"Ошибка: Переменная x1 или другой регистр больше основного регистра ");
		ImGui::TextColored(ImVec4(0, 1.0, 0.0, 1.0), (const char*)u8"Нужно увеличить регистр или уменьшить переменную");

	}
	else if (error == 5)
	{
		ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), (const char*)u8"Ошибка: Выход за диапазон адресов ");
		ImGui::TextColored(ImVec4(0.0, 1.0, 0.0, 1.0), (const char*)u8"Нужно уменьшить адрес ");
		greenadres = 0;
		blueadres = 0;
		redadres = 1;
	}
	else if (error == 6)
	{
		ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), (const char*)u8"Выберите команду");

	}
	else if (error == 7)
	{
		ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), (const  char*)u8"Ошибка: Нельзя найти адрес регистра");
		ImGui::TextColored(ImVec4(0.0, 1.0, 0.0, 1.0), (const char*)u8"Нужно выбрать переменную");
	}
	else if (error == 8)
	{
		ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), (const  char*)u8"Ошибка: Указатель должен быть с базовым регистром");
		ImGui::TextColored(ImVec4(0.0, 1.0, 0.0, 1.0), (const char*)u8"Нужно выбрать указатель с четырехбайтным регистром");
	}
	else if (error == 9)
	{
		ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), (const  char*)u8"Ошибка: Выход за пределы значения переменной");
		ImGui::TextColored(ImVec4(0.0, 1.0, 0.0, 1.0), (const char*)u8"Нужно увеличить адрес или уменьшить значение указателя");
	}
	ImGui::EndChild();
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)u8"Мои ссылки");
	ImGui::Separator();

	if (ImGui::Button("YouTube")) {
		ShellExecute(NULL, "open", "https://www.youtube.com/c/@king174rus", 0, 0, SW_SHOWNORMAL);
	}
	ImGui::SameLine(); ImGui::Text(" "); ImGui::SameLine();
	if (ImGui::Button((const char*)u8"GitHub")) {
		ShellExecute(NULL, "open", "https://github.com/king174rus", 0, 0, SW_SHOWNORMAL);
	}
	ImGui::PopStyleColor(4); // Возвращаем стандартный цвет фона окна

	ImGui::End();
}