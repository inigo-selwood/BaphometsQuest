#include "text.hpp"

#include "../format/format.hpp"
#include "../game/game.hpp"
#include "../parse/parse.hpp"

#include <stdexcept>

namespace {

bool isCenterValue(const std::string &value) {
    return value == "center" || value == "centre";
}

} // namespace

Text::Text() {
    this->registerProperty<Alignment>(
        "alignment",
        "Text",
        [](const std::string &value, const std::string &) {
            return Text::parseAlignment(value);
        },
        [this] { return this->alignment; },
        [this](const Alignment &value) { this->alignment = value; },
        Text::formatAlignment
    );
    this->registerProperty<SDL_Color>(
        "background",
        "Text",
        [this] { return this->background.value_or(SDL_Color{0, 0, 0, 0}); },
        [this](const SDL_Color &value) { this->background = value; }
    );
    this->registerProperty<SDL_Color>(
        "colour",
        "Text",
        [this] { return this->colour; },
        [this](const SDL_Color &value) {
            this->colour = value;
            this->textureDirty = true;
        }
    );
    this->registerProperty<std::string>(
        "font",
        "Text",
        [this] { return this->fontPath; },
        [this](const std::string &value) {
            this->fontPath = value;
            this->textureDirty = true;
        },
        [](const std::string &value) {
            return Engine::Format::filePath(value);
        }
    );
    this->registerProperty<int>(
        "fontSize",
        "Text",
        [this] { return this->fontSize; },
        [this](const int &value) {
            this->fontSize = value;
            this->textureDirty = true;
        }
    );
    this->registerProperty<Justification>(
        "justification",
        "Text",
        [](const std::string &value, const std::string &) {
            return Text::parseJustification(value);
        },
        [this] { return this->justification; },
        [this](const Justification &value) { this->justification = value; },
        Text::formatJustification
    );
    this->registerProperty<std::string>(
        "text",
        "Text",
        [this] { return this->text; },
        [this](const std::string &value) {
            this->text = value;
            this->textureDirty = true;
        }
    );
    this->setRenderFunction([this](SDL_Renderer *renderer) {
        this->renderSelf(renderer);
    });
}

void Text::registerType() {
    Engine::Node::registerType<Text>("Text");
}

std::string Text::formatAlignment(Alignment value) {
    switch(value) {
    case Alignment::Bottom:
        return "bottom";
    case Alignment::Middle:
        return "middle";
    case Alignment::Top:
        return "top";
    }

    return "unknown";
}

std::string Text::formatJustification(Justification value) {
    switch(value) {
    case Justification::Center:
        return "center";
    case Justification::Left:
        return "left";
    case Justification::Right:
        return "right";
    }

    return "unknown";
}

Text::Alignment Text::parseAlignment(const std::string &value) {
    if(value == "bottom") {
        return Alignment::Bottom;
    }

    if(value == "middle" || isCenterValue(value)) {
        return Alignment::Middle;
    }

    if(value == "top") {
        return Alignment::Top;
    }

    throw std::runtime_error("Invalid text alignment: " + value);
}

Text::Justification Text::parseJustification(const std::string &value) {
    if(isCenterValue(value)) {
        return Justification::Center;
    }

    if(value == "left") {
        return Justification::Left;
    }

    if(value == "right") {
        return Justification::Right;
    }

    throw std::runtime_error("Invalid text justification: " + value);
}

SDL_Rect Text::getContentDestination(
    const SDL_Point &position,
    const SDL_Point &textureSize
) const {
    const SDL_Rect layout = this->getLayoutDestination(position, textureSize);
    SDL_Rect destination{
        layout.x,
        layout.y,
        textureSize.x,
        textureSize.y,
    };

    if(this->justification == Justification::Center) {
        destination.x += (layout.w - textureSize.x) / 2;
    } else if(this->justification == Justification::Right) {
        destination.x += layout.w - textureSize.x;
    }

    if(this->alignment == Alignment::Bottom) {
        destination.y += layout.h - textureSize.y;
    } else if(this->alignment == Alignment::Middle) {
        destination.y += (layout.h - textureSize.y) / 2;
    }

    return destination;
}

SDL_Point Text::getLayoutSize(const SDL_Point &textureSize) const {
    return this->getSize().value_or(textureSize);
}

SDL_Rect Text::getLayoutDestination(
    const SDL_Point &position,
    const SDL_Point &textureSize
) const {
    const SDL_Point layoutSize = this->getLayoutSize(textureSize);

    return SDL_Rect{
        position.x,
        position.y,
        layoutSize.x,
        layoutSize.y,
    };
}

void Text::renderSelf(SDL_Renderer *renderer) {
    if(this->textureDirty) {
        this->rebuildTexture(renderer);
    }

    if(!this->textureID.has_value()) {
        return;
    }

    auto &assets = Engine::Game::getInstance().getAssets();
    auto &texture = assets.get<SDL_Texture>(*this->textureID);
    const SDL_Point textureSize = assets.getTextureSize(*this->textureID);
    const SDL_Point position = this->getWorldPosition();
    const SDL_Rect destination =
        this->getContentDestination(position, textureSize);

    if(this->background.has_value()) {
        SDL_BlendMode previousBlendMode;
        SDL_GetRenderDrawBlendMode(renderer, &previousBlendMode);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(
            renderer,
            this->background->r,
            this->background->g,
            this->background->b,
            this->background->a
        );

        SDL_Rect backgroundDestination =
            this->getLayoutDestination(position, textureSize);
        SDL_RenderFillRect(renderer, &backgroundDestination);
        SDL_SetRenderDrawBlendMode(renderer, previousBlendMode);
    }

    SDL_RenderCopy(renderer, &texture, nullptr, &destination);
}

void Text::rebuildTexture(SDL_Renderer *renderer) {
    this->textureID.reset();
    this->textureDirty = false;

    if(this->text.empty()) {
        return;
    }

    if(this->fontPath.empty()) {
        throw std::runtime_error(
            "Text node is missing required font property."
        );
    }

    if(this->fontSize <= 0) {
        throw std::runtime_error(
            "Text node font size must be greater than 0."
        );
    }

    this->textureID = Engine::Game::getInstance().getAssets().loadTextTexture(
        renderer,
        this->fontPath,
        this->fontSize,
        this->colour,
        this->text
    );
}
