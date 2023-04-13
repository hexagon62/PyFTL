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
        if (!this->unrecoverable.empty())
        {
            this->unrecoverableWindow();
            return;
        }

        auto& io = ImGui::GetIO();

        io.MouseDrawCursor = this->wantInput() || !Input::humanMouseAllowed();

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

    bool unrecoverableAcknowledged() const
    {
        return this->unrecoverableAck;
    }

    void setUnrecoverable(const std::string& str)
    {
        this->unrecoverable = str;
    }

    void output(const std::string& str, TextOutputType type = TextOutputType::Default)
    {
        if (type == TextOutputType::Command || type == TextOutputType::CommandOutput)
        {
            auto extra = type == TextOutputType::Command
                ? TextOutputType::CommandExtraLines
                : TextOutputType::CommandOutput;

            for (size_t i = 0; i < str.size();)
            {
                size_t j = str.find('\n', i);

                if(i == 0) this->history.emplace_back(str.substr(i, j), currentTime(), type);
                else this->history.emplace_back(str.substr(i, j), currentTime(), extra);

                i = j + int(j < str.size());
            }
        }
        else
        {
            this->history.emplace_back(str, currentTime(), type);
        }

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
    bool unrecoverableAck = false;

    bool demoGui = false;

    bool consoleGui = false;
    bool pythonGui = false;
    bool pythonHasFocus = false;
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
        if (this->unrecoverableAck) return;

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
                "Well, looks like we couldn't start things up fully."
                "\n\n"
            );

            ImGui::TextColored(RED, unrecoverable.c_str());

            ImGui::Text(
                "\n"
                "PyFTL is still attached to the process but won't run any Python code.\n"
                "You will need to re-attach the DLL using the launcher.\n"
                "In the future you will be able to click a button to reload your Python code from here."
                "\n\n"
            );

            if (ImGui::Button("OK"))
            {
                this->unrecoverableAck = true;

                if (Input::ready())
                {
                    Input::allowHumanMouse(true);
                    Input::allowHumanKeyboard(true);
                }
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

            ImGui::BeginChild("output");

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
            this->pythonHasFocus = ImGui::IsItemFocused();

            auto code = this->editor.GetText();

            auto runCmd = [&] {
                if (code.empty()) return;

                if (!this->scope)
                {
                    this->output("No Python module to run the code on!", TextOutputType::InternalError);
                }
                else
                {
                    try
                    {
                        py::gil_scoped_acquire gil;

                        this->output(code, TextOutputType::Command);

                        if (this->editor.GetTotalLines() > 1)
                        {
                            // multiple statements
                            py::exec(code, this->scope);
                        }
                        else
                        {
                            // single statement
                            auto result = py::eval(code, this->scope);

                            if (!result.is_none())
                            {
                                auto str = py::str(result);
                                this->output(str, TextOutputType::CommandOutput);
                            }
                        }
                    }
                    catch (const std::exception& e)
                    {
                        this->output(e.what(), TextOutputType::Error);
                    }
                }
            };

            if (ImGui::BeginMenuBar())
            {
                if (code.empty()) ImGui::BeginDisabled();
                ImGui::PushItemWidth(-1.f);
                if (ImGui::Button("Run!")) runCmd();
                if (code.empty()) ImGui::EndDisabled();
                ImGui::EndMenuBar();
            }
        }
        ImGui::End();
    }
};
