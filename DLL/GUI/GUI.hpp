#pragma once

//#include "../GameState/Reader.hpp"
//#include "../Player/Player.hpp"

#include <imgui_impl_opengl3.h>
#include <imgui_impl_win32.h>
#include <imgui_memory_editor.h>
#include <imgui_stl.h>

extern bool g_overlay;
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
    //const StateReader* getReader() const
    //{
    //    return this->reader;
    //}
    //
    //void setReader(const StateReader& r)
    //{
    //    this->reader = &r;
    //}
    //
    //const Player* getPlayer() const
    //{
    //    return this->player;
    //}
    //
    //void setPlayer(const Player& p)
    //{
    //    this->player = &p;
    //}

    bool inGame() const
    {
        return false;
    //    if (!this->reader)
    //        return false;
    //
    //    auto state = this->reader->getState();
    //
    //    return state.inGame;
    }

    void render()
    {
        if (!g_overlay)
            return;

        //if (ImGui::BeginMainMenuBar())
        //{
        //    if (ImGui::BeginMenu("View"))
        //    {
        //        ImGui::Checkbox("Overlay", &g_overlay);
        //        ImGui::Checkbox("Game state", &this->gameStateGui);
        //        ImGui::Checkbox("AI Panel", &this->playerGui);
        //        ImGui::Checkbox("imgui Demo", &this->demoGui);
        //
        //        ImGui::EndMenu();
        //    }
        //
        //    ImGui::EndMainMenuBar();
        //}

        //this->gameStateWindow();
        //this->playerWindow();
        this->demoWindow();
    }

