#include "layoutNode.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace Engine {

namespace {

bool isCenterValue(const std::string &value) {
    return value == "center" || value == "centre";
}

} // namespace

LayoutNode::LayoutNode() {
    this->registerProperty<Align>(
        "align",
        "LayoutNode",
        [](const std::string &value, const std::string &) {
            return LayoutNode::parseAlign(value);
        },
        [this] { return this->align; },
        [this](const Align &value) { this->align = value; },
        LayoutNode::formatAlign
    );
    this->registerProperty<int>(
        "bottom",
        "LayoutNode",
        [this] { return this->bottom; },
        [this](const int &value) { this->bottom = value; }
    );
    this->registerProperty<Flow>(
        "flow",
        "LayoutNode",
        [](const std::string &value, const std::string &) {
            return LayoutNode::parseFlow(value);
        },
        [this] { return this->flow; },
        [this](const Flow &value) { this->flow = value; },
        LayoutNode::formatFlow
    );
    this->registerProperty<int>(
        "gap",
        "LayoutNode",
        [this] { return this->gap; },
        [this](const int &value) { this->gap = std::max(0, value); }
    );
    this->registerProperty<float>(
        "grow",
        "LayoutNode",
        [this] { return this->grow; },
        [this](const float &value) { this->grow = std::max(0.0F, value); }
    );
    this->registerProperty<int>(
        "height",
        "LayoutNode",
        [this] { return this->getOwnSize().y; },
        [this](const int &value) {
            const SDL_Point size = this->getOwnSize();
            this->setProperty("size", SDL_Point{size.x, std::max(0, value)});
        }
    );
    this->registerProperty<Justify>(
        "justify",
        "LayoutNode",
        [](const std::string &value, const std::string &) {
            return LayoutNode::parseJustify(value);
        },
        [this] { return this->justify; },
        [this](const Justify &value) { this->justify = value; },
        LayoutNode::formatJustify
    );
    this->registerProperty<int>(
        "left",
        "LayoutNode",
        [this] { return this->left; },
        [this](const int &value) { this->left = value; }
    );
    this->registerProperty<SDL_Rect>(
        "margin",
        "LayoutNode",
        [this] { return this->margin; },
        [this](const SDL_Rect &value) {
            this->margin = LayoutNode::clampEdges(value);
        }
    );
    this->registerProperty<SDL_Rect>(
        "padding",
        "LayoutNode",
        [this] { return this->padding; },
        [this](const SDL_Rect &value) {
            this->padding = LayoutNode::clampEdges(value);
        }
    );
    this->registerProperty<Positioning>(
        "positioning",
        "LayoutNode",
        [](const std::string &value, const std::string &) {
            return LayoutNode::parsePositioning(value);
        },
        [this] { return this->positioning; },
        [this](const Positioning &value) { this->positioning = value; },
        LayoutNode::formatPositioning
    );
    this->registerProperty<int>(
        "right",
        "LayoutNode",
        [this] { return this->right; },
        [this](const int &value) { this->right = value; }
    );
    this->registerProperty<int>(
        "top",
        "LayoutNode",
        [this] { return this->top; },
        [this](const int &value) { this->top = value; }
    );
    this->registerProperty<int>(
        "width",
        "LayoutNode",
        [this] { return this->getOwnSize().x; },
        [this](const int &value) {
            const SDL_Point size = this->getOwnSize();
            this->setProperty("size", SDL_Point{std::max(0, value), size.y});
        }
    );
}

void LayoutNode::registerType() {
    Node::registerType<LayoutNode>("LayoutNode");
}

void LayoutNode::render(SDL_Renderer *renderer) {
    this->layoutChildren();
    GraphicNode::render(renderer);
}

std::string LayoutNode::formatAlign(Align value) {
    switch(value) {
    case Align::Between:
        return "between";
    case Align::Bottom:
        return "bottom";
    case Align::Even:
        return "even";
    case Align::Middle:
        return "middle";
    case Align::Top:
        return "top";
    }

    return "unknown";
}

std::string LayoutNode::formatFlow(Flow value) {
    switch(value) {
    case Flow::Horizontal:
        return "horizontal";
    case Flow::Vertical:
        return "vertical";
    }

    return "unknown";
}

