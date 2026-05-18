#pragma once

#include "../../engine/nodes/base.hpp"
#include "../../engine/nodes/native/box.hpp"
#include "../../engine/nodes/native/image.hpp"
#include "../../engine/nodes/native/label.hpp"

#include <memory>
#include <string>

namespace Game::Scenes {

/** Default scene used to boot the game */
class Main : public Engine::Nodes::Base {
  public:
    Main() {
        this->declareHook(Engine::Nodes::Hook::Enter);

        auto box = std::make_shared<Engine::Nodes::Box>();
        box->setProperty("colour", SDL_Color{255, 0, 0, 255});
        box->setProperty("size", SDL_Rect{8, 8, 16, 16});

        this->addChild("red-box", box);
    }

    void enter() override {
        if(this->loaded) {
            return;
        }

        auto image = std::make_shared<Engine::Nodes::Image>();
        this->addChild("tileset-sample", image);
        image->setProperty(
            "path",
            std::string("resources/textures/tileset.png")
        );
        image->setProperty("position", SDL_Point{32, 8});
        image->setProperty("region", SDL_Rect{0, 0, 16, 16});

        auto label = std::make_shared<Engine::Nodes::Label>();
        this->addChild("sample-label", label);
        label->setProperty("font", std::string("resources/fonts/04B_03.TTF"));
        label->setProperty("size", 8);
        label->setProperty("text", std::string("Baphomet"));
        label->setProperty("colour", SDL_Color{255, 255, 255, 255});
        label->setProperty("position", SDL_Point{8, 32});

        this->loaded = true;
    }

  private:
    bool loaded = false;
};

} // namespace Game::Scenes