private:
    //const StateReader* reader = nullptr;
    //const Player* player = nullptr;

    bool gameStateGui = false;
    bool playerGui = false;
    bool demoGui = false;

    //void gameStateWindow()
    //{
    //   if (!this->reader || !this->inGame() || !this->gameStateGui)
    //       return;
    //
    //   State& s = const_cast<State&>(this->reader->getState());
    //   raw::State& rs = const_cast<raw::State&>(this->reader->getRawState());
    //
    //   if(ImGui::Begin("Game state", &this->gameStateGui))
    //   {
    //       if (rs.app)
    //       {
    //           this->rawStateSection(rs);
    //       }
    //
    //       ImGui::Checkbox("Running", &s.running);
    //       ImGui::Checkbox("In game", &s.inGame);
    //
    //       if (s.inGame)
    //       {
    //           this->guiStateSection(s);
    //           this->shipStateSection("Player ship", s.player);
    //           if (s.targetPresent) this->shipStateSection("Target ship", s.target);
    //           this->locationStateSection(s);
    //           this->crewStateSection(s);
    //           this->projectilesSection(s);
    //           this->spaceDronesSection(s);
    //       }
    //   }
    //   ImGui::End();
    //}
    //
    //void rawStateSection(raw::State& s) const
    //{
    //    if (s.app && ImGui::CollapsingHeader("Raw State"))
    //    {
    //        if (ImGui::Button("Trigger breakpoint"))
    //        {
    //            __asm nop
    //        }
    //
    //        auto& app = *s.app;
    //        HelpMarker("Only touch if you know what you're doing.");
    //        ImGui::Checkbox("Running", &app.Running);
    //        ImGui::Checkbox("shift_held", &app.shift_held);
    //        ImGui::InputInt("screen_x", &app.screen_x);
    //        ImGui::InputInt("screen_y", &app.screen_y);
    //        ImGui::InputInt("modifier_x", &app.modifier_x);
    //        ImGui::InputInt("modifier_y", &app.modifier_y);
    //        ImGui::Checkbox("fullScreenLastState", &app.fullScreenLastState);
    //        ImGui::Checkbox("minimized", &app.minimized);
    //        ImGui::Checkbox("minLastState", &app.minLastState);
    //        ImGui::Checkbox("focus", &app.focus);
    //        ImGui::Checkbox("focusLastState", &app.focusLastState);
    //        ImGui::Checkbox("steamOverlay", &app.steamOverlay);
    //        ImGui::Checkbox("steamOverlayLastState", &app.steamOverlayLastState);
    //        ImGui::Checkbox("rendering", &app.rendering);
    //        ImGui::Checkbox("gameLogic", &app.gameLogic);
    //        ImGui::InputFloat("mouseModifier_x", &app.mouseModifier_x);
    //        ImGui::InputFloat("mouseModifier_y", &app.mouseModifier_y);
    //        ImGui::Checkbox("fboSupport", &app.fboSupport);
    //        ImGui::InputInt("x_bar", &app.x_bar);
    //        ImGui::InputInt("y_bar", &app.y_bar);
    //        ImGui::Checkbox("lCtrl", &app.lCtrl);
    //        ImGui::Checkbox("useFrameBuffer", &app.useFrameBuffer);
    //        ImGui::Checkbox("manualResolutionError", &app.manualResolutionError);
    //        ImGui::InputInt("manualResErrorX", &app.manualResErrorX);
    //        ImGui::InputInt("manualResErrorY", &app.manualResErrorY);
    //        ImGui::Checkbox("nativeFullScreenError", &app.nativeFullScreenError);
    //        ImGui::Checkbox("fbStretchError", &app.fbStretchError);
    //        ImGui::InputText("lastLanguage", app.lastLanguage, 24);
    //        ImGui::Checkbox("inputFocus", &app.inputFocus);
    //        ImGui::Checkbox("useDirect3D", &app.useDirect3D);
    //    }
    //}
    //
    //void guiStateSection(State& s)
    //{
    //    if (ImGui::CollapsingHeader("GUI"))
    //    {
    //        ImGui::Checkbox("Minimized", &s.minimized);
    //        ImGui::Checkbox("Focused", &s.focused);
    //
    //        if (ImGui::TreeNode("Pause"))
    //        {
    //            ImGui::Checkbox("Normal pause", &s.pause.normal);
    //            ImGui::Checkbox("Auto pause", &s.pause.automatic);
    //            ImGui::Checkbox("Menu pause", &s.pause.menu);
    //            ImGui::Checkbox("Event pause", &s.pause.event);
    //            ImGui::Checkbox("Touch pause", &s.pause.touch);
    //
    //            ImGui::TreePop();
    //            ImGui::Separator();
    //        }
    //
    //        ImGui::InputInt2("Player Ship Position", &s.shipPosition.x);
    //        ImGui::InputInt2("Target Ship Position", &s.targetShipPosition.x);
    //    }
    //}
    //
    //void genericSystemSection(SystemState& s)
    //{
    //    if (ImGui::TreeNode("Generic"))
    //    {
    //        if (ImGui::TreeNode("Power"))
    //        {
    //            ImGui::InputInt("Total", &s.power.total);
    //            ImGui::InputInt("Reactor", &s.power.reactor);
    //            ImGui::InputInt("Zoltan", &s.power.zoltan);
    //            ImGui::InputInt("Battery", &s.power.battery);
    //            ImGui::InputInt("Limit", &s.power.limit);
    //
    //            ImGui::TreePop();
    //            ImGui::Separator();
    //        }
    //
    //        if (ImGui::TreeNode("Health"))
    //        {
    //            ImGui::InputInt("Health", &s.health);
    //            ImGui::InputInt("Ion Damage", &s.ion.first);
    //            ImGui::InputFloat("Ion Timer", &s.ion.second);
    //            ImGui::InputFloat2("Partial health", &s.partialHP.first);
    //
    //            ImGui::TreePop();
    //            ImGui::Separator();
    //        }
    //
    //        ImGui::InputInt("Room", &s.room);
    //        ImGui::InputInt("Manning Level", &s.manningLevel);
    //        ImGui::InputInt("Hack Level", &s.hackLevel);
    //
    //        ImGui::TreePop();
    //        ImGui::Separator();
    //    }
    //}
    //
    //void weaponSection(const char* label, WeaponState& w)
    //{
    //    if (ImGui::TreeNode(label))
    //    {
    //        int ptr = int(w.ptr);
    //        ImGui::InputInt("ptr", &ptr, 1, 100, ImGuiInputTextFlags_CharsHexadecimal);
    //        ImGui::InputText("Name", w.blueprint->name, 32);
    //        ImGui::InputFloat2("Cooldown", &w.cooldown.first);
    //        ImGui::InputFloat2("Sub-cooldown", &w.subCooldown.first);
    //        ImGui::InputFloat("Base cooldown", &w.baseCooldown);
    //        ImGui::Checkbox("Auto-fire", &w.autoFire);
    //        ImGui::Checkbox("Powered", &w.powered);
    //        ImGui::InputInt("Power required", &w.powerRequired);
    //        ImGui::InputInt("Target ID", &w.targetId);
    //        ImGui::InputFloat("Angle", &w.angle);
    //        ImGui::InputFloat("Cooldown Modifier", &w.cooldownModifier);
    //        ImGui::InputInt("Radius", &w.radius);
    //        ImGui::InputInt("Boost", &w.boost);
    //        ImGui::InputInt2("Charge", &w.charge.first);
    //        ImGui::Checkbox("Artillery", &w.artillery);
    //        ImGui::InputInt("Slot", &w.slot);
    //
    //        if (ImGui::TreeNode("Blueprint"))
    //        {
    //            auto&& bp = *w.blueprint;
    //            int ptr = int(bp.ptr);
    //            int type = int(bp.type);
    //
    //            ImGui::InputInt("ptr", &ptr, 1, 100, ImGuiInputTextFlags_CharsHexadecimal);
    //            ImGui::InputText("Name", bp.name, 32);
    //            ImGui::InputInt("Type", &type);
    //            ImGui::InputInt("Cost", &bp.cost);
    //            ImGui::InputInt("Rarity", &bp.rarity);
    //            ImGui::InputInt("Base Rarity", &bp.baseRarity);
    //            this->damageSection(bp.damage);
    //            ImGui::InputInt("Shots", &bp.shots);
    //            ImGui::InputInt("Missiles", &bp.missiles);
    //            ImGui::InputFloat("Cooldown", &bp.cooldown);
    //            ImGui::InputInt("Beam length", &bp.beamLength);
    //            ImGui::InputInt("Power", &bp.power);
    //            ImGui::InputFloat("Speed", &bp.speed);
    //            ImGui::InputInt("Radius", &bp.radius);
    //            ImGui::InputInt("Mini projectiles", &bp.miniProjectiles);
    //            ImGui::InputInt("Fake projectiles", &bp.fakeProjectiles);
    //            ImGui::InputInt("Boost type", &bp.boostType);
    //            ImGui::InputFloat("Boost amount", &bp.boostAmount);
    //            ImGui::InputInt("Boost count", &bp.boostCount);
    //            ImGui::InputInt("Charge levels", &bp.chargeLevels);
    //            ImGui::InputInt("Drone targetable", &bp.droneTargetable);
    //            ImGui::InputInt("Spin", &bp.spin);
    //
    //            ImGui::TreePop();
    //            ImGui::Separator();
    //        }
    //
    //        ImGui::TreePop();
    //        ImGui::Separator();
    //    }
    //}
    //
    //void droneSection(const char* label, DroneState& d)
    //{
    //    if (ImGui::TreeNode(label))
    //    {
    //        ImGui::InputText("Name", d.blueprint->name, 32);
    //        ImGui::InputInt("Ship affiliation", &d.shipAffiliation);
    //        ImGui::Checkbox("Powered", &d.powered);
    //        ImGui::Checkbox("Deployed", &d.deployed);
    //        ImGui::InputInt("Power required", &d.powerRequired);
    //        ImGui::InputFloat("Destroy timer", &d.destroyTimer);
    //        ImGui::InputInt("Slot", &d.slot);
    //
    //        if (ImGui::TreeNode("Blueprint"))
    //        {
    //            auto&& bp = *d.blueprint;
    //            int ptr = int(bp.ptr);
    //            int type = int(bp.type);
    //
    //            ImGui::InputInt("ptr", &ptr, 1, 100, ImGuiInputTextFlags_CharsHexadecimal);
    //            ImGui::InputText("Name", bp.name, 32);
    //            ImGui::InputInt("Type", &type);
    //
    //            ImGui::TreePop();
    //            ImGui::Separator();
    //        }
    //
    //        ImGui::TreePop();
    //        ImGui::Separator();
    //    }
    //}
    //
    //void systemStatesSection(SystemStates& s)
    //{
    //    if (ImGui::TreeNode("Systems"))
    //    {
    //        if (s.shields.present && ImGui::TreeNode("Shields"))
    //        {
    //            this->genericSystemSection(s.shields);
    //            ImGui::InputInt("Bubbles", &s.shields.bubbles);
    //            ImGui::InputFloat("Charge", &s.shields.charge);
    //            ImGui::InputInt2("Center", &s.shields.center.x);
    //            ImGui::InputFloat2("Ellipse", &s.shields.ellipse.first);
    //
    //            ImGui::TreePop();
    //            ImGui::Separator();
    //        }
    //
    //        if (s.engines.present && ImGui::TreeNode("Engines"))
    //        {
    //            this->genericSystemSection(s.engines);
    //            ImGui::Checkbox("FTL Boost", &s.engines.ftlBoost);
    //
    //            ImGui::TreePop();
    //            ImGui::Separator();
    //        }
    //
    //        if (s.oxygen.present && ImGui::TreeNode("Oxygen"))
    //        {
    //            this->genericSystemSection(s.oxygen);
    //            ImGui::InputFloat("Max", &s.oxygen.max);
    //            ImGui::InputFloat("Total", &s.oxygen.total);
    //            ImGui::Checkbox("Leaking", &s.oxygen.leaking);
    //
    //            if (ImGui::TreeNode("Levels"))
    //            {
    //                for (size_t i = 0; i < s.oxygen.levels.size(); i++)
    //                {
    //                    std::string label = "Room " + i;
    //                    ImGui::InputFloat(label.data(), &s.oxygen.levels[i]);
    //                }
    //
    //                ImGui::TreePop();
    //                ImGui::Separator();
    //            }
    //
    //            ImGui::TreePop();
    //            ImGui::Separator();
    //        }
    //
    //        if (s.weapons.present && ImGui::TreeNode("Weapons"))
    //        {
    //            this->genericSystemSection(s.weapons);
    //
    //            if (ImGui::TreeNode("Weapons"))
    //            {
    //                for (size_t i = 0; i < s.weapons.list.size(); i++)
    //                {
    //                    auto&& w = s.weapons.list[i];
    //                    std::string label = std::to_string(i) + " - " + w.blueprint->name;
    //
    //                    this->weaponSection(label.data(), w);
    //                }
    //
    //                ImGui::TreePop();
    //                ImGui::Separator();
    //            }
    //
    //            ImGui::InputInt("Slots", &s.weapons.slots);
    //
    //            ImGui::TreePop();
    //            ImGui::Separator();
    //        }
    //
    //        if (s.drones.present && ImGui::TreeNode("Drones"))
    //        {
    //            this->genericSystemSection(s.drones);
    //
    //            if (ImGui::TreeNode("Drones"))
    //            {
    //                for (size_t i = 0; i < s.drones.list.size(); i++)
    //                {
    //                    auto&& d = s.drones.list[i];
    //                    std::string label = std::to_string(i) + " - " + d.blueprint->name;
    //
    //                    this->droneSection(label.data(), d);
    //                }
    //
    //                ImGui::TreePop();
    //                ImGui::Separator();
    //            }
    //
    //            ImGui::InputInt("Slots", &s.weapons.slots);
    //
    //            ImGui::TreePop();
    //            ImGui::Separator();
    //        }
    //
    //        if (s.medbay.present && ImGui::TreeNode("Medbay"))
    //        {
    //            this->genericSystemSection(s.medbay);
    //
    //            ImGui::TreePop();
    //            ImGui::Separator();
    //        }
    //
    //        if (s.pilot.present && ImGui::TreeNode("Piloting"))
    //        {
    //            this->genericSystemSection(s.pilot);
    //
    //            ImGui::TreePop();
    //            ImGui::Separator();
    //        }
    //
    //        if (s.sensors.present && ImGui::TreeNode("Sensors"))
    //        {
    //            this->genericSystemSection(s.sensors);
    //
    //            ImGui::TreePop();
    //            ImGui::Separator();
    //        }
    //
    //        if (s.doors.present && ImGui::TreeNode("Doors"))
    //        {
    //            this->genericSystemSection(s.doors);
    //
    //            ImGui::TreePop();
    //            ImGui::Separator();
    //        }
    //
    //        if (s.teleporter.present && ImGui::TreeNode("Teleporter"))
    //        {
    //            this->genericSystemSection(s.teleporter);
    //            ImGui::InputInt2("Crew", &s.teleporter.crew.first);
    //            ImGui::Checkbox("Can send", &s.teleporter.canSend);
    //            ImGui::Checkbox("Can receive", &s.teleporter.canReceive);
    //
    //            ImGui::TreePop();
    //            ImGui::Separator();
    //        }
    //
    //        if (s.cloaking.present && ImGui::TreeNode("Cloaking"))
    //        {
    //            this->genericSystemSection(s.cloaking);
    //            ImGui::Checkbox("On", &s.cloaking.on);
    //            ImGui::InputFloat2("Timer", &s.cloaking.timer.first);
    //
    //            ImGui::TreePop();
    //            ImGui::Separator();
    //        }
    //
    //        if (s.battery.present && ImGui::TreeNode("Backup Battery"))
    //        {
    //            this->genericSystemSection(s.battery);
    //            ImGui::Checkbox("On", &s.battery.on);
    //            ImGui::InputFloat2("Timer", &s.battery.timer.first);
    //
    //            ImGui::TreePop();
    //            ImGui::Separator();
    //        }
    //
    //        if (s.clonebay.present && ImGui::TreeNode("Clonebay"))
    //        {
    //            this->genericSystemSection(s.clonebay);
    //            ImGui::InputFloat2("Timer", &s.clonebay.timer.first);
    //            ImGui::InputFloat("Death timer", &s.clonebay.deathTimer);
    //
    //            ImGui::TreePop();
    //            ImGui::Separator();
    //        }
    //
    //        if (s.mind.present && ImGui::TreeNode("Mind Control"))
    //        {
    //            this->genericSystemSection(s.mind);
    //            ImGui::InputFloat2("Timer", &s.mind.timer.first);
    //            ImGui::Checkbox("Can use", &s.mind.canUse);
    //            ImGui::Checkbox("Armed", &s.mind.armed);
    //
    //            ImGui::TreePop();
    //            ImGui::Separator();
    //        }
    //
    //        if (s.hacking.present && ImGui::TreeNode("Hacking"))
    //        {
    //            this->genericSystemSection(s.hacking);
    //            ImGui::Checkbox("Deployed", &s.hacking.deployed);
    //            ImGui::Checkbox("Arrived", &s.hacking.arrived);
    //            ImGui::Checkbox("Can use", &s.hacking.canUse);
    //            ImGui::InputFloat2("Timer", &s.hacking.timer.first);
    //            ImGui::InputFloat2("Start", &s.hacking.start.x);
    //            ImGui::InputFloat2("Destination", &s.hacking.destination.x);
    //
    //            ImGui::TreePop();
    //            ImGui::Separator();
    //        }
    //
    //        if (!s.artillery.empty() && ImGui::TreeNode("Artillery"))
    //        {
    //            for (size_t i = 0; i < s.artillery.size(); i++)
    //            {
    //                auto&& w = s.artillery[i];
    //                std::string label = std::to_string(i) + " - " + w.weapon.blueprint->name;
    //
    //                if (ImGui::TreeNode(label.data()))
    //                {
    //                    this->genericSystemSection(w);
    //                    this->weaponSection("Weapon", w.weapon);
    //
    //                    ImGui::TreePop();
    //                    ImGui::Separator();
    //                }
    //            }
    //
    //            ImGui::TreePop();
    //            ImGui::Separator();
    //        }
    //
    //        ImGui::TreePop();
    //        ImGui::Separator();
    //    }
    //}
    //
    //void shipStateSection(const char* label, ShipState& s)
    //{
    //    if (ImGui::CollapsingHeader(label))
    //    {
    //        this->systemStatesSection(s.systems);
    //
    //        if (ImGui::TreeNode("Rooms"))
    //        {
    //            for (auto&& [k, v] : s.rooms)
    //            {
    //                std::string label = "Room " + std::to_string(k);
    //
    //                if (ImGui::TreeNode(label.data()))
    //                {
    //                    ImGui::InputInt("System", reinterpret_cast<int*>(&v.system));
    //                    ImGui::InputInt2("Position", &v.position.x);
    //                    ImGui::InputInt2("Size", &v.size.x);
    //                    ImGui::InputInt2("ID", &v.roomId);
    //                    ImGui::InputInt2("Computer", &v.computer);
    //                    ImGui::InputInt("Hack level", &v.hackLevel);
    //
    //                    ImGui::TreePop();
    //                    ImGui::Separator();
    //                }
    //            }
    //
    //            ImGui::TreePop();
    //            ImGui::Separator();
    //        }
    //
    //        if (ImGui::TreeNode("Doors"))
    //        {
    //            for (auto&& [k, v] : s.doors)
    //            {
    //                std::string label = "Door " + std::to_string(k);
    //
    //                if (ImGui::TreeNode(label.data()))
    //                {
    //                    int ptr = int(v.ptr);
    //
    //                    ImGui::InputInt("ptr", &ptr, 1, 100, ImGuiInputTextFlags_CharsHexadecimal);
    //                    ImGui::InputInt2("Position", &v.position.x);
    //                    ImGui::InputInt("ID", &v.doorId);
    //                    ImGui::InputInt("Room A", &v.roomA);
    //                    ImGui::InputInt("Room B", &v.roomB);
    //                    ImGui::Checkbox("Open", &v.open);
    //                    ImGui::InputInt("Base health", &v.baseHealth);
    //                    ImGui::InputInt("Health", &v.health);
    //                    ImGui::InputInt("Level", &v.level);
    //                    ImGui::InputFloat("Forced Timer", &v.forcedOpenTimer);
    //                    ImGui::InputInt("Hack level", &v.hackLevel);
    //                    ImGui::Checkbox("Vertical", &v.vertical);
    //                    ImGui::Checkbox("Airlock", &v.airlock);
    //
    //                    ImGui::TreePop();
    //                    ImGui::Separator();
    //                }
    //            }
    //
    //            ImGui::TreePop();
    //            ImGui::Separator();
    //        }
    //
    //        ImGui::InputInt2("Reactor", &s.reactor.first);
    //        ImGui::InputInt("Reactor limit", &s.reactorLimit);
    //        ImGui::InputInt2("Hull", &s.hull.first);
    //        ImGui::InputInt("Fuel", &s.fuel);
    //        ImGui::InputInt("Missles", &s.missiles);
    //        ImGui::InputInt("Drone Parts", &s.droneParts);
    //        ImGui::InputInt("Scrap", &s.scrap);
    //        ImGui::InputFloat2("FTL", &s.ftl.first);
    //        ImGui::InputInt2("Super Shield", &s.superShield.first);
    //    }
    //}
    //
    //void locationStateSection(State& s)
    //{
    //    if (ImGui::CollapsingHeader("Location"))
    //    {
    //        ImGui::Checkbox("Store", &s.store);
    //
    //        this->hazardsStateSection(s.hazards);
    //    }
    //}
    //
    //void hazardsStateSection(HazardsState& h)
    //{
    //    if (ImGui::TreeNode("Hazards"))
    //    {
    //        std::string hazardName;
    //
    //        switch (h.hazard)
    //        {
    //        case Hazard::Asteroids:
    //            hazardName = "Asteroids";
    //            if (ImGui::TreeNode("Asteroid info"))
    //            {
    //                if (ImGui::TreeNode("Asteroid spawn rates"))
    //                {
    //                    ImGui::InputInt2("1", &h.asteroidSpawnRates[0].first);
    //                    ImGui::InputInt2("2", &h.asteroidSpawnRates[1].first);
    //                    ImGui::InputInt2("3", &h.asteroidSpawnRates[2].first);
    //                    ImGui::TreePop();
    //                    ImGui::Separator();
    //                }
    //
    //                if (ImGui::TreeNode("Asteroid state lengths"))
    //                {
    //                    ImGui::InputInt2("1", &h.asteroidStateLengths[0].first);
    //                    ImGui::InputInt2("2", &h.asteroidStateLengths[1].first);
    //                    ImGui::InputInt2("3", &h.asteroidStateLengths[2].first);
    //                    ImGui::TreePop();
    //                    ImGui::Separator();
    //                }
    //
    //                ImGui::InputInt("Asteroid state", &h.asteroidState);
    //                ImGui::InputInt("Asteroid space", &h.asteroidSpace);
    //                ImGui::InputFloat("Asteroid volley timer", &h.asteroidVolleyTimer);
    //                ImGui::InputFloat("Asteroid timer", &h.asteroidTimer);
    //                ImGui::InputInt("Shield Init", &h.shieldInit);
    //
    //                ImGui::TreePop();
    //                ImGui::Separator();
    //            }
    //            break;
    //        case Hazard::Sun:
    //            hazardName = "Sun";
    //            if (ImGui::TreeNode("Sun info"))
    //            {
    //                ImGui::InputFloat2("Timer", &h.timer.first);
    //            }
    //            break;
    //        case Hazard::Pulsar:
    //            hazardName = "Pulsar";
    //            if (ImGui::TreeNode("Pulsar info"))
    //            {
    //                ImGui::InputFloat2("Timer", &h.timer.first);
    //            }
    //            break;
    //        case Hazard::PDS:
    //            hazardName = "PDS";
    //            if (ImGui::TreeNode("PDS info"))
    //            {
    //                ImGui::InputInt("Target", &h.target);
    //                ImGui::InputFloat2("Timer", &h.timer.first);
    //            }
    //            break;
    //        case Hazard::Nebula: hazardName = "Nebula"; break;
    //        case Hazard::Storm: hazardName = "Ion Storm"; break;
    //        default: hazardName = "None"; break;
    //        }
    //
    //        ImGui::InputText("Hazard type", &hazardName);
    //
    //        ImGui::TreePop();
    //        ImGui::Separator();
    //    }
    //}
    //
    //void crewStateSection(State& s)
    //{
    //    if (ImGui::CollapsingHeader("Crew"))
    //    {
    //        for (size_t i = 0; i < s.crew.size(); ++i)
    //        {
    //            auto&& crew = s.crew[i];
    //            std::string label = std::to_string(i) + " - " + crew.name;
    //
    //            if (ImGui::TreeNode(label.data()))
    //            {
    //                if (ImGui::TreeNode("About"))
    //                {
    //                    ImGui::InputInt("Ship affiliation", &crew.shipAffiliation);
    //                    ImGui::InputText("Species", crew.species, 32);
    //                    ImGui::InputText("Name", crew.name, 32);
    //
    //                    ImGui::TreePop();
    //                    ImGui::Separator();
    //                }
    //
    //                if (ImGui::TreeNode("Positioning/Pathing"))
    //                {
    //                    ImGui::InputFloat2("Position", &crew.position.x);
    //                    ImGui::InputInt("Room", &crew.roomId);
    //                    ImGui::InputInt("System", &crew.systemId);
    //                    ImGui::InputInt("Ship", &crew.shipId);
    //                    ImGui::InputInt("Destination room", &crew.destinationRoomId);
    //                    ImGui::InputInt("Destination slot", &crew.destinationSlotId);
    //                    ImGui::InputFloat("Path length", &crew.pathLength);
    //                    ImGui::InputFloat2("Next position", &crew.next.x);
    //                    ImGui::Checkbox("In door", &crew.inDoor);
    //
    //                    ImGui::TreePop();
    //                    ImGui::Separator();
    //                }
    //
    //                if (ImGui::TreeNode("Actions"))
    //                {
    //                    ImGui::Checkbox("Fighting", &crew.fighting);
    //
    //                    ImGui::TreePop();
    //                    ImGui::Separator();
    //                }
    //
    //                if (ImGui::TreeNode("Health"))
    //                {
    //                    ImGui::Checkbox("Dead", &crew.dead);
    //                    ImGui::InputFloat2("Health", &crew.health.first);
    //                    ImGui::Checkbox("Suffocating", &crew.suffocating);
    //                    ImGui::InputFloat("Stun", &crew.stun);
    //
    //                    if (ImGui::TreeNode("Clone status"))
    //                    {
    //                        ImGui::Checkbox("Cloning", &crew.cloning);
    //                        ImGui::InputFloat("Clone dying", &crew.cloneDying);
    //
    //                        ImGui::TreePop();
    //                        ImGui::Separator();
    //                    }
    //
    //                    ImGui::TreePop();
    //                    ImGui::Separator();
    //                }
    //
    //                if (ImGui::TreeNode("Mind control info"))
    //                {
    //                    ImGui::Checkbox("Mind controlled", &crew.mindControlled);
    //                    ImGui::InputInt("Health boost", &crew.healthBoost);
    //                    ImGui::InputFloat("Damage boost", &crew.damageBoost);
    //
    //                    ImGui::TreePop();
    //                    ImGui::Separator();
    //                }
    //
    //                ImGui::InputInt("GUI slot", &crew.guiSlot);
    //
    //                ImGui::TreePop();
    //                ImGui::Separator();
    //            }
    //        }
    //
    //        ImGui::InputInt("Player Crew Count", &s.playerCrewCount);
    //        ImGui::InputInt("Target Crew Count", &s.targetCrewCount);
    //    }
    //}
    //
    //void damageSection(Damage& d)
    //{
    //    if (ImGui::TreeNode("Damage"))
    //    {
    //        ImGui::InputInt("Main", &d.main);
    //        ImGui::InputInt("Ion", &d.ion);
    //        ImGui::InputInt("System", &d.system);
    //        ImGui::InputInt("Crew", &d.crew);
    //        ImGui::InputInt("Stun", &d.stun);
    //        ImGui::InputInt("Pierce", &d.pierce);
    //        ImGui::InputInt("Fire chance", &d.fireChance);
    //        ImGui::InputInt("Breach chance", &d.breachChance);
    //        ImGui::InputInt("Stun chance", &d.stunChance);
    //
    //        ImGui::Checkbox("Hull bonus", &d.hullBonus);
    //        ImGui::Checkbox("Lockdown", &d.lockdown);
    //        ImGui::Checkbox("Friendly Fire", &d.friendlyFire);
    //
    //        ImGui::TreePop();
    //        ImGui::Separator();
    //    }
    //}
    //
    //void projectilesSection(State& s)
    //{
    //    if (ImGui::CollapsingHeader("Projectiles"))
    //    {
    //        for (size_t i = 0; i < s.projectiles.size(); i++)
    //        {
    //            auto&& p = s.projectiles[i];
    //            std::string label = std::to_string(i);
    //
    //            if (ImGui::TreeNode(label.data()))
    //            {
    //                int ptr = int(p.ptr);
    //
    //                ImGui::InputInt("ptr", &ptr, 1, 100, ImGuiInputTextFlags_CharsHexadecimal);
    //                ImGui::InputFloat2("Position", &p.position.x);
    //                ImGui::InputFloat2("Destination", &p.destination.x);
    //                ImGui::InputFloat("Speed", &p.speed);
    //                ImGui::InputInt("Space", &p.space);
    //                ImGui::InputInt("Destination space", &p.destinationSpace);
    //                ImGui::InputFloat("Heading", &p.heading);
    //                ImGui::InputInt("Owner", &p.owner);
    //                ImGui::InputInt("Target", &p.target);
    //                ImGui::Checkbox("Hit", &p.hit);
    //                ImGui::Checkbox("Missed", &p.missed);
    //                ImGui::Checkbox("Dead", &p.dead);
    //                ImGui::InputFloat("Lifespan", &p.lifespan);
    //
    //                this->damageSection(p.damage);
    //
    //                ImGui::TreePop();
    //                ImGui::Separator();
    //            }
    //        }
    //    }
    //}
    //
    //void spaceDronesSection(State& s)
    //{
    //    if (ImGui::CollapsingHeader("Space Drones"))
    //    {
    //    }
    //}
    //
    //void playerWindow()
    //{
    //    if (!this->player || !this->playerGui)
    //        return;
    //
    //    if (ImGui::Begin("AI Panel", &this->playerGui))
    //    {
    //        ImGui::Checkbox("Active", &g_active);
    //    }
    //    ImGui::End();
    //}

    void demoWindow()
    {
        if (!this->demoGui)
            return;

        ImGui::ShowDemoWindow(&this->demoGui);
    }
};

//void imguiStuff(const State& state)
//{
//}