std::string LayoutNode::formatJustify(Justify value) {
    switch(value) {
    case Justify::Between:
        return "between";
    case Justify::Center:
        return "center";
    case Justify::Even:
        return "even";
    case Justify::Left:
        return "left";
    case Justify::Right:
        return "right";
    }

    return "unknown";
}

std::string LayoutNode::formatPositioning(Positioning value) {
    switch(value) {
    case Positioning::Absolute:
        return "absolute";
    case Positioning::Fixed:
        return "fixed";
    case Positioning::Relative:
        return "relative";
    }

    return "unknown";
}

LayoutNode::Align LayoutNode::parseAlign(const std::string &value) {
    if(value == "between") {
        return Align::Between;
    }

    if(value == "bottom") {
        return Align::Bottom;
    }

    if(value == "even") {
        return Align::Even;
    }

    if(value == "middle" || isCenterValue(value)) {
        return Align::Middle;
    }

    if(value == "top") {
        return Align::Top;
    }

    throw std::runtime_error("Invalid layout align value: " + value);
}

LayoutNode::Flow LayoutNode::parseFlow(const std::string &value) {
    if(value == "horizontal") {
        return Flow::Horizontal;
    }

    if(value == "vertical") {
        return Flow::Vertical;
    }

    throw std::runtime_error("Invalid layout flow value: " + value);
}

LayoutNode::Justify LayoutNode::parseJustify(const std::string &value) {
    if(value == "between") {
        return Justify::Between;
    }

    if(isCenterValue(value)) {
        return Justify::Center;
    }

    if(value == "even") {
        return Justify::Even;
    }

    if(value == "left") {
        return Justify::Left;
    }

    if(value == "right") {
        return Justify::Right;
    }

    throw std::runtime_error("Invalid layout justify value: " + value);
}

LayoutNode::Positioning
LayoutNode::parsePositioning(const std::string &value) {
    if(value == "absolute") {
        return Positioning::Absolute;
    }

    if(value == "fixed") {
        return Positioning::Fixed;
    }

    if(value == "relative") {
        return Positioning::Relative;
    }

    throw std::runtime_error("Invalid layout positioning value: " + value);
}

SDL_Rect LayoutNode::clampEdges(const SDL_Rect &value) {
    return SDL_Rect{
        std::max(0, value.x),
        std::max(0, value.y),
        std::max(0, value.w),
        std::max(0, value.h),
    };
}

void LayoutNode::applyAbsoluteLayout(const LayoutChild &child) const {
    const SDL_Point containerSize = this->getOwnSize();
    int x = this->padding.x + child.margin.x + child.left;
    int y = this->padding.y + child.margin.y + child.top;

    if(child.left == 0 && child.right != 0) {
        x = containerSize.x - this->padding.w - child.margin.w - child.size.x
            - child.right;
    }

    if(child.top == 0 && child.bottom != 0) {
        y = containerSize.y - this->padding.h - child.margin.h - child.size.y
            - child.bottom;
    }

    const SDL_Point position{x, y};
    const SDL_Point currentPosition =
        child.node->getProperty<SDL_Point>("position");

    if(currentPosition.x != position.x || currentPosition.y != position.y) {
        child.node->setProperty("position", position);
    }
}

int LayoutNode::distributeGrowth(
    const std::vector<LayoutChild> &children,
    int index,
    int remainingSpace
) {
    if(remainingSpace <= 0 || children[index].grow <= 0.0F) {
        return 0;
    }

    float totalGrow = 0.0F;

    for(const LayoutChild &child : children) {
        if(child.positioning == Positioning::Relative) {
            totalGrow += child.grow;
        }
    }

    if(totalGrow <= 0.0F) {
        return 0;
    }

    return static_cast<int>(std::round(
        static_cast<float>(remainingSpace) * children[index].grow / totalGrow
    ));
}

