//88-Line 2D Moving Least Squares Material Point Method (MLS-MPM)[with comments]
//#define TC_IMAGE_IO   // Uncomment this line for image exporting functionality
#include "taichi.h"    // Note: You DO NOT have to install taichi or taichi_mpm.


//imgui
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <glfw3.h> // Will drag system OpenGL headers
// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

using namespace taichi;// You only need [taichi.h] - see below for instructions.
const int n = 80 /*grid resolution (cells)*/, window_size = 800;
const real dt = 1e-4_f, frame_dt = 1e-4_f, dx = 1.0_f / n, inv_dx = 1.0_f / dx;

const int num1 = 1000, num2 = 100;
int move_direction_player = 0;   //-1左，1右，0不动
int move_direction_computer = 0;
real location_player = 0;     //0到1
real location_computer = 1;
real lpx = 0, lpy = 0;    //location_player_x,location_player_y
real lcx = 0, lcy = 0;    //location_computer_x,location_computer_y
const real speed = 66;  //玩家和电脑的移动速度
bool hit_player = false;  //是否命中 
bool hit_computer = false;
int jump_player = 0;  //1或0，跳或不跳。  目前击球动作只有跳（将来还可以做站立、蹲）
int jump_computer = 0;
bool start_game = false; //游戏开始
double xpos, ypos;  //鼠标位置
int timer_p = 0;    //timer_player
int timer_c = 0;  //timer_computer 
int timer_cc = 0; //timer_computer_cooling  电脑起跳的冷却时间
real radius = 0.03;
double force_x = 20.0, force_y = 10.0;


auto particle_mass = 1.0_f, vol = 1.0_f;   /*默认粒子质量和体积*/
auto hardening = 10.0_f, E = 1e4_f, nu = 0.2_f;
real mu_0 = E / (2 * (1 + nu)), lambda_0 = E * nu / ((1 + nu) * (1 - 2 * nu));
using Vec = Vector2; using Mat = Matrix2; bool plastic = true;
struct Particle
{
	Vec x, v; Mat F, C; real Jp; int c/*color*/;
	int ptype/*0: fluid 1: jelly 2: snow*/;
	real mass; /*粒子质量*/
	Particle(Vec x, int c, Vec v = Vec(0), int ptype = 2, real mass = particle_mass) : x(x), v(v), F(1), C(0), Jp(1), c(c), ptype(ptype), mass(mass) {}
};
std::vector<Particle> particles;
Vector3 grid[n + 1][n + 1];          // velocity + mass, node_res = cell_res + 1
void advance(real dt)
{
	std::memset(grid, 0, sizeof(grid));       // Reset grid
	for (auto& p : particles)
	{                                                       // P2G
		Vector2i base_coord = (p.x * inv_dx - Vec(0.5_f)).cast<int>();//element-wise floor
		Vec fx = p.x * inv_dx - base_coord.cast<real>();
		// Quadratic kernels  [http://mpm.graphics   Eqn. 123, with x=fx, fx-1,fx-2]
		Vec w[3]{ Vec(0.5) * sqr(Vec(1.5) - fx), Vec(0.75) - sqr(fx - Vec(1.0)),
				 Vec(0.5) * sqr(fx - Vec(0.5)) };
		auto e = std::exp(hardening * (1.0_f - p.Jp));
		if (p.ptype == 1) e = 1.0;
		auto mu = mu_0 * e, lambda = lambda_0 * e;
		if (p.ptype == 0) mu = 0;
		real J = determinant(p.F);         // Current volume
		Mat r, s; polar_decomp(p.F, r, s); //Polar decomp. for fixed corotated model
		auto stress =                           // Cauchy stress times dt and inv_dx
			-4 * inv_dx * inv_dx * dt * vol * (2 * mu * (p.F - r) * transposed(p.F) + lambda * (J - 1) * J);
		auto affine = stress + p.mass * p.C;
		for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++)
		{ // Scatter to grid
			auto dpos = (Vec(i, j) - fx) * dx;
			Vector3 mv(p.v * p.mass, p.mass); //translational momentum
			grid[base_coord.x + i][base_coord.y + j] +=
				w[i].x * w[j].y * (mv + Vector3(affine * dpos, 0));
		}
	}
	for (int i = 0; i <= n; i++) for (int j = 0; j <= n; j++)
	{ //For all grid nodes
		auto& g = grid[i][j];
		if (g[2] > 0)
		{                                // No need for epsilon here
			g /= g[2];                                   //        Normalize by mass
			g += dt * Vector3(0, -2000, 0);               //                  Gravity
			//g = Vector3(10,0,0);//g是速度？
			real boundary = 0.05, x = (real)i / n, y = real(j) / n; //boundary thick.,node coord
			if (x < boundary || x > 1 - boundary || y > 1 - boundary) g = Vector3(0); //Sticky
			if (y < boundary) g[1] = std::max(0.0_f, g[1]);             //"Separate"
		}
	}
	for (auto& p : particles)
	{                                // Grid to particle
		Vector2i base_coord = (p.x * inv_dx - Vec(0.5_f)).cast<int>();//element-wise floor
		Vec fx = p.x * inv_dx - base_coord.cast<real>();
		Vec w[3]{ Vec(0.5) * sqr(Vec(1.5) - fx), Vec(0.75) - sqr(fx - Vec(1.0)),
				 Vec(0.5) * sqr(fx - Vec(0.5)) };
		p.C = Mat(0); p.v = Vec(0);
		for (int i = 0; i < 3; i++) for (int j = 0; j < 3; j++)
		{
			auto dpos = (Vec(i, j) - fx),
				grid_v = Vec(grid[base_coord.x + i][base_coord.y + j]);
			auto weight = w[i].x * w[j].y;
			p.v += weight * grid_v;                                      // Velocity
			p.C += 4 * inv_dx * Mat::outer_product(weight * grid_v, dpos); // APIC C
		}
		p.x += dt * p.v;                                                // Advection
		auto F = (Mat(1) + dt * p.C) * p.F;                      // MLS-MPM F-update
		if (p.ptype == 0) { p.F = Mat(1) * sqrt(determinant(F)); }
		else if (p.ptype == 1) { p.F = F; }
		else if (p.ptype == 2)
		{
			Mat svd_u, sig, svd_v; svd(F, svd_u, sig, svd_v);
			for (int i = 0; i < 2 * int(plastic); i++)                // Snow Plasticity
				sig[i][i] = clamp(sig[i][i], 1.0_f - 2.5e-2_f, 1.0_f + 7.5e-3_f);
			real oldJ = determinant(F); F = svd_u * sig * transposed(svd_v);
			real Jp_new = clamp(p.Jp * oldJ / determinant(F), 0.6_f, 20.0_f);
			p.Jp = Jp_new; p.F = F;
		}
	}
}
void add_object(Vec center, real radius, int c, int num = 500, Vec velocity = Vec(0.0_f), int ptype = 2, real mass = particle_mass)
{   // Seed particles with position and color
	for (int i = 0; i < num; i++)  // Randomly sample 1000 particles in the square
		particles.push_back(Particle((Vec::rand() * 2.0f - Vec(1)) * radius + center, c, velocity, ptype, mass));
}

