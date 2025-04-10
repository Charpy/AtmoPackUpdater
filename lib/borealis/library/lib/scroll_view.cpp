/*
    Borealis, a Nintendo Switch UI Library
    Copyright (C) 2020  natinusala

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <math.h>

#include <borealis/application.hpp>
#include <borealis/scroll_view.hpp>
#include <borealis/sidebar.hpp>

namespace brls
{

void ScrollView::draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, Style* style, FrameContext* ctx)
{
    if (!this->contentView)
        return;

    // Update scrolling if needed - try until it works
    if (this->updateScrollingOnNextFrame && this->updateScrolling(false))
        this->updateScrollingOnNextFrame = false;

    // Enable scissoring
    nvgSave(vg);
    nvgScissor(vg, x, y, this->width, this->height);

    // Draw content view
    this->contentView->frame(ctx);

    //Disable scissoring
    nvgRestore(vg);

    // Draw scrollbar
    float contentViewHeight = static_cast<float>(this->contentView->getHeight());
    float scrollViewHeight = static_cast<float>(this->height - style->List.scrollBarPadding * 2);

    if (contentViewHeight > scrollViewHeight)
    {
        float scrollbarHeight = (scrollViewHeight * scrollViewHeight) / contentViewHeight;
        if (scrollbarHeight < style->List.scrollBarMinimumHeight) scrollbarHeight = style->List.scrollBarMinimumHeight;

        float scrollbarPos = (this->scrollY * contentViewHeight) / (contentViewHeight - scrollViewHeight) * (scrollViewHeight - std::ceil(scrollbarHeight));

        nvgFillColor(vg, RGBA(ctx->theme->scrollBarColor.r, ctx->theme->scrollBarColor.g, ctx->theme->scrollBarColor.b, this->scrollBarAlpha * 0xFF));
        nvgBeginPath(vg);
        nvgRoundedRect(vg,
            this->x + this->width - style->List.scrollBarWidth,
            this->y + style->List.scrollBarPadding + std::floor(scrollbarPos),
            style->List.scrollBarWidth,
            std::floor(scrollbarHeight),
            style->List.scrollBarRadius);
        nvgFill(vg);
    }
}

unsigned ScrollView::getYCenter(View* view)
{
    return view->getY() + view->getHeight() / 2;
}

void ScrollView::layout(NVGcontext* vg, Style* style, FontStash* stash)
{
    this->prebakeScrolling();

    // Update scrolling if needed
    if (this->updateScrollingOnNextLayout)
    {
        this->updateScrollingOnNextLayout = false;
        this->updateScrolling(false);
    }

    // Layout content view
    if (this->contentView)
    {
        unsigned contentHeight = this->contentView->getHeight();
        this->contentView->setBoundaries(
            this->getX(),
            this->getY() - roundf(this->scrollY * (float)contentHeight),
            this->getWidth(),
            contentHeight);
        this->contentView->invalidate();
    }

    this->ready = true;
}

void ScrollView::willAppear(bool resetState)
{
    this->prebakeScrolling();

    // First scroll all the way to the top
    // then wait for the first frame to scroll
    // to the selected view if needed (only known then)
    if (resetState)
    {
        this->startScrolling(false, 0.0f);
        this->updateScrollingOnNextFrame = true; // focus may have changed since
    }

    if (this->contentView)
        this->contentView->willAppear(resetState);
}

void ScrollView::willDisappear(bool resetState)
{
    // Send event to content view
    if (this->contentView)
        this->contentView->willDisappear(resetState);
}

View* ScrollView::getDefaultFocus()
{
    return this->contentView->getDefaultFocus();
}

void ScrollView::setContentView(View* view)
{
    this->contentView = view;

    if (this->contentView)
    {
        this->contentView->setParent(this);
        this->contentView->willAppear(true);
    }

    this->invalidate();
}

View* ScrollView::getContentView()
{
    return this->contentView;
}

void ScrollView::prebakeScrolling()
{
    // Prebaked values for scrolling
    this->middleY = this->y + this->height / 2;
    this->bottomY = this->y + this->height;
}

bool ScrollView::updateScrolling(bool animated)
{
    // Don't scroll if layout hasn't been called yet
    if (!this->ready || !this->contentView)
        return false;

    float contentHeight = (float)this->contentView->getHeight();

    // Ensure content is laid out too
    if (contentHeight == 0)
        return false;

    // Ensure we're dealing with a focused view with a parent
    View* focusedView = Application::getCurrentFocus();
    if (!focusedView || !focusedView->hasParent())
        return false;

    // We're only going to use focusedView's properties if its parent matches our contentView
    bool useFocusedView = (focusedView->getParent() == this->contentView);

    // Calculate scroll value
    int currentSelectionMiddleOnScreen = (useFocusedView ? (focusedView->getY() + focusedView->getHeight() / 2) : this->contentView->getY());
    float newScroll                    = -(this->scrollY * contentHeight) - ((float)currentSelectionMiddleOnScreen - (float)this->middleY);

    // Bottom boundary
    if ((float)this->y + newScroll + contentHeight < (float)this->bottomY)
        newScroll = (float)this->height - contentHeight;

    // Top boundary
    if (newScroll > 0.0f)
        newScroll = 0.0f;

    // Apply 0.0f -> 1.0f scale
    newScroll = abs(newScroll) / contentHeight;

    //Start animation
    this->startScrolling(animated, newScroll);

    return true;
}

void ScrollView::startScrolling(bool animated, float newScroll)
{
    if (newScroll == this->scrollY)
        return;

    menu_animation_ctx_tag tag_scroll = (uintptr_t) & this->scrollY;
    menu_animation_kill_by_tag(&tag_scroll);

    menu_animation_ctx_tag tag_bar = (uintptr_t) & this->scrollBarAlpha;
    menu_animation_kill_by_tag(&tag_bar);

    if (animated)
    {
        // Get style
        Style* style = Application::getStyle();

        // Animate scrolling
        menu_animation_ctx_entry_t entry_scroll;
        entry_scroll.cb           = [](void* userdata) {};
        entry_scroll.duration     = style->AnimationDuration.highlight;
        entry_scroll.easing_enum  = EASING_OUT_QUAD;
        entry_scroll.subject      = &this->scrollY;
        entry_scroll.tag          = tag_scroll;
        entry_scroll.target_value = newScroll;
        entry_scroll.tick         = [this](void* userdata) { this->scrollAnimationTick(); };
        entry_scroll.userdata     = nullptr;

        menu_animation_push(&entry_scroll);

        // Animate scrollbar
        this->scrollBarAlpha = Application::getTheme()->scrollBarAlphaFull;
        menu_animation_ctx_entry_t entry_bar;
        entry_bar.cb           = nullptr;
        entry_bar.duration     = 500;
        entry_bar.easing_enum  = EASING_LINEAR;
        entry_bar.subject      = &this->scrollBarAlpha;
        entry_bar.tag          = tag_bar;
        entry_bar.target_value = Application::getTheme()->scrollBarAlphaNormal;
        entry_bar.tick         = [](void* userdata) {};
        entry_bar.userdata     = nullptr;

        menu_animation_push(&entry_bar);
    }
    else
    {
        this->scrollY = newScroll;
        this->scrollBarAlpha = Application::getTheme()->scrollBarAlphaNormal;
    }

    this->invalidate(!animated); // layout immediately if not animated
}

void ScrollView::scrollAnimationTick()
{
    this->invalidate();
}

void ScrollView::onChildFocusGained(View* child)
{
    // layout hasn't been called yet, don't scroll
    if (!this->ready)
        return;

    // Safety check to ensure that we don't have
    // two children (setContentView called twice)
    if (child != this->contentView)
        return;

    // Start scrolling
    this->updateScrolling(true);

    View::onChildFocusGained(child);
}

void ScrollView::onWindowSizeChanged()
{
    this->updateScrollingOnNextLayout = true;

    if (this->contentView)
        this->contentView->onWindowSizeChanged();
}

ScrollView::~ScrollView()
{
    if (this->contentView)
    {
        this->contentView->willDisappear(true);
        delete this->contentView;
    }
}

} // namespace brls
