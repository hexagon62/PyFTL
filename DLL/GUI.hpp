#pragma once

#include "Input.hpp"
#include "Python/Bind.hpp"

#define GLEW_STATIC
#include <gl/glew.h>

#include <imgui_impl_opengl3.h>
#include <imgui_impl_win32.h>
#include <imgui_stl.h>
#include "TextEditor.h"

#include <deque>
#include <iostream>
#include <iomanip>
#include <sstream>

extern bool g_imguiMainMenu;

static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

enum class TextOutputType
{
    Default, Error,
    Internal, InternalError,
    Command, CommandOutput, CommandExtraLines
};

class GUIHelper
{
public:
    GUIHelper()
    {
        // Many thanks to https://github.com/BalazsJako/ImGuiColorTextEdit/issues/113
        // for this code to define the python language in ImGuiColorTextEdit

        static const char* const keywords[] = {
            "False", "None", "True", "and", "as", "assert", "break", "class",
            "continue", "def", "del", "elif", "else", "except", "finally",
            "for", "from", "global", "if", "import", "in", "is", "lambda",
            "nonlocal", "not", "or", "pass", "raise", "return", "try", "while",
            "with", "yield",
            "__class__", "__delattr__", "__dict__", "__dir__", "__doc__", "__eq__",
            "__format__", "__ge__", "__getattribute__", "__gt__", "__hash__",
            "__init__", "__le__", "__lt__", "__module__", "__ne__", "__new__",
            "__reduce__", "__reduce_ex__", "__repr__", "__setattr__", "__sizeof__",
            "__str__", "__subclasshook__", "__weakref__"
        };

        for (auto& k : keywords) langDef.mKeywords.insert(k);

        static const char* const identifiers[]{
            "abs", "delattr", "hash", "memoryview", "set", "all", "dict", "help",
            "min", "setattr", "any", "dir", "hex", "next", "slice", "ascii", "divmod",
            "id", "object", "sorted", "bin", "enumerate", "input", "oct",
            "staticmethod", "bool", "eval", "int", "open", "str", "breakpoint",
            "exec", "isinstance", "ord", "sum", "bytearray", "filter", "issubclass",
            "pow", "super", "bytes", "float", "iter", "print", "tuple", "callable",
            "format", "len", "property", "type", "chr", "frozenset", "list", "range",
            "vars", "classmethod", "getattr", "locals", "repr", "zip", "compile",
            "globals", "map", "reversed", "__import__", "complex", "hasattr", "max",
            "round"
        };

        for (auto& k : identifiers) {
            TextEditor::Identifier id;
            id.mDeclaration = "Built-in function";
            langDef.mIdentifiers.insert(std::make_pair(std::string(k), id));
        }

        this->langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, TextEditor::PaletteIndex>("f?[\\\"'](\\\\.|[^\\\"'])*[\\\"']", TextEditor::PaletteIndex::String));
        this->langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, TextEditor::PaletteIndex>("[+-]?([0-9]+([.][0-9]*)?|[.][0-9]+)([eE][+-]?[0-9]+)?[fF]?", TextEditor::PaletteIndex::Number));
        this->langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, TextEditor::PaletteIndex>("[+-]?[0-9]+[Uu]?[lL]?[lL]?", TextEditor::PaletteIndex::Number));
        this->langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, TextEditor::PaletteIndex>("0[0-7]+[Uu]?[lL]?[lL]?", TextEditor::PaletteIndex::Number));
        this->langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, TextEditor::PaletteIndex>("0[xX][0-9a-fA-F]+[uU]?[lL]?[lL]?", TextEditor::PaletteIndex::Number));
        this->langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, TextEditor::PaletteIndex>("[a-zA-Z_][a-zA-Z0-9_]*", TextEditor::PaletteIndex::Identifier));
        this->langDef.mTokenRegexStrings.push_back(std::make_pair<std::string, TextEditor::PaletteIndex>("[\\[\\]\\{\\}\\!\\%\\^\\&\\*\\(\\)\\-\\+\\=\\~\\|\\<\\>\\?\\/\\;\\,\\.]", TextEditor::PaletteIndex::Punctuation));

        this->langDef.mCommentStart = R"(""")";
        this->langDef.mCommentEnd = R"(""")";
        this->langDef.mSingleLineComment = "#";

        this->langDef.mCaseSensitive = true;
        this->langDef.mAutoIndentation = true;

        this->langDef.mName = "Python";

        this->editor.SetLanguageDefinition(this->langDef);
    }

    void render()
    {
        auto& io = ImGui::GetIO();
        io.MouseDrawCursor = this->wantInput() || !Input::humanMouseAllowed();

        if (this->unrecoverableOpen())
        {
            this->unrecoverableWindow();
            return;
        }

        // Use alt+` to activate/deactivate the main menu
        if (ImGui::IsKeyDown(ImGuiKey_ModAlt) && ImGui::IsKeyPressed(ImGuiKey_GraveAccent, false))
        {
            g_imguiMainMenu = !g_imguiMainMenu;
        }

        if (g_imguiMainMenu)
        {
            constexpr int MAIN_MENU_FLAGS =
                ImGuiWindowFlags_NoDecoration |
                ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove;

            constexpr ImVec2 MAIN_MENU_POS = {
                float(Input::GAME_WIDTH - 1), 0.f
            };

            ImGui::SetNextWindowPos(MAIN_MENU_POS, 0, { 1.f, 0.f });

            ImGui::Begin("##mainMenu", nullptr, MAIN_MENU_FLAGS);
            {
                if (ImGui::BeginMenu("<<< PyFTL"))
                {
                    ImGui::Checkbox("Console", &this->consoleGui);
                    ImGui::Checkbox("Run Python Code", &this->pythonGui);
                    ImGui::Checkbox("imgui Demo", &this->demoGui);

                    ImGui::EndMenu();
                }
            }
            ImGui::End();
        }

        this->demoWindow();
        this->console();
        this->python();
    }

    void setScope(const py::object& scope)
    {
        this->scope = scope;
    }

    void clearScope()
    {
        this->scope.release();
    }

    const py::object& getScope() const
    {
        return this->scope;
    }

    bool requestedPythonCode() const
    {
        return this->wantsRunPython;
    }

    void runPythonCode()
    {
        this->wantsRunPython = false;
        auto code = this->editor.GetText();

        if (code.empty()) return;

        if (!this->scope)
        {
            this->output("No Python module to run the code on!", TextOutputType::InternalError);
        }
        else
        {
            try
            {
                auto lines = this->editor.GetTextLines();
                this->output(lines[0], TextOutputType::Command);

                for (size_t i = 1; i < lines.size(); i++)
                {
                    this->output(lines[i], TextOutputType::CommandExtraLines);
                }

                try
                {
                    // try evaling first
                    auto result = py::eval(code, this->scope);

                    if (!result.is_none())
                    {
                        auto str = py::str(result);
                        this->output(str, TextOutputType::CommandOutput);
                    }
                }
                catch (const py::error_already_set& e)
                {
                    if (e.matches(PyExc_SyntaxError))
                    {
                        // try execing instead, don't catch next syntax error if user is actually wrong though
                        py::exec(code, this->scope);
                    }
                    else
                    {
                        throw e;
                    }
                }
            }
            catch (const std::exception& e)
            {
                this->output(e.what(), TextOutputType::Error);
            }
        }
    }

    enum class UnrecoverableResponse
    {
        None, OK, Reload
    };

    bool unrecoverableOpen() const
    {
        return
            !this->unrecoverable.empty() &&
            this->unrecoverableResponse == UnrecoverableResponse::None;
    }

    UnrecoverableResponse getUnrecoverableResponse() const
    {
        return this->unrecoverableResponse;
    }

    void setUnrecoverable(const std::string& str, bool offerReload = false)
    {
        this->unrecoverableResponse = UnrecoverableResponse::None;
        this->unrecoverable = str;
        this->offerReload = offerReload;
        while (this->unrecoverableOpen());
    }

    void output(const std::string& str, TextOutputType type = TextOutputType::Default)
    {
        this->history.emplace_back(str, currentTime(), type);

        while (this->history.size() > HISTORY_LIMIT)
        {
            this->history.pop_front();
        }
    }

    bool wantInput() const
    {
        auto& io = ImGui::GetIO();
        bool imguiWants = io.WantCaptureMouse || io.WantCaptureKeyboard || io.WantTextInput;
        return imguiWants || this->pythonHasFocus;
    }

