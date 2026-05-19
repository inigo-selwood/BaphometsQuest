#pragma once

#include "components/cursor.hpp"

#include "../../engine/nodes/base.hpp"
#include "../../engine/nodes/native/label.hpp"
#include "../../engine/nodes/native/music.hpp"

#include <memory>
#include <string>
#include <vector>

namespace Scenes::MainMenu {

/** Main menu scene used to boot the game */
class Scene : public Engine::Nodes::Base {
  public:
    Scene() {
        this->declareHook(Engine::Nodes::Hook::Enter);
    }

    void enter() override {
        if(this->loaded) {
            return;
        }

        // Background music
        auto music = std::make_shared<Engine::Nodes::Music>();
        this->addChild("background-music", music);
        music->setProperty(
            "path",
            std::string("resources/music/Koyaanisqatsi.wav")
        );
        music->setProperty("loop", true);
        music->start();

        // Title label
        auto title = std::make_shared<Engine::Nodes::Label>();
        this->addChild("title", title);
        title->setProperty(
            "font",
            std::string("resources/fonts/Early GameBoy.ttf")
        );
        title->setProperty("size", 8);
        title->setProperty("text", std::string("baphomet's quest"));
        title->setProperty("colour", SDL_Color{255, 255, 255, 255});
        title->setProperty("position", SDL_Point{80, 24});
        title->setProperty(
            "justification",
            Engine::Nodes::Label::Justification::Centre
        );

        // Play option
        auto play = std::make_shared<Engine::Nodes::Label>();
        this->addChild("play-option", play);
        play->setProperty("font", std::string("resources/fonts/04B_03.TTF"));
        play->setProperty("size", 8);
        play->setProperty("text", std::string("play"));
        play->setProperty("colour", SDL_Color{255, 255, 255, 255});
        play->setProperty("position", SDL_Point{64, 76});

        // Quit option
        auto quit = std::make_shared<Engine::Nodes::Label>();
        this->addChild("quit-option", quit);
        quit->setProperty("font", std::string("resources/fonts/04B_03.TTF"));
        quit->setProperty("size", 8);
        quit->setProperty("text", std::string("quit"));
        quit->setProperty("colour", SDL_Color{255, 255, 255, 255});
        quit->setProperty("position", SDL_Point{64, 92});

        // Menu cursor
        auto cursor = std::make_shared<Components::Cursor>(
            std::vector<SDL_Point>{{48, 76}, {48, 92}}
        );
        this->addChild("cursor", cursor);
        cursor->setProperty(
            "path",
            std::string("resources/textures/tileset.png")
        );
        cursor->setProperty("region", SDL_Rect{0, 0, 8, 8});
        cursor->align();

        this->loaded = true;
    }

  private:
    bool loaded = false;
};

} // namespace Scenes::MainMenu
