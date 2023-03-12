#pragma once

#include "../GameState/Reader.hpp"
#include "../Player/Player.hpp"

#include <imgui_impl_opengl3.h>
#include <imgui_impl_win32.h>

extern bool g_overlay = true;
extern bool g_active;

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

class GUIHelper
{
public:
    const StateReader* getReader() const
    {
        return this->reader;
    }

    void setReader(const StateReader& r)
    {
        this->reader = &r;
    }

    const Player* getPlayer() const
    {
        return this->player;
    }

    void setPlayer(const Player& p)
    {
        this->player = &p;
    }

    bool inGame() const
    {
        if (!this->reader)
            return false;

        auto state = this->reader->getState();

        return state.inGame;
    }

    void render()
    {
        if (!g_overlay)
            return;

        if (ImGui::BeginMainMenuBar())
        {
            if (ImGui::BeginMenu("AI"))
            {
                ImGui::Checkbox("Active", &g_active);
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View"))
            {
                ImGui::Checkbox("Overlay", &g_overlay);
                ImGui::Checkbox("Game state", &this->gameStateGui);
                ImGui::Checkbox("AI Panel", &this->playerGui);
                ImGui::Checkbox("imgui Demo", &this->demoGui);

                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        this->gameStateWindow();
        this->playerWindow();
        this->demoWindow();
    }

private:
    const StateReader* reader = nullptr;
    const Player* player = nullptr;

    bool gameStateGui = false;
    bool playerGui = false;
    bool demoGui = false;

    void gameStateWindow()
    {
        if (!this->reader || !this->inGame() || !this->gameStateGui)
            return;

        State& s = const_cast<State&>(this->reader->getState());
        raw::State& rs = const_cast<raw::State&>(this->reader->getRawState());

        if(ImGui::Begin("Game state", &this->gameStateGui))
        {
            if (rs.app)
            {
                this->rawStateSection();
            }

            ImGui::Checkbox("In game?", &s.inGame);

            if (s.inGame)
            {
                if (ImGui::CollapsingHeader("Pause state"))
                {
                    ImGui::Checkbox("Normal pause", &s.pause.normal);
                    ImGui::Checkbox("Auto pause", &s.pause.automatic);
                    ImGui::Checkbox("Menu pause", &s.pause.menu);
                    ImGui::Checkbox("Event pause", &s.pause.event);
                    ImGui::Checkbox("Touch pause", &s.pause.touch);
                }

                if (ImGui::CollapsingHeader("Ship state"))
                {
                    ImGui::InputInt("Hull", &s.player.hull.first);
                    ImGui::InputInt("Max hull", &s.player.hull.second);
                    ImGui::InputFloat("FTL", &s.player.ftl.first);
                    ImGui::InputFloat("Max FTL", &s.player.ftl.second);
                }

            }
        }
        ImGui::End();
    }

    void rawStateSection()
    {
        raw::State& rs = const_cast<raw::State&>(this->reader->getRawState());
        
        HelpMarker("Only touch this if you know what you're doing.");

        if (ImGui::CollapsingHeader("Raw state"))
        {
            ImGui::Checkbox("shift_held", &rs.app->shift_held);

            if (rs.app->gui && ImGui::TreeNode("gui"))
            {
                auto&& gui = rs.app->gui;

                this->rawShipStatusSection(gui->shipStatus);

                ImGui::TreePop();
                ImGui::Separator();
            }

            if (rs.app->world && ImGui::TreeNode("world"))
            {
                ImGui::TreePop();
                ImGui::Separator();
            }
        }
    }

    void rawShipStatusSection(raw::ShipStatus& ship) 
    {
        if (ImGui::TreeNode("shipStatus"))
        {
            if (ImGui::TreeNode("location"))
            {
                ImGui::InputInt("x", &ship.location.x);
                ImGui::InputInt("y", &ship.location.y);

                ImGui::TreePop();
                ImGui::Separator();
            }

            ImGui::InputFloat("size", &ship.size);

            this->rawShipManagerSection("ship", ship.ship);

            ImGui::TreePop();
            ImGui::Separator();
        }
    }

    void rawShipManagerSection(const char* label, raw::ShipManager* ship)
    {
        if (ship && ImGui::TreeNode(label))
        {
            bool oxygen = ship->oxygenSystem;
            bool teleport = ship->teleportSystem;
            bool cloak = ship->cloakSystem;
            bool battery = ship->batterySystem;
            bool mind = ship->mindSystem;
            bool clone = ship->cloneSystem;
            bool hacking = ship->hackingSystem;
            bool shield = ship->shieldSystem;
            bool weapon = ship->weaponSystem;
            bool drone = ship->droneSystem;
            bool medbay = ship->medbaySystem;
            bool engine = ship->engineSystem;
            int artillery = ship->artillerySystems.size();
            ImGui::Checkbox("oxygenSystem", &oxygen);
            ImGui::Checkbox("teleportSystem", &teleport);
            ImGui::Checkbox("cloakSystem", &cloak);
            ImGui::Checkbox("batterySystem", &battery);
            ImGui::Checkbox("mindSystem", &mind);
            ImGui::Checkbox("cloneSystem", &clone);
            ImGui::Checkbox("hackingSystem", &hacking);
            ImGui::Checkbox("shieldSystem", &shield);
            ImGui::Checkbox("weaponSystem", &weapon);
            ImGui::Checkbox("droneSystem", &drone);
            ImGui::Checkbox("medbaySystem", &medbay);
            ImGui::Checkbox("engineSystem", &engine);
            ImGui::InputInt("artillerySystems", &artillery);

            int crew = ship->vCrewList.size();
            ImGui::InputInt("vCrewList", &crew);

            if (ImGui::TreeNode("fireSpreader"))
            {
                ImGui::InputInt("count", &ship->fireSpreader.count);

                if (ImGui::TreeNode("roomCount"))
                {
                    for (int* i = ship->fireSpreader.roomCount.begin; i != ship->fireSpreader.roomCount.end; ++i)
                        ImGui::InputInt("room", i);

                    ImGui::TreePop();
                    ImGui::Separator();
                }

                int test = ship->fireSpreader.grid.size();
                ImGui::InputInt("help", &test);

                if (ImGui::TreeNode("grid"))
                {
                    for (size_t i = 0; i < ship->fireSpreader.grid.size(); ++i)
                    {
                        auto&& fire = ship->fireSpreader.grid[i];

                        if (ImGui::TreeNode("fire"))
                        {
                            if (ImGui::TreeNode("super_Spreadable"))
                            {
                                ImGui::InputText("soundName", fire.soundName, 32);
                                ImGui::TreePop();
                                ImGui::Separator();
                            }

                            ImGui::InputFloat("fDeathTimer", &fire.fDeathTimer);
                            ImGui::InputFloat("fStartTimer", &fire.fStartTimer);
                            ImGui::InputFloat("fOxygen", &fire.fOxygen);
                            ImGui::Checkbox("bWasOnFire", &fire.bWasOnFire);

                            ImGui::TreePop();
                            ImGui::Separator();
                        }
                    }

                    ImGui::TreePop();
                    ImGui::Separator();
                }

                ImGui::TreePop();
                ImGui::Separator();
            }

            if (ImGui::TreeNode("jump_timer"))
            {
                ImGui::InputFloat("first", &ship->jump_timer.first);
                ImGui::InputFloat("second", &ship->jump_timer.second);

                ImGui::TreePop();
                ImGui::Separator();
            }

            ImGui::TreePop();
            ImGui::Separator();
        }
    }

    void rawCompleteShipSection(const char* label, raw::CompleteShip* ship)
    {
        if (ship && ImGui::TreeNode(label))
        {
            ImGui::TreePop();
            ImGui::Separator();
        }
    }

    void playerWindow()
    {
        if (!this->player || !this->playerGui)
            return;

        if (ImGui::Begin("AI Panel", &this->playerGui))
        {
            ImGui::Checkbox("Active", &g_active);
        }
        ImGui::End();
    }

    void demoWindow()
    {
        if (!this->demoGui)
            return;

        ImGui::ShowDemoWindow(&this->demoGui);
    }
};

void imguiStuff(const State& state)
{
}
