#include "gui.h"

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx9.h"
#include "../imgui/imgui_impl_win32.h"

#include <iostream>
#include "string"
#include <chrono>
#include <algorithm> // ��� std::transform
#include <cctype>    // ��� std::toupper

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

		// ������� ���� ����
		HBRUSH hBrush = CreateSolidBrush(RGB(255, 0, 0)); // ������� ���� ����

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
	ImGui::GetIO().Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Arial.ttf", 16.5f, NULL, ImGui::GetIO().Fonts->GetGlyphRangesCyrillic());
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

const char* Registres[] = { "EAX", "AX", "AH", "AL","EBX", "BX", "BH", "BL" ,"ECX", "CX", "CH", "CL" ,"EDX", "DX", "DH", "DL" };
static int NumberRegistr = 0;
const char* TypePeremennoi[] = { "dd", "dw", "db"};
static int NumberTypePeremennoi = 0;
const char* Elements[] = { "x1", "[x1]", "EAX", "AX", "AH", "AL","EBX", "BX", "BH", "BL" ,"ECX", "CX", "CH", "CL" ,"EDX", "DX", "DH", "DL" };
static int NumberElements = 0;


const char* Commads[] = {"LEA", "MOV"};
static int NumberCommand = -1;
float timer = 0.0f;
float chet = 0.0f;
int error = 0;
bool isBlinking = false;
string byteregistrs[4][4] = { {"00","00","00","00"},{"00","00","00","00"} ,{"00","00","00","00"} ,{"00","00","00","00"} };
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
void BlinkingCombo(const char* text, int* a, const char* const b, int c)
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
			ImGui::Combo(text, a, b, c);
			return;
		}
	}
}

