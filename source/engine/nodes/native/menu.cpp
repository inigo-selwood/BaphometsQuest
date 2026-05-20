#include "menu.hpp"

#include "../../resources/types/font.hpp"
#include "../../resources/types/image_texture.hpp"
#include "../../resources/types/text_texture.hpp"
#include "../../runtime/game.hpp"

#include <tinyxml2.h>

#include <stdexcept>
#include <string>
#include <vector>

namespace Engine::Nodes {

Menu::Menu() {
    this->declareHook(Engine::Nodes::Hook::Input);
    this->declareHook(Engine::Nodes::Hook::Render);
    this->declareProperty(
        "font",
        this->font,
        [this](const std::string &value) {
            this->font = value;
            this->rebuild();
        }
    );
    this->declareProperty("size", this->size, [this](const int &value) {
        this->size = value;
        this->rebuild();
    });
    this->declareProperty(
        "colour",
        this->colour,
        [this](const SDL_Color &value) {
            this->colour = value;
            this->rebuild();
        }
    );
    this->declareProperty("position", this->position);
    this->declareProperty("line-height", this->lineHeight);
    this->declareProperty("cursor-offset", this->cursorOffset);
    this->declareProperty(
        "cursor-path",
        this->cursorPath,
        [this](const std::string &value) {
            this->cursorPath = value;
            this->rebuild();
        }
    );
    this->declareProperty(
        "cursor-region",
        this->cursorRegion,
        [this](const SDL_Rect &value) {
            this->cursorRegion = value;
            this->rebuild();
        }
    );
}

bool Menu::loadXmlChildren(const tinyxml2::XMLElement &element) {
    std::vector<Option> parsedOptions;

    for(const tinyxml2::XMLElement *optionElement = element.FirstChildElement();
        optionElement != nullptr;
        optionElement = optionElement->NextSiblingElement()) {
        parsedOptions.push_back(parseOption(*optionElement));
    }

    if(parsedOptions.empty()) {
        throw std::runtime_error("Menu requires at least one option child");
    }

    if(this->cursorPath.empty()) {
        throw std::runtime_error("Menu requires a cursor path");
    }

    this->options = parsedOptions;
    this->selectedOption = 0;
    this->getGame().signals.declare<std::string>(
        this->shared_from_this(),
        "selected"
    );
    this->rebuild();

    return true;
}

void Menu::input(const SDL_Event &event) {
    if(event.type != SDL_KEYDOWN || this->options.empty()) {
        return;
    }

    switch(event.key.keysym.sym) {
    case SDLK_UP:
        if(this->selectedOption == 0) {
            this->selectedOption = this->options.size() - 1;
        } else {
            this->selectedOption--;
        }
        break;
    case SDLK_DOWN:
        this->selectedOption =
            (this->selectedOption + 1) % this->options.size();
        break;
    case SDLK_RETURN:
    case SDLK_KP_ENTER:
        this->selectCurrent();
        break;
    default:
        break;
    }
}

void Menu::render(SDL_Renderer &renderer) {
    Engine::Game &game = this->getGame();
    const int rowHeight =
        this->lineHeight > 0 ? this->lineHeight : this->size * 2;
    const int cursorOffset =
        this->cursorOffset > 0 ? this->cursorOffset : this->size * 2;

    for(std::size_t index = 0; index < this->options.size(); index++) {
        const Option &option = this->options[index];

        if(option.textResourceID == 0) {
            continue;
        }

        const Engine::Resource::TextTexture &texture =
            game.resources.get<Engine::Resource::TextTexture>(
                option.textResourceID
            );
        const SDL_Rect destination{
            this->position.x,
            this->position.y + static_cast<int>(index) * rowHeight,
            texture.size.w,
            texture.size.h,
        };

        if(SDL_RenderCopy(
               &renderer,
               texture.handle.get(),
               nullptr,
               &destination
           ) != 0) {
            throw std::runtime_error(
                std::string("Failed to render menu option: ") + SDL_GetError()
            );
        }
    }

    if(this->cursorResourceID == 0 || this->options.empty()) {
        return;
    }

    const Engine::Resource::ImageTexture &cursor =
        game.resources.get<Engine::Resource::ImageTexture>(
            this->cursorResourceID
        );
    SDL_Rect source = cursor.size;

    if(this->cursorRegion.w > 0 && this->cursorRegion.h > 0) {
        source = this->cursorRegion;
    }

    const SDL_Rect destination{
        this->position.x - cursorOffset,
        this->position.y + static_cast<int>(this->selectedOption) * rowHeight,
        source.w,
        source.h,
    };

    if(SDL_RenderCopy(&renderer, cursor.handle.get(), &source, &destination)
        != 0) {
        throw std::runtime_error(
            std::string("Failed to render menu cursor: ") + SDL_GetError()
        );
    }
}

void Menu::rebuild() {
    if(this->font.empty() || this->size <= 0 || this->options.empty()) {
        this->fontResourceID = 0;
        for(Option &option : this->options) {
            option.textResourceID = 0;
        }
    } else {
        Engine::Game &game = this->getGame();

        if(game.renderer == nullptr) {
            throw std::runtime_error(
                "Menu requires a renderer before rebuilding textures"
            );
        }

        this->fontResourceID =
            game.resources.load<Engine::Resource::Font>(this->font, this->size);

        for(Option &option : this->options) {
            option.textResourceID =
                game.resources.load<Engine::Resource::TextTexture>(
                    game.renderer.get(),
                    this->fontResourceID,
                    this->colour,
                    option.text
                );
        }
    }

    if(this->cursorPath.empty()) {
        this->cursorResourceID = 0;
        return;
    }

    Engine::Game &game = this->getGame();

    if(game.renderer == nullptr) {
        throw std::runtime_error(
            "Menu requires a renderer before rebuilding cursor texture"
        );
    }

    this->cursorResourceID =
        game.resources.load<Engine::Resource::ImageTexture>(
            game.renderer.get(),
            this->cursorPath
        );
}

void Menu::selectCurrent() {
    this->getGame().signals.emit(
        this->shared_from_this(),
        "selected",
        this->options[this->selectedOption].tag
    );
}

Menu::Option Menu::parseOption(const tinyxml2::XMLElement &optionElement) {
    const std::string elementName = optionElement.Name();

    if(elementName != "option") {
        throw std::runtime_error(
            "Menu child element '" + elementName + "' must be <option>"
        );
    }

    const char *tagAttribute = optionElement.Attribute("tag");
    const char *textAttribute = optionElement.Attribute("text");

    if(tagAttribute == nullptr || std::string{tagAttribute}.empty()) {
        throw std::runtime_error(
            "Menu option requires a non-empty tag attribute"
        );
    }

    if(textAttribute == nullptr || std::string{textAttribute}.empty()) {
        throw std::runtime_error(
            "Menu option requires a non-empty text attribute"
        );
    }

    return Option{
        tagAttribute,
        textAttribute,
        0,
    };
}

} // namespace Engine::Nodes