void add_object_circle(Vec center, real radius, int c, int num = 500, Vec velocity = Vec(0.0_f), int ptype = 2, real mass = particle_mass)
//input: circle center & radius, color, number of particles, initial velocity
{
	int i = 0;
	while (i < num)
	{
		auto pos = (Vec::rand() * 2.0_f - Vec(1)) * radius;
		if (pos.x * pos.x + pos.y * pos.y < radius * radius)
		{
			particles.push_back(Particle(pos + center, c, velocity, ptype, mass));
			i++;
		}
	}
}

void add_object_rectangle(Vec v1, Vec v2, int c, int num = 500, Vec velocity = Vec(0.0_f), int ptype = 0, real mass = particle_mass)
{
	Vec box_min(min(v1.x, v2.x), min(v1.y, v2.y)), box_max(max(v1.x, v2.x), max(v1.y, v2.y));
	int i = 0;
	while (i < num)
	{
		auto pos = Vec::rand();
		if (pos.x > box_min.x && pos.y > box_min.y && pos.x < box_max.x && pos.y < box_max.y)
		{
			particles.push_back(Particle(pos, c, velocity, ptype, mass));
			i++;
		}
	}
}
void add_jet(Vec v1, Vec v2, int c, int num, Vec velocity = Vec(7.0, 0.0), int ptype = 0)
{
	add_object_rectangle(v1, v2, c, num, velocity, ptype);
	//add_object_rectangle(Vec(0.5, 0.5), Vec(0.51, 0.51), 0x87CEFA, 10, Vec(0.0, -10.0));
}

void add_jet()
{
	add_object_rectangle(Vec(0.05, 0.5), Vec(0.06, 0.51), 0x87CEFA, 10, Vec(7.0, 0.0));
}

//imgui
static void glfw_error_callback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

bool LoadTextureFromFile(const char* filename, GLuint* out_texture, int* out_width, int* out_height)
{
	// Load from file
	int image_width = 0;
	int image_height = 0;
	unsigned char* image_data = stbi_load(filename, &image_width, &image_height, NULL, 4);
	if (image_data == NULL)
		return false;

	// Create a OpenGL texture identifier
	GLuint image_texture;
	glGenTextures(1, &image_texture);
	glBindTexture(GL_TEXTURE_2D, image_texture);

	// Setup filtering parameters for display
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // This is required on WebGL for non power-of-two textures
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); // Same

	// Upload pixels into texture
