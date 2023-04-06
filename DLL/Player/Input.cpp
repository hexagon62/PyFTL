#include "Input.hpp"
#include "../Utility/WindowsButWithoutAsMuchCancer.hpp"

extern HWND g_hGameWindow;
extern WNDPROC g_hGameWindowProc;

class Input::Impl
{
public:
	static constexpr int INPUT_CAP = 1;

	Impl() = default;
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
			case Command::Type::MouseMove:
				this->mouseMove(cmd.mouseMove.pos);
				break;
			case Command::Type::MouseDown:
				[[fallthrough]];
			case Command::Type::MouseUp:
				this->mouseMove(cmd.mouseClick.pos);
				this->mouseInput(cmd);
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

private:
	friend class Input;

	struct MouseMove
	{
		Point<int> pos;
	};

	struct MouseClick
	{
		Point<int> pos;
		MouseButton button;
		bool shift = false, ctrl = false;
	};

	struct Command
	{
		enum class Type
		{
			MouseMove, MouseDown, MouseUp,
			KeyDown, KeyUp
		};

		Type type;
		TimePoint time;

		union
		{
			MouseMove mouseMove;
			MouseClick mouseClick;
		};
	};

	struct CommandCompare
	{
		bool operator()(const Command& a, const Command& b) const
		{
			return a.time < b.time;
		}
	};

	static void mouseMove(const Point<int>& pos)
	{
		LPARAM l = (pos.x & 0xffff) | ((pos.y & 0xffff) << 16);
		postMessage(WM_MOUSEMOVE, 0, l);
	}

	static void mouseInput(const Command& cmd)
	{
		auto&& click = cmd.mouseClick;
		bool shift = cmd.mouseClick.shift;
		bool ctrl = cmd.mouseClick.ctrl;

		WPARAM w = (shift ? MK_SHIFT : 0) | (ctrl ? MK_CONTROL : 0);
		UINT uMsg = 0;

		if (cmd.type == Command::Type::MouseDown)
		{
			switch (cmd.mouseClick.button)
			{
			case MouseButton::Left: uMsg = WM_LBUTTONDOWN; break;
			case MouseButton::Middle: uMsg = WM_MBUTTONDOWN; break;
			case MouseButton::Right: uMsg = WM_RBUTTONDOWN; break;
			}
		}
		else if (cmd.type == Command::Type::MouseUp)
		{
			switch (cmd.mouseClick.button)
			{
			case MouseButton::Left: uMsg = WM_LBUTTONUP; break;
			case MouseButton::Middle: uMsg = WM_MBUTTONUP; break;
			case MouseButton::Right: uMsg = WM_RBUTTONUP; break;
			}
		}

		if (uMsg) postMessage(uMsg, w, 0);
	}

	static void postMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		PostMessage(g_hGameWindow, uMsg | WM_FTLAI, wParam, lParam);
	}

	static Point<int> gameToScreen(const Point<int>& p)
	{
		RECT rect;
		GetClientRect(g_hGameWindow, &rect);

		Point<float> scale{
			float(rect.right) / float(Input::GAME_WIDTH),
			float(rect.bottom) / float(Input::GAME_HEIGHT)
		};

		scale.x *= float(p.x);
		scale.y *= float(p.y);

		Point<int> result = { int(rect.left) + int(scale.x), int(rect.top) + int(scale.y) };

		if (result.x < rect.left) result.x = rect.left;
		if (result.y < rect.top) result.y = rect.top;
		if (result.x >= rect.right) result.x = rect.right - 1;
		if (result.y >= rect.bottom) result.y = rect.bottom - 1;

		return { int(result.x), int(result.y) };
	}

	using Queue = std::priority_queue<Command, std::vector<Command>, CommandCompare>;
	Queue queue;
};

Input::Impl Input::impl;
bool Input::good = false;
bool Input::humanMouse = false;
bool Input::humanKeyboard = false;

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
	const Point<int>& pos,
	Duration delay)
{
	impl.queue.push(Impl::Command{
		.type = Impl::Command::Type::MouseMove,
		.time = Clock::now() + delay,
		.mouseMove = {
			.pos = Impl::gameToScreen(pos)
		},
	});
}

void Input::mouseMove(
	int x, int y,
	Duration delay)
{
	Input::mouseMove({ x, y }, delay);
}

void Input::mouseDown(
	const Point<int>& pos,
	MouseButton button,
	bool shift,
	bool ctrl,
	Duration delay)
{
	impl.queue.push(Impl::Command{
		.type = Impl::Command::Type::MouseDown,
		.time = Clock::now() + delay,
		.mouseClick = {
			.pos = Impl::gameToScreen(pos),
			.button = button,
			.shift = shift,
			.ctrl = ctrl
		}
	});
}

void Input::mouseDown(
	int x, int y,
	MouseButton button,
	bool shift,
	bool ctrl,
	Duration delay)
{
	Input::mouseDown({ x, y }, button, shift, ctrl, delay);
}

void Input::mouseUp(
	const Point<int>& pos,
	MouseButton button,
	bool shift,
	bool ctrl,
	Duration delay)
{
	impl.queue.push(Impl::Command{
		.type = Impl::Command::Type::MouseUp,
		.time = Clock::now() + delay,
		.mouseClick = {
			.pos = Impl::gameToScreen(pos),
			.button = button,
			.shift = shift,
			.ctrl = ctrl
		}
	});
}

void Input::mouseUp(
	int x, int y,
	MouseButton button,
	bool shift,
	bool ctrl,
	Duration delay)
{
	Input::mouseUp({ x, y }, button, shift, ctrl, delay);
}

void Input::mouseClick(
	const Point<int>& pos,
	MouseButton button,
	bool shift,
	bool ctrl,
	Duration delay)
{
	impl.queue.push(Impl::Command{
		.type = Impl::Command::Type::MouseDown,
		.time = Clock::now() + delay,
		.mouseClick = {
			.pos = Impl::gameToScreen(pos),
			.button = button,
			.shift = shift,
			.ctrl = ctrl
		}
	});

	impl.queue.push(Impl::Command{
		.type = Impl::Command::Type::MouseUp,
		.time = Clock::now() + delay + Duration(1),
		.mouseClick = {
			.pos = Impl::gameToScreen(pos),
			.button = button,
			.shift = shift,
			.ctrl = ctrl
		}
	});
}

void Input::mouseClick(
	int x, int y,
	MouseButton button,
	bool shift,
	bool ctrl,
	Duration delay)
{
	Input::mouseClick({ x, y }, button, shift, ctrl, delay);
}
