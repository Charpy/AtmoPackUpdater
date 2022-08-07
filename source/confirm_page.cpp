#include "confirm_page.hpp"
#include "main_frame.hpp"
#include "reboot.hpp"
#include "utils.hpp"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <string>

namespace i18n = brls::i18n;
using namespace i18n::literals;

ConfirmPage::ConfirmPage(brls::StagedAppletFrame* frame, const std::string& text, bool done, bool reboot, bool erista, bool restart) : done(done), reboot(reboot), erista(erista), restart(restart) {
    this->button = (new brls::Button(brls::ButtonStyle::REGULAR))->setLabel(done ? "menu/confirm/continue"_i18n : "menu/confirm/cancel"_i18n);
    this->button->setParent(this);
    this->button->getClickEvent()->subscribe([frame, this](View* view) {
        if (!frame->isLastStage()) {
            frame->nextStage();
        }
        else if (this->done) {
            brls::Application::pushView(new MainFrame());
        }
        if (this->reboot) {
            if (this->erista) {
                reboot::rebootNow();
            }
            else {
                //I'll do this later
            }
        }

        if (this->restart) {
            util::extractArchive(contentType::app);
        }
    });

    this->label = new brls::Label(brls::LabelStyle::DIALOG, text, true);
    this->label->setHorizontalAlign(NVG_ALIGN_CENTER);
    this->label->setParent(this);

    if (this->done || this->reboot) {
        this->registerAction("", brls::Key::B, [this] { return true; });
    }
}

void ConfirmPage::draw(NVGcontext* vg, int x, int y, unsigned width, unsigned height, brls::Style* style, brls::FrameContext* ctx) {
    if (!this->done) {
        auto end = std::chrono::high_resolution_clock::now();
        auto missing = std::max(1l - std::chrono::duration_cast<std::chrono::seconds>(end - start).count(), 0l);
        auto text = std::string("menu/confirm/continue"_i18n);
        if (missing > 0) {
            this->button->setLabel(text + " (" + std::to_string(missing) + ")");
            this->button->setState(brls::ButtonState::DISABLED);
        }
        else {
            this->button->setLabel(text);
            this->button->setState(brls::ButtonState::ENABLED);
        }
    }
    else {
        this->button->setState(brls::ButtonState::ENABLED);
    }
    this->label->frame(ctx);
    this->button->frame(ctx);
}

brls::View* ConfirmPage::getDefaultFocus()
{
    return this->button;
}

void ConfirmPage::layout(NVGcontext* vg, brls::Style* style, brls::FontStash* stash)
{
    this->label->setWidth(this->width);
    this->label->invalidate(true);
    // this->label->setBackground(brls::ViewBackground::DEBUG);
    this->label->setBoundaries(
        this->x + this->width / 2 - this->label->getWidth() / 2,
        this->y + (this->height - this->label->getHeight() - this->y - style->CrashFrame.buttonHeight) / 2,
        this->label->getWidth(),
        this->label->getHeight());

    this->button->setBoundaries(
        this->x + this->width / 2 - style->CrashFrame.buttonWidth / 2,
        this->y + (this->height - style->CrashFrame.buttonHeight * 3),
        style->CrashFrame.buttonWidth,
        style->CrashFrame.buttonHeight);
    this->button->invalidate();

    start = std::chrono::high_resolution_clock::now() + std::chrono::milliseconds(150);
}

ConfirmPage::~ConfirmPage()
{
    delete this->label;
    delete this->button;
}