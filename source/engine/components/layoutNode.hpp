#pragma once

#include "../node/graphicNode.hpp"

#include <string>
#include <vector>

namespace Engine {

/** Lays out graphic child nodes in a horizontal or vertical flow. */
class LayoutNode : public GraphicNode {
  public:
    enum class Align {
        Between,
        Bottom,
        Even,
        Middle,
        Top,
    };

    enum class Flow {
        Horizontal,
        Vertical,
    };

    enum class Justify {
        Between,
        Center,
        Even,
        Left,
        Right,
    };

    enum class Positioning {
        Absolute,
        Fixed,
        Relative,
    };

    LayoutNode();

    /** Register the LayoutNode XML node type. */
    static void registerType();

    /** Render children after applying layout. */
    void render(SDL_Renderer *renderer) override;

  private:
    struct LayoutChild {
        GraphicNode *node = nullptr;
        LayoutNode *layoutNode = nullptr;
        SDL_Rect margin{0, 0, 0, 0};
        Positioning positioning = Positioning::Relative;
        SDL_Point size{0, 0};
        float grow = 0.0F;
        int bottom = 0;
        int left = 0;
        int right = 0;
        int top = 0;
    };

    static std::string formatAlign(Align value);
    static std::string formatFlow(Flow value);
    static std::string formatJustify(Justify value);
    static std::string formatPositioning(Positioning value);
    static Align parseAlign(const std::string &value);
    static Flow parseFlow(const std::string &value);
    static Justify parseJustify(const std::string &value);
    static Positioning parsePositioning(const std::string &value);

    static SDL_Rect clampEdges(const SDL_Rect &value);
    void applyAbsoluteLayout(const LayoutChild &child) const;
    static int distributeGrowth(
        const std::vector<LayoutChild> &children,
        int index,
        int remainingSpace
    );
    float getMainOffset(
        int availableSpace,
        int usedSpace,
        int index,
        int childCount,
        bool vertical
    ) const;
    static int getMainEdgeOffset(const LayoutChild &child, bool vertical);
    static int getMainMarginEnd(const LayoutChild &child, bool vertical);
    static int getMainMarginStart(const LayoutChild &child, bool vertical);
    static int getMainSize(const LayoutChild &child, bool vertical);
    static int getOuterMainSize(const LayoutChild &child, bool vertical);
    static int getOuterCrossSize(const LayoutChild &child, bool vertical);
    SDL_Point getOwnSize() const;
    static int getCrossEdgeOffset(const LayoutChild &child, bool vertical);
    static int getCrossMarginEnd(const LayoutChild &child, bool vertical);
    static int getCrossMarginStart(const LayoutChild &child, bool vertical);
    static int getCrossSize(const LayoutChild &child, bool vertical);
    int getAvailableCrossSpace(bool vertical) const;
    int getAvailableMainSpace(bool vertical) const;
    std::vector<LayoutChild> getLayoutChildren();
    void layoutChildren();
    void onEnterTree() override;
    SDL_Point positionChild(
        const LayoutChild &child,
        int mainPosition,
        bool vertical
    ) const;

    Align align = Align::Top;
    int bottom = 0;
    Flow flow = Flow::Horizontal;
    int gap = 0;
    float grow = 0.0F;
    Justify justify = Justify::Left;
    int left = 0;
    SDL_Rect margin{0, 0, 0, 0};
    SDL_Rect padding{0, 0, 0, 0};
    Positioning positioning = Positioning::Relative;
    int right = 0;
    int top = 0;
};

} // namespace Engine
