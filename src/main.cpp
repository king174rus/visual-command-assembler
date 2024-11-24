// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>
#include <string>
#include <chrono>
#include <algorithm> //  std::transform
#include <cctype>    // std::toupper
#include <sstream>
#include <cstddef>
#include <vector>
#include <iterator>
#include <cstdlib>
#pragma execution_character_set("utf-8")
using namespace std::chrono;
using namespace std;

#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}


        // Start the Dear ImGui frame
       bool isRunning = true;
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

const char* Registres[] = { "EAX", "AX", "AH", "AL","EBX", "BX", "BH", "BL" ,"ECX", "CX", "CH", "CL" ,"EDX", "DX", "DH", "DL" };
static int NumberRegistr = 0;
const char* TypePeremennoi[] = { "dd", "dw", "db"};
static int NumberTypePeremennoi = 0;
const char* Elements[] = { "x1", "[x1]", "EAX", "AX", "AH", "AL","EBX", "BX", "BH", "BL" ,"ECX", "CX", "CH", "CL" ,"EDX", "DX", "DH", "DL",
						  "[EAX]", "[AX]", "[AH]", "[AL]","[EBX]", "[BX]", "[BH]", "[BL]" ,"[ECX]", "[CX]", "[CH]", "[CL]" ,"[EDX]", "[DX]", "[DH]", "[DL]" };
static int NumberElements = 0;


const char* Commads[] = {"LEA", "MOV"};
static int NumberCommand = -1;
float timer = 0.0f;
float chet = 0.0f;
int error = 0;
bool isBlinking = false;
string byteregistrs[4][4] = { {"00","00","00","00"},{"00","00","00","00"} ,{"00","00","00","00"} ,{"00","00","00","00"} };
int otchet = 0;

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
		id = i+1;
		ImGui::BeginChild(id, ImVec2(40, 40), true);
		if (byte[i] == "Blink")
		{
			BlinkingText((const char*)byteregistrs[NumberRegistr / 4][i].c_str());
		}
		else if (byte[i] == "White")
		{
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)byteregistrs[NumberRegistr / 4][i].c_str());
		}
		else if (byte[i] == "Red")
		{
			ImGui::TextColored(ImVec4(1, 0, 0, 1), (const char*)byteregistrs[NumberRegistr / 4][i].c_str());
		}
		else if (byte[i] == "Gray")
		{
			ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), (const char*)byteregistrs[NumberRegistr / 4][i].c_str());
		}
		ImGui::EndChild();
		if(i != 3) ImGui::SameLine(0, 0);
	}
	return;
}



// Main code
int main(int, char**)
{
    setlocale(0, "RUSSIAN");
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    //glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
	GLFWwindow* window = glfwCreateWindow(800, 500, "Visual Command Assembler", nullptr, nullptr);
	
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImFont* font = io.Fonts->AddFontFromFileTTF("/home/user/Desktop/kurs/fonts/arialmt.ttf", 16.5f, nullptr, io.Fonts->GetGlyphRangesCyrillic());
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");
#endif
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
	//ImFont* font = io.Fonts->AddFontFromFileTTF("/\\home\\user\\Desktop\\kurs\\fonts\\arialmt.ttf", 18.0f, nullptr, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != nullptr);

    // Our state

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!glfwWindowShouldClose(window))
#endif
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();


        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to create a named window.
       