void gui::Render() noexcept
{
	ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::SetNextWindowSize({ WIDTH, HEIGHT });
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(red, green, blue, 1.0f)); // ������������� ���� ���� ���� �� ����-�����


	ImGui::Begin(
		(const char*)u8"Visual Command Assembler",
		&isRunning,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_MenuBar
	);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(red1, green1, blue1, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(redbutton, greenbutton, bluebutton, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(red, green, blue, 1.0f)); // ���������� ���� ��� ����������� ����
	
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)u8"�������");
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
	if (ImGui::Button((const char*)u8"������� ����")) {
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
	ImGui::BeginChild("����� ������", ImVec2(404, 300), true);
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1),"31           ");
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)registr.c_str());
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1),"             0");
	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.00, 1.00, 0, 1));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0, 0, 0, 1.0f));
	if (ImGui::Button((const char*)u8"�������� �������"))
	{
		byteregistrs[NumberRegistr / 4][0] = "00";
		byteregistrs[NumberRegistr / 4][1] = "00";
		byteregistrs[NumberRegistr / 4][2] = "00";
		byteregistrs[NumberRegistr / 4][3] = "00";
	}
	ImGui::PopStyleColor(2);
	if (NumberRegistr % 4 == 0)
	{
		if (chet >= 9 and chet <= 13.5 and NumberCommand == 0)
		{
			ImGui::BeginChild("byte1", ImVec2(40, 40), true);
			BlinkingText((const char*)byteregistrs[NumberRegistr / 4][0].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte2", ImVec2(40, 40), true);
			BlinkingText((const char*)byteregistrs[NumberRegistr / 4][1].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte3", ImVec2(40, 40), true);
			BlinkingText((const char*)byteregistrs[NumberRegistr / 4][2].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte4", ImVec2(40, 40), true);
			BlinkingText((const char*)byteregistrs[NumberRegistr / 4][3].c_str());
			ImGui::EndChild();
		}
		else if (chet >= 4.5 and chet <= 9 and NumberCommand == 1)
		{
			ImGui::BeginChild("byte1", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][0].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte2", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][1].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte3", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][2].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte4", ImVec2(40, 40), true);
			BlinkingText((const char*)byteregistrs[NumberRegistr / 4][3].c_str());
			ImGui::EndChild();
		}
		else if (chet >= 9 and chet <= 13.5 and NumberCommand == 1)
		{
			byteregistrs[NumberRegistr / 4][3] = znachbyte1;
			ImGui::BeginChild("byte1", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][0].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte2", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][1].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte3", ImVec2(40, 40), true);
			BlinkingText((const char*)byteregistrs[NumberRegistr / 4][2].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte4", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][3].c_str());
			ImGui::EndChild();
		}
		else if (chet >= 13.5 and chet <= 18 and NumberCommand == 1)
		{
			byteregistrs[NumberRegistr / 4][2] = znachbyte2;
			ImGui::BeginChild("byte1", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][0].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte2", ImVec2(40, 40), true);
			BlinkingText((const char*)byteregistrs[NumberRegistr / 4][1].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte3", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][2].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte4", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][3].c_str());
			ImGui::EndChild();
		}
		else if (chet >= 18 and chet <= 22.5 and NumberCommand == 1)
		{
			byteregistrs[NumberRegistr / 4][1] = znachbyte3;
			ImGui::BeginChild("byte1", ImVec2(40, 40), true);
			BlinkingText((const char*)byteregistrs[NumberRegistr / 4][0].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte2", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][1].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte3", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][2].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte4", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][3].c_str());
			ImGui::EndChild();
		}
		else if (chet >= 22.5 and NumberCommand == 1) {
			chet = 0.0f;
			timer = 0.0f;
			isBlinking = !isBlinking;
			byteregistrs[NumberRegistr / 4][0] = znachbyte4;
			ImGui::BeginChild("byte1", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][0].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte2", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][1].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte3", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][2].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte4", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][3].c_str());
			ImGui::EndChild();
		}
		else if (chet >= 13.5 and NumberCommand == 0) {
			chet = 0.0f;
			timer = 0.0f;
			isBlinking = !isBlinking;
			byteregistrs[NumberRegistr / 4][0] = adresbyte1;
			byteregistrs[NumberRegistr / 4][1] = adresbyte2;
			byteregistrs[NumberRegistr / 4][2] = adresbyte3;
			byteregistrs[NumberRegistr / 4][3] = adresbyte4;
			ImGui::BeginChild("byte1", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][0].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte2", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][1].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte3", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][2].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte4", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][3].c_str());
			ImGui::EndChild();
		}
		else if (error == 3 and NumberTypePeremennoi == 1)
		{
			ImGui::BeginChild("byte1", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][0].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte2", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][1].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte3", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(1,0,0,1),(const char*)byteregistrs[NumberRegistr / 4][2].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte4", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(1,0,0,1),(const char*)byteregistrs[NumberRegistr / 4][3].c_str());
			ImGui::EndChild();
			}
		else if (error == 3 and NumberTypePeremennoi == 2)
		{
			ImGui::BeginChild("byte1", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][0].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte2", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][1].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte3", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(1, 1, 1, 1), (const char*)byteregistrs[NumberRegistr / 4][2].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte4", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(1, 0, 0, 1), (const char*)byteregistrs[NumberRegistr / 4][3].c_str());
			ImGui::EndChild();
			}
		else
		{
			ImGui::BeginChild("byte1", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][0].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte2", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][1].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte3", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][2].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte4", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][3].c_str());
			ImGui::EndChild();
		}

	}
	else if (NumberRegistr % 4 == 1)
	{
		if (chet >= 9 and chet <= 13.5 and NumberCommand == 0)
		{
			ImGui::BeginChild("byte1", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(0.5,0.5,0.5,1),(const char*)byteregistrs[NumberRegistr / 4][0].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte2", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][1].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte3", ImVec2(40, 40), true);
			BlinkingText((const char*)byteregistrs[NumberRegistr / 4][2].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte4", ImVec2(40, 40), true);
			BlinkingText((const char*)byteregistrs[NumberRegistr / 4][3].c_str());
			ImGui::EndChild();
		}
		else if (chet >= 4.5 and chet <= 9 and NumberCommand == 1)
		{
			ImGui::BeginChild("byte1", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][0].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte2", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][1].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte3", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][2].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte4", ImVec2(40, 40), true);
			BlinkingText((const char*)byteregistrs[NumberRegistr / 4][3].c_str());
			ImGui::EndChild();
		}
		else if (chet >= 9 and chet <= 13.5 and NumberCommand == 1)
		{
			byteregistrs[NumberRegistr / 4][3] = znachbyte1;
			ImGui::BeginChild("byte1", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][0].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte2", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][0].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte3", ImVec2(40, 40), true);
			BlinkingText((const char*)byteregistrs[NumberRegistr / 4][2].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte4", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][3].c_str());
			ImGui::EndChild();
		}
		else if (chet >= 13.5 and NumberCommand == 1) {
			chet = 0.0f;
			timer = 0.0f;
			isBlinking = !isBlinking;
			byteregistrs[NumberRegistr / 4][2] = znachbyte2;
			ImGui::BeginChild("byte1", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][0].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte2", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][1].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte3", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][2].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte4", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][3].c_str());
			ImGui::EndChild();
		}
		else if (chet >= 13.5 and NumberCommand == 0) {
			chet = 0.0f;
			timer = 0.0f;
			isBlinking = !isBlinking;
			byteregistrs[NumberRegistr / 4][2] = adresbyte3;
			byteregistrs[NumberRegistr / 4][3] = adresbyte4;
			ImGui::BeginChild("byte1", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][0].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte2", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][1].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte3", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][2].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte4", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][3].c_str());
			ImGui::EndChild();
		}
		else if (error == 4 and NumberTypePeremennoi == 0)
		{
			ImGui::BeginChild("byte1", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(1, 0, 0, 1), (const char*)byteregistrs[NumberRegistr / 4][0].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte2", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(1, 0, 0, 1), (const char*)byteregistrs[NumberRegistr / 4][1].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte3", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(1, 0, 0, 1), (const char*)byteregistrs[NumberRegistr / 4][2].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte4", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(1, 0, 0, 1), (const char*)byteregistrs[NumberRegistr / 4][3].c_str());
			ImGui::EndChild();
		}
		else if (error == 3 and NumberTypePeremennoi == 2)
		{
			ImGui::BeginChild("byte1", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][0].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte2", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][1].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte3", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(1, 1, 1, 1), (const char*)byteregistrs[NumberRegistr / 4][2].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte4", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(1, 0, 0, 1), (const char*)byteregistrs[NumberRegistr / 4][3].c_str());
			ImGui::EndChild();
		}
		else
		{
			ImGui::BeginChild("byte1", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][0].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte2", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][1].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte3", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][2].c_str());
			ImGui::EndChild();
			ImGui::SameLine(0, 0);
			ImGui::BeginChild("byte4", ImVec2(40, 40), true);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][3].c_str());
			ImGui::EndChild();
		}
	}
	else if (NumberRegistr % 4 == 2)
		{
			if (NumberCommand == 0 and error == 1)
			{

				ImGui::BeginChild("byte1", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(1, 0, 0, 1), (const char*)byteregistrs[NumberRegistr / 4][0].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte2", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(1, 0, 0, 1), (const char*)byteregistrs[NumberRegistr / 4][1].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte3", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(1, 0, 0, 1), (const char*)byteregistrs[NumberRegistr / 4][2].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte4", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(1, 0, 0, 1), (const char*)byteregistrs[NumberRegistr / 4][3].c_str());
				ImGui::EndChild();
			}
			else if (chet >= 4.5 and chet <= 9 and NumberCommand == 1)
			{
				ImGui::BeginChild("byte1", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][0].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte2", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][1].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte3", ImVec2(40, 40), true);
				BlinkingText((const char*)byteregistrs[NumberRegistr / 4][2].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte4", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][3].c_str());
				ImGui::EndChild();
			}
			else if (chet >= 9 and NumberCommand == 1) {
				chet = 0.0f;
				timer = 0.0f;
				isBlinking = !isBlinking;
				byteregistrs[NumberRegistr / 4][2] = znachbyte1;
				ImGui::BeginChild("byte1", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][0].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte2", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][1].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte3", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][2].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte4", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][3].c_str());
				ImGui::EndChild();
			}
			else if (error == 4 and NumberTypePeremennoi == 0)
			{
				ImGui::BeginChild("byte1", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(1, 0, 0, 1), (const char*)byteregistrs[NumberRegistr / 4][0].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte2", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(1, 0, 0, 1), (const char*)byteregistrs[NumberRegistr / 4][1].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte3", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(1, 0, 0, 1), (const char*)byteregistrs[NumberRegistr / 4][2].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte4", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(1, 0, 0, 1), (const char*)byteregistrs[NumberRegistr / 4][3].c_str());
				ImGui::EndChild();
			}
			else if (error == 4 and NumberTypePeremennoi == 1)
			{
				ImGui::BeginChild("byte1", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][0].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte2", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][1].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte3", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(1, 0, 0, 1), (const char*)byteregistrs[NumberRegistr / 4][2].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte4", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(1, 0, 0, 1), (const char*)byteregistrs[NumberRegistr / 4][3].c_str());
				ImGui::EndChild();
			}
			else
			{
				ImGui::BeginChild("byte1", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][0].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte2", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][1].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte3", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)byteregistrs[NumberRegistr / 4][2].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte4", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][3].c_str());
				ImGui::EndChild();
			}
		}
		else if (NumberRegistr % 4 == 3)
		{
			if (NumberCommand == 0 and error==1)
			{

				ImGui::BeginChild("byte1", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(1, 0, 0, 1), (const char*)byteregistrs[NumberRegistr / 4][0].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte2", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(1, 0, 0, 1), (const char*)byteregistrs[NumberRegistr / 4][1].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte3", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(1, 0, 0, 1), (const char*)byteregistrs[NumberRegistr / 4][2].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte4", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(1, 0, 0, 1), (const char*)byteregistrs[NumberRegistr / 4][3].c_str());
				ImGui::EndChild();
			}
			else if (chet >= 4.5 and chet <= 9 and NumberCommand == 1)
			{
				ImGui::BeginChild("byte1", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][0].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte2", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][1].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte3", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][2].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte4", ImVec2(40, 40), true);
				BlinkingText((const char*)byteregistrs[NumberRegistr / 4][3].c_str());
				ImGui::EndChild();
			}
			else if (chet >= 9 and NumberCommand == 1) {
				chet = 0.0f;
				timer = 0.0f;
				isBlinking = !isBlinking;
				byteregistrs[NumberRegistr / 4][3] = znachbyte1;
				ImGui::BeginChild("byte1", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][0].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte2", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][1].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte3", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][2].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte4", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)byteregistrs[NumberRegistr / 4][3].c_str());
				ImGui::EndChild();
			}
			else if (error == 4 and NumberTypePeremennoi == 0)
			{
				ImGui::BeginChild("byte1", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(1, 0, 0, 1), (const char*)byteregistrs[NumberRegistr / 4][0].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte2", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(1, 0, 0, 1), (const char*)byteregistrs[NumberRegistr / 4][1].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte3", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(1, 0, 0, 1), (const char*)byteregistrs[NumberRegistr / 4][2].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte4", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(1, 0, 0, 1), (const char*)byteregistrs[NumberRegistr / 4][3].c_str());
				ImGui::EndChild();
			}
			else if (error == 4 and NumberTypePeremennoi == 1)
			{
				ImGui::BeginChild("byte1", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][0].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte2", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][1].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte3", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(1, 0, 0, 1), (const char*)byteregistrs[NumberRegistr / 4][2].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte4", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(1, 0, 0, 1), (const char*)byteregistrs[NumberRegistr / 4][3].c_str());
				ImGui::EndChild();
			}
			else
			{
				ImGui::BeginChild("byte1", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][0].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte2", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][1].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte3", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][2].c_str());
				ImGui::EndChild();
				ImGui::SameLine(0, 0);
				ImGui::BeginChild("byte4", ImVec2(40, 40), true);
				ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)byteregistrs[NumberRegistr / 4][3].c_str());
				ImGui::EndChild();
			}
		}
	ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
	ImGui::Combo((const char*)u8"������� �������", &NumberCommand, Commads, IM_ARRAYSIZE(Commads));
	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0.5, 0, 1));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1.0f));
	if (ImGui::Button((const char*)u8"������������ �������"))
	{
		error = 0;
		if (NumberCommand == -1) error = 6;
		else if (NumberCommand == 0)
		{
			if ((NumberRegistr % 4 == 0 or NumberRegistr % 4 == 1) and NumberElements <= 1) isBlinking = !isBlinking;
			else if (NumberElements > 1) error = 7;
			else error = 1;
		}
		
		else
		{
			if (NumberRegistr % 4 == 0 and (NumberTypePeremennoi == 0 or (NumberElements-2)%4==0)) isBlinking = !isBlinking;
			else if (NumberRegistr % 4 == 0 and (NumberTypePeremennoi != 0 or (NumberElements - 2) % 4 != 0)) error = 3;
			else if (NumberRegistr % 4 == 1 and (NumberTypePeremennoi == 1 or (NumberElements - 2) % 4 == 1)) isBlinking = !isBlinking;
			else if (NumberRegistr % 4 == 1 and (NumberTypePeremennoi == 0 or (NumberElements - 2) % 4 == 0)) error = 4;
			else if (NumberRegistr % 4 == 1 and (NumberTypePeremennoi == 2 or (NumberElements - 2) % 4 >= 2)) error = 3;
			else if ((NumberRegistr % 4 == 2 or NumberRegistr % 4 == 3) and (NumberTypePeremennoi == 2 or (NumberElements - 2) % 4 >= 2)) isBlinking = !isBlinking;
			else error = 4;
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
			//BlinkingCombo("",  &NumberElements, Elements, IM_ARRAYSIZE(Elements));
			ImGui::Combo((const char*)u8"", &NumberElements, Elements, IM_ARRAYSIZE(Elements));
		}
		else
		{
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "LEA");
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)registr.c_str());
			ImGui::SameLine();
			ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
			ImGui::Combo((const char*)u8"", &NumberElements, Elements, IM_ARRAYSIZE(Elements));
		}

	}
	if (NumberCommand == 1)
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
			ImGui::Combo((const char*)u8"", &NumberElements, Elements, IM_ARRAYSIZE(Elements));
	
		}
		else
		{
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),"MOV");
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)registr.c_str());
			ImGui::SameLine();
			ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
			ImGui::Combo((const char*)u8"", &NumberElements, Elements, IM_ARRAYSIZE(Elements));
	
		}
	}
	if (NumberElements>1) 
	{
		znachbyte1 = byteregistrs[(NumberElements - 2) / 4][3];
		znachbyte2 = byteregistrs[(NumberElements - 2) / 4][2];
		znachbyte3 = byteregistrs[(NumberElements - 2) / 4][1];
		znachbyte4 = byteregistrs[(NumberElements - 2) / 4][0];
	}
	else 
	{
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
	if (ImGui::Button((const char*)u8"������ ���������� �1"))
	{
		peremennai = bufznach;
		error = 0;
		if (NumberTypePeremennoi == 0)
		{
			while (size(peremennai) < 8) peremennai = "0" + peremennai;
			if (NumberRegistr % 4 == 0)
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
	ImGui::Combo((const char*)u8"������� ��� ������ ���������� x1", &NumberTypePeremennoi, TypePeremennoi, IM_ARRAYSIZE(TypePeremennoi));
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1),"x1");
	ImGui::SameLine();
	std::transform(peremennai.begin(), peremennai.end(), peremennai.begin(),
		[](unsigned char c) { return std::toupper(c); });
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1),peremennai.c_str());
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(TypePeremennoi[NumberTypePeremennoi]));
	ImGui::EndChild();
	ImGui::SameLine(410, 0);
	ImGui::BeginChild("�����", ImVec2(115, 300), true);
	ImGui::InputText("##hidden", bufaddress, IM_ARRAYSIZE(bufaddress), ImGuiInputTextFlags_CharsHexadecimal);
	if (ImGui::Button((const char*)u8"������ �����"))
	{
		adres = bufaddress;
		error = 0;
		while (size(adres) < 8) adres = adres + "0";
		adresbyte1 = adres.substr(0, 2);
		adresbyte2 = adres.substr(2, 2);
		adresbyte3 = adres.substr(4, 2);
		adresbyte4 = adres.substr(6, 2);
	}
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1)," ");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1)," ");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)u8"�����");
	std::transform(adresbyte1.begin(), adresbyte1.end(), adresbyte1.begin(),
		[](unsigned char c) { return std::toupper(c); });
	std::transform(adresbyte2.begin(), adresbyte2.end(), adresbyte2.begin(),
		[](unsigned char c) { return std::toupper(c); });
	std::transform(adresbyte3.begin(), adresbyte3.end(), adresbyte3.begin(),
		[](unsigned char c) { return std::toupper(c); });
	std::transform(adresbyte4.begin(), adresbyte4.end(), adresbyte4.begin(),
		[](unsigned char c) { return std::toupper(c); });
	if (NumberCommand == 0) {
		if (chet >= 4.5 and chet <= 9  and NumberRegistr %4  == 0)
		{

			BlinkingText(adresbyte1.c_str());
			ImGui::SameLine(0, 0);
			BlinkingText(adresbyte2.c_str());
			ImGui::SameLine(0, 0);
			BlinkingText(adresbyte3.c_str());
			ImGui::SameLine(0, 0);
			BlinkingText(adresbyte4.c_str());
			ImGui::SameLine(0, 0);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),"h==>");
		}
		else if (chet >= 4.5 and chet <= 9 and NumberRegistr % 4 == 1)
		{
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1), adresbyte1.c_str());
			ImGui::SameLine(0, 0);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1), adresbyte2.c_str());
			ImGui::SameLine(0, 0);			
			BlinkingText(adresbyte3.c_str());
			ImGui::SameLine(0, 0);
			BlinkingText(adresbyte4.c_str());
			ImGui::SameLine(0, 0);
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "h==>");
		}
		else 
		{
			ImGui::TextColored(ImVec4(redadres, greenadres, blueadres, 1),adresbyte1.c_str());
			ImGui::SameLine(0, 0);
			ImGui::TextColored(ImVec4(redadres, greenadres, blueadres, 1),adresbyte2.c_str());
			ImGui::SameLine(0, 0);
			ImGui::TextColored(ImVec4(redadres, greenadres, blueadres, 1),adresbyte3.c_str());
			ImGui::SameLine(0, 0);
			ImGui::TextColored(ImVec4(redadres, greenadres, blueadres, 1),adresbyte4.c_str());
			ImGui::SameLine(0, 0);
			ImGui::TextColored(ImVec4(redadres, greenadres, blueadres, 1),"h==>");
		}
	}
	else
	{
		ImGui::TextColored(ImVec4(redadres, greenadres, blueadres, 1), adresbyte1.c_str());
		ImGui::SameLine(0, 0);
		ImGui::TextColored(ImVec4(redadres, greenadres, blueadres, 1), adresbyte2.c_str());
		ImGui::SameLine(0, 0);
		ImGui::TextColored(ImVec4(redadres, greenadres, blueadres, 1), adresbyte3.c_str());
		ImGui::SameLine(0, 0);
		ImGui::TextColored(ImVec4(redadres, greenadres, blueadres, 1), adresbyte4.c_str());
		ImGui::SameLine(0, 0);
		ImGui::TextColored(ImVec4(redadres, greenadres, blueadres, 1), "h==>");
	}
		if (NumberTypePeremennoi % 4 == 1)
		{

			unsigned long long hexadres;
			std::stringstream ss;
			ss << std::hex << adres;
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
			ss << std::hex << adres;
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
			ss3 << std::hex << adres;
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
			ss5 << std::hex << adres;
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
	ImGui::BeginChild("����������", ImVec2(110, 300), ImGuiWindowFlags_NoMove);
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)u8"�����������");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)u8"������");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1)," ");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1)," ");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1)," ");
	ImGui::BeginChild("��������1", ImVec2(87, 40), true);
	if (chet >= 4.5f and chet <= 9.0f and NumberCommand==1)
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
		if (NumberTypePeremennoi % 4 == 0)
		{
			ImGui::BeginChild("��������2", ImVec2(87, 40), true);
			if (chet >= 9.0f and chet <= 13.5f and NumberCommand == 1)
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
			ImGui::BeginChild("��������3", ImVec2(87, 40), true);
			if (chet >= 13.5 and chet <= 18 and NumberCommand == 1)
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
			ImGui::BeginChild("��������4", ImVec2(87, 40), true);
			if (chet >= 18 and chet <= 22.5)
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
		else if (NumberTypePeremennoi % 4 == 1)
		{
			ImGui::BeginChild("��������2", ImVec2(87, 40), true);
			if (chet >= 9.0f and chet <= 13.5f and NumberCommand == 1)
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
	ImGui::BeginChild("��������", ImVec2(140, 300), ImGuiWindowFlags_NoMove);
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)u8"  ��������");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1), " ");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1), " ");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1), " ");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "EAX=");
	ImGui::SameLine(0,0);
	for (int i = 0; i < 4; i++) {
		ImGui::TextColored(ImVec4(red1, green1, blue1, 1), byteregistrs[0][i].c_str());
		if (i != 3) ImGui::SameLine(0,0);
	}
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1), " ");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "EBX=");
	ImGui::SameLine(0, 0);
	for (int i = 0; i < 4; i++) {
		ImGui::TextColored(ImVec4(red1, green1, blue1, 1), byteregistrs[1][i].c_str());
		if (i != 3) ImGui::SameLine(0, 0);
	}
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1), " ");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "ECX=");
	ImGui::SameLine(0, 0);
	for (int i = 0; i < 4; i++) {
		ImGui::TextColored(ImVec4(red1, green1, blue1, 1), byteregistrs[2][i].c_str());
		if (i != 3) ImGui::SameLine(0, 0);
	}
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1), " ");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "EDX=");
	ImGui::SameLine(0, 0);
	for (int i = 0; i < 4; i++) {
		ImGui::TextColored(ImVec4(red1, green1, blue1, 1), byteregistrs[3][i].c_str());
		if (i != 3) ImGui::SameLine(0, 0);
	}
	ImGui::EndChild();
	ImGui::BeginChild("��������", ImVec2(625, 150), true);
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
		ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), (const char*)u8"������: ���� ������� �� ����� ���� ����������� ��� ������� lea.");
		ImGui::TextColored(ImVec4(0.0, 1.0, 0.0, 1.0), (const char*)u8"����� ������������ ������� ������� x16 ��� x32");
	}
	else if (error == 2)
	{
		ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), (const char*)u8"������: �������� �������� ���������� ������ �����������");
		ImGui::TextColored(ImVec4(0, 1.0, 0.0, 1.0), (const char*)u8"����� ��������� ��� ���������� ��� ��������� �� �������� ��������");
		greenznach = 0;
		blueznach = 0;
		redznach = 1;
	}
	else if (error == 3)
	{
		ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), (const char*)u8"������: ������� ������ ���������� x1");
		ImGui::TextColored(ImVec4(0, 1.0, 0.0, 1.0), (const char*)u8"����� ��������� ���������� ��� ��������� �������");
	}
	else if (error == 4)
	{
		ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), (const char*)u8"������: ���������� x1 ������ �������� ");
		ImGui::TextColored(ImVec4(0, 1.0, 0.0, 1.0), (const char*)u8"����� ��������� ������� ��� ��������� ����������");

	}
	else if (error == 5)
	{
		ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), (const char*)u8"������: ����� �� �������� ������� ");
		ImGui::TextColored(ImVec4(0.0, 1.0, 0.0, 1.0), (const char*)u8"����� ��������� ����� ");
		greenadres = 0;
		blueadres = 0;
		redadres = 1;
	}
	else if (error == 6)
	{
		ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), (const char*)u8"�������� �������");

	}
	else if (error == 7)
	{
		ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), (const  char*)u8"������ ����� ����� ��������");
		ImGui::TextColored(ImVec4(0.0, 1.0, 0.0, 1.0), (const char*)u8"����� ������� ����������");
	}
	ImGui::EndChild();
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)u8"��� ������");
	ImGui::Separator();

	if (ImGui::Button("YouTube")) {
		ShellExecute(NULL, "open", "https://www.youtube.com/c/@king174rus", 0, 0, SW_SHOWNORMAL);
	}
	ImGui::SameLine(); ImGui::Text(" "); ImGui::SameLine();
	if (ImGui::Button((const char*)u8"GitHub")) {
		ShellExecute(NULL, "open", "https://github.com/king174rus", 0, 0, SW_SHOWNORMAL);
	}
	ImGui::PopStyleColor(4); // ���������� ����������� ���� ���� ����

	ImGui::End();
}