float LayoutNode::getMainOffset(
    int availableSpace,
    int usedSpace,
    int index,
    int childCount,
    bool vertical
) const {
    const int remainingSpace = std::max(0, availableSpace - usedSpace);

    if(vertical) {
        switch(this->align) {
        case Align::Bottom:
            return static_cast<float>(remainingSpace);
        case Align::Middle:
            return static_cast<float>(remainingSpace) / 2.0F;
        case Align::Between:
            return childCount > 1 ? static_cast<float>(remainingSpace * index)
                    / static_cast<float>(childCount - 1)
                                  : 0.0F;
        case Align::Even:
            return static_cast<float>(remainingSpace * (index + 1))
                / static_cast<float>(childCount + 1);
        case Align::Top:
            return 0.0F;
        }
    }

    switch(this->justify) {
    case Justify::Right:
        return static_cast<float>(remainingSpace);
    case Justify::Center:
        return static_cast<float>(remainingSpace) / 2.0F;
    case Justify::Between:
        return childCount > 1 ? static_cast<float>(remainingSpace * index)
                / static_cast<float>(childCount - 1)
                              : 0.0F;
    case Justify::Even:
        return static_cast<float>(remainingSpace * (index + 1))
            / static_cast<float>(childCount + 1);
    case Justify::Left:
        return 0.0F;
    }

    return 0.0F;
}

int LayoutNode::getMainSize(const LayoutChild &child, bool vertical) {
    return vertical ? child.size.y : child.size.x;
}

int LayoutNode::getMainEdgeOffset(const LayoutChild &child, bool vertical) {
    return vertical ? child.top - child.bottom : child.left - child.right;
}

int LayoutNode::getMainMarginEnd(const LayoutChild &child, bool vertical) {
    return vertical ? child.margin.h : child.margin.w;
}

int LayoutNode::getMainMarginStart(const LayoutChild &child, bool vertical) {
    return vertical ? child.margin.y : child.margin.x;
}

int LayoutNode::getOuterMainSize(const LayoutChild &child, bool vertical) {
    return LayoutNode::getMainMarginStart(child, vertical)
        + LayoutNode::getMainSize(child, vertical)
        + LayoutNode::getMainMarginEnd(child, vertical);
}

int LayoutNode::getOuterCrossSize(const LayoutChild &child, bool vertical) {
    return LayoutNode::getCrossMarginStart(child, vertical)
        + LayoutNode::getCrossSize(child, vertical)
        + LayoutNode::getCrossMarginEnd(child, vertical);
}

SDL_Point LayoutNode::getOwnSize() const {
    return this->getSize().value_or(SDL_Point{0, 0});
}

int LayoutNode::getCrossSize(const LayoutChild &child, bool vertical) {
    return vertical ? child.size.x : child.size.y;
}

int LayoutNode::getCrossEdgeOffset(const LayoutChild &child, bool vertical) {
    return vertical ? child.left - child.right : child.top - child.bottom;
}

int LayoutNode::getCrossMarginEnd(const LayoutChild &child, bool vertical) {
    return vertical ? child.margin.w : child.margin.h;
}

int LayoutNode::getCrossMarginStart(const LayoutChild &child, bool vertical) {
    return vertical ? child.margin.x : child.margin.y;
}

int LayoutNode::getAvailableCrossSpace(bool vertical) const {
    const SDL_Point size = this->getOwnSize();

    if(vertical) {
        return std::max(0, size.x - this->padding.x - this->padding.w);
    }

    return std::max(0, size.y - this->padding.y - this->padding.h);
}

int LayoutNode::getAvailableMainSpace(bool vertical) const {
    const SDL_Point size = this->getOwnSize();

    if(vertical) {
        return std::max(0, size.y - this->padding.y - this->padding.h);
    }

    return std::max(0, size.x - this->padding.x - this->padding.w);
}

std::vector<LayoutNode::LayoutChild> LayoutNode::getLayoutChildren() {
    std::vector<LayoutChild> layoutChildren;

    this->forEachChild([&layoutChildren](Node &node) {
        auto *graphicNode = dynamic_cast<GraphicNode *>(&node);

        if(graphicNode == nullptr) {
            return;
        }

        auto *layoutNode = dynamic_cast<LayoutNode *>(graphicNode);
        LayoutChild layoutChild;
        layoutChild.node = graphicNode;
        layoutChild.layoutNode = layoutNode;
        layoutChild.size = graphicNode->getSize().value_or(SDL_Point{0, 0});

        if(layoutNode != nullptr) {
            layoutChild.bottom = layoutNode->bottom;
            layoutChild.grow = layoutNode->grow;
            layoutChild.left = layoutNode->left;
            layoutChild.margin = layoutNode->margin;
            layoutChild.positioning = layoutNode->positioning;
            layoutChild.right = layoutNode->right;
            layoutChild.top = layoutNode->top;
        }

        layoutChildren.push_back(layoutChild);
    });

    return layoutChildren;
}