#if defined(GL_UNPACK_ROW_LENGTH) && !defined(__EMSCRIPTEN__)
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
#endif
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_data);
	stbi_image_free(image_data);

	*out_texture = image_texture;
	*out_width = image_width;
	*out_height = image_height;

	return true;
}


int main()
{

	//Setup window
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
	GLFWwindow* window = glfwCreateWindow(1500, 800, "My Jellyball", NULL, NULL);
	if (window == NULL)
		return 1;
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Load Fonts
	ImFont* font1 = io.Fonts->AddFontDefault();
	ImFont* font2 = io.Fonts->AddFontFromFileTTF("../fonts/Roboto-Medium.ttf", 22.00f);
	ImFont* font3 = io.Fonts->AddFontFromFileTTF("../fonts/Cousine-Regular.ttf", 22.0f);
	ImFont* font4 = io.Fonts->AddFontFromFileTTF("../fonts/DroidSans.ttf", 22.0f);
	ImFont* font5 = io.Fonts->AddFontFromFileTTF("../fonts/ProggyTiny.ttf", 23.0f);
	ImFont* font6 = io.Fonts->AddFontFromFileTTF("../fonts/ProggyTiny.ttf", 15.0f);

	// Our state
	bool show_demo_window = true;
	bool show_another_window = false;
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

	add_object_circle(Vec(0.20, 0.3), radius, 0xFFFAFA, num1, Vec(0.0, 0.0), 1, 1);    //这是一个排球
	//add_object_rectangle(Vec(0.12, 0.1), Vec(0.15, 0.23), 0xFFFAFA, num2, Vec(60.0, 20.0), 1, 100.0);  //模拟手击球的过程

	//Load images
	int w, h;
	int width_image1 = 0, width_image2 = 0, width_image3 = 0;
	int height_image1 = 0, height_image2 = 0, height_image3 = 0;
	GLuint image1 = 0, image2 = 0, image3 = 0, image4 = 0, image5 = 0, image6 = 0;
	LoadTextureFromFile("../image/panel3.png", &image1, &width_image1, &height_image1);
	LoadTextureFromFile("../image/player1.png", &image2, &width_image2, &height_image2);
	LoadTextureFromFile("../image/player2.png", &image3, &width_image3, &height_image3);
	LoadTextureFromFile("../image/net.png", &image6, &w, &h);
	LoadTextureFromFile("../image/computer1.png", &image4, &w, &h);
	LoadTextureFromFile("../image/computer2.png", &image5, &w, &h);


	int i = -1;
	// Main loop
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		// Start the Dear ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();         //全都重画？如果想有些东西不用重画该怎么做？


		// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
		{
			ImGui::PushFont(font6);
			ImGui::Begin("Welcome to My JellyBall");                
			ImGui::Text("Welcome to My JellyBall! ");
			ImGui::Text("It's a game similar to volleyball. You can use");
			ImGui::Text("A/D or direction keys to controll your little man,");
			ImGui::Text("and click the panel below to hit the ball with ");
			ImGui::Text("the chosen direction and force. ");
			ImGui::Text("Have a good time! ");



			//ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color
			ImGui::PushFont(font5);
			if (ImGui::Button("(Re)Start!", ImVec2(400, 80)))
			{
				particles.clear();
				add_object_circle(Vec(0.20, 0.3), radius, 0xFFFAFA, num1, Vec(0.0, 0.0), 1, 1);    //这是一个排球
				start_game = true;
			}
			ImGui::PopFont();
			if (start_game)
			{
				i++;
				//本来打算把这一段发到advance里面的，但glfw的键盘响应是按窗口的，还是放在这儿吧。
				move_direction_computer = 0;
				move_direction_player = 0;
				if ((glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS ||
					glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) && location_player > 0.0)
					move_direction_player = -1;
				if ((glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS ||
					glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) && location_player < 0.5 - 40.0 / 450)
					move_direction_player = 1;
				location_player += move_direction_player * speed * dt;


				lcx = 50 + location_computer * 900;
				lcy = 550 - 40 * jump_computer;
				double right = 0, bottom = 1;
				for (int ii = 0; ii < num1; ii++)
				{
					auto p = particles[ii];
					if (p.x[0] > right) right = p.x[0];
					if (p.x[1] < bottom) bottom = p.x[1];
				}

				if ((right - 0.04) / 0.92 * 900 + 50 - lcx < -10 && location_computer > 0.5)
				{
					move_direction_computer = -1;
				}
				if ((right - 0.04) / 0.92 * 900 + 50 - lcx > 20 && location_computer < 1 - 40.0 / 450)
				{
					move_direction_computer = 1;
				}

				location_computer += move_direction_computer * speed * dt;

				if ((right - 0.04) / 0.92 * 900 + 50 - lcx <20 && (right - 0.04) / 0.92 * 900 + 50 - lcx >-10 &&
					(0.96 - bottom) / 0.92 * 900 + 50 - 300 - lcy<5 && (0.96 - bottom) / 0.92 * 900 + 50 - 300 - lcy>-40 && timer_c == 0 && timer_cc == 0)
				{
					jump_computer = 1;
					hit_computer = true;
					timer_c = 0;
					real xx = (lcx - 50) / 900 * 0.92 + 0.04 + 0.03;
					real yy = 0.96 - (lcy + 250) / 900 * 0.92;
					add_object_rectangle(Vec(right + 0.005, bottom - 0.03), Vec(right + 0.015, bottom + 0.02), 0x87CEFA, num2, Vec(-40.0, 20.0), 1, 100);

				}

				if (glfwGetMouseButton(window, 0) == GLFW_PRESS && timer_p == 0)
				{
					glfwGetCursorPos(window, &xpos, &ypos);
					jump_player = 1;

					if (/*(right - 0.04) / 0.92 * 900 + 50 - lpx < 100 && (right - 0.04) / 0.92 * 900 + 50 - lpx>0 &&*/
						(0.96 - bottom) / 0.92 * 900 + 50 - 300 - lpy<5 && (0.96 - bottom) / 0.92 * 900 + 50 - 300 - lpy>-40)
					{
						hit_player = true;    //在适当范围内则命中
						timer_p = 0;
					}

				}
				lpx = 50 + location_player * 900;
				lpy = 550 - 40 * jump_player;
				if (hit_player)
				{
					int x, y;  //获取当前子窗口左上角位置
					x = ImGui::GetCursorScreenPos()[0];
					y = ImGui::GetCursorScreenPos()[1];
					force_x = (xpos - x - 20) / 5;
					force_y = -(ypos - y - 328) / 5;

					real xx = (lpx - 50) / 900 * 0.92 + 0.04 + 0.03;
					real yy = 0.96 - (lpy + 250) / 900 * 0.92;
					add_object_rectangle(Vec(xx, yy - 0.05), Vec(xx + 0.01, yy + 0.01), 0x87CEFA, num2, Vec(force_x, force_y), 1, 100);
				}
				if (timer_p == 20)
				{
					particles.erase(particles.begin() + num1, particles.end());
					jump_player = 0;
					hit_player = false;
					timer_p = 0;
				}
				if (timer_c == 20)
				{
					particles.erase(particles.begin() + num1, particles.end());
					jump_computer = 0;
					hit_computer = false;
					timer_c = 0;
				}
				if (timer_cc == 50) timer_cc = 0;

				if (jump_player == 1)  timer_p++;
				if (jump_computer == 1) timer_c++;
				if (timer_c > 0 || timer_cc > 0) timer_cc++;
				advance(dt);
			}


			ImGui::Image((void*)(intptr_t)image1, ImVec2(width_image1, height_image1));
			GLuint image = image2;
			if (jump_player == 1)
				image = image3;
			ImGui::GetForegroundDrawList()->AddImage((void*)(intptr_t)image, ImVec2(lpx, lpy), ImVec2(lpx + 40, lpy + 100));
			ImGui::GetForegroundDrawList()->AddImage((void*)(intptr_t)image6, ImVec2(470, 520), ImVec2(530, 650));
			GLuint image_c = image4;
			if (jump_computer == 1)
				image_c = image5;
			ImGui::GetForegroundDrawList()->AddImage((void*)(intptr_t)image_c, ImVec2(lcx, lcy), ImVec2(lcx + 40, lcy + 100));

			ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
			ImGui::End();
		}

		ImGui::GetBackgroundDrawList()->AddRect(ImVec2(50, 50), ImVec2(950, 650), ImColor(0, 0, 0), 0.0f, 0, 3.0f);
		if (i % int(frame_dt / dt) == 0)
		{

			for (int i = 0; i < num1; i++)
			{
				auto p = particles[i];
				if ((1 - p.x[1] - 0.04) * 900 / 0.92 - 300 > 0)
					ImGui::GetForegroundDrawList()->AddCircleFilled(ImVec2((p.x[0] - 0.04) / 0.92 * 900 + 50, (1 - p.x[1] - 0.04) / 0.92 * 900 + 50 - 300), 2.0f, ImColor(223, 166, 10));
			}
			/*for (int i = 0; i < num2; i++)
			{
				auto p = particles[i + num1];
				if ((1 - p.x[1] - 0.04) * 900 / 0.92 - 300 > 0)
					ImGui::GetForegroundDrawList()->AddCircleFilled(ImVec2((p.x[0] - 0.04) / 0.92 * 900 + 50, (1 - p.x[1] - 0.04) / 0.92 * 900 + 50 - 300), 2.0f, ImColor(0, 255, 255));
			}*/
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

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}


