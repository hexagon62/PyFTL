#include "Input.hpp"
#include "../Utility/Memory.hpp"

#include <array>

extern HWND g_hGameWindow;
extern WNDPROC g_hGameWindowProc;

class Input::Impl
{
public:
	static constexpr int INPUT_CAP = 1;

	Impl()
	{
		auto&& mrs = Reader::getRawState(MutableRawState{});
	}

	~Impl() = default;

	void iterate()
	{
		int inputsMade = 0;

		while (inputsMade < INPUT_CAP && this->inputs())
		{
			if (this->queue.top().time > Clock::now()) break;

			auto&& cmd = this->queue.top();

			switch (cmd.type)
			{
			case Command::Type::Mouse:
				this->mouseInput(cmd.mouse);
				break;
			}

			this->queue.pop();
			inputsMade++;
		}
	}

	bool inputs() const
	{
		return !this->queue.empty();
	}

	void unhookAll()
	{
		this->shiftStateHook.unhook();
		this->ctrlStateHook.unhook();
	}

private:
	friend class Input;

	static constexpr size_t SHIFT_STATE_ADDR = 0x00178BE0;
	static constexpr size_t CTRL_STATE_ADDR = 0x00178C20;

	// Hookable "function" that simply returns 1
	static constexpr std::array<BYTE, 10> RET_1{

		0x55, // push ebp
		0x89, 0xE5, // mov ebp,esp
		0xB8, 0x01, 0x00, 0x00, 0x00, // mov eax,1
		0xC9, // leave
		0xC3  // ret
	};

	// We hook into the functions that grab if these keys are down
	// and replace them with the above if we want to force them down
	mem::Hook shiftStateHook, ctrlStateHook;

	struct MouseCommand
	{
		Point<int> position{ -1, -1 };
		MouseButton button;
		MouseWillBe state = MouseWillBe::Unchanged;
		bool shift = false, ctrl = false;
	};

	struct Command
	{
		enum class Type
		{
			Mouse, Key
		};

		Type type;
		TimePoint time;

		union
		{
			MouseCommand mouse;
		};
	};

	struct CommandCompare
	{
		bool operator()(const Command& a, const Command& b) const
		{
			return a.time < b.time;
		}
	};

	// Forces shift to be held, regardless of player input
	void setShiftHeld(bool held)
	{
		if (held)
		{
			if (!this->shiftStateHook.hooked())
			{
				void* addr = Reader::getMemory(SHIFT_STATE_ADDR, {});
				this->shiftStateHook.hook((BYTE*)addr, RET_1.data(), RET_1.size(), true);
			}
		}
		else
		{
			this->shiftStateHook.unhook();
		}
	}

	// Forces ctrl to be held, regardless of player input
	void setCtrlHeld(bool held)
	{
		if (held)
		{
			if (!this->ctrlStateHook.hooked())
			{
				void* addr = Reader::getMemory(CTRL_STATE_ADDR, {});
				this->ctrlStateHook.hook((BYTE*)addr, RET_1.data(), RET_1.size(), true);
			}
		}
		else
		{
			this->ctrlStateHook.unhook();
		}
	}

	void mouseInput(const MouseCommand& mouse)
	{
		auto&& mrs = Reader::getRawState({});
		Point<int> pos = mouse.position;

		// Position is invalid (intentionally or not), don't move
		if (this->offScreen(pos))
		{
			auto&& curr = mrs.app->gui->crewControl.currentMouse;
			pos = { curr.x, curr.y };
		}

		mrs.app->OnMouseMove(pos.x, pos.y, 0, 0, false, false, false);

		if (mouse.state == MouseWillBe::Unchanged || mouse.button == MouseButton::None)
		{
			return; // not clicking
		}

		this->setShiftHeld(mouse.shift);
		this->setCtrlHeld(mouse.ctrl);

		if (mouse.state == MouseWillBe::Down)
		{
			switch (mouse.button)
			{
			case MouseButton::Left: mrs.app->OnLButtonDown(pos.x, pos.y); break;
			case MouseButton::Middle: mrs.app->OnMButtonDown(pos.x, pos.y); break;
			case MouseButton::Right: mrs.app->OnRButtonDown(pos.x, pos.y); break;
			}
		}

		if (mouse.state == MouseWillBe::Up)
		{
			switch (mouse.button)
			{
			case MouseButton::Left: mrs.app->OnLButtonUp(pos.x, pos.y); break;
			case MouseButton::Middle: mrs.app->OnMButtonUp(pos.x, pos.y); break;
			case MouseButton::Right: mrs.app->OnRButtonUp(pos.x, pos.y); break;
			}
		}
	}

	static bool offScreen(const Point<int>& p)
	{
		return
			p.x < 0 || p.x >= Input::GAME_WIDTH ||
			p.y < 0 || p.y >= Input::GAME_HEIGHT;
	}

	static void postMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		PostMessage(g_hGameWindow, uMsg | WM_FTLAI, wParam, lParam);
	}

	using Queue = std::priority_queue<Command, std::vector<Command>, CommandCompare>;
	Queue queue;
};

Input::Impl Input::impl;
bool Input::good = false;
bool Input::humanMouse = true;
bool Input::humanKeyboard = true;

void Input::iterate()
{
	impl.iterate();
}

bool Input::inputs()
{
	return impl.inputs();
}

bool Input::ready()
{
	return good;
}

void Input::setReady(bool ready)
{
	good = ready;

	if (!ready)
	{
		impl.unhookAll();
	}
}

void Input::allowHumanMouse(bool allow)
{
	humanMouse = allow;
}

void Input::allowHumanKeyboard(bool allow)
{
	humanKeyboard = allow;
}

bool Input::humanMouseAllowed()
{
	return humanMouse;
}

bool Input::humanKeyboardAllowed()
{
	return humanKeyboard;
}

void Input::mouseMove(
	const Point<int>& position,
	Duration delay)
{
	impl.queue.push(Impl::Command{
		.type = Impl::Command::Type::Mouse,
		.time = Clock::now() + delay,
		.mouse = {
			.position = position
		},
	});
}

void Input::mouseDown(
	MouseButton button,
	const Point<int>& position,
	bool shift,
	bool ctrl,
	Duration delay)
{
	impl.queue.push(Impl::Command{
		.type = Impl::Command::Type::Mouse,
		.time = Clock::now() + delay,
		.mouse = {
			.position = position,
			.button = button,
			.state = MouseWillBe::Down,
			.shift = shift,
			.ctrl = ctrl
		}
	});
}

void Input::mouseUp(
	MouseButton button,
	const Point<int>& position,
	bool shift,
	bool ctrl,
	Duration delay)
{
	impl.queue.push(Impl::Command{
		.type = Impl::Command::Type::Mouse,
		.time = Clock::now() + delay,
		.mouse = {
			.position = position,
			.button = button,
			.state = MouseWillBe::Up,
			.shift = shift,
			.ctrl = ctrl
		}
	});
}

void Input::mouseClick(
	MouseButton button,
	const Point<int>& position,
	bool shift,
	bool ctrl,
	Duration delay)
{
	mouseDown(button, position, shift, ctrl, delay);
	mouseUp(button, position, false, false, delay + std::chrono::nanoseconds(1));
}