void LayoutNode::layoutChildren() {
    std::vector<LayoutChild> children = this->getLayoutChildren();

    if(children.empty()) {
        return;
    }

    const bool vertical = this->flow == Flow::Vertical;
    const int availableMainSpace = this->getAvailableMainSpace(vertical);

    int relativeChildCount = 0;
    int usedMainSpace = 0;

    for(const LayoutChild &child : children) {
        if(child.positioning != Positioning::Relative) {
            continue;
        }

        relativeChildCount++;
        usedMainSpace += LayoutNode::getOuterMainSize(child, vertical);
    }

    usedMainSpace += this->gap * std::max(0, relativeChildCount - 1);
    const bool hasGrowableChild = std::any_of(
        children.begin(),
        children.end(),
        [](const LayoutChild &child) {
            return child.positioning == Positioning::Relative
                && child.grow > 0.0F;
        }
    );
    const int remainingSpace = std::max(0, availableMainSpace - usedMainSpace);
    const int layoutUsedMainSpace =
        hasGrowableChild ? usedMainSpace + remainingSpace : usedMainSpace;
    int mainPosition = 0;
    int relativeIndex = 0;

    for(int index = 0; index < static_cast<int>(children.size()); index++) {
        LayoutChild &child = children[index];

        if(child.positioning != Positioning::Relative) {
            this->applyAbsoluteLayout(child);
            continue;
        }

        const int growth =
            LayoutNode::distributeGrowth(children, index, remainingSpace);

        if(growth > 0 && child.layoutNode != nullptr) {
            child.size = vertical
                ? SDL_Point{child.size.x, child.size.y + growth}
                : SDL_Point{child.size.x + growth, child.size.y};

            const SDL_Point currentSize =
                child.layoutNode->getProperty<SDL_Point>("size");

            if(currentSize.x != child.size.x
                || currentSize.y != child.size.y) {
                child.layoutNode->setProperty("size", child.size);
            }
        }

        const int offset = static_cast<int>(std::round(this->getMainOffset(
            availableMainSpace,
            layoutUsedMainSpace,
            relativeIndex,
            relativeChildCount,
            vertical
        )));
        const SDL_Point position =
            this->positionChild(child, mainPosition + offset, vertical);
        const SDL_Point currentPosition =
            child.node->getProperty<SDL_Point>("position");

        if(currentPosition.x != position.x
            || currentPosition.y != position.y) {
            child.node->setProperty("position", position);
        }

        mainPosition +=
            LayoutNode::getOuterMainSize(child, vertical) + this->gap;
        relativeIndex++;
    }
}

void LayoutNode::onEnterTree() {
    this->layoutChildren();
}

SDL_Point LayoutNode::positionChild(
    const LayoutChild &child,
    int mainPosition,
    bool vertical
) const {
    const int crossSpace = this->getAvailableCrossSpace(vertical);
    const int crossRemaining = std::max(
        0,
        crossSpace - LayoutNode::getOuterCrossSize(child, vertical)
    );
    int crossPosition = 0;

    if(vertical) {
        switch(this->justify) {
        case Justify::Center:
        case Justify::Between:
        case Justify::Even:
            crossPosition = crossRemaining / 2;
            break;
        case Justify::Right:
            crossPosition = crossRemaining;
            break;
        case Justify::Left:
            crossPosition = 0;
            break;
        }

        return SDL_Point{
            this->padding.x + crossPosition
                + LayoutNode::getCrossMarginStart(child, vertical)
                + LayoutNode::getCrossEdgeOffset(child, vertical),
            this->padding.y + mainPosition
                + LayoutNode::getMainMarginStart(child, vertical)
                + LayoutNode::getMainEdgeOffset(child, vertical),
        };
    }

    switch(this->align) {
    case Align::Between:
    case Align::Even:
    case Align::Middle:
        crossPosition = crossRemaining / 2;
        break;
    case Align::Bottom:
        crossPosition = crossRemaining;
        break;
    case Align::Top:
        crossPosition = 0;
        break;
    }

    return SDL_Point{
        this->padding.x + mainPosition
            + LayoutNode::getMainMarginStart(child, vertical)
            + LayoutNode::getMainEdgeOffset(child, vertical),
        this->padding.y + crossPosition
            + LayoutNode::getCrossMarginStart(child, vertical)
            + LayoutNode::getCrossEdgeOffset(child, vertical),
    };
}

} // namespace Engine