private:
    static constexpr ImVec4 RED{ 1.f, 0.f, 0.f, 1.f };
    static constexpr ImVec4 ORANGE{ 1.f, 0.5f, 0.f, 1.f };
    static constexpr ImVec4 GREEN{ 0.f, 1.f, 0.f, 1.f };
    static constexpr ImVec4 BLUE{ 0.f, 0.f, 1.f, 1.f };
    static constexpr ImVec4 CYAN{ 0.f, 1.f, 1.f, 1.f };
    static constexpr ImVec4 MAGENTA{ 1.f, 0.f, 1.f, 1.f };
    static constexpr ImVec4 GRAY{ 0.5f, 0.5f, 0.5f, 1.f };
    static constexpr ImVec4 WHITE{ 1.f, 1.f, 1.f, 1.f };
    static constexpr size_t HISTORY_LIMIT = 100;

    struct HistoryEntry
    {
        std::string text, time;
        TextOutputType type = TextOutputType::Default;
    };

    py::object scope;
    std::deque<HistoryEntry> history;
    std::string unrecoverable;
    UnrecoverableResponse unrecoverableResponse = UnrecoverableResponse::None;
    bool offerReload = false;

    bool demoGui = false;

    bool consoleGui = false;
    bool pythonGui = false;
    bool pythonHasFocus = false;
    bool wantsRunPython = false;
    TextEditor editor;
    TextEditor::LanguageDefinition langDef;

    static std::string currentTime()
    {
        std::ostringstream ss;

        ss
            << "T+"
            << std::setprecision(3) << std::fixed
            << std::setfill('0') << std::setw(9)
            << Reader::now();

        return ss.str();
    }

    static void Marker(
        const char* text, const char* desc,
        ImVec4 color, ImVec4 tooltipColor = WHITE)
    {
        ImGui::PushStyleColor(ImGuiCol_TextDisabled, color);
        ImGui::TextDisabled(text);
        ImGui::PopStyleColor();
        ImGui::SameLine();

        if (ImGui::IsItemHovered())
        {
            ImGui::BeginTooltip();
            ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
            ImGui::PushStyleColor(ImGuiCol_Text, tooltipColor);
            ImGui::TextUnformatted(desc);
            ImGui::PopStyleColor();
            ImGui::PopTextWrapPos();
            ImGui::EndTooltip();
        }
    }

    static void PyFTLMarker(ImVec4 color)
    {
        Marker("[PyFTL]", "Internally generated by PyFTL", color);
    }

    static void CommandMarker(ImVec4 color)
    {
        Marker(">>>", "Command entered by user", color);
    }

    static void CommandExtraLineMarker(ImVec4 color)
    {
        Marker("...", "Command entered by user", color);
    }

    static void Output(const std::string& str, ImVec4 color)
    {
        Marker(str.c_str(), "Output from a user command", color);
        ImGui::NewLine();
    }

    static void Time(const std::string& str, ImVec4 color)
    {
        Marker(str.c_str(), "Timestamp of output", color);
    }

    void unrecoverableWindow()
    {
        if (Input::ready())
        {
            Input::allowHumanMouse(false);
            Input::allowHumanKeyboard(false);
        }

        constexpr int FLAGS =
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoCollapse;

        ImGui::SetNextWindowSize({ 0.f, 0.f });
        ImGui::SetNextWindowFocus();
        ImGui::SetNextWindowPos({ 640.f, 360.f }, 0, { 0.5f, 0.5f });
        ImGui::Begin("Unrecoverable error", nullptr, FLAGS);
        {
            ImGui::Text(
                "Well, looks like a really bad error happened."
                "\n\n"
            );

            ImGui::BeginChild("error", {640.f, 180.f}, true);
            ImGui::TextColored(RED, unrecoverable.c_str());
            ImGui::EndChild();

            ImGui::Text("\nNote that:");
            ImGui::BulletText("PyFTL is still attached to the process but won't run any Python code.");
            ImGui::BulletText("To restart PyFTL, you need to restart the game.");
            if(this->offerReload) ImGui::BulletText(
                "This error originated from the Python code that was running.\n"
                "You can try to reload the main module of it with the below button.\n"
                "This does not recursively reload other modules it may have imported."
            );

            ImGui::PushItemWidth(160.f);
            if (ImGui::Button("OK"))
            {
                this->unrecoverableResponse = UnrecoverableResponse::OK;

                if (Input::ready())
                {
                    Input::allowHumanMouse(true);
                    Input::allowHumanKeyboard(true);
                }
            }
            ImGui::PopItemWidth();

            if (this->offerReload)
            {
                ImGui::SameLine();
                ImGui::PushItemWidth(479.f);
                if (ImGui::Button("Reload Main Python Module"))
                {
                    this->unrecoverableResponse = UnrecoverableResponse::Reload;
                    Reader::reload();

                    if (Input::ready())
                    {
                        Input::allowHumanMouse(true);
                        Input::allowHumanKeyboard(true);
                    }
                }
                ImGui::PopItemWidth();
            }
        }
        ImGui::End();
    }

    void demoWindow()
    {
        if (!this->demoGui)
            return;

        ImGui::ShowDemoWindow(&this->demoGui);
    }

    void console()
    {
        if (!this->consoleGui)
            return;

        ImGui::Begin("Console", &this->consoleGui, ImGuiWindowFlags_MenuBar);
        {
            if (ImGui::BeginMenuBar())
            {
                ImGui::PushItemWidth(-1.f);
                if (ImGui::Button("Clear the console"))
                {
                    this->history.clear();
                }
                ImGui::EndMenuBar();
            }

            if (ImGui::BeginChild("output"))
            {
                for (auto&& [text, time, type] : history)
                {
                    ImGui::PushTextWrapPos(0.f);

                    Time(time, GRAY);

                    switch (type)
                    {
                    case TextOutputType::Error:
                        ImGui::TextColored(ORANGE, text.c_str());
                        break;
                    case TextOutputType::Internal:
                        PyFTLMarker(MAGENTA);
                        ImGui::TextColored(GREEN, text.c_str());
                        break;
                    case TextOutputType::InternalError:
                        PyFTLMarker(MAGENTA);
                        ImGui::TextColored(RED, text.c_str());
                        break;
                    case TextOutputType::Command:
                        CommandMarker(MAGENTA);
                        ImGui::TextColored(CYAN, text.c_str());
                        break;
                    case TextOutputType::CommandOutput:
                        Output(text.c_str(), CYAN);
                        break;
                    case TextOutputType::CommandExtraLines:
                        CommandExtraLineMarker(MAGENTA);
                        ImGui::TextColored(CYAN, text.c_str());
                        break;
                    default:
                        ImGui::TextColored(WHITE, text.c_str());
                    }

                    ImGui::PopTextWrapPos();
                }

                if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
                    ImGui::SetScrollHereY(1.0f);
            }

            ImGui::EndChild();
        }
        ImGui::End();
    }

    void python()
    {
        if (!this->pythonGui)
            return;

        ImGui::Begin("Python", &this->pythonGui, ImGuiWindowFlags_MenuBar);
        {
            this->editor.Render("TextEditor");
            this->pythonHasFocus = this->editor.Focused();

            auto code = this->editor.GetText();

            if (ImGui::BeginMenuBar())
            {
                if (code.empty()) ImGui::BeginDisabled();
                ImGui::PushItemWidth(-1.f);
                if (ImGui::Button("Run!")) this->wantsRunPython = true;
                if (code.empty()) ImGui::EndDisabled();
                ImGui::EndMenuBar();
            }
        }
        ImGui::End();
    }
};
