/*
 * Copyright 2011-2024 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bgfx/blob/master/LICENSE
 */

#include <bx/uint32_t.h>
#include "common.h"
#include "bgfx_utils.h"
#include "logo.h"
#include "imgui/imgui.h"

namespace
{

class ExampleHelloWorld : public entry::AppI
{
public:
	ExampleHelloWorld(const char* _name, const char* _description, const char* _url)
		: entry::AppI(_name, _description, _url)
	{
	}

	void init(int32_t _argc, const char* const* _argv, uint32_t _width, uint32_t _height) override
	{
		// 使用传入的参数创建Args对象，这个对象将被用于解析命令行参数。
		Args args(_argc, _argv);

		// 初始化成员变量，将传入的宽度和高度赋值给类成员变量。
		m_width  = _width;
		m_height = _height;
		// 设置调试标志为BGFX_DEBUG_TEXT，表示启用调试文本。
		m_debug  = BGFX_DEBUG_TEXT;
		// 设置重置标志为BGFX_RESET_VSYNC，表示在垂直同步时重置。
		m_reset  = BGFX_RESET_VSYNC;

		// 创建bgfx的初始化结构体init。
		bgfx::Init init;
		// 设置渲染API类型。
		init.type     = args.m_type;
		// 设置渲染设备的厂商ID。
		init.vendorId = args.m_pciId;
		// 设置平台数据，包括窗口句柄和显示器句柄等。
		init.platformData.nwh  = entry::getNativeWindowHandle(entry::kDefaultWindowHandle);
		init.platformData.ndt  = entry::getNativeDisplayHandle();
		init.platformData.type = entry::getNativeWindowHandleType();
		// 设置渲染分辨率。
		init.resolution.width  = m_width;
		init.resolution.height = m_height;
		// 设置渲染重置状态。
		init.resolution.reset  = m_reset;
		// 使用bgfx库的初始化函数进行初始化。
		bgfx::init(init);

		// 启用调试文本。
		bgfx::setDebug(m_debug);

		// 设置视图0的清除状态，这里清除颜色和深度缓冲。
		bgfx::setViewClear(0
			, BGFX_CLEAR_COLOR|BGFX_CLEAR_DEPTH
			, 0x303030ff // 清除颜色，使用RGBA格式，0x303030ff表示灰色。
			, 1.0f        // 清除深度，1.0表示最远处。
			, 0           // 清除模板缓冲。
			);

		// 创建imgui的相关资源，用于在渲染中集成ImGui。
		imguiCreate();
	}

	virtual int shutdown() override
	{
		imguiDestroy();

		// Shutdown bgfx.
		bgfx::shutdown();

		return 0;
	}

	bool update() override
	{
		// 处理窗口事件，包括鼠标、键盘等输入事件。
		if (!entry::processEvents(m_width, m_height, m_debug, m_reset, &m_mouseState))
		{
			// 开始ImGui的渲染帧，传入鼠标状态和窗口尺寸等信息。
			imguiBeginFrame(m_mouseState.m_mx
				, m_mouseState.m_my
				, (m_mouseState.m_buttons[entry::MouseButton::Left] ? IMGUI_MBUT_LEFT : 0)
				| (m_mouseState.m_buttons[entry::MouseButton::Right] ? IMGUI_MBUT_RIGHT : 0)
				| (m_mouseState.m_buttons[entry::MouseButton::Middle] ? IMGUI_MBUT_MIDDLE : 0)
				, m_mouseState.m_mz
				, uint16_t(m_width)
				, uint16_t(m_height)
				);

			// 在ImGui中显示示例对话框。
			showExampleDialog(this);

			// 结束ImGui的渲染帧。
			imguiEndFrame();

			// 设置视图0的默认视口大小。
			bgfx::setViewRect(0, 0, 0, uint16_t(m_width), uint16_t(m_height));

			// 如果没有其他绘制调用提交给视图0，则提交一个虚拟的绘制调用来清除视图0。
			bgfx::touch(0);

			// 清除调试文本。
			bgfx::dbgTextClear();

			// 获取bgfx统计数据。
			const bgfx::Stats* stats = bgfx::getStats();

			// 在调试文本中显示Logo图像。
			bgfx::dbgTextImage(
				bx::max<uint16_t>(uint16_t(stats->textWidth / 2), 20) - 20
				, bx::max<uint16_t>(uint16_t(stats->textHeight / 2), 6) - 6
				, 40
				, 12
				, s_logo
				, 160
			);

			// 在调试文本中使用ANSI码打印信息，改变颜色。
			bgfx::dbgTextPrintf(0, 1, 0x0f, "Color can be changed with ANSI \x1b[9;me\x1b[10;ms\x1b[11;mc\x1b[12;ma\x1b[13;mp\x1b[14;me\x1b[0m code too.");

			// 在调试文本中打印颜色示例。
			bgfx::dbgTextPrintf(80, 1, 0x0f, "\x1b[;0m    \x1b[;1m    \x1b[; 2m    \x1b[; 3m    \x1b[; 4m    \x1b[; 5m    \x1b[; 6m    \x1b[; 7m    \x1b[0m");
			bgfx::dbgTextPrintf(80, 2, 0x0f, "\x1b[;8m    \x1b[;9m    \x1b[;10m    \x1b[;11m    \x1b[;12m    \x1b[;13m    \x1b[;14m    \x1b[;15m    \x1b[0m");

			// 在调试文本中打印窗口和调试文本的尺寸信息。
			bgfx::dbgTextPrintf(0, 2, 0x0f, "Backbuffer %dW x %dH in pixels, debug text %dW x %dH in characters."
				, stats->width
				, stats->height
				, stats->textWidth
				, stats->textHeight
			);

			// 提交渲染帧，开始下一帧的渲染。
			bgfx::frame();

			return true; // 返回true表示更新成功。
		}

		return false; // 返回false表示更新失败。
	}

	entry::MouseState m_mouseState;

	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_debug;
	uint32_t m_reset;
};

} // namespace

ENTRY_IMPLEMENT_MAIN(
	  ExampleHelloWorld
	, "00-helloworld"
	, "Initialization and debug text."
	, "https://bkaradzic.github.io/bgfx/examples.html#helloworld"
	);
