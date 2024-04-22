/*
 * Copyright 2011-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#include "common.h"
#include "bgfx_utils.h"
#include "imgui/imgui.h"

namespace
{

// 定义了一个结构体 PosColorVertex，用于表示带有位置和颜色信息的顶点
struct PosColorVertex
{
    // 顶点的 x 坐标
    float m_x;
    // 顶点的 y 坐标
    float m_y;
    // 顶点的 z 坐标
    float m_z;
    // 顶点的颜色，使用 uint32_t 类型存储，采用 ABGR 格式（Alpha、Blue、Green、Red）
    uint32_t m_abgr;

    // 初始化函数，用于设置顶点布局
    static void init()
    {
        // 开始定义顶点布局
        ms_layout
            .begin()
            // 添加位置信息，使用 3 个 float 类型数据表示
            .add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
            // 添加颜色信息，使用 4 个 uint8_t 类型数据表示，采用 true 表示颜色数据会被归一化处理
            .add(bgfx::Attrib::Color0,   4, bgfx::AttribType::Uint8, true)
            // 结束定义顶点布局
            .end();
    };

    // 定义顶点布局的静态成员变量
    static bgfx::VertexLayout ms_layout;
};

bgfx::VertexLayout PosColorVertex::ms_layout;

// 定义一个静态数组 s_cubeVertices，用于存储立方体的顶点数据
static PosColorVertex s_cubeVertices[] =
{
    // 顶点1：(-1.0, 1.0, 1.0)，颜色为红色
    {-1.0f,  1.0f,  1.0f, 0xff000000 },
    // 顶点2：(1.0, 1.0, 1.0)，颜色为蓝色
    { 1.0f,  1.0f,  1.0f, 0xff0000ff },
    // 顶点3：(-1.0, -1.0, 1.0)，颜色为绿色
    {-1.0f, -1.0f,  1.0f, 0xff00ff00 },
    // 顶点4：(1.0, -1.0, 1.0)，颜色为青色
    { 1.0f, -1.0f,  1.0f, 0xff00ffff },
    // 顶点5：(-1.0, 1.0, -1.0)，颜色为黄色
    {-1.0f,  1.0f, -1.0f, 0xffff0000 },
    // 顶点6：(1.0, 1.0, -1.0)，颜色为洋红色
    { 1.0f,  1.0f, -1.0f, 0xffff00ff },
    // 顶点7：(-1.0, -1.0, -1.0)，颜色为白色
    {-1.0f, -1.0f, -1.0f, 0xffffff00 },
    // 顶点8：(1.0, -1.0, -1.0)，颜色为透明色
    { 1.0f, -1.0f, -1.0f, 0xffffffff },
};

static const uint16_t s_cubeTriList[] =
{
	0, 1, 2, // 0
	1, 3, 2,
	4, 6, 5, // 2
	5, 6, 7,
	0, 2, 4, // 4
	4, 2, 6,
	1, 5, 3, // 6
	5, 7, 3,
	0, 4, 1, // 8
	4, 5, 1,
	2, 3, 6, // 10
	6, 3, 7,
};

static const uint16_t s_cubeTriStrip[] =
{
	0, 1, 2,
	3,
	7,
	1,
	5,
	0,
	4,
	2,
	6,
	7,
	4,
	5,
};

static const uint16_t s_cubeLineList[] =
{
	0, 1,
	0, 2,
	0, 4,
	1, 3,
	1, 5,
	2, 3,
	2, 6,
	3, 7,
	4, 5,
	4, 6,
	5, 7,
	6, 7,
};

static const uint16_t s_cubeLineStrip[] =
{
	0, 2, 3, 1, 5, 7, 6, 4,
	0, 2, 6, 4, 5, 7, 3, 1,
	0,
};

static const uint16_t s_cubePoints[] =
{
	0, 1, 2, 3, 4, 5, 6, 7
};

static const char* s_ptNames[]
{
	"Triangle List",
	"Triangle Strip",
	"Lines",
	"Line Strip",
	"Points",
};

static const uint64_t s_ptState[]
{
	UINT64_C(0),
	BGFX_STATE_PT_TRISTRIP,
	BGFX_STATE_PT_LINES,
	BGFX_STATE_PT_LINESTRIP,
	BGFX_STATE_PT_POINTS,
};
BX_STATIC_ASSERT(BX_COUNTOF(s_ptState) == BX_COUNTOF(s_ptNames) );

class ExampleCubes : public entry::AppI
{
public:
	ExampleCubes(const char* _name, const char* _description, const char* _url)
		: entry::AppI(_name, _description, _url)
		, m_pt(0)
		, m_r(true)
		, m_g(true)
		, m_b(true)
		, m_a(true)
	{
	}

	void init(int32_t _argc, const char* const* _argv, uint32_t _width, uint32_t _height) override
	{
		Args args(_argc, _argv);

		m_width  = _width;
		m_height = _height;
		m_debug  = BGFX_DEBUG_NONE;
		m_reset  = BGFX_RESET_VSYNC;

		bgfx::Init init;
		init.type     = args.m_type;
		init.vendorId = args.m_pciId;
		init.platformData.nwh  = entry::getNativeWindowHandle(entry::kDefaultWindowHandle);
		init.platformData.ndt  = entry::getNativeDisplayHandle();
		init.platformData.type = entry::getNativeWindowHandleType();
		init.resolution.width  = m_width;
		init.resolution.height = m_height;
		init.resolution.reset  = m_reset;
		bgfx::init(init);

		// Enable debug text.
		bgfx::setDebug(m_debug);

		// Set view 0 clear state.
		bgfx::setViewClear(0
			, BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH
			, 0x303030ff
			, 1.0f
			, 0
			);

		// Create vertex stream declaration.
		PosColorVertex::init();

		// Create static vertex buffer.
		m_vbh = bgfx::createVertexBuffer(
			// Static data can be passed with bgfx::makeRef
			  bgfx::makeRef(s_cubeVertices, sizeof(s_cubeVertices) )
			, PosColorVertex::ms_layout
			);

		// Create static index buffer for triangle list rendering.
		m_ibh[0] = bgfx::createIndexBuffer(
			// Static data can be passed with bgfx::makeRef
			bgfx::makeRef(s_cubeTriList, sizeof(s_cubeTriList) )
			);

		// Create static index buffer for triangle strip rendering.
		m_ibh[1] = bgfx::createIndexBuffer(
			// Static data can be passed with bgfx::makeRef
			bgfx::makeRef(s_cubeTriStrip, sizeof(s_cubeTriStrip) )
			);

		// Create static index buffer for line list rendering.
		m_ibh[2] = bgfx::createIndexBuffer(
			// Static data can be passed with bgfx::makeRef
			bgfx::makeRef(s_cubeLineList, sizeof(s_cubeLineList) )
			);

		// Create static index buffer for line strip rendering.
		m_ibh[3] = bgfx::createIndexBuffer(
			// Static data can be passed with bgfx::makeRef
			bgfx::makeRef(s_cubeLineStrip, sizeof(s_cubeLineStrip) )
			);

		// Create static index buffer for point list rendering.
		m_ibh[4] = bgfx::createIndexBuffer(
			// Static data can be passed with bgfx::makeRef
			bgfx::makeRef(s_cubePoints, sizeof(s_cubePoints) )
			);

		// Create program from shaders.
		m_program = loadProgram("vs_cubes", "fs_cubes");

		m_timeOffset = bx::getHPCounter();

		imguiCreate();
	}

	virtual int shutdown() override
	{
		imguiDestroy();

		// Cleanup.
		for (uint32_t ii = 0; ii < BX_COUNTOF(m_ibh); ++ii)
		{
			bgfx::destroy(m_ibh[ii]);
		}

		bgfx::destroy(m_vbh);
		bgfx::destroy(m_program);

		// Shutdown bgfx.
		bgfx::shutdown();

		return 0;
	}

	bool update() override
	{
		if (!entry::processEvents(m_width, m_height, m_debug, m_reset, &m_mouseState) )
		{
			imguiBeginFrame(m_mouseState.m_mx
				,  m_mouseState.m_my
				, (m_mouseState.m_buttons[entry::MouseButton::Left  ] ? IMGUI_MBUT_LEFT   : 0)
				| (m_mouseState.m_buttons[entry::MouseButton::Right ] ? IMGUI_MBUT_RIGHT  : 0)
				| (m_mouseState.m_buttons[entry::MouseButton::Middle] ? IMGUI_MBUT_MIDDLE : 0)
				,  m_mouseState.m_mz
				, uint16_t(m_width)
				, uint16_t(m_height)
				);

			showExampleDialog(this);

			ImGui::SetNextWindowPos(
				  ImVec2(m_width - m_width / 5.0f - 10.0f, 10.0f)
				, ImGuiCond_FirstUseEver
				);
			ImGui::SetNextWindowSize(
				  ImVec2(m_width / 5.0f, m_height / 3.5f)
				, ImGuiCond_FirstUseEver
				);
			ImGui::Begin("Settings"
				, NULL
				, 0
				);

			ImGui::Checkbox("Write R", &m_r);
			ImGui::Checkbox("Write G", &m_g);
			ImGui::Checkbox("Write B", &m_b);
			ImGui::Checkbox("Write A", &m_a);

			ImGui::Text("Primitive topology:");
			ImGui::Combo("##topology", (int*)&m_pt, s_ptNames, BX_COUNTOF(s_ptNames) );

			ImGui::End();

			imguiEndFrame();

			float time = (float)( (bx::getHPCounter()-m_timeOffset)/double(bx::getHPFrequency() ) );

			const bx::Vec3 at  = { 0.0f, 0.0f,   0.0f };
			const bx::Vec3 eye = { 0.0f, 0.0f, -35.0f };

			// Set view and projection matrix for view 0.
			{
				float view[16];
				bx::mtxLookAt(view, eye, at);

				float proj[16];
				bx::mtxProj(proj, 60.0f, float(m_width)/float(m_height), 0.1f, 100.0f, bgfx::getCaps()->homogeneousDepth);
				bgfx::setViewTransform(0, view, proj);

				// Set view 0 default viewport.
				bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height) );
			}

			// This dummy draw call is here to make sure that view 0 is cleared
			// if no other draw calls are submitted to view 0.
			bgfx::touch(0);

			bgfx::IndexBufferHandle ibh = m_ibh[m_pt];
			uint64_t state = 0
				| (m_r ? BGFX_STATE_WRITE_R : 0)
				| (m_g ? BGFX_STATE_WRITE_G : 0)
				| (m_b ? BGFX_STATE_WRITE_B : 0)
				| (m_a ? BGFX_STATE_WRITE_A : 0)
				| BGFX_STATE_WRITE_Z
				| BGFX_STATE_DEPTH_TEST_LESS
				| BGFX_STATE_CULL_CW
				| BGFX_STATE_MSAA
				| s_ptState[m_pt]
				;

			// Submit 11x11 cubes.
			for (uint32_t yy = 0; yy < 11; ++yy)
			{
				for (uint32_t xx = 0; xx < 11; ++xx)
				{
					float mtx[16];
					bx::mtxRotateXY(mtx, time + xx*0.21f, time + yy*0.37f);
					mtx[12] = -15.0f + float(xx)*3.0f;
					mtx[13] = -15.0f + float(yy)*3.0f;
					mtx[14] = 0.0f;

					// Set model matrix for rendering.
					bgfx::setTransform(mtx);

					// Set vertex and index buffer.
					bgfx::setVertexBuffer(0, m_vbh);
					bgfx::setIndexBuffer(ibh);

					// Set render states.
					bgfx::setState(state);

					// Submit primitive for rendering to view 0.
					bgfx::submit(0, m_program);
				}
			}

			// Advance to next frame. Rendering thread will be kicked to
			// process submitted rendering primitives.
			bgfx::frame();

			return true;
		}

		return false;
	}

	entry::MouseState m_mouseState;

	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_debug;
	uint32_t m_reset;
	bgfx::VertexBufferHandle m_vbh;
	bgfx::IndexBufferHandle m_ibh[BX_COUNTOF(s_ptState)];
	bgfx::ProgramHandle m_program;
	int64_t m_timeOffset;
	int32_t m_pt;

	bool m_r;
	bool m_g;
	bool m_b;
	bool m_a;
};

} // namespace

ENTRY_IMPLEMENT_MAIN(
	  ExampleCubes
	, "01-cubes"
	, "Rendering simple static mesh."
	, "https://bkaradzic.github.io/bgfx/examples.html#cubes"
	);
