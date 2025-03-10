#include "dialogue_page.hpp"

#include <algorithm>
#include <filesystem>

#include "fs.hpp"
#include "main_frame.hpp"
#include "utils.hpp"

namespace i18n = brls::i18n;
using namespace i18n::literals;

void DialoguePage::CreateView()
{
    this->button1 = (new brls::Button(brls::ButtonStyle::REGULAR))->setLabel("menu/dialogue/yes"_i18n);
    this->button1->setParent(this);
    this->button2 = (new brls::Button(brls::ButtonStyle::REGULAR))->setLabel("menu/dialogue/no"_i18n);
    this->button2->setParent(this);

    this->instantiateButtons();

    this->label->setHorizontalAlign(NVG_ALIGN_CENTER);
    this->label->setParent(this);

    this->navigationMap.add(
        this->button1,
        brls::FocusDirection::RIGHT,
        this->button2);

    this->navigationMap.add(
        this->button2,
        brls::FocusDirection::LEFT,
        this->button1);

    this->registerAction("", brls::Key::B, [this] { return true; });
}

void DialoguePage::draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx)
{
    this->label->frame(ctx);
    this->button1->frame(ctx);

    auto end = std::chrono::high_resolution_clock::now();
    auto missing = std::max(1l - std::chrono::duration_cast<std::chrono::seconds>(end - start).count(), 0l);
    auto text = std::string("menus/common/no"_i18n);
    if (missing > 0) {
        this->button2->setLabel(text + " (" + std::to_string(missing) + ")");
        this->button2->setState(brls::ButtonState::DISABLED);
    }
    else {
        this->button2->setLabel(text);
        this->button2->setState(brls::ButtonState::ENABLED);
    }
    this->button2->invalidate();
    this->button2->frame(ctx);
}

void DialoguePage::layout(NVGcontext* vg, brls::Style* style, brls::FontStash* stash)
{
    this->label->setWidth(this->width);
    this->label->invalidate(true);
    this->label->setBoundaries(
        this->x + this->width / 2 - this->label->getWidth() / 2,
        this->y + (this->height - this->label->getHeight() - this->y - style->CrashFrame.buttonHeight) / 2,
        this->label->getWidth(),
        this->label->getHeight());
    this->button1->setBoundaries(
        this->x + this->width / 2 - style->CrashFrame.buttonWidth / 2 - 200,
        this->y + (this->height - style->CrashFrame.buttonHeight * 3),
        style->CrashFrame.buttonWidth,
        style->CrashFrame.buttonHeight);
    this->button1->invalidate();

    this->button2->setBoundaries(
        this->x + this->width / 2 - style->CrashFrame.buttonWidth / 2 + 200,
        this->y + (this->height - style->CrashFrame.buttonHeight * 3),
        style->CrashFrame.buttonWidth,
        style->CrashFrame.buttonHeight);
    this->button2->invalidate();

    start = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(150);
}

brls::View* DialoguePage::getDefaultFocus()
{
    return this->button1;
}

brls::View* DialoguePage::getNextFocus(brls::FocusDirection direction, brls::View* currentView)
{
    return this->navigationMap.getNextFocus(direction, currentView);
}

void DialoguePage_fw::instantiateButtons()
{
    this->button2->getClickEvent()->subscribe([this](View* view) {
        if (!frame->isLastStage())
            frame->nextStage();
        else {
            brls::Application::pushView(new MainFrame());
        }
    });

    this->button1->getClickEvent()->subscribe([this](View* view) {
        envSetNextLoad(DAYBREAK_PATH.c_str(), fmt::format("\"{}\"", DAYBREAK_PATH).c_str());
        romfsExit();
        brls::Application::quit();
    });

    this->label = new brls::Label(brls::LabelStyle::DIALOG, fmt::format("{}\n\n{}", this->text, "menu/dialogue/launch_daybreak"_i18n), true);
}

void DialoguePage_fw::draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx)
{
    this->label->frame(ctx);
    this->button1->frame(ctx);
    this->button2->frame(ctx);
}

void DialoguePage_theme::instantiateButtons() {
    this->button1->getClickEvent()->subscribe([this](View* view) {
        util::deleteTheme();
        if (!frame->isLastStage()){
            frame->nextStage();
        }
        else {
            brls::Application::pushView(new MainFrame());
        }
    });

    this->button2->getClickEvent()->subscribe([this](View* view) {
        if (!frame->isLastStage())
            frame->nextStage();
        else {
            brls::Application::pushView(new MainFrame());
        }
    });

    this->label = new brls::Label(brls::LabelStyle::DIALOG, fmt::format("{}\n\n{}", this->text, "menu/dialogue/delete_theme"_i18n), true);
}

void DialoguePage_theme::draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx)
{
    this->label->frame(ctx);
    this->button1->frame(ctx);
    this->button2->frame(ctx);
}