{
    ImGui::SetNextWindowPos({ 0, 0 });
	ImGui::SetNextWindowSize({ 800, 500 });
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(red, green, blue, 1.0f)); 
	ImGui::Begin(
	"Visual Command Assembler",
    	&isRunning,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoSavedSettings |
		ImGuiWindowFlags_NoCollapse |
		ImGuiWindowFlags_NoMove
	);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(red1, green1, blue1, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(redbutton, greenbutton, bluebutton, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(red, green, blue, 1.0f)); 
	
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)u8"Регистр");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.08f);
	ImGui::Combo((const char*)u8"##hiddencombo", &NumberRegistr, Registres, IM_ARRAYSIZE(Registres));
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
			byteregistrs[NumberRegistr / 4][3] = znachbyte1;
			string bytes[4] = { "White", "White", "Blink", "White" };
			ByteText(bytes);
		}
		else if (chet >= 13.5 and chet <= 18 and NumberCommand == 1)
		{
			byteregistrs[NumberRegistr / 4][2] = znachbyte2;
			string bytes[4] = { "White", "Blink", "White", "White" };
			ByteText(bytes);
		}
		else if (chet >= 18 and chet <= 22.5 and NumberCommand == 1)
		{
			byteregistrs[NumberRegistr / 4][1] = znachbyte3;
			string bytes[4] = { "Blink", "White", "White", "White" };
			ByteText(bytes);
		}
		else if (chet >= 22.5 and NumberCommand == 1) {
			chet = 0.0f;
			timer = 0.0f;
			isBlinking = !isBlinking;
			byteregistrs[NumberRegistr / 4][0] = znachbyte4;
			string bytes[4] = { "White", "White", "White", "White" };
			ByteText(bytes);
		}
		else if (chet >= 13.5 and NumberCommand == 0 and NumberElements<=1) {
			chet = 0.0f;
			timer = 0.0f;
			isBlinking = !isBlinking;
			byteregistrs[NumberRegistr / 4][0] = adresbyte1;
			byteregistrs[NumberRegistr / 4][1] = adresbyte2;
			byteregistrs[NumberRegistr / 4][2] = adresbyte3;
			byteregistrs[NumberRegistr / 4][3] = adresbyte4;
			string bytes[4] = { "White", "White", "White", "White" };
			ByteText(bytes);
		}
		else if (chet >= 13.5 and NumberCommand == 0 and NumberElements>=18 and (NumberElements-2)%4==0) {
			chet = 0.0f;
			timer = 0.0f;
			isBlinking = !isBlinking;
			byteregistrs[NumberRegistr / 4][0] = byteregistrs[(NumberElements-18) / 4][0];
			byteregistrs[NumberRegistr / 4][1] = byteregistrs[(NumberElements - 18) / 4][1];
			byteregistrs[NumberRegistr / 4][2] = byteregistrs[(NumberElements - 18) / 4][2];
			byteregistrs[NumberRegistr / 4][3] = byteregistrs[(NumberElements - 18) / 4][3];
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
	else if (NumberRegistr % 4 == 1)
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
			byteregistrs[NumberRegistr / 4][3] = znachbyte1;
			string bytes[4] = { "Gray", "Gray", "Blink", "White" };
			ByteText(bytes);
		}
		else if (chet >= 13.5 and NumberCommand == 1) {
			chet = 0.0f;
			timer = 0.0f;
			isBlinking = !isBlinking;
			byteregistrs[NumberRegistr / 4][2] = znachbyte2;
			ImGui::BeginChild("byte1", ImVec2(40, 40), true);
			string bytes[4] = { "Gray", "Gray", "White", "White" };
			ByteText(bytes);
		}
		else if (chet >= 13.5 and NumberCommand == 0) {
			chet = 0.0f;
			timer = 0.0f;
			isBlinking = !isBlinking;
			byteregistrs[NumberRegistr / 4][2] = adresbyte3;
			byteregistrs[NumberRegistr / 4][3] = adresbyte4;
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
	else if (NumberRegistr % 4 == 2)
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
				byteregistrs[NumberRegistr / 4][2] = znachbyte1;
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
		else if (NumberRegistr % 4 == 3)
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
				byteregistrs[NumberRegistr / 4][3] = znachbyte1;
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
	ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
	ImGui::Combo((const char*)u8"Выбрать команду", &NumberCommand, Commads, IM_ARRAYSIZE(Commads));
	ImGui::SameLine();
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0.5, 0, 1));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1.0f));
	if (ImGui::Button((const char*)u8"Активировать команду"))
	{
		if ((NumberElements - 2) % 4 != 0 and NumberElements >= 18) error = 8;
		else
		{
			error = 0;
			if (NumberCommand == -1) error = 6;
			else if (NumberCommand == 0)
			{
				if ((NumberRegistr % 4 == 0 or NumberRegistr % 4 == 1) and (NumberElements <= 1 or (NumberElements >= 18 and (NumberElements - 2) % 4 == 0))) isBlinking = !isBlinking;
				else if (NumberElements > 1 and NumberElements < 18) error = 7;
				else if (NumberElements >= 18 and (NumberElements - 2) % 4 != 0) error = 8;
				else error = 1;
			}

			else
			{
				if (otchet == 4 or ((NumberElements >= 18 and otchet != 0) and NumberRegistr%4==0) or ((NumberElements >= 18 and otchet == 3) and NumberRegistr % 4 == 1)) error = 9;
				else if (NumberRegistr % 4 == 0 and ((NumberTypePeremennoi == 0 and NumberElements <= 1) or (((NumberElements - 2) % 4 == 0) and NumberElements < 18) or (NumberElements >= 18 and otchet == 0))) isBlinking = !isBlinking;
				else if (NumberRegistr % 4 == 0 and ((NumberTypePeremennoi != 0 and NumberElements <= 1) or ((NumberElements - 2) % 4 != 0 and NumberElements<18) )) error = 3;
				else if (NumberRegistr % 4 == 1 and ((NumberTypePeremennoi == 1 and NumberElements <= 1) or ((NumberElements - 2) % 4 == 1 and NumberElements < 18) or (NumberElements >= 18 and otchet != 3))) isBlinking = !isBlinking;
				else if (NumberRegistr % 4 == 1 and ((NumberTypePeremennoi == 0 and NumberElements <= 1) or ((NumberElements - 2) % 4 == 0 and NumberElements < 18))) error = 4;
				else if (NumberRegistr % 4 == 1 and ((NumberTypePeremennoi == 2 and NumberElements <= 1) or ((NumberElements - 2) % 4 >= 2 and NumberElements < 18))) error = 3;
				else if ((NumberRegistr % 4 == 2 or NumberRegistr % 4 == 3) and ((NumberTypePeremennoi == 2 and NumberElements <= 1) or ((NumberElements - 2) % 4 >= 2 and NumberElements < 18) or NumberElements >= 18)) isBlinking = !isBlinking;
				else error = 4;
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
			if (!isBlinking) ImGui::Combo((const char*)u8"##hiddencombo2", &NumberElements, Elements, IM_ARRAYSIZE(Elements));
			else BlinkingText((const char*)Elements[NumberElements]);
		}
		else
		{
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "LEA");
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)registr.c_str());
			ImGui::SameLine();
			ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
			if (!isBlinking) ImGui::Combo((const char*)u8"##Combo", &NumberElements, Elements, IM_ARRAYSIZE(Elements));
			else ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)Elements[NumberElements]);
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
			if (!isBlinking) ImGui::Combo((const char*)u8"##Combo2", &NumberElements, Elements, IM_ARRAYSIZE(Elements));
			else BlinkingText((const char*)Elements[NumberElements]);
	
		}
		else
		{
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),"MOV");
			ImGui::SameLine();
			ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)registr.c_str());
			ImGui::SameLine();
			ImGui::SetNextItemWidth(ImGui::GetWindowWidth() * 0.15f);
			if (!isBlinking) ImGui::Combo((const char*)u8"##Combo3", &NumberElements, Elements, IM_ARRAYSIZE(Elements));
			else ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)Elements[NumberElements]);
	
		}
	}
	if (!isBlinking)
	{
		if (NumberElements > 1 and NumberElements < 18)
		{
			otchet = 0;
			znachbyte1 = byteregistrs[(NumberElements - 2) / 4][3];
			znachbyte2 = byteregistrs[(NumberElements - 2) / 4][2];
			znachbyte3 = byteregistrs[(NumberElements - 2) / 4][1];
			znachbyte4 = byteregistrs[(NumberElements - 2) / 4][0];
		}
		else if (NumberElements >= 18)
		{
			if (NumberTypePeremennoi == 0)
			{
				if (adresbyte1 == byteregistrs[(NumberElements - 18) / 4][0] and adresbyte2 == byteregistrs[(NumberElements - 18) / 4][1] and adresbyte3 == byteregistrs[(NumberElements - 18) / 4][2] and adresbyte4 == byteregistrs[(NumberElements - 18) / 4][3])
				{
					otchet = 0;
					znachbyte4 = peremennai.substr(0, 2);
					znachbyte3 = peremennai.substr(2, 2);
					znachbyte2 = peremennai.substr(4, 2);
					znachbyte1 = peremennai.substr(6, 2);
				}
				else
				{
					glavregistr = byteregistrs[(NumberElements - 18) / 4][0] + byteregistrs[(NumberElements - 18) / 4][1] + byteregistrs[(NumberElements - 18) / 4][2] + byteregistrs[(NumberElements - 18) / 4][3];
					unsigned long long hexadres;
					std::stringstream sss;
					sss << std::hex << adres;
					sss >> hexadres;
					unsigned long long hexregistr;
					std::stringstream sss1;
					sss1 << std::hex << glavregistr;
					sss1 >> hexregistr;
					int otchet1;
					otchet1 = hexregistr - hexadres;
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
			else if (NumberTypePeremennoi == 2)
			{
				if (adresbyte1 == byteregistrs[(NumberElements - 18) / 4][0] and adresbyte2 == byteregistrs[(NumberElements - 18) / 4][1] and adresbyte3 == byteregistrs[(NumberElements - 18) / 4][2] and adresbyte4 == byteregistrs[(NumberElements - 18) / 4][3])
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
			else if (NumberTypePeremennoi == 1)
			{
				if (adresbyte1 == byteregistrs[(NumberElements - 18) / 4][0] and adresbyte2 == byteregistrs[(NumberElements - 18) / 4][1] and adresbyte3 == byteregistrs[(NumberElements - 18) / 4][2] and adresbyte4 == byteregistrs[(NumberElements - 18) / 4][3])
				{
					otchet = 2;
					znachbyte4 = "";
					znachbyte3 = "";
					znachbyte2 = peremennai.substr(0, 2);
					znachbyte1 = peremennai.substr(2, 2);
				}
				else
				{
					glavregistr = byteregistrs[(NumberElements - 18) / 4][0] + byteregistrs[(NumberElements - 18) / 4][1] + byteregistrs[(NumberElements - 18) / 4][2] + byteregistrs[(NumberElements - 18) / 4][3];
					unsigned long long hexadres;
					std::stringstream sss;
					sss << std::hex << adres;
					sss >> hexadres;
					unsigned long long hexregistr;
					std::stringstream sss1;
					sss1 << std::hex << glavregistr;
					sss1 >> hexregistr;
					int otchet1;
					otchet1 = hexregistr - hexadres;
					if (otchet1 == 1)
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
	ImGui::InputText("##hidden1", bufznach, IM_ARRAYSIZE(bufznach), ImGuiInputTextFlags_CharsHexadecimal);
	ImGui::PopStyleColor(1);
	ImGui::SameLine();
	if (ImGui::Button((const char*)u8"Ввести переменную х1"))
	{
		peremennai = bufznach;
		error = 0;
		if (NumberTypePeremennoi == 0)
		{
			while (peremennai.size() < 8) peremennai = "0" + peremennai;
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
			while (peremennai.size() < 4) peremennai = "0" + peremennai;
			if (peremennai.size() <= 4)
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
			while (peremennai.size() < 2) peremennai = "0" + peremennai;
			if (peremennai.size() <= 2)
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
	ImGui::BeginChild("Адрес ", ImVec2(115, 300), true);
	ImGui::InputText("##hidden", bufaddress, IM_ARRAYSIZE(bufaddress), ImGuiInputTextFlags_CharsHexadecimal);
	if (ImGui::Button((const char*)u8"Ввести адрес"))
	{
		adres = bufaddress;
		error = 0;
		while (adres.size() < 8) adres = adres + "0";
		adresbyte1 = adres.substr(0, 2);
		adresbyte2 = adres.substr(2, 2);
		adresbyte3 = adres.substr(4, 2);
		adresbyte4 = adres.substr(6, 2);
	}
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1)," ");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1)," ");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)u8"Адрес");
	std::transform(adresbyte1.begin(), adresbyte1.end(), adresbyte1.begin(),
		[](unsigned char c) { return std::toupper(c); });
	std::transform(adresbyte2.begin(), adresbyte2.end(), adresbyte2.begin(),
		[](unsigned char c) { return std::toupper(c); });
	std::transform(adresbyte3.begin(), adresbyte3.end(), adresbyte3.begin(),
		[](unsigned char c) { return std::toupper(c); });
	std::transform(adresbyte4.begin(), adresbyte4.end(), adresbyte4.begin(),
		[](unsigned char c) { return std::toupper(c); });
	if (NumberCommand == 0) {
		if (chet >= 4.5 and chet <= 9  and NumberRegistr %4  == 0 and NumberElements<=1)
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
			if (ss2.str().size() == 9) error = 5;
			else {
				resultString = ss2.str();
				std::transform(resultString.begin(), resultString.end(), resultString.begin(),
					[](unsigned char c) { return std::toupper(c); });
				while (resultString.size() < 8) resultString = "0" + resultString;
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
			if (ss2.str().size() == 9) error = 5;
			else {
			resultString = ss2.str();
			std::transform(resultString.begin(), resultString.end(), resultString.begin(),
				[](unsigned char c) { return std::toupper(c); });
				while (resultString.size() < 8) resultString = "0" + resultString;
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
			if (ss4.str().size() == 9) error = 5;
			else {
				resultString = ss4.str();
				std::transform(resultString.begin(), resultString.end(), resultString.begin(),
					[](unsigned char c) { return std::toupper(c); });
				while (resultString.size() < 8) resultString = "0" + resultString;
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
			if (ss6.str().size() == 9) error = 5;
			else {
				resultString = ss6.str();
				std::transform(resultString.begin(), resultString.end(), resultString.begin(),
					[](unsigned char c) { return std::toupper(c); });
				while (resultString.size() < 8) resultString = "0" + resultString;
				resultString += "h==>";
				ImGui::TextColored(ImVec4(red1, green1, blue1, 1),"");
				ImGui::TextColored(ImVec4(0.5, 0.5, 0.5, 1), resultString.c_str());
			}
		}

	ImGui::EndChild();
	ImGui::SameLine(525, 0);
	ImGui::BeginChild("Оперативка", ImVec2(110, 300), true, ImGuiWindowFlags_NoMove);
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)u8"Оперативная");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)u8"Память");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1)," ");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1)," ");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1)," ");
	ImGui::BeginChild("Значение1", ImVec2(87, 40), true);
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
			ImGui::BeginChild("Значение2", ImVec2(87, 40), true);
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
			
			ImGui::BeginChild("Значение3", ImVec2(87, 40), true);
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
			ImGui::BeginChild("Значение4", ImVec2(87, 40), true);
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
			ImGui::BeginChild("Значение2", ImVec2(87, 40), true);
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
	ImGui::BeginChild("Регистры", ImVec2(140, 300), true, ImGuiWindowFlags_NoMove);
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1), (const char*)u8"  Регистры");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1), " ");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1), " ");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1), " ");
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1), "EAX=");
	ImGui::SameLine(0,0);
	for (int i = 0; i < 4; i++) {
		if (NumberCommand==0 and chet >= 4.5 and chet <= 9 and NumberRegistr % 4 == 0 and NumberElements >= 18  and (NumberElements-2)/4==4)
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
		if (NumberCommand == 0 and chet >= 4.5 and chet <= 9 and NumberRegistr % 4 == 0 and NumberElements >= 18 and (NumberElements - 2) / 4 == 5)
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
		if (NumberCommand == 0 and chet >= 4.5 and chet <= 9 and NumberRegistr % 4 == 0 and NumberElements >= 18 and (NumberElements - 2) / 4 == 6)
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
		if (NumberCommand == 0 and chet >= 4.5 and chet <= 9 and NumberRegistr % 4 == 0 and NumberElements >= 18 and (NumberElements - 2) / 4 == 7)
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
		ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), (const char*)u8"Ошибка: Переменная x1 или другой регистр больше основного регистра");
		ImGui::TextColored(ImVec4(0, 1.0, 0.0, 1.0), (const char*)u8"Нужно увеличить регистр или уменьшить переменную");

	}
	else if (error == 5)
	{
		ImGui::TextColored(ImVec4(1.0, 0.0, 0.0, 1.0), (const char*)u8"Ошибка: Выход за диапазон адресов");
		ImGui::TextColored(ImVec4(0.0, 1.0, 0.0, 1.0), (const char*)u8"Нужно уменьшить адрес");
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
		ImGui::TextColored(ImVec4(0.0, 1.0, 0.0, 1.0), (const char*)u8"Нужно увеличить адрес или уменьшить значение указатель");
	}
	ImGui::EndChild();
	ImGui::TextColored(ImVec4(red1, green1, blue1, 1),(const char*)u8"Мои ссылки");
	ImGui::Separator();

	if (ImGui::Button("YouTube")) {
		system("xdg-open https://www.youtube.com/c/@king174rus");
	}
	ImGui::SameLine(); ImGui::Text(" "); ImGui::SameLine();
	if (ImGui::Button((const char*)u8"GitHub")) {
		system("xdg-open https://github.com/king174rus");
	}
	ImGui::PopStyleColor(5); 

	
    ImGui::End();
        }


        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
